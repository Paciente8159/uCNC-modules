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
#include "../softi2c.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "../system_menu.h"

#ifndef KEYPAD_PORT
// use 1 for HW I2C and 2 for HW UART
// use 3 for SW I2C and 4 for SW UART
#define KEYPAD_PORT 3
#endif

#ifndef KEYPAD_BUFFER_SIZE
#define KEYPAD_BUFFER_SIZE 16
#endif

// I2C
#if (KEYPAD_PORT & 1)
// interruptable pins

// halt pin is optional
// #ifndef KEYPAD_HALT
// #define KEYPAD_HALT DIN6
// #endif
#ifndef KEYPAD_DOWN
#define KEYPAD_DOWN DIN7
#define KEYPAD_DOWN_MASK DIN7_MASK
#endif
static softi2c_port_t *keypad_port;
// use emulated I2C
#if (KEYPAD_PORT & 2)
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
#elif !defined(MCU_HAS_I2C)
#error "The board does not support HW I2C"
#endif

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

#define KEYPAD_MAX_CMD_LEN 64
#if defined(DECL_SERIAL_SIMPLE_STREAM)
// this will expose keypad_stream_send_cmd to send commands to the machine
// 64 chars should do
DECL_SERIAL_SIMPLE_STREAM(keypad_stream, KEYPAD_MAX_CMD_LEN);
#elif defined(DECL_SERIAL_STREAM)
// this is for backward compatibility with pre version 010807
static uint8_t keypad_stream_stream_buffer_bufferdata[KEYPAD_MAX_CMD_LEN];
static const uint8_t keypad_stream_stream_buffer_size = KEYPAD_MAX_CMD_LEN;
static ring_buffer_t keypad_stream_stream_buffer;
static uint8_t keypad_stream_getc(void)
{
	uint8_t c = 0;
	{
		if (!(!keypad_stream_stream_buffer.count))
		{
			uint8_t tail;
			for (uint8_t __restore_atomic__ __attribute__((__cleanup__(__atomic_out))) = ((*(volatile uint8_t *)((0x3F) + 0x20)) & 0x80), __AtomLock = __atomic_in(); __AtomLock; __AtomLock = 0)
			{
				tail = keypad_stream_stream_buffer.tail;
			}
			memcpy(&c, &keypad_stream_stream_buffer_bufferdata[tail], sizeof(keypad_stream_stream_buffer_bufferdata[0]));
			tail++;
			if (tail >= keypad_stream_stream_buffer_size)
			{
				tail = 0;
			}
			for (uint8_t __restore_atomic__ __attribute__((__cleanup__(__atomic_out))) = ((*(volatile uint8_t *)((0x3F) + 0x20)) & 0x80), __AtomLock = __atomic_in(); __AtomLock; __AtomLock = 0)
			{
				keypad_stream_stream_buffer.tail = tail;
				keypad_stream_stream_buffer.count--;
			}
		}
	};
	return c;
}
uint8_t keypad_stream_available(void) { return (keypad_stream_stream_buffer.count); }
void keypad_stream_clear(void)
{
	{
		for (uint8_t __restore_atomic__ __attribute__((__cleanup__(__atomic_out))) = ((*(volatile uint8_t *)((0x3F) + 0x20)) & 0x80), __AtomLock = __atomic_in(); __AtomLock; __AtomLock = 0)
		{
			keypad_stream_stream_buffer_bufferdata[0] = 0;
			keypad_stream_stream_buffer.tail = 0;
			keypad_stream_stream_buffer.head = 0;
			keypad_stream_stream_buffer.count = 0;
		}
	};
}
serial_stream_t keypad_stream_stream = {keypad_stream_getc, keypad_stream_available, keypad_stream_clear, ((void *)0), ((void *)0), ((void *)0)};
uint8_t keypad_stream_send_cmd(const char *__s)
{
	if (cnc_get_exec_state(1 | 4) == 1)
	{
		return 9;
	}
	uint8_t len = strlen(__s);
	uint8_t w;
	if ((keypad_stream_stream_buffer_size - keypad_stream_stream_buffer.count) < len)
	{
		return 58;
	}
	({ uint8_t count, head; for (uint8_t __restore_atomic__ __attribute__((__cleanup__(__atomic_out))) = ((*(volatile uint8_t *)((0x3F) + 0x20)) & 0x80), __AtomLock = __atomic_in(); __AtomLock; __AtomLock = 0) { head = keypad_stream_stream_buffer.head; count = keypad_stream_stream_buffer.count; } count = (((keypad_stream_stream_buffer_size - count) <= (len)) ? (keypad_stream_stream_buffer_size - count) : (len)); w = 0; if (count) { uint8_t avail = (keypad_stream_stream_buffer_size - head); if (avail < count && avail) { memcpy(&keypad_stream_stream_buffer_bufferdata[head], __s, avail * sizeof(keypad_stream_stream_buffer_bufferdata[0])); w = avail; count -= avail; head = 0; } else { avail = 0; } if (count) { memcpy(&keypad_stream_stream_buffer_bufferdata[head], &__s[avail], count * sizeof(keypad_stream_stream_buffer_bufferdata[0])); w += count; for (uint8_t __restore_atomic__ __attribute__((__cleanup__(__atomic_out))) = ((*(volatile uint8_t *)((0x3F) + 0x20)) & 0x80), __AtomLock = __atomic_in(); __AtomLock; __AtomLock = 0) { head += count; if (head == keypad_stream_stream_buffer_size) { head = 0; } keypad_stream_stream_buffer.head = head; keypad_stream_stream_buffer.count += w; } } } });
	return 0;
}
#endif

