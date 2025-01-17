#include "Logic.h"
#include "Bsp.h"
#include "Input.h"
#include <FreeRTOS.h>
#include "IRremote/RMTLib.h"
//ooooooooooooooooooooooooooooooooooooooooooooooooooooooooo
//------------------ Queue --------------------------------
//------------ LOGIC ====> Network ------------------------
//ooooooooooooooooooooooooooooooooooooooooooooooooooooooooo

QueueHandle_t HumidityMessage = NULL;
QueueHandle_t TemperatureMessage = NULL;
QueueHandle_t AlarmMessage = NULL;

//ooooooooooooooooooooooooooooooooooooooooooooooooooooooooo
//------------------ logic input --------------------------
//ooooooooooooooooooooooooooooooooooooooooooooooooooooooooo

//---------------------------------------------------------
// Humidity threshold
int Max_Humidity_Allowed = 85;  // 85 %
int Nomal_Humidity       = 75;  // 75 %

//---------------------------------------------------------
// Temperature threshold
float Temperature_of_Alarm_Disable = 28; // if the temperature is higher than 28� the alarm is disabled
float Temperature_Hysteresis       = 1;  // Temperature Hysteresis 1�

//---------------------------------------------------------
// Time threshold
int Time_Max_Humidity    = 5400;  // 5400 secondi - 90 minuti
int Time_of_one_cycle    = 400;   // 400 mS 
int Return_PreAlarm      = 60;    // minute , time of wait before Prealarm, after wait from alarm

int Humidity = 0; 
float Temperature = 0; 


//ooooooooooooooooooooooooooooooooooooooooooooooooooooooooo
//----------------- logic variable -----------------------
//ooooooooooooooooooooooooooooooooooooooooooooooooooooooooo
typedef enum _humidity_node {Node_Boot, Node_Run, Node_PreAlarm, Node_Alarm, No_Alarm, Node_Error} _humidity_node;
volatile _humidity_node humidity_node = Node_Boot;
uint32_t Wait_PreAlarm = 0;   // mS
float Humidity_Int = 0;
float  Delta_Humidity_Integral = 0;
bool UserSw = false;
extern RMTLib RemoteControl;

// Logic function 

float Humidity_Int_Threshold(int H_allowed, int Time_min)
{
	float Threshold = 0;
	Threshold = (100 - H_allowed) * Time_min;
	return Threshold;
}


float Humidity_Integral(int Current_Humidity, int H_allowed, int Cycle_time)
{
	Humidity_Int += ((Current_Humidity - H_allowed) * Cycle_time / 1000);
	return Humidity_Int;
} 
//ooooooooooooooooooooooooooooooooooooooooooooooooooooooooo
//ooooooooooooooooooooooooooooooooooooooooooooooooooooooooo
// Logic Init
//ooooooooooooooooooooooooooooooooooooooooooooooooooooooooo
//ooooooooooooooooooooooooooooooooooooooooooooooooooooooooo

void LogicInit(void) 
{
	HumidityMessage    = xQueueCreate(1, sizeof(uint32_t));
	TemperatureMessage = xQueueCreate(1, sizeof(uint32_t));
	AlarmMessage       = xQueueCreate(1, sizeof(uint8_t));
	humidity_node = Node_Boot;
}

