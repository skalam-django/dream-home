#ifndef _pinMapping_H_
#define _pinMapping_H_

#include <Arduino.h>


class pinMapping
{

	public:
		pinMapping();
		~pinMapping();
		uint16_t* NodeMCU_D1_Mini_Dual_Pins(String port);
		uint16_t NodeMCU_D1_Mini_Single_Pin(String port);
};

#endif

			