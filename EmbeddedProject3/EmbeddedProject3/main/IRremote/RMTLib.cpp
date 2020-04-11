#include "Arduino.h"
#include "RMTLib.h"

#include "IRremote/esp32_rmt_remotes.h"
#include "IRremote/esp32_rmt_common.h"

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "driver/rmt.h"
#include "driver/periph_ctrl.h"
#include "soc/rmt_reg.h"

#include "Bsp.h"
#include "Output.h"

#ifdef __cplusplus
extern "C" {
#endif
/* Include C code here */
#ifdef __cplusplus
}
#endif


RMTLib::RMTLib () 
{ 
	rx_pin = RMT_RX_GPIO_NUM;
	for (uint8_t i = 0; i < 4; i++)
	{
		Data[i] = 0;
	}
}
	
void RMTLib::Init() 
{ 
	nec_rx_init();
}

void RMTLib::Clk()
{
	LedStateGreen.SetState(false);
	// variable
	uint32_t* Data = NULL;
	volatile uint32_t Data_Lenght = 0;
	
	// read ir data
	Data = rmtlib_nec_receive();

	Data_Lenght = (sizeof(Data) / sizeof(Data[0]));

	// if data is received
	if(Data_Lenght && (Data[0] != 0))
	{
		LedStateGreen.SetState(true);
	}
	
	if (save)
	{
		
	}
	vPortFree(Data); 
}


// Private


