/*
	Name: grblhal_keypad.c
	Description: Implements GrblHAL keypad for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 18-01-2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../cnc.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "../system_menu.h"

#define KEYPAD_PORT_HW_I2C 1
#define KEYPAD_PORT_SW_I2C 2
#define KEYPAD_PORT_HW_UART 3
#define KEYPAD_PORT_SW_UART 4
#define KEYPAD_PORT_HW_UART2 5

#ifndef KEYPAD_PORT
// use 1 for HW I2C and 2 for HW UART
// use 3 for SW I2C and 4 for SW UART
#define KEYPAD_PORT KEYPAD_PORT_HW_I2C
#endif

// enable this to get MPG mode
// #define KEYPAD_MPG_MODE_ENABLED
#ifdef KEYPAD_MPG_MODE_ENABLED
volatile bool keypad_has_control;
#endif

// #ifndef KEYPAD_BUFFER_SIZE
// #define KEYPAD_BUFFER_SIZE 16
// #endif

// I2C
#if (KEYPAD_PORT == KEYPAD_PORT_HW_I2C) || (KEYPAD_PORT == KEYPAD_PORT_SW_I2C)
#include "../softi2c.h"
// interruptable pins

// halt pin is optional
// #ifndef KEYPAD_HALT
// #define KEYPAD_HALT DIN6
// #endif
#ifndef KEYPAD_DOWN
#define KEYPAD_DOWN DIN7
#define KEYPAD_DOWN_MASK DIN7_MASK
#endif

// use emulated I2C
#if (KEYPAD_PORT == KEYPAD_PORT_SW_I2C)
#ifndef KEYPAD_SCL
#define KEYPAD_SCL DIN16
#endif
#ifndef KEYPAD_SDA
#define KEYPAD_SDA DIN17
#endif
#if !ASSERT_PIN(KEYPAD_SCL) || !ASSERT_PIN(KEYPAD_SDA)
#error "The pins are not defined for this board"
#else
SOFTI2C(keypad_i2c, 400000, KEYPAD_SCL, KEYPAD_SDA);
#endif
// #elif !defined(MCU_HAS_I2C)
// #error "The board does not support HW I2C"
// #endif
#endif
#endif

// UART
#if (KEYPAD_PORT == KEYPAD_PORT_SW_UART)
#include "../softuart.h"
// interruptable pins
#ifndef KEYPAD_RX
#define KEYPAD_RX DIN7
#define KEYPAD_RX_MASK DIN7_MASK
#endif
#ifndef KEYPAD_TX
#define KEYPAD_TX DOUT20
#endif
#if !ASSERT_PIN(KEYPAD_RX) || !ASSERT_PIN(KEYPAD_TX)
#error "The pins are not defined for this board"
#else
SOFTUART(keypad_uart, 115200, KEYPAD_TX, KEYPAD_RX);
#endif
#endif

#if (KEYPAD_PORT == KEYPAD_PORT_SW_I2C)
#define keypad_getc(ptr) softi2c_receive(&keypad_i2c, 0x49, ptr, 1, 1)
#elif (KEYPAD_PORT == KEYPAD_PORT_SW_UART)
#define keypad_getc(ptr)                                  \
	{                                                     \
		*(ptr) = (uint8_t)softuart_getc(&keypad_uart, 1); \
	}
#elif (KEYPAD_PORT == KEYPAD_PORT_HW_I2C)
#ifndef MCU_HAS_I2C
#error "This board does not have hardware I2C or is not defined."
#endif
#define keypad_getc(ptr) mcu_i2c_receive(0x49, ptr, 1, 1)
#elif (KEYPAD_PORT == KEYPAD_PORT_HW_UART)
#ifndef MCU_HAS_UART
#error "This board does not have hardware UART or is not defined."
#endif
#ifndef DETACH_UART_FROM_MAIN_PROTOCOL
#error "To use UART for keypad you need to detach it from the main protocol."
#endif
#define keypad_getc(ptr)                   \
	{                                      \
		*(ptr) = (uint8_t)mcu_uart_getc(); \
	}
#elif (KEYPAD_PORT == KEYPAD_PORT_HW_UART2)
#ifndef MCU_HAS_UART2
#error "This board does not have hardware UART2 or is not defined."
#endif
#ifndef DETACH_UART2_FROM_MAIN_PROTOCOL
#error "To use UART2 for keypad you need to detach it from the main protocol."
#endif
#define keypad_getc(ptr)                    \
	{                                       \
		*(ptr) = (uint8_t)mcu_uart2_getc(); \
	}
#endif

#if (UCNC_MODULE_VERSION < 10801 || UCNC_MODULE_VERSION > 99999)
#error "This module is not compatible with the current version of µCNC"
#endif

static volatile uint8_t keypad_value;

/**
 *
 * GrblHAL setting
 *	$50=<float> : default plugin dependent.
 *	Jogging step speed in mm/min. Not used by core, indended use by driver and/or sender. Senders may query this for keyboard jogging modified by CTRL key.
 *
 *	$51=<float> : default plugin dependent.
 *	Jogging slow speed in mm/min. Not used by core, indended use by driver and/or sender. Senders may query this for keyboard jogging.
 *
 *	$52=<float> : default plugin dependent.
 *	Jogging fast speed in mm/min. Not used by core, indended use by driver and/or sender. Senders may query this for keyboard jogging modified by SHIFT key.
 *
 *	$53=<float> : default plugin dependent.
 *	Jogging step distance in mm. Not used by core, indended use by driver and/or sender. Senders may query this for keyboard jogging modified by CTRL key.
 *
 *	$54=<float> : default plugin dependent.
 *	Jogging slow distance in mm. Not used by core, indended use by driver and/or sender. Senders may query this for keyboard jogging.
 *
 *	$55=<float> : default plugin dependent.
 *	Jogging fast distance in mm. Not used by core, indended use by driver and/or sender. Senders may query this for keyboard jogging modified by SHIFT key.
 *
 *
 * **/

