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
| 25         | DIO25 | PWM0     |
| 26         | DIO26 | PWM1     |
| 27         | DIO27 | PWM2     |
| 28         | DIO28 | PWM3     |
| 29         | DIO29 | PWM4     |
| 30         | DIO30 | PWM5     |
| 31         | DIO31 | PWM6     |
| 32         | DIO32 | PWM7     |
| 33         | DIO33 | PWM8     |
| 34         | DIO34 | PWM9     |
| 35         | DIO35 | PWM10     |
| 36         | DIO36 | PWM11     |
| 37         | DIO37 | PWM12     |
| 38         | DIO38 | PWM13     |
| 39         | DIO39 | PWM14     |
| 40         | DIO40 | PWM15     |
| 41         | DIO41 | SERVO0     |
| 42         | DIO42 | SERVO1     |
| 43         | DIO43 | SERVO2     |
| 44         | DIO44 | SERVO3     |
| 45         | DIO45 | SERVO4     |
| 46         | DIO46 | SERVO5     |
| 47         | DIO47 | DOUT0     |
| 48         | DIO48 | DOUT1     |
| 49         | DIO49 | DOUT2     |
| 50         | DIO50 | DOUT3     |
| 51         | DIO51 | DOUT4     |
| 52         | DIO52 | DOUT5     |
| 53         | DIO53 | DOUT6     |
| 54         | DIO54 | DOUT7     |
| 55         | DIO55 | DOUT8     |
| 56         | DIO56 | DOUT9     |
| 57         | DIO57 | DOUT10     |
| 58         | DIO58 | DOUT11     |
| 59         | DIO59 | DOUT12     |
| 60         | DIO60 | DOUT13     |
| 61         | DIO61 | DOUT14     |
| 62         | DIO62 | DOUT15     |
| 63         | DIO63 | DOUT16     |
| 64         | DIO64 | DOUT17     |
| 65         | DIO65 | DOUT18     |
| 66         | DIO66 | DOUT19     |
| 67         | DIO67 | DOUT20     |
| 68         | DIO68 | DOUT21     |
| 69         | DIO69 | DOUT22     |
| 70         | DIO70 | DOUT23     |
| 71         | DIO71 | DOUT24     |
| 72         | DIO72 | DOUT25     |
| 73         | DIO73 | DOUT26     |
| 74         | DIO74 | DOUT27     |
| 75         | DIO75 | DOUT28     |
| 76         | DIO76 | DOUT29     |
| 77         | DIO77 | DOUT30     |
| 78         | DIO78 | DOUT31     |
| 79         | DIO79 | DOUT32     |
| 80         | DIO80 | DOUT33     |
| 81         | DIO81 | DOUT34     |
| 82         | DIO82 | DOUT35     |
| 83         | DIO83 | DOUT36     |
| 84         | DIO84 | DOUT37     |
| 85         | DIO85 | DOUT38     |
| 86         | DIO86 | DOUT39     |
| 87         | DIO87 | DOUT40     |
| 88         | DIO88 | DOUT41     |
| 89         | DIO89 | DOUT42     |
| 90         | DIO90 | DOUT43     |
| 91         | DIO91 | DOUT44     |
| 92         | DIO92 | DOUT45     |
| 93         | DIO93 | DOUT46     |
| 94         | DIO94 | DOUT47     |
| 95         | DIO95 | DOUT48     |
| 96         | DIO96 | DOUT49     |
