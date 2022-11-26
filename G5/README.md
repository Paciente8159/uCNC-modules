# uCNC-modules

Addon modules for µCNC - Universal CNC firmware for microcontrollers

## About G5 and G5.1 for µCNC

This module adds custom G5 (cubic spline) and G5.1 (quadratic spline) code to the µCNC parser. This similar to Linux CNC G5 and G5.1 and allows to make motions based on splines via control points µCNC.

## Adding G5 and G5.1 to µCNC

To use the G5 and G5.1 parser module follow these steps:

1. Copy the the `G5` directory and place it inside the `src/modules/` directory of µCNC
2. Then you need load the module inside µCNC. Open `src/module.c` and at the bottom of the file add the following lines inside the function `load_modules()`

```
LOAD_MODULE(g5);
```

3. The last step is to enable `ENABLE_PARSER_MODULES` inside `cnc_config.h`
