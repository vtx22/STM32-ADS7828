#include "ADS7828.hpp"

/**
 * Constructor for ADS7828 object
 *
 * @param hi2c Pointer to an initialized I2C_HandleTypeDef for the I2C commands
 * @param address I2C address of the device, default is 0x48 for AD0 = AD1 = 0
 */
ADS7828::ADS7828(I2C_HandleTypeDef *hi2c, uint8_t address) : _hi2c(hi2c), _address(address)
{
	init();

	// Set the default power mode for internal ref voltage
	set_power_mode(REF_ON_AD_ON);
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
	init();
	set_ref_voltage_external(external_ref_voltage);
}

ADS7828::~ADS7828()
{
#ifdef ADS7828_DYNAMIC_MEM
	for (uint8_t c = 0; c < ADS7828_CHANNELS; c++)
	{
		delete[] _buffers[c].data;
	}
#endif
}

/**
 * ADC class initialisation
 */
void ADS7828::init()
{
	reset_scaling();
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
	return (read_digit(channel) / 4095.0 * _ref_voltage * _scaling[channel]);
}

/**
 * Reads the digit of a specified channel configuration.
 * ADS7828 has 12 Bit resolution, so values from 0 - 4095!
 *
 * @param channel The ADS7828_CHANNEL configuration you want the digit from
 * @return Measured ADC digit (0 - 4095) of given channel configuration
 */
float ADS7828::read_digit(ADS7828_CHANNEL channel)
{
	uint8_t command = 0x00;

	command |= (((uint8_t)channel) << 4);
	command |= (((uint8_t)_pd_mode) << 2);

	uint8_t data[2] = {0};

	HAL_I2C_Master_Transmit(_hi2c, (_address << 1), &command, 1, HAL_MAX_DELAY);
	HAL_I2C_Master_Receive(_hi2c, (_address << 1), data, 2, HAL_MAX_DELAY);

	uint16_t digit = (uint16_t)((data[0] << 8) + data[1]);

	// No averaging
	if (_buffers[channel].n <= 1)
	{
		return digit;
	}

	// Update the buffer and calculate average
	_buffers[channel].append(digit);
	return _buffers[channel].average();
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
		read_digit(CHANNEL_0_COM);
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

/**
 * Set the scaling for a channel voltage so that read_voltage returns voltage * scaling
 *
 * @param channel The Channel to set the scaling for
 * @param scaling Scaling Factor that will be multiplied with the voltage
 */
void ADS7828::set_scaling(ADS7828_CHANNEL channel, float scaling)
{
	_scaling[channel] = scaling;
}

/**
 * Get the current voltage scaling for a channel
 *
 * @param channel The Channel to get the scaling for
 * @return The current scaling factor of the channel
 */
float ADS7828::get_scaling(ADS7828_CHANNEL channel)
{
	return _scaling[channel];
}

/**
 * Reset the scaling for a channel voltage back to 1
 *
 * @param channel The Channel to reset the scaling for
 */
void ADS7828::reset_scaling(ADS7828_CHANNEL channel)
{
	set_scaling(channel, 1);
}

/**
 * Reset the scaling for all channels back to 1
 *
 */
void ADS7828::reset_scaling()
{
	for (uint8_t c = 0; c < ADS7828_CHANNELS; c++)
	{
		reset_scaling(static_cast<ADS7828_CHANNEL>(c));
	}
}

/**
 * Enables averaging for a certain channel.
 *	Whenever get_digit or get_voltage is called, the result will be the average of the last N values.
 *
 * @param channel The channel to enable averaging fot
 * @param n Number of values to average
 */
void ADS7828::set_averaging(ADS7828_CHANNEL channel, uint8_t n)
{
	// Averaging over 1 value is useless
	if (n == 1)
	{
		return;
	}

#ifdef ADS7828_DYNAMIC_MEM
	// Reserve memory to store n last values
	_buffers[channel].n = n;
	_buffers[channel].data = new uint16_t[n]{0};
#else
	_buffers[channel].n = (n > ADS7828_AVG_MAX) ? ADS7828_AVG_MAX : n;
	clear_averaging(channel);
#endif
}

/**
 * Clears all current values of the channel and sets them to 0
 *
 * @param channel The channel to clear the old values for
 */
void ADS7828::clear_averaging(ADS7828_CHANNEL channel)
{
	for (uint8_t n = 0; n < _buffers[channel].n; n++)
	{
		_buffers[channel].data[n] = 0;
	}
}

/**
 * Disables the averaging and deletes all stored values
 *
 * @param channel The channel to disable averaging for
 */
void ADS7828::disable_averaging(ADS7828_CHANNEL channel)
{
	// Averaging is already disabled
	if (_buffers[channel].n == 1)
	{
		return;
	}

	_buffers[channel].n = 1;

#ifdef ADS7828_DYNAMIC_MEM
	delete[] _buffers[channel].data;
#else
	clear_averaging(channel);
#endif
}
