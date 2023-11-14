# STM32 ADS7828 ADC Library
C++ Library for interfacing the ADS7828 AD-Converter with STM32 microcontrollers like the STM32F103C8 using HAL

# Features
- Read all Channel Combinations, Single-Ended and Differential
- Read Digit Values or Voltages
- Internal 2.5V or External Manual Voltage Reference, switchable at runtime
- Power Down Modes with implicit switching
- Fixed Scaling for Voltage Divider applications
- Averaging of the last N values for every channel (dynamic or static storage options)

# Usage
### Includes and Compilation
Include the Library via
```
#include "ADS7828.hpp"
```
Make sure to define which STM32 controller you are using! This is relevant for the selection of the HAL Library. You can select the MCU family with the `-D STM32F1` flag while compiling.
Or simply define it at the start of your code with `#define STM32F1`. Change accordingly for your STM32!

---
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

---
### Reading a Channel
The ADS7828 has 8 Channels in total. You can read the digit value of each channel combination by calling 
```
float digit = adc.read_digit(ADS7828_CHANNEL channel);
```
With 12 Bits resolution the returned value is between 0 and 4095.

:warning: Average filters may apply when enabeled, see [Moving Average](#moving-average-filter)

The ADS7828 supports two types of readings:
- ***Single-Ended:*** Reads the Channel Voltage with reference to COM
- ***Differential:*** Reads the Voltage between two channels

All supported measurements are specified in the `ADS7828_CHANNEL` enum, you can choose between:
| **ADS7828_CHANNEL**      | **IN+** | **IN-** |
|---------------|---------|---------|
| CHANNEL_X_COM | CHX     | COM     |
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

---
### Scaling
If you want to scale the voltage reading every time you call `read_voltage` you can set a fixed scaling factor. This is especially useful when working with voltages dividers, 
as the ADC voltage gets converted to the actual voltage at the divider.
To set a scaling factor for a certain Channel, call
```
adc.set_scaling(ADS7828_CHANNEL channel, float scaling);
```
To get the current scaling factor, call
```
float cur_scaling = adc.get_scaling(ADS7828_CHANNEL channel);
```
To reset the scaling back to 1, you can call 
```
adc.reset_scaling(ADS7828_CHANNEL channel);
```
or for all channels 
```
adc.reset_scaling();
```

:warning: Keep in mind, that `CHANNEL_0_1` and `CHANNEL_1_0` for example have different scaling factors!

:warning: Scaling only applies to the **Voltage Reading**, not to the **Digit Reading**!

---
### Moving Average Filter
You have the option to enable averaging of the last `n` values for every channel seperately by calling
```
adc.set_averaging(ADS7828_CHANNEL channel, uint8_t n);
```
The averaging will be applied directly to the digit value, so that `get_digit` returns the average instead of the last value.
If you want to reset the last `n` values to `0`, call
```
adc.clear_averaging(ADS7828_CHANNEL channel);
```
To disable averaging call 
```
adc.disable_averaging(ADS7828_CHANNEL channel);
```
You can choose the way the last values are stored. Generally, the last digits have to be held in an array of at least size `n`. 
There are two memory usage options available with a define in the header:
- **Dynamic:** The array gets dynamically allocated by calling `new/delete`
- **Static:** You can choose the maximum number of values with `ADS7828_AVG_MAX`, for every channel one array of that size is allocated at compile time

To choose, use `#define ADS7828_DYNAMIC_MEM` for dynamic allocation. Otherwise static allocation is used.

---
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

---
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
