/*
 * esp32_rmt_remotes.h
 *
 */

#ifndef ESP32_RMT_REMOTES_H
#define ESP32_RMT_REMOTES_H

#include "sdkconfig.h"
#include "Arduino.h"

/* Available remote protocols */
#define SEND_NEC			1
#define RECEIVE_NEC			1
#define SEND_SAMSUNG		1
#define RECEIVE_SAMSUNG		1
#define SEND_RC5			1
#define RECEIVE_RC5			1

#ifdef __cplusplus
extern "C" {
#endif



#ifdef RECEIVE_NEC
extern	void nec_rx_init();
uint32_t* rmtlib_nec_receive();
#endif


#ifdef __cplusplus
} // extern C
#endif


#endif /* ESP32_RMT_REMOTES_H */
