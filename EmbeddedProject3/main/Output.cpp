#include "Output.h"


//-------------------------------------------------------------
//******************	Digital Output		*******************
//-------------------------------------------------------------
Output::Output(int pin)  // output costructor
{
	Pin = pin;
	pinMode(Pin, OUTPUT);
	digitalWrite(Pin, LOW); 
}
void Output::SetState(bool state)	{State = state;} 
void Output::Toggle(void)	{State = !State;} 
void Output::Clk(void)	{digitalWrite(Pin, State);}     


//-------------------------------------------------------------
//******************	Analog Output	(PWM)	***************
//-------------------------------------------------------------
AnalogOutput::AnalogOutput(int new_pin, int timer_channel, int freq, int resolution)
{
	//resolution = 8 - 10 - 12 - 15
	if(resolution > 15)  resolution = 15;
	Resolution = resolution;
	Pin = new_pin;
	pinMode(new_pin, OUTPUT);
	ledcSetup(new_pin, freq, resolution);	
	ledcAttachPin(new_pin, new_pin);
}
void AnalogOutput::SetState(int state)	{State = state;}    
void AnalogOutput::Toggle(void)	{State = (State) ? 0 : ((2 ^ Resolution) - 1); } 
void AnalogOutput::Clk(void)		{ledcWrite(Pin, State); }       
















