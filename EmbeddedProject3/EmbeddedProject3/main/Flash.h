#pragma once
#include "Arduino.h"
class Flash
{
public:
	void Init (int size);
	int ReadPassword(uint32_t* Password);
	int SavePassword(uint32_t* Password);
protected: 

private: 
};