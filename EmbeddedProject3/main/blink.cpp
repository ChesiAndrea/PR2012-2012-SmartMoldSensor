#include "Arduino.h"
#include <FreeRTOS.h>
#include "Bsp.h"
#include "Logic.h"
#include "Network.h"


TaskHandle_t RemoteTaskHandle = NULL;
TaskHandle_t BlinkTaskHandle = NULL;
TaskHandle_t LocalTaskHandle = NULL;
TaskHandle_t BspTaskHandle = NULL;


void blink_task(void *pvParameter)
{
	while (1) 
	{
		//LedDbgRed.Toggle();
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}
	
void local_task(void *pvParameter)
{
	LogicInit();
	while (1)
	{
		LogicClk();
		vTaskDelay(400 / portTICK_PERIOD_MS);
	}
}

void bsp_task(void *pvParameter)
{
	BspInit();
	while (1)
	{
		BspClk();
		vTaskDelay(200 / portTICK_PERIOD_MS);
	}
}

void remote_task(void *pvParameter)
{
//	Network_Init();
	while (1) 
	{
//		Network_Clk();
		vTaskDelay(300 / portTICK_PERIOD_MS);
	}
}


void setup()
{
//	//	//core 1
	xTaskCreateUniversal(remote_task, "remote_task", 32768, NULL, 1, &RemoteTaskHandle, 1);		
//	//	//core 0
	xTaskCreateUniversal(blink_task, "blink_task", 8196, NULL, 1, &BlinkTaskHandle, 1);
	xTaskCreateUniversal(local_task, "local_task", 16384, NULL, 1, &LocalTaskHandle, 1);
	xTaskCreateUniversal(bsp_task, "bsp_task", 32768, NULL, 1, &BspTaskHandle, 1);

}


void loop()
{
	vTaskDelay(500 / portTICK_PERIOD_MS);
}

