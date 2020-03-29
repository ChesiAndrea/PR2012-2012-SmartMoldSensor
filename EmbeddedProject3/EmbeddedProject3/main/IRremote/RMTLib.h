/*
 * Protocols:
 *  - Pulse Distance Modulation (NEC)
 *  - Manchester Encoding (RC5)
 *
 */
 
#ifndef	RMTLib_H
#define	RMTLib_H

#include "esp32_rmt_remotes.h"

#ifdef __cplusplus
extern "C" {
#endif
/* Include C code here */
#ifdef __cplusplus
}
#endif

class RMTLib {
	public:
		RMTLib();
	    int32_t Data[4];
		gpio_num_t rx_pin;

		void Clk();

};

#endif