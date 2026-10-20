# uCNC-modules

Addon modules for µCNC - Universal CNC firmware for microcontrollers

## About G33 for µCNC

This module adds custom G33 code to the µCNC parser. This similar to Linux CNC G33 and allows to make motions synched with the spindle in µCNC.

## Adding G33 to µCNC

To use the G33 parser module follow these steps:

1. Copy the the `G33` directory and place it inside the `src/modules/` directory of µCNC
2. Then you need load the module inside µCNC. Open `src/module.c` and at the bottom of the file add the following lines inside the function `load_modules()`

```
LOAD_MODULE(g33);
```

3. You must configure the encoder that will handle the RPM reading in the `cnc_hal_config.h`

```
#define ENCODERS 1

// Uses DIN7 input as ENCODER0 in counter mode
#define ENC0_PULSE DIN7
#define ENC0_DIR DIN7

// define an encoder index pin if the pin also changes state synchronized with the PULSE pin
#define ENC0_INDEX DIN8

/** OR **/

// define a custom interruptable pin that will be used to dispatch the index event for G33 like this
#define G33_INDEX_PIN DIN6

// Assign which encoder will be used by G33
#define G33_ENCODER ENC0
```

### Virtual Index Updates

On ESP32 builds using the `esp32_pcnt_encoder` module, G33 can run from the
encoder module's virtual index hook instead of a dedicated `G33_INDEX_PIN`.
PCNT0 A/B remains the spindle position source, and the encoder module fires the
normal `enc0_index` hook whenever the PCNT count crosses a configurable modulo
boundary.

```c
#define G33_ENCODER ENC0
#define ENC0_VIRTUAL_INDEX 1
#define ENC0_VIRTUAL_INDEX_CPR 600
#define ENC0_VIRTUAL_INDEX_OFFSET 0
```

`ENC0_VIRTUAL_INDEX_CPR` sets the encoder-count interval between synthetic
index updates. If the encoder resolution is 3000 counts/rev and the virtual
index CPR is 600, G33 sees 5 index updates per spindle revolution.

No physical G33 index pin is required in virtual-index mode. A physical encoder
Z/index pin may still be used by the encoder module as an optional phase
reference, and physical index references take priority unless virtual-only mode
is configured. G33 itself only consumes the common encoder index hook.

When ENC0 virtual indexing is enabled, G33 derives its updates-per-revolution
value from `ENC0_VIRTUAL_INDEX_CPR` and the runtime encoder resolution.
`G33_INDEXES_PER_REV` remains available for non-ENC0 or external index sources.

RP2350 builds using `rp2350_pio_encoder` should normally use
`G33_FEEDBACK_LOOP_USE_HW_COUNTER`. In that mode the PIO encoder count is the
spindle position truth and virtual index hooks are only synchronization/update
triggers. Do not diagnose that setup from the standard `EC/RPM` line alone: the
generic encoder module still prints that line even when the backend is the
custom RP2350 PIO reader. See `src/modules/rp2350_pio_encoder/README.md` for
the RP2350 virtual-index debug fields and the original empty-interpolator start
race that could leave G33 stuck in `SYNC_STARTING`.

Inside the index ISR a floating point math operation is performed. If this causes issues on a specific architecture you can enable an option to replace it by a fixed point operation.

`#define G33_REPLACE_FP_OPERATION_IN_ISR`

4. You should also enable RPM counter on the tool `cnc_hal_config.h`. This will allow reading the tool actual speed and not the programmed speed. For example for spindle_pwm tool it's done like this:

```
// assign the tools from 1 to 16
#define TOOL1 spindle_pwm

// assign ENC0 to the spindle rpm encoder
  #define SPINDLE_RPM_ENCODER ENC0
```

5. The last step is to enable `ENABLE_MAIN_LOOP_MODULES`, `ENABLE_PARSER_MODULES`, `ENABLE_IO_MODULES` and `ENABLE_RT_SYNC_MOTIONS` inside `cnc_config.h`
