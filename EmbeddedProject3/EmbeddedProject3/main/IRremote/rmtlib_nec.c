/*
8 bit address and 8 bit command length.
Address and command are transmitted twice for reliability. (32bit)
Extended mode available, doubling the address size.
Pulse distance modulation.
Carrier frequency of 38kHz.
Header is 9000 + 4500
Bit time of 1.125ms or 2.25ms
Each pulse is a 560us long 38kHz carrier burst (about 21 cycles).
A logical "1" takes 2.25ms to transmit, while a logical "0" is only half of that, being 1.125ms. 
The recommended carrier duty-cycle is 1/4 or 1/3.
*/
//#include "Arduino.h"
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

const char* NEC_TAG = "NEC";

uint32_t remote_code;

/* NEC Protocol */
#define NEC_BITS              	32
#define NEC_HEADER_HIGH_US    	9000		/*!< header: positive 9ms */
#define NEC_HEADER_LOW_US     	4500        /*!< header: negative 4.5ms*/
#define NEC_BIT_ONE_HIGH_US    	560         /*!< data bit 1: positive 0.56ms */
#define NEC_BIT_ONE_LOW_US    	1690		/*!< data bit 1: negative 1.69ms */
#define NEC_BIT_ZERO_HIGH_US   	560         /*!< data bit 0: positive 0.56ms */
#define NEC_BIT_ZERO_LOW_US    	560			/*!< data bit 0: negative 0.56ms */
#define NEC_BIT_END            	560         /*!< end: positive 0.56ms */

#define NEC_BIT_MARGIN         	60         /*!< NEC parse error margin time */
#define NEC_DATA_ITEM_NUM   	34			/*!< NEC code item number: header + 32bit data + end */

/*
 * RECEIVE
 */

#if RECEIVE_NEC

static bool nec_header_if(rmt_item32_t* item)
{
    if((item->level0 == RMT_RX_ACTIVE_LEVEL && item->level1 != RMT_RX_ACTIVE_LEVEL)
        && rmt_check_in_range(item->duration0, NEC_HEADER_HIGH_US, NEC_BIT_MARGIN)
        && rmt_check_in_range(item->duration1, NEC_HEADER_LOW_US, NEC_BIT_MARGIN)) {

    	//ESP_LOGI(NEC_TAG, "Header: (%d) %dms, (%d) %dms", item->level0, item->duration0, item->level1, item->duration1);
        return true;
    }
    return false;
}

static bool nec_bit_one_if(rmt_item32_t* item)
{
    if((item->level0 == RMT_RX_ACTIVE_LEVEL && item->level1 != RMT_RX_ACTIVE_LEVEL)
        && rmt_check_in_range(item->duration0, NEC_BIT_ONE_HIGH_US, NEC_BIT_MARGIN)
        && rmt_check_in_range(item->duration1, NEC_BIT_ONE_LOW_US, NEC_BIT_MARGIN)) {

    	//ESP_LOGI(NEC_TAG, "One: (%d) %dms, (%d) %dms", item->level0, item->duration0, item->level1, item->duration1);
        return true;
    }
    return false;
}

static bool nec_bit_zero_if(rmt_item32_t* item)
{
    if((item->level0 == RMT_RX_ACTIVE_LEVEL && item->level1 != RMT_RX_ACTIVE_LEVEL)
        && rmt_check_in_range(item->duration0, NEC_BIT_ZERO_HIGH_US, NEC_BIT_MARGIN)
        && rmt_check_in_range(item->duration1, NEC_BIT_ZERO_LOW_US, NEC_BIT_MARGIN)) {

    	//ESP_LOGI(NEC_TAG, "Zero: (%d) %dms, (%d) %dms", item->level0, item->duration0, item->level1, item->duration1);
        return true;
    }
    return false;
}

static bool nec_end_if(rmt_item32_t* item)
{
    if((item->level0 == RMT_RX_ACTIVE_LEVEL && item->level1 != RMT_RX_ACTIVE_LEVEL)
        && rmt_check_in_range(item->duration0, NEC_BIT_END, NEC_BIT_MARGIN)
        && rmt_check_in_range(item->duration1, 0, NEC_BIT_MARGIN)) {

    	//ESP_LOGI(NEC_TAG, "Zero: (%d) %dms, (%d) %dms", item->level0, item->duration0, item->level1, item->duration1);
        return true;
    }
    return false;
}

