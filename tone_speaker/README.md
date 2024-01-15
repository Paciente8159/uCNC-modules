# uCNC-modules

Addon modules for µCNC - Universal CNC firmware for microcontrollers

## About tone speaker for µCNC

Tone speaker gives your machine the ability to play simple square wave tones through a speaker or a buzzer.
It requires a single PWM pin and a dedicated MCU timer to function.

## Adding tone speaker to µCNC

To use the tone speaker follow these steps:

1. Copy the `tone_speaker` directory and place it inside the `src/modules/` directory of µCNC
2. Assign `SPEAKER_PWM` to a PWM pin, the speaker needs to be assigned to a dedicated timer or else it will mess with other PWM peripherals
```c
// Assign a pin and a timer to PWM1
#define PWM1_BIT 8
#define PWM1_PORT A
#define PWM1_CHANNEL 1
#define PWM1_TIMER 1
// Assign SPEAKER_PWM to PWM1
#define SPEAKER_PWM PWM1
```

3. Then you need load the module inside µCNC. Open `src/module.c` and at the bottom of the file add the following lines inside the function `load_modules()`

```c
LOAD_MODULE(tone_speaker);
```

4. Enable the desired features, this module provides two features which you can choose to enable.

```c
// Enables the tone speaker to be accessible using 'M300 Px Sx' command.
// This feature needs the 'ENABLE_PARSER_MODULES' feature.
#define ENABLE_TONE_SPEAKER

// Enables the boot jingle, a short sequences of notes
// played on boot by your machine.
#define ENABLE_BOOT_JINGLE
// You can optionally specify a custom boot jingle.
// First number specifies the note duration, second specifies the frequency.
#define BOOT_JINGLE_TUNE { 250, 440, 250, 660, 250, 880 }
```

5. Last step is to enable `ENABLE_MAIN_LOOP_MODULES` feature in `cnc_config.h`, without it this module will not function correctly.