//ooooooooooooooooooooooooooooooooooooooooooooooooooooooooo
//ooooooooooooooooooooooooooooooooooooooooooooooooooooooooo
// Logic Clk
//ooooooooooooooooooooooooooooooooooooooooooooooooooooooooo
//ooooooooooooooooooooooooooooooooooooooooooooooooooooooooo
void LogicClk(void) 
{
	bool Exit_From_Alarm_Remote = false;
	uint32_t ulNotifiedValue;
	xTaskNotifyWait( 0x00,					 /* Don't clear any notification bits on entry. */
                     ULONG_MAX,				 /* Reset the notification value to 0 on exit. */
                     &ulNotifiedValue,		 /* Notified value pass out in ulNotifiedValue. */
                     0);		             /* Block 0 tick. */
	if ((ulNotifiedValue & 0x03) != 0)
	{
		
		//BIT 1 = Network button
		//BIT 1 = Ir
		Exit_From_Alarm_Remote = true;
	}
	
	// humidity and temperature sampling  
	Humidity = GetHumidity();
	Temperature = GetTemperature();
	int Alarm;
	Alarm = (humidity_node == Node_Alarm);
	// Send message to network
	xQueueSend(HumidityMessage,    &Humidity,    (TickType_t) 0);
	xQueueSend(TemperatureMessage, &Temperature, (TickType_t) 0);
	xQueueSend(AlarmMessage,       &Alarm,       (TickType_t) 0);
	
	if (UserSwitch.GetState() == 0)
	{
		RemoteControl.Save();
	}

//******************************************************************************
//***********************    Check if the alarm is enabled       ***************
//******************************************************************************	
	if (isnan(Temperature)) //dht22 error
	{			
		humidity_node = Node_Error;
	}	
	else if (Temperature > (Temperature_of_Alarm_Disable + (Temperature_Hysteresis / 2.0f)))
	{
		humidity_node = No_Alarm;
	}
	
	
//ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo
//ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo	
//ooooooooooooooooooooo          State machine          ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo	
//ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo	
//ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo	
	switch (humidity_node) 
	{
//******************************************************************************
//****************************          Boot       *****************************
//******************************************************************************
	case Node_Boot:
		if (isnan(Humidity) || isnan(Temperature)) //dht22 error
		{			
			humidity_node = Node_Error;
		}
		else
		{
			humidity_node = Node_Run;
		}
		//------------ ALL Out off-------------------
		Buzzer.SetState(false);
		LedStateRed.SetState(false);	
		//--------------------------------------------
		break;

//******************************************************************************
//****************************          RUN        *****************************
//******************************************************************************	
	case Node_Run:
		if (Wait_PreAlarm < Time_of_one_cycle)
		{
		    if (isnan(Humidity) || isnan(Temperature))
			{
				humidity_node = Node_Error;
			}
			else if (Humidity > Max_Humidity_Allowed)
			{
				humidity_node = Node_PreAlarm;
			}
			else		
			{
				// Send temperature and humidity to the server
			}
		}
		else
		{
			Wait_PreAlarm -= Time_of_one_cycle;
		}	
		//------------ Led green toggle --------------
		Buzzer.SetState(false);
		LedStateRed.SetState(false);	
		//--------------------------------------------		
    break;

//******************************************************************************
//****************************    PRE - Alarm       ****************************
//******************************************************************************
	case Node_PreAlarm:
	//--------------------------------------------------------------------------
	// Remember :
	//
	// Time_Max_Humidity; is in second
	// Humidity_Int_Threshold = (100% - Humidity_Allowed%) * Time_Max_Humidity;
	// Time_of_one_cycle; is in millisecond
	// Humidity_Integral += ((Current_Humidity - H_allowed) * Cycle_time / 1000);
	//
		Delta_Humidity_Integral = Humidity_Integral(Humidity, Max_Humidity_Allowed, Time_of_one_cycle);
			
		if (Delta_Humidity_Integral > Humidity_Int_Threshold(Max_Humidity_Allowed, Time_Max_Humidity))
		{
			humidity_node = Node_Alarm;
		}
		else if (Humidity < Nomal_Humidity)
		{
			humidity_node = Node_Run;
			Humidity_Int = 0;
		}	
		//------------  Led red toggle  --------------
		Buzzer.SetState(false);
		LedStateRed.Toggle();	
		//--------------------------------------------	
		break;

//******************************************************************************
//****************************        Alarm          ***************************
//******************************************************************************
	case Node_Alarm:
		Buzzer.SetState(true);			// Buzzer != Buzzer;
		LedStateRed.Toggle();			// led_red != led_red;
		// Send the alarm message to the server
		// wait stop from the server or ir
		if(Exit_From_Alarm_Remote)
		{
			Humidity_Int  = 0;
			humidity_node = Node_Run;
			Wait_PreAlarm = (Return_PreAlarm * 60 * 1000);
		}
    break ;

//******************************************************************************
//************************        No  Alarm          ***************************
//******************************************************************************
	case No_Alarm :		
		if(Temperature < (Temperature_of_Alarm_Disable - (Temperature_Hysteresis / 2.0f)))
		{
			humidity_node = Node_Boot;
		}
	break ;
		
	//-------------- dht22 error ------------------
    case Node_Error :		
		// Send dht22 error message to the server	
		humidity_node = Node_Boot ;
    break ;
		
    default :
	   humidity_node = Node_Boot ;
    break ;
	}
}		






