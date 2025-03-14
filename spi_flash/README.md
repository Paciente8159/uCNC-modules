# uCNC-modules

Addon modules for µCNC - Universal CNC firmware for microcontrollers

## About SPI Flash for µCNC

Adds external SPI Flash support (W25Qxxx) support to µCNC via hardware/software SPI to store settings.

## Adding SPI Flash to µCNC

To use the and SPI Flash follow these steps:

1. Copy the the `spi_flash` directory and place it inside the `src/modules/` directory of µCNC
2. If needed you may redifine some IO pin and SPI options. By default this module tries to use the hardware SPI port if available and if not the software SPI pins. Please refer to [PINOUTS.md](https://github.com/Paciente8159/uCNC/blob/master/PINOUTS.md) to check the default pin associations.
To redefine the IO pins and if software or hardware SPI can is used open `cnc_hal_config.h` and add the needed configurations.

```
// By default uses hardware SPI. Uncomment to use HW SPI
// #define FLASH_SPI_INTERFACE FLASH_SPI_HW_SPI
// uncomment this change the SPI Flash address initial offset. By default is 0x0
// #define SPI_FLASH_SEC_OFFSET 0x0
// uncomment to setup used pins with SW_SPI
// #define FLASH_SPI_CLK DOUT30
// #define FLASH_SPI_SDO DOUT29
// #define FLASH_SPI_SDI DIN29
// #define FLASH_SPI_CS SPI_CS
```

4. Then you need load the module inside µCNC. Open `src/module.c` and at the bottom of the file add the following lines inside the function `load_modules()`

```
LOAD_MODULE(spi_flash);
```

5. The last step is to enable `ENABLE_SETTINGS_MODULES` inside `cnc_config.h`

