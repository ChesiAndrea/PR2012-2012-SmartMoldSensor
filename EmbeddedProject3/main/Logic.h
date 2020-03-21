#pragma once

extern void LogicInit(void);
extern void LogicClk(void);

extern float Humidity_Int_Threshold(int H_allowed, int Time_min);
extern float Humidity_Integral(int Current_Humidity, int Cycle_time);   // integral of the temperature above the allowed threshold

