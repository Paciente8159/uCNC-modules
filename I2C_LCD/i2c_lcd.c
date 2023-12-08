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

#include "../../cnc.h"
#include "../softi2c.h"
#include "lcddriver.h"
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include "../system_menu.h"

#if (UCNC_MODULE_VERSION < 10800 || UCNC_MODULE_VERSION > 99999)
#error "This module is not compatible with the current version of µCNC"
#endif

// #define I2C_LCD_USE_HW_I2C

#ifndef LCD_ROWS
#define LCD_ROWS 2
#endif
#ifndef LCD_COLUMNS
#define LCD_COLUMNS 16
#endif
#ifndef LCD_I2C_SCL
#define LCD_I2C_SCL DIN30
#endif
#ifndef LCD_I2C_SDA
#define LCD_I2C_SDA DIN31
#endif

#define LCDWIDTH LCD_COLUMNS
#define LCDMID (LCD_COLUMNS >> 1)
#define LCDHEIGHT LCD_ROWS
#define FONTHEIGHT 1
#define ALIGN_CENTER(t) ((LCD_COLUMNS - strlen(t)) >> 1)
#define ALIGN_RIGHT(t) (LCD_COLUMNS - strlen(t))
#define ALIGN_LEFT 0
#define JUSTIFY_CENTER ((LCD_ROWS >> 1) - 1)
#define JUSTIFY_BOTTOM (LCD_ROWS - 1)
#define JUSTIFY_TOP 0
#define TEXT_WIDTH(t) strlen(t)

#if (!defined(I2C_LCD_USE_HW_I2C) || !defined(MCU_HAS_I2C))
SOFTI2C(lcdi2c, 100000, LCD_I2C_SCL, LCD_I2C_SDA);
#endif

void i2clcd_rw(uint8_t rlow, uint8_t data)
{
	uint8_t val = (((data << 4) | rlow) & ~0x04);
	uint8_t val2 = (val | 0x04);

#if (defined(I2C_LCD_USE_HW_I2C) && defined(MCU_HAS_I2C))
	softi2c_send(NULL, 0x27, &val, 1, false);
	softi2c_send(NULL, 0x27, &val2, 1, false);
	softi2c_send(NULL, 0x27, &val, 1, true);
#else
	softi2c_send(&lcdi2c, 0x27, &val, 1, false);
	softi2c_send(&lcdi2c, 0x27, &val2, 1, false);
	softi2c_send(&lcdi2c, 0x27, &val, 1, true);
#endif
}

void i2clcd_delay(uint8_t delay)
{
	cnc_delay_ms((uint32_t)delay);
}

DECL_LCD(ucnc_lcd, LCD_ROWS, LCD_COLUMNS, &i2clcd_rw, &i2clcd_delay);

#define LCD (&ucnc_lcd)

// copied from serial
void lcd_print_str(lcd_driver_t *lcd, const char *__s)
{
	while (*__s != 0)
	{
		lcd_putc(lcd, *__s++);
	}
}

void system_menu_render_startup(void)
{
	char str[LCDWIDTH] = {0xe4, 'C', 'N', 'C'};
	lcd_clear(LCD);
	lcd_gotoxy(LCD, ALIGN_CENTER(str), JUSTIFY_CENTER);
	lcd_print_str(LCD, str);
	memset(str, 0, LCDWIDTH);
	rom_strcpy(str, __romstr__("v" CNC_VERSION));
	lcd_gotoxy(LCD, ALIGN_CENTER(str), JUSTIFY_CENTER + 1);
	lcd_print_str(LCD, str);
}

static uint8_t current_pos;
static void lcd_go_home(void)
{
	lcd_gotoxy(LCD, 0, 0);
	current_pos = 0;
}

static void lcd_print_str_info(const char *s)
{
	char str[LCDMID + 1];
	memset(str, ' ', LCDMID);
	if (s)
	{
		memcpy(str, s, MIN(LCDMID, strlen(s)));
		str[MIN(LCDMID, strlen(s))] = ' ';
	}
	str[LCDMID] = 0;
	lcd_gotoxy(LCD, (current_pos & 1) ? LCDMID : 0, current_pos >> 1);
	lcd_print_str(LCD, str);
	current_pos++;
}

static void lcd_blank_fill(void)
{
	while (current_pos < (LCD_ROWS << 1))
	{
		lcd_print_str_info(NULL);
	}
}

