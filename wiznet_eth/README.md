# uCNC-modules

Addon modules for µCNC - Universal CNC firmware for microcontrollers

## About Wiznet ethernet for µCNC

This module adds Wiznet ethernet to µCNC.
This overrides the microcontroller WiFi network (only one network device can be used)

## Adding Wiznet ethernet to µCNC

To use the Wiznet ethernet module follow these steps:

1. Copy the the `wiznet_eth` directory and place it inside the `src/modules/` directory of µCNC
2. If needed you may redifine some IO pin and SPI options. By default this module tries to use the hardware SPI port if available and if not the software SPI pins. Please refer to [PINOUTS.md](https://github.com/Paciente8159/uCNC/blob/master/PINOUTS.md) to check the default pin associations.
To redefine the IO pins and if software or hardware SPI can is used open `cnc_hal_config.h` and add the needed configurations.
**NOTE:** This module requires using `USE_STATIC_IP` and manually set a fixed IP address on build.

```
// to allow use $ip command to get the board IP
#ifdef ENABLE_PARSER_MODULES
// use hardware spi
#define WIZNET_INTERFACE WIZNET_HW_SPI
#define WIZNET_SPI_FREQ 1000000
// define the chip select pin
#define WIZNET_CS DOUT40
```

3. Then you need load the module inside µCNC. Open `src/module.c` and at the bottom of the file add the following lines inside the function `load_modules()`

```
LOAD_MODULE(wiznet_eth);
```

