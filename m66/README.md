# uCNC-modules

Addon modules for µCNC - Universal CNC firmware for microcontrollers

## About M66 for µCNC

This module adds custom M66 code to the µCNC parser. This similar to Marlin M66 and allows to wait on digital or analog input pin of the µCNC.

## Adding M66 to µCNC

To use the M66 parser module follow these steps:

1. Copy the the `M66` directory and place it inside the `src/modules/` directory of µCNC
2. Then you need load the module inside µCNC. Open `src/module.c` and at the bottom of the file add the following lines inside the function `load_modules()`

```
LOAD_MODULE(m66);
```

3. The last step is to enable `ENABLE_PARSER_MODULES` inside `cnc_config.h`

## Using M66 to µCNC

The M66 can be used like this.

```
M66 P<0-31> | E<0-15> <L->
```
- P- - specifies the digital input (DIN pins) number from 0 to 31.
- E- - specifies the analog input number from 0 to 15.
- L- - specifies the wait mode type.
  - Mode 0: IMMEDIATE - no waiting, returns immediately. The current value of the input is stored in parameter #5399
  - Mode 1: RISE - waits for the selected input to perform a rise event. (only one permitted for an analog input)
  - Mode 2: FALL - waits for the selected input to perform a fall event.
  - Mode 3: HIGH - waits for the selected input to go to the HIGH state.
  - Mode 4: LOW - waits for the selected input to go to the LOW state.
- Q- - specifies the timeout in seconds for waiting. If the timeout is exceeded, the wait is interrupt, and the variable #5399 will be holding the value -1. The Q value is ignored if the L-word is zero - (IMMEDIATE). A Q value of zero is an error if the L-word is non-zero.
