/*
	Name: parser_m17_m18.c
	Description: Implements a parser extension for Marlin M17 (Enable steppers) and M81 (Disable steppers) for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 30/06/2022

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

#if (UCNC_MODULE_VERSION < 10800 || UCNC_MODULE_VERSION > 99999)
#error "This module is not compatible with the current version of µCNC"
#endif

// this ID must be unique for each code
#define M17 EXTENDED_MCODE(17)
#define M18 EXTENDED_MCODE(18)

bool m17_m18_parse(void *args);
bool m17_m18_exec(void *args);

CREATE_EVENT_LISTENER(gcode_parse, m17_m18_parse);
CREATE_EVENT_LISTENER(gcode_exec, m17_m18_exec);

// this just parses and acceps the code
bool m17_m18_parse(void *args)
{
	gcode_parse_args_t *ptr = (gcode_parse_args_t *)args;
	if (ptr->word == 'M')
	{
		if (ptr->cmd->group_extended != 0)
		{
			// there is a collision of custom gcode commands (only one per line can be processed)
			*(ptr->error) = STATUS_GCODE_MODAL_GROUP_VIOLATION;
			return EVENT_HANDLED;
		}

		switch (ptr->code)
		{
		case 17:
		case 18:
			ptr->cmd->group_extended = EXTENDED_MCODE(ptr->code);
			// stops event propagation
			*(ptr->error) = STATUS_OK;
			return EVENT_HANDLED;
		}
	}

	// if this is not catched by this parser, just send back the error so other extenders can process it
	return EVENT_CONTINUE;
}

// this actually performs 2 steps in 1 (validation and execution)
bool m17_m18_exec(void *args)
{
	gcode_exec_args_t *ptr = (gcode_exec_args_t *)args;
	switch (ptr->cmd->group_extended)
	{
	case M17:
		io_enable_steppers(g_settings.step_enable_invert);
		// stops event propagation
		*(ptr->error) = STATUS_OK;
		return EVENT_HANDLED;
	case M18:
		io_enable_steppers(~g_settings.step_enable_invert);
		// stops event propagation
		*(ptr->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	return EVENT_CONTINUE;
}

#endif

DECL_MODULE(m17_m18)
{
#ifdef ENABLE_PARSER_MODULES
	ADD_EVENT_LISTENER(gcode_parse, m17_m18_parse);
	ADD_EVENT_LISTENER(gcode_exec, m17_m18_exec);
#else
#warning "Parser extensions are not enabled. M17 and M18 code extension will not work."
#endif
}
