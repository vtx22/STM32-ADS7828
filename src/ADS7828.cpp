#include "ADS7828.hpp"

/**
 * Constructor for ADS7828 object
 *
 * @param hi2c Pointer to an initialized I2C_HandleTypeDef for the I2C commands
 * @param address I2C address of the device, default is 0x48 for AD0 = AD1 = 0
 */
ADS7828::ADS7828(I2C_HandleTypeDef *hi2c, uint8_t address) : _hi2c(hi2c), _address(address)
{
}

/**
 * Constructor for ADS7828 object for usage with external reference voltage.
 * Implicitly changes the power down mode to disable the internal voltage reference!
 *
 * @param hi2c Pointer to an initialized I2C_HandleTypeDef for the I2C commands
 * @param address I2C address of the device, default is 0x48 for AD0 = AD1 = 0
 * @param external_ref_voltage The external reference voltage (in Volts) connected to the ADC, should be between 0.05V and 5V
 */
ADS7828::ADS7828(I2C_HandleTypeDef *hi2c, uint8_t address, float external_ref_voltage) : _hi2c(hi2c), _address(address)
{
	set_ref_voltage_external(external_ref_voltage);
}

ADS7828::~ADS7828()
{
}

/**
 * Reads the voltage of a specified channel configuration.
 * Conversion from digits to voltage is done with the set reference voltage!
 *
 * @param channel The ADS7828_CHANNEL configuration you want the voltage from
 * @return Measured ADC Voltage [V] of given channel configuration
 */
float ADS7828::read_voltage(ADS7828_CHANNEL channel)
{
	return read_digit(channel) * _ref_voltage / 4095.0;
}

/**
 * Reads the digit of a specified channel configuration.
 * ADS7828 has 12 Bit resolution, so values from 0 - 4095!
 *
 * @param channel The ADS7828_CHANNEL configuration you want the digit from
 * @return Measured ADC digit (0 - 4095) of given channel configuration
 */
uint16_t ADS7828::read_digit(ADS7828_CHANNEL channel)
{
	uint8_t command = 0x00;

	command |= (((uint8_t)channel) << 4);
	command |= (((uint8_t)_pd_mode) << 2);

	uint8_t data[2] = {0x00, 0x00};

	HAL_I2C_Master_Transmit(_hi2c, (_address << 1), &command, 1, HAL_MAX_DELAY);
	HAL_I2C_Master_Receive(_hi2c, (_address << 1), data, 2, HAL_MAX_DELAY);

	return (uint16_t)((data[0] << 8) + data[1]);
}

/**
 * Set your external reference voltage for operation without the internal reference.
 * Implicitly switches the power down mode to turn the internal reference OFF!
 *
 * @param ref_voltage External reference voltage in [V]
 */
void ADS7828::set_ref_voltage_external(float ref_voltage)
{
	_ref_voltage = ref_voltage;

	if (_pd_mode == REF_OFF)
	{
		return;
	}

	// If you choose an external voltage reference we have to change the mode accordingly
	set_power_mode(REF_OFF, true);
}

/**
 * Set the reference voltage back to internal (2.5V).
 * Implicitly switches the power down mode to turn the internal reference ON!
 *
 * @param ref_voltage External reference voltage in [V]
 */
void ADS7828::set_ref_voltage_internal()
{
	_ref_voltage = 2.5;

	// If you choose an internal voltage reference we have to change the mode accordingly
	set_power_mode(REF_ON_AD_ON, true);
}

/**
 * Set the ADC power down mode (see datasheet for more info).
 * Implicitly switches the reference voltage to internal (2.5V) for modes with REF_ON_x
 *
 * @param mode The mode you want to switch to
 * @param update_now If true, the mode is switched instantly by sending a command. Otherwise mode is changed with next read request!
 */
void ADS7828::set_power_mode(ADS7828_PD_MODE mode, bool update_now)
{
	_pd_mode = mode;

	// If you choose a mode with internal reference we have to set the voltage back
	if (mode == REF_ON_AD_OFF || mode == REF_ON_AD_ON)
	{
		set_ref_voltage_internal();
	}

	// To update the mode we have to transmit the command, so just do a random request
	if (update_now)
	{
		read_digit(CHANNEL_0_GND);
	}
}

/**
 * Set the ADC power down mode (see datasheet for more info).
 * Implicitly switches the reference voltage to internal (2.5V) for modes with REF_ON_x
 *
 * @param mode The mode you want to switch to
 */
void ADS7828::set_power_mode(ADS7828_PD_MODE mode)
{
	set_power_mode(mode, true);
}