# RP2350 PIO Encoder

RP2350 custom encoder backend for uCNC using a PIO state machine to count
quadrature A/B spindle encoder pulses.

This module is used as an `ENC_TYPE_CUSTOM` encoder. The normal uCNC encoder
module still owns the public encoder state and status fields, so seeing the
standard `EC` and `RPM` status output is expected. The RP2350 PIO code is the
backend behind `enc_custom_read()`.

## Basic Configuration

Example for encoder 0:

```c
#define ENABLE_RP2350_PIO_ENCODER

#define ENCODERS 1
#define ENC0_TYPE ENC_TYPE_CUSTOM

#define ENC0_PULSE_GPIO 20  // A
// B is ENC0_PULSE_GPIO + 1, so GPIO21 here.

#define ENC0_INDEX_GPIO 26  // optional physical Z/index phase reference

#define ENC0_PIO_INDEX 0
#define ENC0_PIO_SM 0
#define ENC0_MAX_STEP_RATE 0
#define ENC0_PIO_PROGRAM_OFFSET 0

#define ENC0_IS_INCREMENTAL
#define ENC0_CPR 4000

#define SPINDLE_PWM_RPM_ENCODER ENC0

#define G33_ENCODER ENC0
#define G33_FEEDBACK_LOOP_USE_HW_COUNTER

#define ENC0_VIRTUAL_INDEX 1
#define ENC0_VIRTUAL_INDEX_CPR (ENC0_CPR / 10)
#define ENC0_VIRTUAL_INDEX_OFFSET 0
#define ENC0_VIRTUAL_INDEX_HYSTERESIS 1

#define LOAD_MODULES_OVERRIDE() ({ \
    LOAD_MODULE(rp2350_pio_encoder); \
    LOAD_MODULE(g33); \
})
```

`ENC0_CPR` should match the effective quadrature counts per spindle revolution.
For a 1000 PPR encoder in x4 quadrature mode, use `4000`.

## How G33 Uses It

The PIO counter is the spindle position truth. The physical index GPIO is only a
phase reference. G33 does not need to use the physical index directly; instead,
this module emits the normal `enc0_index` hook at virtual modulo boundaries.

With `ENC0_CPR = 4000` and `ENC0_VIRTUAL_INDEX_CPR = ENC0_CPR / 10`, a virtual index
should be emitted every 400 encoder counts.

In `G33_FEEDBACK_LOOP_USE_HW_COUNTER` mode:

- raw PIO encoder counts provide the phase ruler
- generic `encoder_get_rpm()` seeds the starting feed
- virtual index hooks start and update the synchronized motion
- timed virtual-index periods are not used as the main RPM source

## Debug Output

When `ENCODER_DEBUG_PRINT_100MS` is enabled, the generic encoder module prints:

```text
[EC:145837 RPM:208]
```

That still means the RP2350 PIO backend is being used when `ENC0_TYPE` is
`ENC_TYPE_CUSTOM`.

This module also prints virtual index statistics:

```text
[ENCIDX EC:145837 ECB:381 LAST:400 AVG:409.4 MIN:-4000 MAX:400 N:381 IGN:9 ISR:38]
```

Fields:

- `EC`: current encoder count reported through the normal encoder layer
- `ECB`: live count since the last accepted virtual index
- `LAST`: count between the last two virtual indexes
- `AVG`: average absolute virtual-index spacing
- `MIN`/`MAX`: observed virtual-index spacing range
- `N`: accepted virtual-index events
- `IGN`: skipped virtual slots during catch-up
- `ISR`: physical index GPIO edges captured

For the 4000 CPR / 10 virtual index example, healthy steady-state output should
show `LAST:400`. `IGN` may increase during startup or after stalls, but should
not climb steadily during normal running.

## Original Failure Mode

The first symptom looked like the custom encoder was not initialized because only
the standard `EC/RPM` debug line was visible. That was misleading: the standard
line is printed by the generic encoder module even when the source is the RP2350
PIO custom backend.

There were three separate issues:

1. The RP2350 index debug helper only updated an internal string. It did not
   print anything, so virtual-index health was invisible.
2. The virtual-index task jumped directly to the newest crossed slot. At about
   200 RPM with 10 virtual indexes per revolution, several slots could pass
   between task calls. That produced `LAST:1600` or `LAST:2000` instead of
   `LAST:400`, and G33 saw bad timing.
3. G33 could start synchronized motion while the interpolator was still empty.
   The log showed `G33 start ... empty=1`, followed by `st=2` forever. The move
   was stuck in `SYNC_STARTING` because no synchronized segment had begun.

The fix was:

- print the RP2350 virtual-index debug line when debug is enabled
- emit bounded catch-up virtual indexes instead of collapsing several slots into
  one hook
- seed G33 feed from the normal encoder RPM / raw hardware counter, not from
  virtual-index timing
- defer `itp_start(false)` until `itp_is_empty()` is false

Healthy G33 logs should now show feed near spindle RPM for `K1`:

```text
[MSG:G33 init ... feed=208.000]
[MSG:G33 start hw=... idx=1 empty=0]
[MSG:G33 exp=... real=... err=0]
```
