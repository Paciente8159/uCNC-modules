# uCNC-modules

Addon modules for µCNC - Universal CNC firmware for microcontrollers

# About G28.1-G30.1 for µCNC

This module adds custom G28.1-G30.1 code to the µCNC parser. This similar to LinuxCNC G28.1-G30.1 and allows to set G28-G30 home position to the current machine position µCNC.

## Adding G28.1-G30.1 to µCNC

To use the G28.1-G30.1 parser module follow these steps:

1. Copy the the `G28_1-G30_1` directory and place it inside the `src/modules/` directory of µCNC
2. Then you need load the module inside µCNC. Open `src/module.c` and at the bottom of the file add the following lines inside the function `load_modules()`

```
LOAD_MODULE(g28_1_g30_1);
```

4. The last step is to enable `ENABLE_PARSER_MODULES` inside `cnc_config.h`
