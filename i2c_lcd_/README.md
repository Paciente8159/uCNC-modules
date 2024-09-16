# uCNC-modules

Addon modules for µCNC - Universal CNC firmware for microcontrollers

## About I2C LCD for µCNC

I2C LCD allows to add an I2C LCD module to µCNC that display some basic info about the current machine position and limits state.
It requires any 2 µCNC generic digital input pins of the board. It uses software I2C so no dedicated I2C hardware is required.

## Adding I2C LCD to µCNC

To use the and I2C LCD follow these steps:

1. Copy the the `I2C_LCD` directory and place it inside the `src/modules/` directory of µCNC
2. If needed you may redifine some IO pin and LCD options. By default this module tries to use the hardware I2C port if available and if not the software I2C pins. Please refer to [PINOUTS.md](https://github.com/Paciente8159/uCNC/blob/master/PINOUTS.md) to check the default pin associations.
To redefine the IO pins and if software or hardware I2C can is used open `cnc_hal_config.h` and add the needed configurations.
The default number of rows and column of your LCD display is 16x2.

```
// Uncomment to change the number of lines of the LCD
// #define LCD_ROWS 2
// Uncomment to change the number of chars per line of the LCD
// #define LCD_COLUMNS 16
// Uncomment and change the default pins used in software I2C mode
// #define LCD_I2C_SCL DIN30
// #define LCD_I2C_SDA DIN31
```

3. Then you need load the module inside µCNC. Open `src/module.c` and at the bottom of the file add the following lines inside the function `load_modules()`

```
LOAD_MODULE(i2c_lcd);
```

4. The last step is to enable `ENABLE_MAIN_LOOP_MODULES` inside `cnc_config.h`
