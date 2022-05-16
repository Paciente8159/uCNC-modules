<p align="center">
<img src="https://github.com/Paciente8159/uCNC/blob/master/docs/logo.png?raw=true">
</p>


# uCNC-modules
Addon modules for µCNC - Universal CNC firmware for microcontrollers

## About µCNC

Heavily inspired by [Grbl](https://github.com/gnea/grbl) and [LinuxCNC](http://linuxcnc.org/), µCNC started to take shape in the second half of 2019, in an attempt to come out with a G-Code interpreter/controller software/firmware that is both compact and powerful as Grbl and flexible and modular as LinuxCNC, with the following goals in mind:

1. µCNC is written in C (GNU99 compliant). This gives the advantage of being supported for a large number of CPU/MCU and compilers available.
2. Modular library based:
   - Independent hardware. All MCU/hardware operations are written in a single translation unit that acts like a standardized HAL interface, leaving the CNC controlling code independent of the MCU architecture has long has it has the necessary abilities to execute code and respond to interrupts in a predictable away. Porting µCNC for a different MCU should be fairly straight forward.
   - Independent kinematics. Another dimension of the HAL is the possibility of defining how the translation between machine coordinates and the motions is translated back and forth. This should theoretically allow µCNC to be easily adaptable to several types of machines like cartesian, coreXY, deltas and others. µCNC supports up to 6 axis.
   - As of version 1.2.0 the addition of a HAL config that allow the user to build link inputs and outputs of the controller to specific functions or modules (like using a generic input has an encoder input or a PWM output has a servo controller with a PID module)
   - As of version 1.3.0 a new dimension to the HAL was added. The tool HAL. This allow to add multiple tools that can perform different task with µCNC.
   - As of version 1.4.0 a special kind of PIN type dedicated to servo motors control was added µCNC.
   - As of version 1.4.0 a new module extension system was introduced. It's now possible to add hooks in the core code and attach multiple listeners to execute additional code further expanding µCNC original capabilities. Modules can be enabled and disabled in the config file to enable feature on per need basis.
3. Compatible with already existing tools and software for Grbl. There is no point in trying to reinvent the wheel (the whole wheel at least :-P). For that reason µCNC uses protocol compatible with Grbl. This allows it to easily integrate with Grbl ecosystem.

You can navigate the [project wiki](https://github.com/Paciente8159/uCNC/wiki) to find out more on how to use it.

## Using a module

Most modules can be added to µCNC simply by adding the file to the root of µCNC project path and add the necessary listeners after cnc_init() call in the uCNC.ino/main file.
Each module has it's own read file with instructions on how to include and use the module.

## Supporting the project

µCNC is a completely free software. It took me a considerable amount of hours and effort to develop and debug so any help is appreciated. Building docs, testing and debugging, whatever. Also if you really like it and want help me keep the project running, you can help me to buy more equipment. Recently I have saved some extra money and bought a laser engraver. This hardware was fundamental to develop and testing version 1.2.0 and beyond. Currently this machine is being used to work on other projects and is running µCNC smoothly. Or you may just want to simply buy me a coffee or two for those extra long nights putting out code ;-)

[![paypal](https://www.paypalobjects.com/webstatic/en_US/i/buttons/PP_logo_h_100x26.png)](https://www.paypal.me/paciente8159)
