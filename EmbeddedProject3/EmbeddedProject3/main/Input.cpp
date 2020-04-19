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


RMTLib RemoteControl;

void IR_Init(void)
{
	RemoteControl.Init();
}

uint64_t ir_Read;
void IR_Clk(void)
{
	RemoteControl.Clk();	
}






