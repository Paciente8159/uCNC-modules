# uCNC-modules

Addon modules for µCNC - Universal CNC firmware for microcontrollers

## About M42 for µCNC

This module adds custom M42 code to the µCNC parser. This similar to Marlin M42 and allows to turn on and off any generic digital pin, PWM or servo pin of the µCNC.

## Adding M42 to µCNC

To use the M42 parser module follow these steps:

1. Copy the the `M42` directory and place it inside the `src/modules/` directory of µCNC
2. Then you need load the module inside µCNC. Open `src/module.c` and at the bottom of the file add the following lines inside the function `load_modules()`

```
LOAD_MODULE(m42);
```

3. The last step is to enable `ENABLE_PARSER_MODULES` inside `cnc_config.h`

## Using M42 to µCNC

The M42 can be used like this.

```
M42 P<µCNC pin number> S<value from 0-255>
```

This is the list of output pins that can be used with this command at the date of release of this module.
Trying to command any other pin will have no effect.
To view the current updated IO list please [go to the wiki](https://github.com/Paciente8159/uCNC/wiki/Understanding-the-HAL#the-microcontroller-hal)

| Pin number | Alias | Pin name |
| ---------- | ----- | -------- |
| 24         | DIO24 | PWM0     |
| 25         | DIO25 | PWM1     |
| 26         | DIO26 | PWM2     |
| 27         | DIO27 | PWM3     |
| 28         | DIO28 | PWM4     |
| 29         | DIO29 | PWM5     |
| 30         | DIO30 | PWM6     |
| 31         | DIO31 | PWM7     |
| 32         | DIO32 | PWM8     |
| 33         | DIO33 | PWM9     |
| 34         | DIO34 | PWM10    |
| 35         | DIO35 | PWM11    |
| 36         | DIO36 | PWM12    |
| 37         | DIO37 | PWM13    |
| 38         | DIO38 | PWM14    |
| 39         | DIO39 | PWM15    |
| 40         | DIO40 | SERVO0   |
| 41         | DIO41 | SERVO1   |
| 42         | DIO42 | SERVO2   |
| 43         | DIO43 | SERVO3   |
| 44         | DIO44 | SERVO4   |
| 45         | DIO45 | SERVO5   |
| 46         | DIO46 | DOUT0    |
| 47         | DIO47 | DOUT1    |
| 48         | DIO48 | DOUT2    |
| 49         | DIO49 | DOUT3    |
| 50         | DIO50 | DOUT4    |
| 51         | DIO51 | DOUT5    |
| 52         | DIO52 | DOUT6    |
| 53         | DIO53 | DOUT7    |
| 54         | DIO54 | DOUT8    |
| 55         | DIO55 | DOUT9    |
| 56         | DIO56 | DOUT10   |
| 57         | DIO57 | DOUT11   |
| 58         | DIO58 | DOUT12   |
| 59         | DIO59 | DOUT13   |
| 60         | DIO60 | DOUT14   |
| 61         | DIO61 | DOUT15   |
| 62         | DIO62 | DOUT16   |
| 63         | DIO63 | DOUT17   |
| 64         | DIO64 | DOUT18   |
| 65         | DIO65 | DOUT19   |
| 66         | DIO66 | DOUT20   |
| 67         | DIO67 | DOUT21   |
| 68         | DIO68 | DOUT22   |
| 69         | DIO69 | DOUT23   |
| 70         | DIO70 | DOUT24   |
| 71         | DIO71 | DOUT25   |
| 72         | DIO72 | DOUT26   |
| 73         | DIO73 | DOUT27   |
| 74         | DIO74 | DOUT28   |
| 75         | DIO75 | DOUT29   |
| 76         | DIO76 | DOUT30   |
| 77         | DIO77 | DOUT31   |
