/*
 * Protocols:
 *  - Pulse Distance Modulation (NEC)
 *  - Manchester Encoding (RC5)
 *
 */
 
#ifndef	RMTLib_H
#define	RMTLib_H

#include "esp32_rmt_remotes.h"
#include <stdio.h>
#include "Arduino.h"

#ifdef __cplusplus
extern "C" {
#endif
/* Include C code here */
#ifdef __cplusplus
}
#endif



class RMTLib 
{
	public:
		RMTLib();
	    int32_t Data[4];
		uint8_t dataPtr = 0;
		gpio_num_t rx_pin;
		void Init();
		void Clk();
		void Save();
	private:
		bool save = false;
		bool remove_alarm = false;
};

#endif