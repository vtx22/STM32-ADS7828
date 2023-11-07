# STM32 ADS7828 ADC Library
C++ Library for interfacing the ADS7828 AD-Converter with STM32 microcontrollers like the STM32F103C8 using HAL

# Features
- Read all Channel Combinations, Single-Ended and Differential
- Read Digit Values or Voltages
- Internal 2.5V or External Manual Voltage Reference, switchable at runtime
- Power Down Modes with implicit switching
- Fixed Scaling for Voltage Divider applications

# Usage
### Init
Create an ADS7828 object with an initialized I2C handle and the device address.
```
ADS7828 adc = ADS7828(&hi2c1, 0x48);
```
You can specify an external reference voltage in the constructor:
```
ADS7828 adc = ADS7828(&hi2c1, 0x48, ref_voltage);
```
:warning: This changes the *Power Down Mode* implicitly, see [Power Down Mode](#power-down-mode)

### Reading a Channel
The ADS7828 has 8 Channels in total. You can read the digit value of each channel combination by calling 
```
uint16_t digit = adc.read_digit(ADS7828_CHANNEL channel);
```
With 12 Bits resolution the returned value is between 0 and 4095.


The ADS7828 supports two types of readings:
- ***Single-Ended:*** Reads the Channel Voltage with reference to GROUND
- ***Differential:*** Reads the Voltage between two channels

All supported measurements are specified in the `ADS7828_CHANNEL` enum, you can choose between:
| **Name**      | **IN+** | **IN-** |
|---------------|---------|---------|
| CHANNEL_X_GND | CHX     | GND     |
| CHANNEL_0_1   | CH0     | CH1     |
| CHANNEL_2_3   | CH2     | CH3     |
| CHANNEL_4_5   | CH4     | CH5     |
| CHANNEL_6_7   | CH6     | CH7     |
| CHANNEL_1_0   | CH1     | CH0     |
| CHANNEL_3_2   | CH3     | CH2     |
| CHANNEL_5_4   | CH5     | CH4     |
| CHANNEL_7_6   | CH7     | CH6     |

Reading the voltage instead of the digit value is also possible by calling
```
float voltage = adc.read_voltage(ADS7828_CHANNEL channel);
```
This function converts from digits to volts by mapping the digit to the reference voltage, thus the result depends on your reference voltage!

### Scaling
If you want to scale the voltage reading every time you call `read_voltage` you can set a fixed scaling factor. This is especially useful when working with voltages dividers, 
as the ADC voltage gets converted to the actual voltage at the divider.
To set a scaling factor for a certain Channel, call
```
adc.set_scaling(ADS7828_CHANNEL channel, float scaling);
```
To reset the scaling back to 1, you can call 
```
adc.reset_scaling(ADS7828_CHANNEL channel);
```

:warning: Keep in mind, that CHANNEL_0_1 and CHANNEL_1_0 for example have different scaling factors!

### Reference Voltage
All measurements done by the ADS7828 are with reference to the specified reference voltage. There are two types of operation:
- ***Internal Reference:*** The ADC uses the internal voltage source of 2.5V as reference
- ***External Reference:*** The ADC uses an external voltage source as reference
Using the internal source means that an input voltage from 0 - 2.5V is mapped to 0 - 4095.
The ADC supports external references from 50mV to 5V, which changes the LSB/voltage resolution. See the datasheet for more information.

If you are using an external voltage you have to set the voltage, either in the constructor or by calling 
```
adc.set_ref_voltage_external(float voltage);
```
:warning: This changes the *Power Down Mode* implicitly, see [Power Down Mode](#power-down-mode)

:warning: Changing from internal to external reference and vice versa takes some time, measurements less than 1ms after the switch might be inaccurate!

Changing back to the internal reference can ONLY be done by calling 
```
adc.set_ref_voltage_internal();
```
:warning: This changes the *Power Down Mode* implicitly, see [Power Down Mode](#power-down-mode)

:warning: Changing from internal to external reference and vice versa takes some time, measurements less than 1ms after the switch might be inaccurate!

Changing the reference voltage to the correct value ensures that `read_voltage` returns the right voltages. Furthermore, the *Power Down Mode* is changed accordingly. 
### Power Down Mode
You can change the Power Down Mode by calling 
```
adc.set_power_mode(ADS7828_PD_MODE mode);
```
There are 4 different modes to choose from.
| **ADS7828_PD_MODE** | **Power Mode**                                      |
|---------------|-----------------------------------------------------|
| POWER_DOWN    | Power Down Between A/D Converter Conversions        |
| REF_OFF       | Internal Reference Voltage OFF and A/D Converter ON |
| REF_ON_AD_OFF | Internal Reference ON and A/D Converter OFF         |
| REF_ON_AD_ON  | Internal Reference ON and A/D Converter ON          |

As you can see, only certain modes can be used with external/internal references. Therefore, switching is done implicitly when changing the type of reference. 
- ***Internal Reference*** 🠦 Switch to REF_ON_AD_ON
- ***External Reference*** 🠦 Switch to REF_OFF

To update the mode, an I2C command has to be send to the ADC. When calling the function, this is done instantly by sending a random read request. 
However, if you do not want to change the mode until you yourself call the next read request, you can disable the command transmit by calling
```
adc.set_power_mode(ADS7828_PD_MODE mode, bool update_now = false);
```
:warning: Changing from internal to external reference and vice versa takes some time, measurements less than 1ms after the switch might be inaccurate!
