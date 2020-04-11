#include "Input.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "IRremote/RMTLib.h"
//********************************************
// DHT define and instance 
//********************************************
#define DHTPIN 4 
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);


//********************************************
// Digital input
//********************************************
DigitalInput::DigitalInput(int pin, int AntiBounce)  // output costructor
{
	Pin = pin;
	GlitchFilterN = AntiBounce;
	StateCnt = GlitchFilterN + 1;
	pinMode(Pin, INPUT_PULLUP);
}
bool DigitalInput::GetState(void)	{return State; }  // false if the switch is press
void DigitalInput::Clk(void)	
{
	if (digitalRead(Pin)==HIGH)
	{
		if (StateCnt > GlitchFilterN) State = true;
		else	StateCnt++;
	}	
	else
	{
		if (StateCnt == 0) State = false;
		else	StateCnt--;		
	}
}   




//********************************************
// IR define and instance 
//********************************************




/*
 * EEProm : the first 40 byte are reserved for remote control and Alarm Password
 * EEProm from 0  to 7  (first  Password character (4 byte == uint_64t))
 * EEProm from 8  to 15 (second Password character (4 byte == uint_64t))
 * EEProm from 16 to 23 (third  Password character (4 byte == uint_64t))
 * EEProm from 24 to 31 (fourth Password character (4 byte == uint_64t))
 */


RMTLib RMTLib;

void IR_Init(void)
{
	RMTLib.Init();
}

uint64_t ir_Read;
void IR_Clk(void)
{
	RMTLib.Clk();	
}






