# uCNC-modules

Addon modules for µCNC - Universal CNC firmware for microcontrollers

## About G7-G8 for µCNC

This module adds custom G7/G8 code to the µCNC parser. This similar to Linux CNC G7/G8 to set radius mode for lathes in µCNC.

## Adding G7-G8 to µCNC

To use the G7-G8 parser module follow these steps:

1. Copy the the `G7-G8` directory and place it inside the `src/modules/` directory of µCNC
2. Then you need load the module inside µCNC. Open `src/module.c` and at the bottom of the file add the following lines inside the function `load_modules()`

```
LOAD_MODULE(g7_g8);
```

3. The last step is to enable `ENABLE_PARSER_MODULES` inside `cnc_config.h`
