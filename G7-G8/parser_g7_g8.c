/*
	Name: parser_g7_g8.c
	Description: Implements a parser extension for LinuxCNC G7-G8 for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 04/01/2023

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
#include <float.h>

#ifdef ENABLE_PARSER_MODULES

#if (UCNC_MODULE_VERSION < 10800 || UCNC_MODULE_VERSION > 99999)
#error "This module is not compatible with the current version of µCNC"
#endif

static uint8_t parser_lathe_radius_mode;

// this ID must be unique for each code
#define G7 1
#define G8 0

bool g7_g8_parse(void *args);
bool g7_g8_before_motion(void *args);
bool g7_g8_after_motion(void *args);
bool g7_g8_reset(void *args);
bool g7_g8_reset(void *args);
bool g7_g8_reset(void *args);
bool g7_g8_send_gcode_modes(void *args);

CREATE_EVENT_LISTENER(gcode_parse, g7_g8_parse);
CREATE_EVENT_LISTENER(gcode_before_motion, g7_g8_before_motion);
CREATE_EVENT_LISTENER(gcode_after_motion, g7_g8_after_motion);
CREATE_EVENT_LISTENER(parser_reset, g7_g8_reset);
CREATE_EVENT_LISTENER(protocol_send_gcode_modes, g7_g8_send_gcode_modes);

// this just parses and accepts the code
bool g7_g8_parse(void *args)
{
	gcode_parse_args_t *ptr = (gcode_parse_args_t *)args;
	if (ptr->word == 'G')
	{
		// stops event propagation
		bool ok = true;

		switch (ptr->code)
		{
		case G7:
			parser_lathe_radius_mode = G7;
			break;
		case G8:
			/* code */
			parser_lathe_radius_mode = G8;
			break;
		default:
			// not able to catch this G code
			ok = false;
			break;
		}

		if (ok)
		{
			// stops event propagation
			*(ptr->error) = STATUS_OK;
			return EVENT_HANDLED;
		}
	}

	// if this is not catched by this parser, just send back the error so other extenders can process it
	return EVENT_CONTINUE;
}

// modifies motion block
bool g7_g8_before_motion(void *args)
{
	gcode_exec_args_t *ptr = (gcode_exec_args_t *)args;

	// if radius mode is active modify the value of X to half
	if (parser_lathe_radius_mode)
	{
		ptr->target[AXIS_X] = fast_flt_div2(ptr->target[AXIS_X]);
	}

	*(ptr->error) = STATUS_OK;
	return EVENT_CONTINUE;
}

// modifies motion block
bool g7_g8_after_motion(void *args)
{
	gcode_exec_args_t *ptr = (gcode_exec_args_t *)args;

	// if radius mode is active modify the value of X to half
	if (parser_lathe_radius_mode)
	{
		ptr->target[AXIS_X] = fast_flt_mul2(ptr->target[AXIS_X]);
	}

	*(ptr->error) = STATUS_OK;
	return EVENT_CONTINUE;
}

bool g7_g8_reset(void *args)
{
	parser_lathe_radius_mode = G8; // default
	return EVENT_CONTINUE;
}

bool g7_g8_send_gcode_modes(void *args)
{
	serial_putc('G');
	if (parser_lathe_radius_mode)
	{
		serial_putc('7');
	}
	else
	{
		serial_putc('8');
	}
	serial_putc(' ');
	return EVENT_CONTINUE;
}

#endif

DECL_MODULE(g7_g8)
{
#ifdef ENABLE_PARSER_MODULES
	ADD_EVENT_LISTENER(gcode_parse, g7_g8_parse);
	ADD_EVENT_LISTENER(gcode_before_motion, g7_g8_before_motion);
	ADD_EVENT_LISTENER(gcode_after_motion, g7_g8_after_motion);
	ADD_EVENT_LISTENER(parser_reset, g7_g8_reset);
	ADD_EVENT_LISTENER(protocol_send_gcode_modes, g7_g8_send_gcode_modes);
#else
#error "Parser extensions are not enabled. G7-G8 code extension will not work."
#endif
}
