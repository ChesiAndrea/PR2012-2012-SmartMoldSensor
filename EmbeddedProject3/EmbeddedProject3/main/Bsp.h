#pragma once
#include "Output.h"
#include "Input.h"
//_______________________________________
// Digital output
//_______________________________________

#define LedRedPin (int)32
#define LedGreenPin (int)33
#define LedDbgRedPin (int)5
#define LedDbgGreenPin (int)19
#define BuzzerPin (int)2
#define SwitchUserPin (int)21// LED
extern Output LedStateRed;
extern Output LedStateGreen;
extern Output LedDbgRed;       // Remember pin 30 ON
extern Output LedDbgGreen;

// Switch
extern DigitalInput UserSwitch;

// BUZZER
extern Output Buzzer;

extern void BspInit(void);
extern void BspClk(void);


