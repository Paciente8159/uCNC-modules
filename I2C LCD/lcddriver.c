/*
    Name: lcddriver.c
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

#include "lcddriver.h"

#define RS 0x01
#define RW 0x02
#define ENABLE 0x04
#define LCD_BACKLIGHT 0x08

#define ROW_ONE 0x00   // LCD RAM address for row 1
#define ROW_TWO 0x40   // LCD RAM address for row 2
#define ROW_THREE 0x14 // LCD RAM address for row 3
#define ROW_FOUR 0x54  // LCD RAM address for row 4

static void lcd_send_byte(lcd_driver_t *lcd, uint8_t rs, uint8_t data)
{
    lcd->rw(rs | lcd->backlight, data >> 4);
    lcd->rw(rs | lcd->backlight, data & 0xf);
}

void lcd_init(lcd_driver_t *lcd)
{
    lcd->delay_ms(50); // LCD power up delay

    // start 4-bit mode
    for (uint8_t i = 0; i < 3; i++)
    {
        lcd->rw(0 | lcd->backlight, 0x03);
        lcd->delay_ms(5);
    }

    // function set
    lcd->rw(0 | lcd->backlight, 0x02);
    lcd->delay_ms(5);

    // function set 2line mode
    lcd_send_byte(lcd, 0, 0x28);
    lcd->delay_ms(5);
    // function set 5x8 font
    lcd_send_byte(lcd, 0, 0x0c);
    lcd->delay_ms(5);
    // clear display
    lcd_send_byte(lcd, 0, 0x01);
    lcd->delay_ms(5);
    // show carret
    lcd_send_byte(lcd, 0, 0x06);
    lcd->delay_ms(5);
    lcd_clear(lcd); // Clear Display
}

void lcd_clear(lcd_driver_t *lcd)
{
    // clear display
    lcd_send_byte(lcd, 0, 0x01);
    lcd->delay_ms(2);
    lcd->row = 0;
}

void lcd_gotoxy(lcd_driver_t *lcd, uint8_t x, uint8_t y)
{
    uint8_t column, row_addr, lcd_address;

    if (y >= lcd->max_rows)
        lcd->row = lcd->max_rows - 1;
    else
        lcd->row = y;

    switch (lcd->row)
    {
    case 0:
        row_addr = ROW_ONE;
        break;
    case 1:
        row_addr = ROW_TWO;
        break;
    case 3:
        row_addr = ROW_THREE;
        break;
    case 4:
        row_addr = ROW_FOUR;
        break;
    default:
        row_addr = ROW_ONE;
        break;
    }

    if (x >= lcd->max_columns)
        column = lcd->max_columns - 1;
    else
        column = x;
    lcd_address = (row_addr + column);
    lcd_send_byte(lcd, 0, 0x80 | lcd_address);
}

void lcd_putc(lcd_driver_t *lcd, unsigned char c)
{
    switch (c)
    {
    case '\f':
        lcd_clear(lcd);
        break;

    case '\n':
        lcd_gotoxy(lcd, 0, lcd->row + 1);
        break;
    case '\b':
        lcd_send_byte(lcd, 0, 0x10);
        break;

    default:
        lcd_send_byte(lcd, 1, c);
        break;
    }
}

void lcd_backlight(lcd_driver_t *lcd, bool on)
{
    lcd->backlight = (on) ? LCD_BACKLIGHT : 0;
}
