# uCNC-modules
Addon modules for µCNC - Universal CNC firmware for microcontrollers

## Adding I2C LCD to µCNC

To use the and I2C LCD follow these steps:
1. Add the `i2c_lcd.c` file to the root directory of µCNC (where the `uCNC.ino` is placed)
2. Open `i2c_lcd.c` and define the number of rows and column of your LCD display. The default is 16x2

```
#ifndef LCD_ROWS
#define LCD_ROWS 2
#endif
#ifndef LCD_COLUMNS
#define LCD_COLUMNS 16
#endif
```

3. Also on `i2c_lcd.c` choose 2 free generic input pins. By default pins `DIN20` and `DIN21` are used but you can configure or use other pins from your board.

```
#ifndef LCD_I2C_SCL
#define LCD_I2C_SCL DIN21
#endif
#ifndef LCD_I2C_SDA
#define LCD_I2C_SDA DIN20
#endif
```

4. Then you need plug the module by attaching it to the listeners inside µCNC. Open `uCNC.ino` and add the following lines after `cnc_init()`

```
#ifdef ENABLE_MAIN_LOOP_MODULES
	ADD_LISTENER(cnc_reset_delegate, ucnc_lcd_init, cnc_reset_event);
	ADD_LISTENER(cnc_dotasks_delegate, ucnc_lcd_refresh, cnc_dotasks_event);
#endif
```

5. The last step is to enable `ENABLE_MAIN_LOOP_MODULES` inside `cnc_config.h`