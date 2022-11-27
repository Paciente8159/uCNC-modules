# uCNC-modules

Addon modules for µCNC - Universal CNC firmware for microcontrollers

## About M67-M68 for µCNC

This module adds custom M67-M68 code to the µCNC parser. This similar to LinuxCNC M67-M68 and allows to turn on and off any analog pin (synched or immediately), of the µCNC.

## Adding M67-M68 to µCNC

To use the M67-M68 parser module follow these steps:

1. Copy the the `M67-M68` directory and place it inside the `src/modules/` directory of µCNC
2. Then you need load the module inside µCNC. Open `src/module.c` and at the bottom of the file add the following lines inside the function `load_modules()`

```
LOAD_MODULE(m67_m68);
```

3. The last step is to enable `ENABLE_PARSER_MODULES` inside `cnc_config.h`

## Using M67-M68 to µCNC

The M67-M68 can be used like this.

```
M67-M68 E<PWM output pin number from 0 to 15> Q<output value from 0 to 255>
```
