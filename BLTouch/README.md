# uCNC-modules
Addon modules for µCNC - Universal CNC firmware for microcontrollers

## About BLTouch to µCNC

This module adds BLTouch to the µCNC parser.

## Adding BLTouch to µCNC

To use the and M42 follow these steps:
1. Add the `bltouch.c` file to the root directory of µCNC (where the `uCNC.ino` is placed)
2. Open `bltouch.c` and define the SERVO pin used by BLTouch. The default is SERVO0 if available on your probe.

```
#ifndef BLTOUCH_PROBE_SERVO
#define BLTOUCH_PROBE_SERVO SERVO0
#endif
```



3. Then you need plug the module by attaching it to the listeners inside µCNC. Open `uCNC.ino` and add the following lines after `cnc_init()`

```
#ifdef ENABLE_IO_MODULES
	ADD_LISTENER(probe_enable_delegate, bltouch_deploy, probe_enable_event);
	ADD_LISTENER(probe_disable_delegate, bltouch_stow, probe_disable_event);
#endif
```

4. The last step is to enable `ENABLE_IO_MODULES` inside `cnc_config.h`
