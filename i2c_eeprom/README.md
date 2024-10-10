# uCNC-modules

Addon modules for µCNC - Universal CNC firmware for microcontrollers

## About I2C EEPROM for µCNC

I2C EEPROM allows to add external I2C EEPROM support to µCNC via hardware/software I2C.

## Adding I2C EEPROM to µCNC

To use the and I2C EEPROM follow these steps:

1. Copy the the `i2c_eeprom` directory and place it inside the `src/modules/` directory of µCNC
2. If needed you may redifine some IO pin and I2C options. By default this module tries to use the hardware I2C port if available and if not the software I2C pins. Please refer to [PINOUTS.md](https://github.com/Paciente8159/uCNC/blob/master/PINOUTS.md) to check the default pin associations.
To redefine the IO pins and if software or hardware I2C can is used open `cnc_hal_config.h` and add the needed configurations.

```
// By default uses hardware I2C. Uncomment to use SW I2C
// #define I2C_EEPROM_INTERFACE SW_I2C
// uncomment this change the EEPROM I2C address. By default is 0x50
// #define I2C_EEPROM_ADDRESS 0x50
// uncomment to change the EEPROM access delay
// #define I2C_EEPROM_ACCESS_DELAY 10
// uncomment to modify the read/write timeout
// #define I2C_EEPROM_RW_TIMEOUT 200
// uncomment to configure the EEPROM size
// #define I2C_EEPROM_SIZE 0x8000
// uncomment write data with an offset
// #define I2C_EEPROM_OFFSET 0
// uncomment to setup used pins with SW_I2C
// #define I2C_EEPROM_I2C_CLOCK DIN30
// #define I2C_EEPROM_I2C_DATA DIN31
```

4. Then you need load the module inside µCNC. Open `src/module.c` and at the bottom of the file add the following lines inside the function `load_modules()`

```
LOAD_MODULE(i2c_eeprom);
```

5. The last step is to enable `ENABLE_SETTINGS_MODULES` inside `cnc_config.h`

