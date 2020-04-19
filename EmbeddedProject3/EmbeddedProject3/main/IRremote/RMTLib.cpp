#include "Arduino.h"
#include "RMTLib.h"

#include "IRremote/esp32_rmt_remotes.h"
#include "IRremote/esp32_rmt_common.h"

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "driver/rmt.h"
#include "driver/periph_ctrl.h"
#include "soc/rmt_reg.h"

#include "Bsp.h"
#include "Output.h"
#include <Flash.h>

#ifdef __cplusplus
extern "C" {
#endif
/* Include C code here */
#ifdef __cplusplus
}
#endif

//------------------------------------------------------------------
//---------------     Global variable    ---------------------------
//------------------------------------------------------------------
uint32_t New_Data = 0;

TaskHandle_t IRTaskHandle = NULL;
void IR_task(void *pvParameter)
{
	while (1) 
	{	
		if (New_Data == 0)
		{
			// read ir data
			New_Data = rmtlib_nec_receive();		
		}		
	}
}

RMTLib::RMTLib () 
{ 
	rx_pin = RMT_RX_GPIO_NUM;
	for (uint8_t i = 0; i < 4; i++)
	{
		Data[i] = 0;
	}
}
	
Flash Password;
void RMTLib::Init()
{ 
	Password.Init(512);
	xTaskCreateUniversal(IR_task, "ir_task", 4096, NULL, 1, &IRTaskHandle, 1);
}


//------------------------------------------------------------------
//---------------------------  Rmt Clk  ----------------------------
//------------------------------------------------------------------
typedef enum _IR_state { Read_To_Save, Save_Psw, Read_To_Disalarm } _IR_state;
volatile _IR_state IR_state = Read_To_Disalarm;
uint32_t  Data_Buff[4] = { 0, 0, 0, 0 };
uint8_t   Data_Num = 0;
#define  Max_Time 501
uint32_t  Data_Time = Max_Time;
uint32_t  Blink = 0;
uint32_t  Flash_Buff[4] = { 0, 0, 0, 0 };
extern  TaskHandle_t LocalTaskHandle;
#define IR_ALARM_DISABLE 0x02
void RMTLib::Clk()
{
	if (save)
	{
		save = false;
		IR_state = Read_To_Save;
		Blink = 16;
	}
	
	// if data is received
	if(New_Data)
	{	
		if (Data_Num == 4)	Data_Num = 0;
		Data_Buff[Data_Num] = New_Data;
		New_Data = 0;
		Data_Num++;
		Data_Time = 0; 
		Blink = 1;
	}
	
	if (Data_Time > (5*1000/100))
	{
		Data_Num = 0;
	}	
	else if (Data_Time == (5 * 1000 / 100))
	{
		Blink = 4;
		Data_Time++;
	}
	else
	{
		Data_Time++;
	}
		
	switch (IR_state)
	{
		case Read_To_Disalarm :
			if (Data_Num == 4)
			{
				Data_Num = 0;
				Data_Time = Max_Time;
				Password.ReadPassword(Flash_Buff);
				uint8_t Equal = 0;
				for (uint8_t Compare = 0; Compare < 4; Compare++)
				{
					if(Flash_Buff[Compare] == Data_Buff[Compare])
					{
						Equal++;	
					}
				}
				if (Equal == sizeof(Data_Buff)/4)
				{
					xTaskNotify(LocalTaskHandle, IR_ALARM_DISABLE, eSetBits);
				}	
			}
		break;
		
		case Read_To_Save :
			if (Data_Num == 4)
			{
				IR_state = Save_Psw;
			}
		break;
		
		case Save_Psw :
			Password.SavePassword(Data_Buff);
			Blink = 8;
			IR_state = Read_To_Disalarm;
			Data_Time = Max_Time;
		break;
		
		default :
			IR_state = Read_To_Disalarm;
		break;
	}
	
	if (Blink)
	{
		LedStateGreen.Toggle();
		Blink--;
	}
	else
	{
		LedStateGreen.SetState(false);		
	}

}

void RMTLib::Save() 
{ 
	save = true;
}


// Private


