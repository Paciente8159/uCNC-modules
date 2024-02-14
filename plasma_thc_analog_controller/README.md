# uCNC-modules

Addon modules for µCNC - Universal CNC firmware for microcontrollers

## About Plasma THC analog controller for µCNC

This module implements the torch height detection via analog pin and evaluation/control to provide the required ARC OK, Torch DOWN/UP signals to the Plasma THC tool to µCNC.

## Adding Plasma THC analog controller to µCNC

To use thePlasma THC analog controller module follow these steps:

1. Copy the the `plasma_thc_analog_controller` directory and place it inside the `src/modules/` directory of µCNC
2. Then you need load the module inside µCNC. Open `src/module.c` and at the bottom of the file add the following lines inside the function `load_modules()`

```
LOAD_MODULE(plasma_thc_analog_controller);
```

3. The last step is to enable `ENABLE_SETTINGS_MODULES` and `ENABLE_MAIN_LOOP_SETTINGS` inside `cnc_config.h`

## Configuring Plasma THC analog controller to µCNC

Plasma THC analog controller PID settings can be configured via $1010 (P), $1011 (I), $1012 (D)
