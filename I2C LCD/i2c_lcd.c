/*
    Name: i2c_lcd.c
    Description: I2C LCD module for µCNC.
    This adds an I2C LCD based in PCF8574T/HD44780

    Copyright: Copyright (c) João Martins
    Author: João Martins
    Date: 20-03-2022

    µCNC is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version. Please see <http://www.gnu.org/licenses/>

    µCNC is distributed WITHOUT ANY WARRANTY;
    Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the	GNU General Public License for more details.
*/

#include "../cnc.h"
#include "softi2c.h"
#include "lcddriver.h"
#include <math.h>

#ifdef ENABLE_MAIN_LOOP_MODULES

#ifndef LCD_ROWS
#define LCD_ROWS 2
#endif
#ifndef LCD_COLUMNS
#define LCD_COLUMNS 16
#endif
#ifndef LCD_I2C_SCL
#define LCD_I2C_SCL DIN21
#endif
#ifndef LCD_I2C_SDA
#define LCD_I2C_SDA DIN20
#endif

SOFTI2C(lcdi2c, LCD_I2C_SCL, LCD_I2C_SDA, 100000);

void i2clcd_rw(uint8_t rlow, uint8_t data)
{
    uint8_t val = (((data << 4) | rlow) & ~0x04);
    softi2c_write_byte(&lcdi2c, 0x27, val);
    softi2c_write_byte(&lcdi2c, 0x27, (val | 0x04));
    softi2c_write_byte(&lcdi2c, 0x27, val);
}

void i2clcd_delay(uint8_t delay)
{
    cnc_delay_ms((uint32_t)delay);
}

LCD(ucnc_lcd, LCD_ROWS, LCD_COLUMNS, &i2clcd_rw, &i2clcd_delay);

// copied from serial
void lcd_print_str(lcd_driver_t *lcd, const char *__s)
{
    unsigned char c = (unsigned char)rom_strptr(__s++);
    do
    {
        lcd_putc(lcd, c);
        c = (unsigned char)rom_strptr(__s++);
    } while (c != 0);
}

void lcd_print_int(lcd_driver_t *lcd, int32_t num)
{
    if (num == 0)
    {
        lcd_putc(lcd, '0');
        return;
    }

    unsigned char buffer[11];
    uint8_t i = 0;

    if (num < 0)
    {
        lcd_putc(lcd, '-');
        num = -num;
    }

    while (num > 0)
    {
        uint8_t digit = num % 10;
        num = (uint32_t)truncf((float)num * 0.1f);
        buffer[i++] = digit;
    }

    do
    {
        i--;
        lcd_putc(lcd, '0' + buffer[i]);
    } while (i);
}

void lcd_print_flt(lcd_driver_t *lcd, float num)
{
    if (num < 0)
    {
        lcd_putc(lcd, '-');
        num = -num;
    }

    uint32_t interger = (uint32_t)floorf(num);
    num -= interger;
    uint32_t mult = (!g_settings.report_inches) ? 1000 : 10000;
    num *= mult;
    uint32_t digits = (uint32_t)roundf(num);
    if (digits == mult)
    {
        interger++;
        digits = 0;
    }

    lcd_print_int(lcd, interger);
    lcd_putc(lcd, '.');
    if (g_settings.report_inches)
    {
        if (digits < 1000)
        {
            lcd_putc(lcd, '0');
        }
    }

    if (digits < 100)
    {
        lcd_putc(lcd, '0');
    }

    if (digits < 10)
    {
        lcd_putc(lcd, '0');
    }

    lcd_print_int(lcd, digits);
}

uint32_t lcd_next_update;

void ucnc_lcd_init()
{
    // runs only once at startup
    if (lcd_next_update == 0)
    {
        lcd_init(&ucnc_lcd);
        lcd_backlight(&ucnc_lcd, true);
        lcd_putc(&ucnc_lcd, 0xe4);
        lcd_print_str(&ucnc_lcd, __romstr__("CNC v" CNC_VERSION));
        lcd_next_update = mcu_millis() + 5000;
    }
}

CREATE_LISTENER(cnc_reset_delegate, ucnc_lcd_init);

void ucnc_lcd_refresh()
{
    if (lcd_next_update < mcu_millis())
    {
        float axis[MAX(AXIS_COUNT, 3)];
        int32_t steppos[STEPPER_COUNT];
        uint8_t limits = io_get_limits();
        itp_get_rt_position(steppos);
        kinematics_apply_forward(steppos, axis);
        kinematics_apply_reverse_transform(axis);
#if LCD_ROWS == 2
        lcd_gotoxy(&ucnc_lcd, 0, 0);
        lcd_putc(&ucnc_lcd, 'X');
        lcd_putc(&ucnc_lcd, ':');
        lcd_print_flt(&ucnc_lcd, axis[0]);
        lcd_gotoxy(&ucnc_lcd, 7, 0);
        lcd_putc(&ucnc_lcd, ' ');
        lcd_putc(&ucnc_lcd, 'Y');
        lcd_putc(&ucnc_lcd, ':');
        lcd_print_flt(&ucnc_lcd, axis[1]);
        lcd_gotoxy(&ucnc_lcd, 0, 1);
        lcd_putc(&ucnc_lcd, 'Z');
        lcd_putc(&ucnc_lcd, ':');
        lcd_print_flt(&ucnc_lcd, axis[2]);
        lcd_gotoxy(&ucnc_lcd, 7, 1);
        lcd_putc(&ucnc_lcd, ' ');
        lcd_putc(&ucnc_lcd, 'P');
        lcd_putc(&ucnc_lcd, 'n');
        lcd_putc(&ucnc_lcd, ':');

        if (CHECKFLAG(limits, LIMIT_X_MASK))
        {
            lcd_putc(&ucnc_lcd, 'X');
        }
        else
        {
            lcd_putc(&ucnc_lcd, ' ');
        }

        if (CHECKFLAG(limits, LIMIT_Y_MASK))
        {
            lcd_putc(&ucnc_lcd, 'Y');
        }
        else
        {
            lcd_putc(&ucnc_lcd, ' ');
        }

        if (CHECKFLAG(limits, LIMIT_Z_MASK))
        {
            lcd_putc(&ucnc_lcd, 'Z');
        }
        else
        {
            lcd_putc(&ucnc_lcd, ' ');
        }

#endif

        lcd_next_update = mcu_millis() + 250;
    }
}

CREATE_LISTENER(cnc_dotasks_delegate, ucnc_lcd_refresh);

#endif

DECL_MODULE(i2c_lcd)
{
#ifdef ENABLE_MAIN_LOOP_MODULES
    ADD_LISTENER(cnc_reset_delegate, ucnc_lcd_init, cnc_reset_event);
    ADD_LISTENER(cnc_dotasks_delegate, ucnc_lcd_refresh, cnc_dotasks_event);
#else
#warning "Main loop extensions are not enabled. I2C LCD will not work."
#endif
}
