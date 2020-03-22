#pragma once

#include "DHT_sensor_library/DHT.h"

extern void IR_Init(void);

extern void IR_Clk(void);


extern DHT dht;
#define	GetHumidity()    dht.readHumidity()
#define	GetTemperature() dht.readTemperature()      // Read temperature as Celsius (the default)


class DigitalInput
{
public:
	DigitalInput(int pin, int AntiBounce);
	bool GetState(void);
	void Clk(void);    
protected: 
	bool State = true;
	int Pin = 22;
	int GlitchFilterN = 0;
	int StateCnt = 0;
private: 
};