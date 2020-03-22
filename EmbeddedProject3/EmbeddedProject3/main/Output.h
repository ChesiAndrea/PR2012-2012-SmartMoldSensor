#pragma once
#include "Arduino.h"
#include <FreeRTOS.h>

class Output
{
public:
	Output(int pin);
	bool GetState()	{return State;}
	void SetState(bool state);
	void Toggle(void);
	void Clk(void);    
protected: 
	bool State = false;
	int Pin = 13;
private: 
};

class AnalogOutput 
{
public:
	AnalogOutput(int new_pin, int timer_channel, int freq, int resolution);
	int GetState()	{return State;}
	void SetState(int state);
	void Toggle(void);
	void Clk(void);   
protected: 
	int State = 0;
	int Pin = 0;
	byte Resolution = 0;
private: 
};