static float keypad_settings[6];
#ifdef ENABLE_SETTINGS_MODULES
// using settings helper macros
#define KEYPAD_SETTING_ID 1050
// declarate the setting with
//  args are:
//  ID - setting ID
//  var - pointer to var
//  type - type of var
//  count - var array length (iff single var set to 1)
//  print_cb - protocol setting print callback

DECL_EXTENDED_SETTING(KEYPAD_SETTING_ID, &keypad_settings, float, 6, protocol_send_gcode_setting_line_flt);

// then just call the initializator from any point in the code
// the initialization will run only once
// ID - setting ID
// var - var as argument (not the pointer)
#endif

#ifdef ENABLE_MAIN_LOOP_MODULES
void __attribute__((weak)) keypad_extended_code(uint8_t &c){};

bool keypad_process(void *args)
{
	static uint8_t jogmode = 1;

	// just do whatever you need here
	uint8_t c = 0;
	__ATOMIC__ { c = keypad_value; }

#ifdef KEYPAD_MPG_MODE_ENABLED
	if (c == 0x8B)
	{
		if (keypad_has_control)
		{
			keypad_has_control = false;
			// free the stream
			serial_stream_change(NULL);
		}
		else
		{
			// tries to grab the stream to itself
			keypad_has_control = serial_stream_readonly(NULL, NULL, NULL);
		}
		return EVENT_CONTINUE;
	}
	bool has_control = keypad_has_control;
#else
	bool has_control = serial_stream_readonly(NULL, NULL, NULL);
#endif

	if (!has_control)
	{
		return EVENT_CONTINUE;
	}

	uint8_t mode = jogmode;
	uint8_t rt = 0;
	uint8_t axis_mask = 0;
	uint8_t axis_dir = 0;
	float feed = 0;

	switch (c)
	{
	case '0':
	case '1':
	case '2':
		jogmode = c - '0';
		break;
	case 'h':
		if (++jogmode == 3)
		{
			jogmode = 0;
		}
		break;
	case 'H':
		cnc_home();
		break;
	case 'L':
		axis_dir = (1 << AXIS_X);
	case 'R':
		axis_mask = (1 << AXIS_X);
		feed = keypad_settings[mode];
		break;
	case 'B':
		axis_dir = (1 << AXIS_Y);
	case 'F':
		axis_mask = (1 << AXIS_Y);
		feed = keypad_settings[mode];
		break;
	case 'D':
		axis_dir = (1 << AXIS_Z);
	case 'U':
		axis_mask = (1 << AXIS_Z);
		feed = keypad_settings[mode];
		break;
	case 'r':
		axis_mask = (1 << AXIS_X) | (1 << AXIS_Y);
		feed = keypad_settings[mode];
		break;
	case 'q':
		axis_dir = (1 << AXIS_Y);
		axis_mask = (1 << AXIS_X) | (1 << AXIS_Y);
		feed = keypad_settings[mode];
		break;
	case 's':
		axis_dir = (1 << AXIS_X);
		axis_mask = (1 << AXIS_X) | (1 << AXIS_Y);
		feed = keypad_settings[mode];
		break;
	case 't':
		axis_dir = (1 << AXIS_X) | (1 << AXIS_Y);
		axis_mask = (1 << AXIS_X) | (1 << AXIS_Y);
		feed = keypad_settings[mode];
		break;
	case 'w':
		axis_mask = (1 << AXIS_X) | (1 << AXIS_Z);
		feed = keypad_settings[mode];
		break;
	case 'v':
		axis_dir = (1 << AXIS_Z);
		axis_mask = (1 << AXIS_X) | (1 << AXIS_Z);
		feed = keypad_settings[mode];
		break;
	case 'u':
		axis_dir = (1 << AXIS_X);
		axis_mask = (1 << AXIS_X) | (1 << AXIS_Z);
		feed = keypad_settings[mode];
		break;
	case 'x':
		axis_dir = (1 << AXIS_X) | (1 << AXIS_Z);
		axis_mask = (1 << AXIS_X) | (1 << AXIS_Z);
		feed = keypad_settings[mode];
		break;
	case 'I':
		rt = CMD_CODE_FEED_100;
		break;
	case 'i':
		rt = CMD_CODE_FEED_INC_COARSE;
		break;
	case 'j':
		rt = CMD_CODE_FEED_DEC_COARSE;
		break;
	case 'K':
		rt = CMD_CODE_SPINDLE_100;
		break;
	case 'k':
		rt = CMD_CODE_SPINDLE_INC_COARSE;
		break;
	case 'z':
		rt = CMD_CODE_SPINDLE_DEC_COARSE;
		break;
	case 'C':
		rt = CMD_CODE_COOL_FLD_TOGGLE;
		break;
	case 'M':
		rt = CMD_CODE_COOL_MST_TOGGLE;
		break;
	default:
		// extended codes hook
		keypad_extended_code(&c);
		rt = c;
		break;
	}

	if (rt)
	{
		cnc_call_rt_command(rt);
	}
	else if (feed != 0)
	{
		float target[AXIS_COUNT];

		if (!mode)
		{
			int32_t steps[STEPPER_COUNT];
			for (uint8_t i = STEPPER_COUNT; i != 0;)
			{
				i--;
				steps[i] = keypad_settings[0];
			}

			kinematics_apply_forward(steps, target);
		}
		else
		{
			for (uint8_t i = 0; i < AXIS_COUNT; i++)
			{
				target[i] = keypad_settings[mode];
			}
		}

		for (uint8_t i = 0; i < AXIS_COUNT; i++)
		{
			if (!((1 << i) & axis_mask))
			{
				target[i] = 0;
			}
			if (((1 << i) & axis_dir))
			{
				target[i] = -target[i];
			}
		}

		motion_data_t block = {0};
		block.feed = feed;
		mc_incremental_jog(target, &block);
	}

#ifndef KEYPAD_MPG_MODE_ENABLED
	// free the stream
	if (has_control)
	{
		serial_stream_change(NULL);
	}
#endif

	// you must return EVENT_CONTINUE to enable other tasks to run or return EVENT_HANDLED to terminate the event handling within this callback
	return EVENT_CONTINUE;
}

