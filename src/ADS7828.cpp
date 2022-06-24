#include "ADS7828.hpp"

ADS7828::ADS7828(I2C_HandleTypeDef* hi2c) : _hi2c(hi2c)
{
}

float ADS7828::readChannelVoltage(CHANNEL channel)
{
	if(channel == CHANNEL_6 || channel == CHANNEL_7)
	{
		float divider = (channel == CHANNEL_6) ? _dividers[0] : _dividers[1];
		return (readChannel(channel) * _refVol / 4095.0) / divider;
	}
	else
	{
		return readChannel(channel) * _refVol / 4095.0;
	}
}


uint16_t ADS7828::readChannel(CHANNEL channel)
{
	uint8_t ch = (uint8_t)channel;

	uint8_t command = 0x00;
	command |= (_mode << 7);
	command |= (ch << 4);
	command |= (_pd1 << 3);
	command |= (_pd0 << 2);

	uint8_t data[2] = {0x00, 0x00};

	HAL_I2C_Master_Transmit(_hi2c, (_address << 1), &command, 1, 100);
	HAL_I2C_Master_Receive(_hi2c, (_address << 1), data, 2, 100);

	return (uint16_t)((data[0] << 8) + data[1]);
}

void ADS7828::readAllChannels(uint16_t* data)
{
	data[0] = readChannel(CHANNEL_0);	//0x8C
	data[1] = readChannel(CHANNEL_1);	//0xCC
	data[2] = readChannel(CHANNEL_2);	//0x9C
	data[3] = readChannel(CHANNEL_3);	//0xDC
	data[4] = readChannel(CHANNEL_4);	//0xAC
	data[5] = readChannel(CHANNEL_5);	//0xEC
	data[6] = readChannel(CHANNEL_6);	//0xBC
	data[7] = readChannel(CHANNEL_7);	//0xFC
}

