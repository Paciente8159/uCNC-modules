/*
    Name: lcddriver.h
    Description: A software library for LCD drivers based on HD44780 for µCNC.

    Copyright: Copyright (c) João Martins
    Author: João Martins
    Date: 15-05-2022

    µCNC is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version. Please see <http://www.gnu.org/licenses/>

    µCNC is distributed WITHOUT ANY WARRANTY;
    Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the	GNU General Public License for more details.
*/

#ifndef LCDDRIVER_H
#define LCDDRIVER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>

    // first arg is RS (register select) and the second the value to write
    typedef void (*lcd_write_nibble)(uint8_t, uint8_t);
    typedef void (*lcd_delay_ms)(uint8_t);

    typedef struct
    {
        uint8_t backlight;
        uint8_t row;
        uint8_t max_rows;
        uint8_t max_columns;
        // write nibble (4bit-mode) function
        lcd_write_nibble rw;
        // delay function needed by LCD waits
        lcd_delay_ms delay_ms;
    } lcd_driver_t;

#define DECL_LCD(NAME, ROWS, COLUMNS, RW_CB, DELAY_CB) lcd_driver_t NAME = {.backlight = 0, .row = 0, .max_rows = ROWS, .max_columns = COLUMNS, .rw = RW_CB, .delay_ms = DELAY_CB}

    void lcd_init(lcd_driver_t *lcd);
    void lcd_clear(lcd_driver_t *lcd);
    void lcd_gotoxy(lcd_driver_t *lcd, uint8_t x, uint8_t y);
    void lcd_putc(lcd_driver_t *lcd, unsigned char c);
    void lcd_backlight(lcd_driver_t *lcd, bool on);

#ifdef __cplusplus
}
#endif

#endif