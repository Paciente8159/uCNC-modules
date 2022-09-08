# uCNC-modules

Addon modules for µCNC - Universal CNC firmware for microcontrollers

## About M80-M81 for µCNC

This module adds custom M80-M81 code to the µCNC parser. This similar to Marlin M80-M81 and allows to turn on and off any generic digital pin, PWM or servo pin of the µCNC.

## Adding M80-M81 to µCNC

To use the M80-M81 parser module follow these steps:

1. Copy the the `M80-M81` directory and place it inside the `src/modules/` directory of µCNC
2. Open `parser_m80_m81.c` and define the the PSU PIN and value to turn the PSU ON

```
#ifndef PSU_PIN
#define PSU_PIN DOUT31
#endif
#ifndef PSU_ON
#define PSU_ON true
#endif
```

3. Then you need load the module inside µCNC. Open `src/module.c` and at the bottom of the file add the following lines inside the function `load_modules()`

```
LOAD_MODULE(m80_m81);
```

4. The last step is to enable `ENABLE_PARSER_MODULES` inside `cnc_config.h`
