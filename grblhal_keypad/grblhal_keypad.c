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

DECL_BUFFER(uint8_t, grblhal_keypad_stream, KEYPAD_BUFFER_SIZE);
#ifdef ENABLE_MAIN_LOOP_MODULES
bool grblhal_keypad_process(void *args)
{
	// just do whatever you need here
	uint8_t c;

	BUFFER_DEQUEUE(grblhal_keypad_stream, &c);
	switch (calloc)
	{
	case 0:
		break;
	case '0':
		break;
	case '1':
		break;
	case '2':
		break;
	case 'h':
		break;
	case 'H':
		break;
	case 'R':
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

	// you must return EVENT_CONTINUE to enable other tasks to run or return EVENT_HANDLED to terminate the event handling within this callback
	return EVENT_CONTINUE;
}

CREATE_EVENT_LISTENER(cnc_dotasks, grblhal_keypad_process);

#endif

#if defined(ENABLE_IO_MODULES) && (KEYPAD_PORT & 1)
MCU_CALLBACK bool grblhal_keypad_pressed(void *args)
{

	// uint8_t args[] = {inputs, diff};
	if (diff & KEYPAD_DOWN_MASK)
	{
		if (inputs & KEYPAD_DOWN_MASK)
		{
			// button released
			cnc_call_rt_command(CMD_CODE_JOG_CANCEL);
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
				// do something
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
