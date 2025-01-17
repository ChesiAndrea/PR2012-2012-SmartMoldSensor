/*
Like NEC but a wee bit different header timings (5000 + 5000)
*/
#include "IRremote/esp32_rmt_common.h"
#include "IRremote/esp32_rmt_remotes.h"

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "esp_err.h"
#include "esp_log.h"

#include "driver/rmt.h"
#include "driver/periph_ctrl.h"
#include "soc/rmt_reg.h"

#include "freertos/ringbuf.h"


#define SAMSUNG_BITS              	32
#define SAMSUNG_HEADER_HIGH_US    	4500
#define SAMSUNG_HEADER_LOW_US     	4500
#define SAMSUNG_BIT_ONE_HIGH_US    	560
#define SAMSUNG_BIT_ONE_LOW_US    	1690
#define SAMSUNG_BIT_ZERO_HIGH_US   	560
#define SAMSUNG_BIT_ZERO_LOW_US    	560
#define SAMSUNG_BIT_END            	560
//
#define SAMSUNG_DATA_ITEM_NUM   	34  /*!< code item number: header + 32bit data + end */
#define SAMSUNG_BIT_MARGIN			60  /* deviation from signal timing */

const char* SAMSUNG_TAG = "SAMSUNG";

/*
 * RECEIVE
 */

#ifdef RECEIVE_SAMSUNG

static bool samsung_header_if(rmt_item32_t* item)
{
    if((item->level0 == RMT_RX_ACTIVE_LEVEL && item->level1 != RMT_RX_ACTIVE_LEVEL)
        && rmt_check_in_range(item->duration0, SAMSUNG_HEADER_HIGH_US, SAMSUNG_BIT_MARGIN)
        && rmt_check_in_range(item->duration1, SAMSUNG_HEADER_LOW_US, SAMSUNG_BIT_MARGIN)) {
        return true;
    }
    return false;
}

static bool samsung_bit_one_if(rmt_item32_t* item)
{
    if((item->level0 == RMT_RX_ACTIVE_LEVEL && item->level1 != RMT_RX_ACTIVE_LEVEL)
        && rmt_check_in_range(item->duration0, SAMSUNG_BIT_ONE_HIGH_US, SAMSUNG_BIT_MARGIN)
        && rmt_check_in_range(item->duration1, SAMSUNG_BIT_ONE_LOW_US, SAMSUNG_BIT_MARGIN)) {
        return true;
    }
    return false;
}

static bool samsung_bit_zero_if(rmt_item32_t* item)
{
    if((item->level0 == RMT_RX_ACTIVE_LEVEL && item->level1 != RMT_RX_ACTIVE_LEVEL)
        && rmt_check_in_range(item->duration0, SAMSUNG_BIT_ZERO_HIGH_US, SAMSUNG_BIT_MARGIN)
        && rmt_check_in_range(item->duration1, SAMSUNG_BIT_ZERO_LOW_US, SAMSUNG_BIT_MARGIN)) {
        return true;
    }
    return false;
}

static bool samsung_end_if(rmt_item32_t* item)
{
    if((item->level0 == RMT_RX_ACTIVE_LEVEL && item->level1 != RMT_RX_ACTIVE_LEVEL)
        && rmt_check_in_range(item->duration0, SAMSUNG_BIT_END, SAMSUNG_BIT_MARGIN)
        && rmt_check_in_range(item->duration1, 0, SAMSUNG_BIT_MARGIN)) {
        return true;
    }
    return false;
}

static int samsung_parse_items(rmt_item32_t* item, int item_num, uint32_t* cmd_data)
{
    if(item_num < SAMSUNG_DATA_ITEM_NUM) {
    	ESP_LOGI(SAMSUNG_TAG, "ITEM NUMBER ERROR");
        return -1;
    }

    int i = 0;

    if(!samsung_header_if(item++)) {
    	ESP_LOGI(SAMSUNG_TAG, "HEADER ERROR");
        return -1;
    } else {
    	i++;
    }

    uint32_t decoded = 0;

    // parse from left to right 32 bits (0x80000000)
    uint32_t mask = 0x01;
    mask <<= SAMSUNG_BITS - 1;

    for (int j = 0; j < SAMSUNG_BITS; j++) {

    	if(samsung_bit_one_if(item)) {
    		decoded |= (mask >> j);
    	} else if (samsung_bit_zero_if(item)) {
    		decoded |= 0 & (mask >> j);
        } else {
        	ESP_LOGI(SAMSUNG_TAG, "BIT ERROR");
            return -1;
        }
    	item++;
    	i++;
    }

    if (!samsung_end_if(item++)) {
    	ESP_LOGI(SAMSUNG_TAG, "END MARKER ERROR");
    	return -1;
    } else {
    	i++;
    }

    *cmd_data = decoded;

    return i;
}

static void samsung_rx_init()
{
    rmt_config_t rmt_rx;
    rmt_rx.channel = RMT_RX_CHANNEL;
    rmt_rx.gpio_num = RMT_RX_GPIO_NUM;
    rmt_rx.clk_div = RMT_CLK_DIV;
    rmt_rx.mem_block_num = 1;
    rmt_rx.rmt_mode = RMT_MODE_RX;
    rmt_rx.rx_config.filter_en = true;
    rmt_rx.rx_config.filter_ticks_thresh = 100;
    rmt_rx.rx_config.idle_threshold = rmt_item32_TIMEOUT_US / 10 * (RMT_TICK_10_US);

    rmt_config(&rmt_rx);
    rmt_driver_install(rmt_rx.channel, 2000, 0);
}

void rmtlib_samsung_receive()
{
	vTaskDelay(10);
	ESP_LOGI(SAMSUNG_TAG, "RMT RX DATA");
	samsung_rx_init();

    //get RMT RX ringbuffer
    RingbufHandle_t rb = NULL;
    rmt_get_ringbuf_handle(RMT_RX_CHANNEL, &rb);

	// rmt_rx_start(channel, rx_idx_rst) - Set true to reset memory index for receiver
    rmt_rx_start(RMT_RX_CHANNEL, 1);

    while(rb) {
        size_t rx_size = 0;
        rmt_item32_t* item = (rmt_item32_t*) xRingbufferReceive(rb, &rx_size, 2000);

        if(item) {
        	ESP_LOGI(SAMSUNG_TAG, "Received waveform - buffer size: %d (%d items)", rx_size, rx_size / 4);

        	rmt_dump_items(item, rx_size / 4);

            uint32_t rmt_data;
            samsung_parse_items(item, rx_size / 4, &rmt_data);
            ESP_LOGI(SAMSUNG_TAG, "IR CODE: 0x%08x", rmt_data);
			remote_code = rmt_data;

            vRingbufferReturnItem(rb, (void*) item);
        } else {
        	ESP_LOGI(SAMSUNG_TAG, "ELSE (ITEM)");
            break;
        }
    }

    ESP_LOGI(SAMSUNG_TAG, "END");
    rmt_driver_uninstall(RMT_RX_CHANNEL);
}

#endif