CREATE_EVENT_LISTENER(cnc_dotasks, keypad_process);

#endif

#if defined(ENABLE_IO_MODULES)
#if ((KEYPAD_PORT == KEYPAD_PORT_HW_I2C) || (KEYPAD_PORT == KEYPAD_PORT_SW_I2C))
MCU_CALLBACK bool keypad_pressed(void *args)
{

	uint8_t *keys = (uint8_t *)args;
	// keys = {inputs, diff};
	if (keys[1] & KEYPAD_DOWN_MASK)
	{
		if (keys[0] & KEYPAD_DOWN_MASK)
		{
			// button released
			cnc_call_rt_command(CMD_CODE_JOG_CANCEL);
			keypad_value = 0;
		}
		else
		{
			// get the char and passes the char to the realtime char handler
			uint8_t c = 0;
			keypad_getc(&c);
			keypad_value = c;
		}
	}
	return EVENT_CONTINUE;
}

CREATE_EVENT_LISTENER(input_change, keypad_pressed);
#elif (KEYPAD_PORT == KEYPAD_PORT_SW_UART)
MCU_CALLBACK bool keypad_rx_ready(void *args)
{

	uint8_t *keys = (uint8_t *)args;
	// keys = {inputs, diff};
	if (keys[1] & KEYPAD_RX_MASK)
	{
		// start bit
		if ((keys[0] & KEYPAD_RX_MASK) == 0)
		{
			// get the char and passes the char to the realtime char handler
			uint8_t c = 0;
			keypad_getc(&c);
			keypad_value = c;
		}
	}
	return EVENT_CONTINUE;
}

