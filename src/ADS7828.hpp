// Library for Interfacing with the ADS7828 Analog to Digital Converter over I2C
#ifndef ADS7828_HPP
#define ADS7828_HPP

#include "stm32f1xx_hal.h"

#ifndef HAL_MAX_DELAY
#define HAL_MAX_DELAY 100
#endif

// Number of ADS7828 channel combinations
constexpr uint8_t ADS7828_CHANNELS = 16;

// Defines the command bits for every possible channel selection (Datasheet Table 2)
// Choice between "Differential" for voltage between two channels or "Single Ended" for voltage to COM
enum ADS7828_CHANNEL
{
	CHANNEL_0_COM = 0b1000, // Channel 0 to COM
	CHANNEL_1_COM = 0b1100, // Channel 1 to COM
	CHANNEL_2_COM = 0b1001, // Channel 2 to COM
	CHANNEL_3_COM = 0b1101, // Channel 3 to COM
	CHANNEL_4_COM = 0b1010, // Channel 4 to COM
	CHANNEL_5_COM = 0b1110, // Channel 5 to COM
	CHANNEL_6_COM = 0b1011, // Channel 6 to COM
	CHANNEL_7_COM = 0b1111, // Channel 7 to COM
	CHANNEL_0_1 = 0b0000,	// Channel 0 to 1
	CHANNEL_2_3 = 0b0001,	// Channel 2 to 3
	CHANNEL_4_5 = 0b0010,	// Channel 4 to 5
	CHANNEL_6_7 = 0b0011,	// Channel 6 to 7
	CHANNEL_1_0 = 0b0100,	// Channel 1 to 0
	CHANNEL_3_2 = 0b0101,	// Channel 3 to 2
	CHANNEL_5_4 = 0b0110,	// Channel 5 to 4
	CHANNEL_7_6 = 0b0111,	// Channel 7 to 6
};

// Sets AD Power Down Mode
// See Datasheet (Table 1) for PD Mode Selection
enum ADS7828_PD_MODE
{
	POWER_DOWN = 0b00,	 // Power Down Between A/D Converter Conversions
	REF_OFF = 0b01,		 // Internal Reference Voltage OFF and A/D Converter ON
	REF_ON_AD_OFF = 0b10, // Internal Reference ON and A/D Converter OFF
	REF_ON_AD_ON = 0b11	 // Internal Reference ON and A/D Converter ON
};

class ADS7828
{
public:
	ADS7828(I2C_HandleTypeDef *hi2c, uint8_t address);
	ADS7828(I2C_HandleTypeDef *hi2c, uint8_t address, float external_ref_voltage);
	~ADS7828();

	float read_voltage(ADS7828_CHANNEL channel);
	uint16_t read_digit(ADS7828_CHANNEL channel);

	void set_ref_voltage_external(float ref_voltage);
	void set_ref_voltage_internal();

	void set_power_mode(ADS7828_PD_MODE mode);
	void set_power_mode(ADS7828_PD_MODE mode, bool update_now);

	void set_scaling(ADS7828_CHANNEL channel, float scaling);
	float get_scaling(ADS7828_CHANNEL channel);
	void reset_scaling(ADS7828_CHANNEL channel);
	void reset_scaling();

private:
	void init();

	float _scaling[ADS7828_CHANNELS];		  // Channel Voltage Scaling
	float _ref_voltage = 2.5;					  // Using the internal 2.5V reference voltage by default
	ADS7828_PD_MODE _pd_mode = REF_ON_AD_ON; // Current Power Down Mode

	uint8_t _address = 0x48;  // I2C Address
	I2C_HandleTypeDef *_hi2c; // I2C Handle
};

#endif // ADS7828_HPP
