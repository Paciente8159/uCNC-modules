# RP Pico PIO Encoder

RP2040/RP2350 custom encoder backend for uCNC. Each configured driver instance
uses one PIO state machine and can count either quadrature A/B input or a
single undirectional pulse input.

The generic encoder module continues to own position, direction inversion, RPM,
index processing, virtual indexes, and status reporting. This module only
provides the hardware count through the selected encoder's
`enc_custom_read_encX()` callback.

## Configuration

Example assigning PIO driver instance 0 to uCNC encoder 0:

```c
#define ENCODERS 1
#define ENC0_TYPE ENC_TYPE_CUSTOM
#define ENC0_PULSE DIN0
#define ENC0_DIR DIN1
#define ENC0_IS_INCREMENTAL
#define ENC0_NO_WRAP_CORRECTION
#define ENC0_CPR 4000

#define RPICO_PIO_ENC0 ENC0
#define RPICO_PIO0_INDEX 0
#define RPICO_PIO0_SM 0
#define RPICO_PIO0_MAX_STEP_RATE 0

#define LOAD_MODULES_OVERRIDE() ({ \
    LOAD_MODULE(rpico_pio_encoder); \
})
```

`RPICO_PIO_ENC0` through `RPICO_PIO_ENC7` are independent driver
assignments. They may target any enabled uCNC encoder configured as
`ENC_TYPE_CUSTOM`.

By default:

- Driver instances 0-3 use PIO0 state machines 0-3.
- Driver instances 4-7 use PIO1 state machines 0-3.
- `RPICO_PIOx_INDEX`, `RPICO_PIOx_SM`, and
  `RPICO_PIOx_MAX_STEP_RATE` can override those defaults independently.

RP2350 may also use PIO index 2. RP2040 supports PIO indices 0 and 1.

## Counter Mode

The assigned encoder's `ENCx_PULSE` and `ENCx_DIR` definitions select the
mode automatically:

- Different pins select x4 quadrature counting. `DIR` must resolve to the GPIO
  immediately following `PULSE`, as required by the canonical Raspberry Pi
  quadrature PIO program.
- Equal pins select a single undirectional pulse counter. It increments once on
  every falling edge.

For quadrature, `ENCx_CPR` is the effective x4 count per revolution. For
example, a 1000 PPR encoder uses a CPR of 4000. For single-wire input, CPR is
the number of falling edges per revolution.

Use `ENCx_IS_INCREMENTAL` so the generic encoder layer calculates deltas,
direction inversion, and RPM. Use `ENCx_NO_WRAP_CORRECTION` because the PIO
backend already exposes a continuous 32-bit counter.

## Core Integration

Because the encoder is `ENC_TYPE_CUSTOM`, `encoder.c` does not include its
pulse pin in the software interrupt mask. The PIO state machine owns pulse
acquisition, while `encoder.c` periodically reads the hardware counter through
the generated custom callback.

Physical index inputs remain on the normal `io_control.c` path. Configure
`ENCx_INDEX`, `ENCx_VIRTUAL_INDEX`, and related options exactly as for other
encoder backends.

The legacy `PIO_ENC`, `PIO_ENC_INDEX`, `PIO_ENC_SM`, and
`PIO_ENC_MAX_STEP_RATE` names remain accepted for driver instance 0.
