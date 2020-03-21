#include "Output.h"
#include "Bsp.h"
#include "Input.h"

// LED
Output LedStateRed(LedRedPin);
Output LedStateGreen(LedGreenPin);

Output LedDbgRed(LedDbgRedPin);     // Remember pin 30 ON
Output LedDbgGreen(LedDbgGreenPin);


// Switch
#define AntiBounceConstant 2
DigitalInput UserSwitch(SwitchUserPin, AntiBounceConstant);

// BUZZER
Output Buzzer(BuzzerPin);

void BspInit(void)
{
	// to use the dbg led
	pinMode(18, OUTPUT);
	digitalWrite(18, HIGH);
	dht.begin();
    IR_Init();
	
}

void BspClk(void)
{
	LedStateRed.Clk();
	LedStateGreen.Clk();
	LedDbgRed.Clk();
//	LedDbgGreen.Clk();
	Buzzer.Clk();
	IR_Clk();
	UserSwitch.Clk();
}