#include "Network.h"
#include "Arduino.h"
#include "BluetoothSerial.h"
#include <FreeRTOS.h>
#include <stdlib.h>
#include <string.h>
#include "Bsp.h"
#include "Logic.h"
#include <sstream>  // for string streams 
#include <stdio.h>   // for string 
using namespace std; 


#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;


void Network_Init(void)
{
	SerialBT.begin("SmartMoldSens");  //Bluetooth device name
}


//void callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) 
//{
//	if (event == ESP_SPP_CLOSE_EVT) 
//	{
//		
//	}
//}

extern QueueHandle_t HumidityMessage;
extern QueueHandle_t TemperatureMessage;
extern QueueHandle_t AlarmMessage;
extern  TaskHandle_t LocalTaskHandle;
#define REMOTE_ALARM_BUTTON_PRESS 0x01
void Network_Clk(void)
{
	char  DataOut[30] = "";
	int   HumidityReceive = 0;
	float TemperatureReceive = 0;
	int AlarmReceive = 0;
	if ((HumidityMessage != NULL) || (TemperatureMessage != NULL)  || (AlarmMessage != NULL))
	{	
		if ((xQueueReceive(HumidityMessage,    &(HumidityReceive),      (TickType_t) 0) == pdPASS) 
		&&  (xQueueReceive(TemperatureMessage, &(TemperatureReceive),   (TickType_t) 0) == pdPASS)
		&&  (xQueueReceive(AlarmMessage,       &(AlarmReceive),         (TickType_t) 0) == pdPASS))
		{
			sprintf(DataOut, "$|%d|%0.2f|%d", HumidityReceive, TemperatureReceive, AlarmReceive);
			if (SerialBT.hasClient()) SerialBT.print(DataOut);
		}
	}
	
	if (SerialBT.hasClient())
	{
		LedDbgGreen.Toggle();
		String ReceiveData = "";
		while (SerialBT.available())  
		{
			ReceiveData = SerialBT.readString();
		}
		if (ReceiveData.compareTo("$A#") == 0)
		{
			xTaskNotify(LocalTaskHandle, REMOTE_ALARM_BUTTON_PRESS, eSetBits);
		}		
	}
	else
	{
		LedDbgGreen.SetState(true);		
	}	
}