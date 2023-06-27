# uCNC-modules

Addon modules for µCNC - Universal CNC firmware for microcontrollers

## About Plasma THC control µCNC

This module adds Plasma THC (Torch Height Control) to µCNC.

## Adding Plasma THC control to µCNC

To use the G5 and G5.1 parser module follow these steps:

1. Copy the the `plasma_thc` directory and place it inside the `src/modules/` directory of µCNC
2. Then you need load the module inside µCNC. Open `src/module.c` and at the bottom of the file add the following lines inside the function `load_modules()`

```
LOAD_MODULE(plasma_thc);
```

3. The last step is to enable `ENABLE_PARSER_MODULES` and `ENABLE_MAIN_LOOP_MODULES` inside `cnc_config.h`
