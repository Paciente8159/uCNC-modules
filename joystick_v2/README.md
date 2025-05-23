# uCNC-modules

Addon modules for µCNC - Universal CNC firmware for microcontrollers

# About joystick_v2 for µCNC

This module adds support for using an analog joystick for jogging input. Supports a
maximum of 6 analog inputs. This module can also be configured for virtual input (i.e. input from another module).
Based on joystick module from Patryk Mierzyński

### Joystick virtual inputs

This feature allows other modules to act as "virtual analog inputs" for the joystick module. To use it you will
have to define a custom `float joystick_virtual_input(uint8_t index)` function where `index` is the input index. This function should return
a `float` value in **<-1.0; 1.0>** range.

It's also possible to create a custom `float joystick_transform_axis(float value)` function to customize your own response curve. This will only take effect if `JOYSTICK_LINEAR_RESPONSE` is set to true

## Adding joystick_v2 to µCNC

To use the joystick_v2 module follow these steps:

1. Copy the `joystick_v2` directory and place it inside the `src/modules/` directory of µCNC
2. Add initialization code for modules inside the `load_modules()` function:
```c
static FORCEINLINE void load_modules(void)
{
	LOAD_MODULE(joystick_v2);
}
```
3. Configure the module, here are all of the available parameters:
```c
// define the number of joystick axis<->mapped machine axis (any value between 0 and 5). 0 will disable the joystick module
#define JOYSTICK_AXIS_COUNT 1
// Maps joystick input 0 - 5 to axis X - C
#define JOYSTICK_INPUT_0_AXIS AXIS_X
#define JOYSTICK_INPUT_1_AXIS AXIS_Y
#define JOYSTICK_INPUT_2_AXIS AXIS_Z
#define JOYSTICK_INPUT_3_AXIS AXIS_A
#define JOYSTICK_INPUT_4_AXIS AXIS_B
#define JOYSTICK_INPUT_5_AXIS AXIS_C

// Specifies the analog pin used for joystick input 0 - 5
#define JOYSTICK_INPUT_0 UNDEF_PIN
#define JOYSTICK_INPUT_1 UNDEF_PIN
#define JOYSTICK_INPUT_2 UNDEF_PIN
#define JOYSTICK_INPUT_3 UNDEF_PIN
#define JOYSTICK_INPUT_4 UNDEF_PIN
#define JOYSTICK_INPUT_5 UNDEF_PIN

// Disables joystick input transformation (dead zone is still applied)
// By default, after applying dead zone, joystick value is raised to
// the power of 2, this should make it more precise at lower feed rates.
#define JOYSTICK_LINEAR_RESPONSE false

// Every joystick position below this value gets clipped to 0.
#define JOYSTICK_DEAD_ZONE 0.05f

// Enable the ability to jog multiple axis at once.
// Jog feed rate gets calculated from the biggest input value,
// direction is calculated by normalizing the vector created
// from all non-zero inputs.
// When disabled, if multiple inputs are non-zero jogging is stopped.
#define JOYSTICK_ALLOW_MULTI_AXIS_JOG 0

// Set maximum feed rate joystick movements can use. In units per minute.
#define JOYSTICK_MAX_FEED_RATE 100

// Joystick delta time used for distance calculation. See https://github.com/gnea/grbl/blob/master/doc/markdown/jogging.md#how-to-compute-incremental-distances
// Also defines how often jog commands are issued.
#define JOYSTICK_DELTA_TIME_MS 50

// define the joystick sensitivy
// this allows to set the sensitivity (value between 1 and 100). This is equivalent to set the size of the step increments in the joystick input.
// the joystick input resolution will be equal to (1/JOYSTICK_SENSITIVITY)
#define JOYSTICK_SENSITIVITY 10
```
4. Make sure to define `ENABLE_MAIN_LOOP_MODULES` inside `cnc_config.h`


