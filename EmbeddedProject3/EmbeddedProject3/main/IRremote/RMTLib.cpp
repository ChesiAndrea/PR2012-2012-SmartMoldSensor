#include "Arduino.h"
#include "RMTLib.h"

#include "IRremote/esp32_rmt_remotes.h"
#include "IRremote/esp32_rmt_common.h"

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
	

void RMTLib::Clk()
{
	rmtlib_nec_receive();
}


// Private


