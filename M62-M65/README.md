# uCNC-modules

Addon modules for µCNC - Universal CNC firmware for microcontrollers

## About M62-M65 for µCNC

This module adds custom M62-M65 code to the µCNC parser. This similar to LinuxCNC M62-M65 and allows to turn on and off any generic digital pin (synched or immediately), of the µCNC.

## Adding M62-M65 to µCNC

To use the M62-M65 parser module follow these steps:

1. Copy the the `M62-M65` directory and place it inside the `src/modules/` directory of µCNC
2. Then you need load the module inside µCNC. Open `src/module.c` and at the bottom of the file add the following lines inside the function `load_modules()`

```
LOAD_MODULE(m62_m65);
```

3. The last step is to enable `ENABLE_PARSER_MODULES` inside `cnc_config.h`

## Using M62-M65 to µCNC

The M62-M65 can be used like this.

```
M62-M65 P<Generic output pin number from 0 to 31>

or 

M62-M65 P<Custom internal function variable number > 64>
```
