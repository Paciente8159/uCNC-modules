# uCNC-modules

Addon modules for µCNC - Universal CNC firmware for microcontrollers

# About M17-M18 for µCNC

This module adds custom M17-M18 code to the µCNC parser. This similar to Marlin M17-M18 and allows enable/disable stepper motors of the µCNC.

## Adding M17-M18 to µCNC

To use the M17-M18 parser module follow these steps:

1. Copy the the `M17-M18` directory and place it inside the `src/modules/` directory of µCNC
2. Then you need load the module inside µCNC. Open `src/module.c` and at the bottom of the file add the following lines inside the function `load_modules()`

```
LOAD_MODULE(m17_m18);
```

4. The last step is to enable `ENABLE_PARSER_MODULES` inside `cnc_config.h`