static int nec_parse_items(rmt_item32_t* item, int item_num, uint32_t* cmd_data)
{
	//ESP_LOGI(NEC_TAG, "Parse %d items", item_num);

    if(item_num < NEC_DATA_ITEM_NUM) {
    	ESP_LOGI(NEC_TAG, "ITEM NUMBER ERROR");
        return -1;
    }

    int i = 0;

    if(!nec_header_if(item++)) {
    	ESP_LOGI(NEC_TAG, "HEADER ERROR");
        return -1;
    } else {
    	i++;
    }

    uint32_t mask = 0x80000000;
    uint32_t decoded = 0;

    for (int j = 0; j < NEC_BITS; j++) {

    	if(nec_bit_one_if(item)) {
    		decoded |= (mask >> j);
    	} else if (nec_bit_zero_if(item)) {
    		decoded |= 0 & (mask >> j);
        } else {
        	ESP_LOGI(NEC_TAG, "BIT ERROR");
            return -1;
        }
    	item++;
    	i++;
    }

    if (!nec_end_if(item++)) {
    	ESP_LOGI(NEC_TAG, "END MARKER ERROR");
    	return -1;
    } else {
    	i++;
    }

    *cmd_data = decoded;

    return i;
}

RingbufHandle_t rb = NULL;

void nec_rx_init()
{
	// idle_treshhold: In receive mode, when no edge is detected on the input signal for longer
	// than idle_thres channel clock cycles, the receive process is finished.

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

uint32_t* rmtlib_nec_receive()
{	
	//get RMT RX ringbuffer
	rmt_get_ringbuf_handle(RMT_RX_CHANNEL, &rb);

	// rmt_rx_start(channel, rx_idx_rst) - Set true to reset memory index for receiver
    rmt_rx_start(RMT_RX_CHANNEL, 1);
	
	uint8_t Ptr = 0;
	uint32_t* Ir_Data_Received = NULL;
	Ir_Data_Received = pvPortMalloc(16);	
	for (uint8_t i = 0; i < 4; i++) Ir_Data_Received[i]=0;
    while(rb) 
    {
        size_t rx_size = 0;
        rmt_item32_t* item = (rmt_item32_t*) xRingbufferReceive(rb, &rx_size, 1);
        if(item) 
        {
        	rmt_dump_items(item, rx_size / 4);
            uint32_t rmt_data;            
	        if (nec_parse_items(item, rx_size / 4, &rmt_data) == 34)
	        {
		        Ir_Data_Received[Ptr] =  rmt_data;
		        Ptr++;
	        }
            vRingbufferReturnItem(rb, (void*) item);
	        if (Ptr == 4) break;
        } 	    
	    else 
        {
	        break;
        }
    }
	return Ir_Data_Received;
}





//int rmtlib_nec_receive()
//{
//	vTaskDelay(10);
//	nec_rx_init();
//
//	//get RMT RX ringbuffer
//	RingbufHandle_t rb = NULL;
//	rmt_get_ringbuf_handle(RMT_RX_CHANNEL, &rb);
//
//	// rmt_rx_start(channel, rx_idx_rst) - Set true to reset memory index for receiver
//    rmt_rx_start(RMT_RX_CHANNEL, 1);
//
//	int res = 0;
//		
//	while (rb) 
//	{
//		size_t rx_size = 0;
//		rmt_item32_t* item = (rmt_item32_t*) xRingbufferReceive(rb, &rx_size, 1);
//		if (item) 
//		{
//			rmt_dump_items(item, rx_size / 4);
//
//			uint32_t rmt_data;
//			res = nec_parse_items(item, rx_size / 4, &rmt_data);
//			vRingbufferReturnItem(rb, (void*) item);
//			remote_code = rmt_data;
//		} 
//		else 
//		{
//			break;
//		}
//	}
//	rmt_driver_uninstall(RMT_RX_CHANNEL);
//	return res;
//}












#endif
