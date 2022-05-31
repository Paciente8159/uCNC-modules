# uCNC-modules

Addon modules for µCNC - Universal CNC firmware for microcontrollers

## About BLTouch for µCNC

This module adds Compact THC 150 controller to µCNC.

## Adding BLTouch to µCNC

To use the and Compact THC 150 follow these steps:

1. Add the `comp_thc150.c` file to the `src/modules/` directory of µCNC
2. Open `comp_thc150.c` and define the ARC_ON output pin and ARC_OK, TORCH_DOWN and TORCH_UP input pins used by the THC150 controller.
You also need to define the STEPPER that runs the tourch up and down. By default this falls to stepper 2 (usually used in Z axis), but and independent stepper can be used.
If the compensation is inverted enable THC_DIR_INV.

```
#ifndef ARC_ON
#define ARC_ON DOUT0
#endif

#ifndef ARC_OK
#define ARC_OK DIN0
#endif
#ifndef TORCH_DOWN
#define TORCH_DOWN DOUT1
#endif
#ifndef TORCH_UP
#define TORCH_UP DOUT2
#endif

//#define THC_DIR_INV

#ifndef TORCH_STEPPER
#define TORCH_STEPPER 2
#endif
```

3. Then you need load the module inside µCNC. Open `src/module.c` and at the bottom of the file add the following lines inside the function `load_modules()``

```
LOAD_MODULE(comp_thc150);
```

4. The last step is to enable `ENABLE_INTERPOLATOR_MODULES` inside `cnc_config.h`
