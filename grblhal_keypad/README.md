# uCNC-modules

Addon modules for µCNC - Universal CNC firmware for microcontrollers

## About web pendant for µCNC

This module adds GrblHAL keypad to µCNC.

## Adding GrblHAL keypad to µCNC

To use the GrblHAL keypad module follow these steps:

1. Copy the the `grblhal_keypad` directory and place it inside the `src/modules/` directory of µCNC
2. Then you need load the module inside µCNC. Open `src/module.c` and at the bottom of the file add the following lines inside the function `load_modules()`

```
LOAD_MODULE(grblhal_keypad);
```

3. The last step is to enable `ENABLE_IO_MODULES` and `ENABLE_MAIN_LOOP_MODULES` inside `cnc_config.h`
