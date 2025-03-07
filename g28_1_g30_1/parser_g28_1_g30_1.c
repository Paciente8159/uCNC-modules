/*
	Name: parser_g28_1_g30_1.c
	Description: Implements a parser extension for LinuxCNC G28.1 (set G28 to current position) and G30.1 (set G30 to current position) for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 19-10-2023

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

#ifdef ENABLE_PARSER_MODULES

#if (UCNC_MODULE_VERSION < 10801)
#error "This module is not compatible with the current version of µCNC"
#endif

// this ID must be unique for each code
#define G28_1 EXTENDED_GCODE(28.1)
#define G30_1 EXTENDED_GCODE(30.1)

bool g28_1_g30_1_parse(void *args);
bool g28_1_g30_1_exec(void *args);

CREATE_EVENT_LISTENER(gcode_parse, g28_1_g30_1_parse);
CREATE_EVENT_LISTENER(gcode_exec, g28_1_g30_1_exec);

// this just parses and acceps the code
bool g28_1_g30_1_parse(void *args)
{
	gcode_parse_args_t *ptr = (gcode_parse_args_t *)args;

	if (ptr->word == 'G')
	{
		if (ptr->cmd->group_extended != 0)
		{
			// there is a collision of custom gcode commands (only one per line can be processed)
			*(ptr->error) = STATUS_GCODE_MODAL_GROUP_VIOLATION;
			return EVENT_HANDLED;
		}

		// check if mantissa is .1
		uint8_t m = (uint8_t)lroundf(((ptr->value - ptr->code) * 100.0f));
		if (m != 10)
		{
			return EVENT_CONTINUE;
		}

		switch (ptr->code)
		{
		case 28:
		case 30:
			ptr->cmd->group_extended = EXTENDED_GCODE(ptr->value);
			// stops event propagation
			*(ptr->error) = STATUS_OK;
			return EVENT_HANDLED;
		}
	}

	// if this is not catched by this parser, just send back the error so other extenders can process it
	return EVENT_CONTINUE;
}

// this actually performs 2 steps in 1 (validation and execution)
bool g28_1_g30_1_exec(void *args)
{
	gcode_exec_args_t *ptr = (gcode_exec_args_t *)args;
	// read parser current position (coordinate system 253)
	float axis[AXIS_COUNT];
	memset(axis, 0, sizeof(axis));
	switch (ptr->cmd->group_extended)
	{
	case G28_1:
		parser_get_coordsys(253, axis);
#if (UCNC_MODULE_VERSION < 11200)
		settings_save(G28ADDRESS, (uint8_t *)axis, sizeof(axis));
#else
		parser_coordinate_system_save(G28HOME, axis);
#endif
		// stops event propagation
		*(ptr->error) = STATUS_OK;
		return EVENT_HANDLED;
	case G30_1:
		parser_get_coordsys(253, axis);
#if (UCNC_MODULE_VERSION < 11200)
		settings_save(G30ADDRESS, (uint8_t *)axis, sizeof(axis));
#else
		parser_coordinate_system_save(G30HOME, axis);
#endif
		// stops event propagation
		*(ptr->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	return EVENT_CONTINUE;
}

#endif

DECL_MODULE(g28_1_g30_1)
{
#ifdef ENABLE_PARSER_MODULES
	ADD_EVENT_LISTENER(gcode_parse, g28_1_g30_1_parse);
	ADD_EVENT_LISTENER(gcode_exec, g28_1_g30_1_exec);
#else
#warning "Parser extensions are not enabled. G28.1 and G30.1 code extension will not work."
#endif
}