CREATE_EVENT_LISTENER(input_change, keypad_rx_ready);
#endif
#endif

#if (KEYPAD_PORT == KEYPAD_PORT_HW_UART)
MCU_RX_CALLBACK void mcu_uart_rx_cb(uint8_t c)
{
	keypad_value = c;
}
#endif

#if (KEYPAD_PORT == KEYPAD_PORT_HW_UART2)
MCU_RX_CALLBACK void mcu_uart2_rx_cb(uint8_t c)
{
	keypad_value = c;
}
#endif

DECL_MODULE(grblhal_keypad)
{
#ifdef ENABLE_SETTINGS_MODULES
	EXTENDED_SETTING_INIT(KEYPAD_SETTING_ID, keypad_settings);
#endif
	// set defaults if zero
	if (!keypad_settings[0])
	{
		keypad_settings[0] = 500;
	}
	if (!keypad_settings[1])
	{
		keypad_settings[1] = 100;
	}
	if (!keypad_settings[2])
	{
		keypad_settings[2] = 500;
	}
	if (!keypad_settings[3])
	{
		keypad_settings[3] = 10;
	}
	if (!keypad_settings[4])
	{
		keypad_settings[4] = 1;
	}
	if (!keypad_settings[5])
	{
		keypad_settings[5] = 10;
	}

#ifdef ENABLE_MAIN_LOOP_MODULES
	ADD_EVENT_LISTENER(cnc_dotasks, keypad_process);
#else
	// just a warning in case you disabled the MAIN_LOOP option on build
#warning "Main loop extensions are not enabled. Your module will not work."
#endif

#if defined(ENABLE_IO_MODULES)
#if ((KEYPAD_PORT == KEYPAD_PORT_HW_I2C) || (KEYPAD_PORT == KEYPAD_PORT_SW_I2C))
	ADD_EVENT_LISTENER(input_change, keypad_pressed);
#elif (KEYPAD_PORT == KEYPAD_PORT_SW_UART)
	ADD_EVENT_LISTENER(input_change, keypad_rx_ready);
#endif
#elif ((KEYPAD_PORT == KEYPAD_PORT_HW_I2C) || (KEYPAD_PORT == KEYPAD_PORT_SW_I2C) || (KEYPAD_PORT == KEYPAD_PORT_SW_UART))
#warning "IO extensions are not enabled. Your module will not work."
#endif
}