#ifdef ENABLE_MAIN_LOOP_MODULES
bool grblhal_keypad_process(void *args)
{
	static uint8_t jogmode = 1;
	uint8_t mode = jogmode;
	// just do whatever you need here
	uint8_t c = 0;
	__ATOMIC__ { c = keypad_value; }
	uint8_t cmd[KEYPAD_MAX_CMD_LEN];
	memset(cmd, 0, KEYPAD_MAX_CMD_LEN);
	char *ptr = cmd;

	switch (c)
	{
	case 0:
		break;
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
		rom_strcpy(ptr, __romstr__("$H"));
		break;
	case 'R':
		rom_strcpy(ptr, __romstr__("$J=G91X"));
		// search for the end of string
		while (*++ptr)
			;
		// makes use of system menu helper to print the float
		system_menu_flt_to_str(ptr, keypad_settings[mode + 3]);
		break;
	case 'L':
		break;
	case 'F':
		break;
	case 'B':
		break;
	case 'U':
		break;
	case 'D':
		break;
	case 'r':
		break;
	case 'q':
		break;
	case 's':
		break;
	case 't':
		break;
	case 'w':
		break;
	case 'v':
		break;
	case 'u':
		break;
	case 'x':
		break;
	case 'I':
		break;
	case 'i':
		break;
	case 'j':
		break;
	case 'K':
		break;
	case 'k':
		break;
	case 'z':
		break;
	case 'C':
		break;
	case 'M':
		break;
	default:
		break;
	}

	keypad_stream_send_cmd(cmd);

	// you must return EVENT_CONTINUE to enable other tasks to run or return EVENT_HANDLED to terminate the event handling within this callback
	return EVENT_CONTINUE;
}

CREATE_EVENT_LISTENER(cnc_dotasks, grblhal_keypad_process);

#endif

#if defined(ENABLE_IO_MODULES) && (KEYPAD_PORT & 1)
MCU_CALLBACK bool grblhal_keypad_pressed(void *args)
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
			softi2c_receive(keypad_port, 0x49, &c, 1, 1);
			// if not handled enqueues the char for processing
			if (mcu_com_rx_cb(c))
			{
				// it's not an extended command
				// enqueue key for process
				keypad_value = c;
			}
		}
	}
	return EVENT_CONTINUE;
}

CREATE_EVENT_LISTENER(input_change, grblhal_keypad_pressed);
#endif

DECL_MODULE(grblhal_keypad)
{
#if (KEYPAD_PORT & 1)
#if (KEYPAD_PORT & 2)
	keypad_port = &keypad_i2c;
#else
	keypad_port = NULL;
#endif
#else
#if (KEYPAD_PORT & 2)
	keypad_port = &keypad_uart;
#else
	keypad_port = NULL;
#endif
#endif

#ifdef ENABLE_SETTINGS_MODULES
	EXTENDED_SETTING_INIT(KEYPAD_SETTING_ID, keypad_settings);
#else
	keypad_settings[0] = 500;
	keypad_settings[1] = 100;
	keypad_settings[2] = 500;
	keypad_settings[3] = 10;
	keypad_settings[4] = 1;
	keypad_settings[5] = 10;
#endif


#ifdef DECL_SERIAL_STREAM
	serial_stream_register(&keypad_stream);
#endif

#ifdef ENABLE_MAIN_LOOP_MODULES
	ADD_EVENT_LISTENER(cnc_dotasks, grblhal_keypad_process);
#else
	// just a warning in case you disabled the MAIN_LOOP option on build
#warning "Main loop extensions are not enabled. Your module will not work."
#endif

#if defined(ENABLE_IO_MODULES) && (KEYPAD_PORT & 1)
	ADD_EVENT_LISTENER(input_change, grblhal_keypad_pressed);
#else
#warning "IO extensions are not enabled. Your module will not work."
#endif
}
