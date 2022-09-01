# uCNC-modules

Addon modules for µCNC - Universal CNC firmware for microcontrollers

## About IC74HC595 for µCNC

This module adds the ability to control the IC74HC595 shift register controller (used for example in the MKS-DLC32 board) to µCNC.

## Adding IC74HC595 to µCNC

To use the IC74HC595 parser module follow these steps:

1. On your boardmap file add the number 74HC595 IC's used (single or chained)

```
#define IC74HC595_COUNT 1 // single IC
```
or

```
#define IC74HC595_COUNT 7 // 7 chained IC's. 7 is the maximum allowed
```

2. Add each µCNC PIN offset. Each 74HC595 has 8 output pins. For example if STEP0 placed in output 3 of IC74HC595 number 0 the offset will be 8*0 + 3. The offset is the PIN name followed by _IO_OFFSET

```
#define STEP0_IO_OFFSET 3 // STEP0 is on output 3 of IC74HC595 0
#define DIR0_IO_OFFSET 11 // DIR0 is on output 3 of IC74HC595 1
```

3. Add the `IC74HC595.c` file to the `src/modules/` directory of µCNC
4. It's not necessary to load the module. This module overrides the event handlers directly.
5. The last step is to enable `ENABLE_IO_MODULES` inside `cnc_config.h`