void system_menu_render_idle(void)
{
	float axis[MAX(AXIS_COUNT, 3)];
	int32_t steppos[STEPPER_COUNT];
	uint8_t limits = io_get_limits();
	itp_get_rt_position(steppos);
	kinematics_apply_forward(steppos, axis);
	kinematics_apply_reverse_transform(axis);
	char str[32];
	lcd_go_home();

//  X/Y
#if (AXIS_COUNT >= 1)
	memset(str, 0, 32);
	str[0] = 'X';
	str[1] = ':';
	system_menu_flt_to_str(&str[2], axis[0]);
	lcd_print_str_info(str);

#if (AXIS_COUNT >= 2)
	memset(str, 0, 32);
#if ((AXIS_COUNT == 2) && defined(USE_Y_AS_Z_ALIAS))
	str[0] = 'Z';
#else
	str[0] = 'Y';
#endif
	str[1] = ':';
	system_menu_flt_to_str(&str[2], axis[1]);
	lcd_print_str_info(str);
#endif
#endif

// Z/A
#if (AXIS_COUNT >= 3)
	memset(str, 0, 32);
	str[0] = 'Z';
	str[1] = ':';
	system_menu_flt_to_str(&str[2], axis[2]);
	lcd_print_str_info(str);
#if (AXIS_COUNT >= 4)
	memset(str, 0, 32);
	str[0] = 'A';
	str[1] = ':';
	system_menu_flt_to_str(&str[2], axis[3]);
	lcd_print_str_info(str);
#endif
#endif

// B/C
#if (AXIS_COUNT >= 5)
	memset(str, 0, 32);
	str[0] = 'B';
	str[1] = ':';
	system_menu_flt_to_str(&str[2], axis[4]);
	lcd_print_str_info(str);
#if (AXIS_COUNT >= 6)
	memset(str, 0, 32);
	str[0] = 'C';
	str[1] = ':';
	system_menu_flt_to_str(&str[2], axis[5]);
	lcd_print_str_info(str);
#endif
#endif

#if ((LCD_ROWS == 2 && AXIS_COUNT <= 3) || (LCD_ROWS == 4))
	memset(str, 0, 32);
	str[0] = 'P';
	str[1] = 'n';
	str[2] = ':';
	uint8_t i = 3;
	if (CHECKFLAG(limits, LINACT0_LIMIT_MASK))
	{
		str[i++] = 'X';
	}

	if (CHECKFLAG(limits, LINACT1_LIMIT_MASK))
	{
#if ((AXIS_COUNT == 2) && defined(USE_Y_AS_Z_ALIAS))
		str[i++] = 'Z';
#else
		str[i++] = 'Y';
#endif
	}

	if (CHECKFLAG(limits, LINACT2_LIMIT_MASK))
	{
		str[i++] = 'Z';
	}

	if (CHECKFLAG(limits, LINACT3_LIMIT_MASK))
	{
		str[i++] = 'A';
	}

	if (CHECKFLAG(limits, LINACT4_LIMIT_MASK))
	{
		str[i++] = 'B';
	}

	if (CHECKFLAG(limits, LINACT5_LIMIT_MASK))
	{
		str[i++] = 'C';
	}

	lcd_print_str_info(str);
#endif

#if ((LCD_ROWS == 2 && AXIS_COUNT <= 2) || (LCD_ROWS == 4))
	memset(str, 0, 32);
	rom_strcpy(str, __romstr__("St:"));
	uint8_t state = cnc_get_exec_state(0xFF);
	uint8_t filter = 0x80;
	while (!(state & filter) && filter)
	{
		filter >>= 1;
	}

	state &= filter;
	if (cnc_has_alarm())
	{
		rom_strcpy(&str[3], MSG_STATUS_ALARM);
	}
	else if (mc_get_checkmode())
	{
		rom_strcpy(&str[3], MSG_STATUS_CHECK);
	}
	else
	{
		switch (state)
		{
		case EXEC_DOOR:
			rom_strcpy(&str[3], MSG_STATUS_DOOR);
			break;
		case EXEC_KILL:
		case EXEC_UNHOMED:
			rom_strcpy(&str[3], MSG_STATUS_ALARM);
			break;
		case EXEC_HOLD:
			rom_strcpy(&str[3], MSG_STATUS_HOLD);
			break;
		case EXEC_HOMING:
			rom_strcpy(&str[3], MSG_STATUS_HOME);
			break;
		case EXEC_JOG:
			rom_strcpy(&str[3], MSG_STATUS_JOG);
			break;
		case EXEC_RUN:
			rom_strcpy(&str[3], MSG_STATUS_RUN);
			break;
		default:
			rom_strcpy(&str[3], MSG_STATUS_IDLE);
			break;
		}
	}
	str[LCDMID] = 0;
	lcd_print_str_info(str);
#endif

	lcd_blank_fill();
}

void system_menu_render_alarm(void)
{
	char str[LCDWIDTH];
	lcd_clear(LCD);
	rom_strcpy(str, __romstr__("ALARM "));
	uint8_t alarm = cnc_get_alarm();
	system_menu_int_to_str(&str[6], alarm);
	lcd_clear(LCD);
	lcd_gotoxy(LCD, ALIGN_CENTER(str), JUSTIFY_CENTER);
	lcd_print_str(LCD, str);
}

#ifdef ENABLE_MAIN_LOOP_MODULES
bool lcd_reset(void *args)
{
	// clear
	system_menu_render_startup();
	return false;
}
CREATE_EVENT_LISTENER(cnc_reset, lcd_reset);

bool lcd_alarm(void *args)
{
	// renders the alarm
	system_menu_render();
	return EVENT_CONTINUE;
}

bool lcd_update(void *args)
{
	static bool running = false;

	if (!running)
	{
		running = true;
		// just update action with no action
		system_menu_action(0);
		// render menu
		system_menu_render();
		running = false;
	}

	return EVENT_CONTINUE;
}

CREATE_EVENT_LISTENER(cnc_dotasks, lcd_update);
CREATE_EVENT_LISTENER(cnc_alarm, lcd_update);

#endif

DECL_MODULE(i2c_lcd)
{
	lcd_init(LCD);
	lcd_backlight(LCD, true);

	// STARTS SYSTEM MENU MODULE
	system_menu_init();
#ifdef ENABLE_MAIN_LOOP_MODULES
	ADD_EVENT_LISTENER(cnc_reset, lcd_reset);
	ADD_EVENT_LISTENER(cnc_alarm, lcd_update);
	ADD_EVENT_LISTENER(cnc_dotasks, lcd_update);
#else
#warning "Main loop extensions are not enabled. Graphic display card will not work."
#endif
}
