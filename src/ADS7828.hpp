//Library for Interfacing with the ADS7828 Analog to Digital Converter over I2C
#pragma once
#include <stdint.h>
#include "stm32f1xx_hal.h"

enum CHANNEL
{
	CHANNEL_0 = 0b000,
	CHANNEL_1 = 0b100,
	CHANNEL_2 = 0b001,
	CHANNEL_3 = 0b101,
	CHANNEL_4 = 0b010,
	CHANNEL_5 = 0b110,
	CHANNEL_6 = 0b011,
	CHANNEL_7 = 0b111
};


class ADS7828
{
	public:
		ADS7828(I2C_HandleTypeDef* hi2c);

		float readChannelVoltage(CHANNEL channel);

		uint16_t readChannel(CHANNEL channel);
		void readAllChannels(uint16_t* data);



	private:
		bool _mode 	= 0x01; //0 = Differential, 1 = Single-Ended Input
		bool _pd1 	= 0x01;	//See Datasheet for PD Mode Selection
		bool _pd0 	= 0x01;

		//Voltage Dividers for 5V ADC and BAT ADC
		float _dividers[2] = {1600.0 / (2400.0 + 1600.0), 2000.0 / (2000.0 + 11000.0)};

		float _refVol = 2.5; 	//Using the internal 2.5V reference voltage as configured by PD-Mode

		const uint8_t _address = 0x48;
		I2C_HandleTypeDef* _hi2c;
};
