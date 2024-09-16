# uCNC-modules

Addon modules for µCNC - Universal CNC firmware for microcontrollers

## About TM driver for µCNC

TMC driver adds support to several Trinamic programmable stepper drivers µCNC.
They can be interfaced either via UART or SPI (all software emulated)

## Adding TM driver to µCNC

To use the and TMC driver follow these steps:

1. Copy the the `tmc_driver` directory and place it inside the `src/modules/` directory of µCNC
2. If needed you may redifine some IO pin and SPI options. Please refer to [PINOUTS.md](https://github.com/Paciente8159/uCNC/blob/master/PINOUTS.md) to check the default pin associations.

TMC driver has to be enabled per stepper.
The full list of settings and options can be checked in the `tmc_driver.h` file

4. Then you need load the module inside µCNC. Open `src/module.c` and at the bottom of the file add the following lines inside the function `load_modules()`

```
LOAD_MODULE(tmc_driver);
```

5. The last step is to enable `ENABLE_MAIN_LOOP_MODULES` and `ENABLE_PARSER_MODULES`(optional) inside `cnc_config.h`
Without `ENABLE_PARSER_MODULES` you will not be able to use the available Mcodes for the TMC driver.
These codes include:
 - M350* (set/get microsteps)
 - M906* (set/get current)
 - M913* (stealthchop threshold)
 - M914* (stall sensitivity-stallGuard capable chips only)
 - M920* (set/get any register) (most registers are protected. To unlock all registers set the `TMC_UNSAFE_MODE` option)

