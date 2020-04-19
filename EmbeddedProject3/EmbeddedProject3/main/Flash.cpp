#include <EEPROM.h>
#include <Flash.h>


void Flash::Init(int size)
{
	EEPROM.begin(size);
}

int Flash::ReadPassword(uint32_t* Password)
{
	union
	{
		uint16_t Psw[4] = { 0, 0, 0, 0 };
		uint8_t  Psw_uint8_t[16];
	};
	for (int i = 0; i < 16; i++) 
	{ 
		Psw_uint8_t[i] = EEPROM.read(i);
	}
	memcpy(Password, Psw, 16);
	return 0;
}


int Flash::SavePassword(uint32_t* Password)
{
	union
	{
		uint32_t Psw[4] = { 0, 0, 0, 0 };
		uint8_t  Psw_uint8_t[16];
	};
	memcpy(Psw, Password, 16);
	for (int i = 0; i < 16; i++) 
	{ 
		EEPROM.write(i, Psw_uint8_t[i]);  
	}
	EEPROM.commit();  
	return 0;	
}






