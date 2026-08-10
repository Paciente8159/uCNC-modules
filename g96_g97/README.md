# uCNC-modules

Addon modules for µCNC - Universal CNC firmware for microcontrollers

## About G33 for µCNC

This module adds custom G96 and G97 code to the µCNC parser. This similar to Linux CNC G96 and G97 and allows to switch constant surface speed (CSS) mode on and off in µCNC.

## Adding G96/G97 to µCNC

To use the G96/G97 parser module follow these steps:

1. Copy the the `g96_g97` directory and place it inside the `src/modules/` directory of µCNC
2. Then you need load the module inside µCNC. Open `src/module.c` and at the bottom of the file add the following lines inside the function `load_modules()`

```
LOAD_MODULE(g96_g97);
```

3. You can configure the the reference lathe axis that will be coupled to the RPM control (X axis by default)

```
// change the default axis to Y
#define G96_REFERENCE_AXIS AXIS_Y

// Change the defaults speed calculation segment distance (by default it adjusts the RPM every 1 unit (mm/inch))
// you can fine tune it (for example to 0.5)
#define G96_MIN_SEGM_SIZE 0.5
```

5. The last step is to enable `ENABLE_MOTION_CONTROL_MODULES`, `ENABLE_PARSER_MODULES` inside `cnc_config.h`
