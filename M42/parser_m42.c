/*
	Name: parser_m42.c
	Description: Implements a parser extension for Marlin M42 for µCNC.
		This is only a partial implementation. Only the state S of the pin will be definable

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 18/01/2022

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
#define M42 EXTENDED_MCODE(42)

bool m42_parse(void *args);
bool m42_exec(void *args);

CREATE_EVENT_LISTENER(gcode_parse, m42_parse);
CREATE_EVENT_LISTENER(gcode_exec, m42_exec);

// this just parses and acceps the code
bool m42_parse(void *args)
{
	gcode_parse_args_t *ptr = (gcode_parse_args_t *)args;
	if (ptr->word == 'M' && ptr->code == 42)
	{
		if (ptr->cmd->group_extended != 0)
		{
			// there is a collision of custom gcode commands (only one per line can be processed)
			*(ptr->error) = STATUS_GCODE_MODAL_GROUP_VIOLATION;
			return EVENT_HANDLED;
		}
		// tells the gcode validation and execution functions this is custom code M42 (ID must be unique)
		ptr->cmd->group_extended = M42;
		*(ptr->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	// if this is not catched by this parser, just send back the error so other extenders can process it
	return EVENT_CONTINUE;
}

// this actually performs 2 steps in 1 (validation and execution)
bool m42_exec(void *args)
{
	gcode_exec_args_t *ptr = (gcode_exec_args_t *)args;
	if (ptr->cmd->group_extended == M42)
	{
		if (CHECKFLAG(ptr->cmd->words, (GCODE_WORD_S | GCODE_WORD_P)) != (GCODE_WORD_S | GCODE_WORD_P))
		{
			*(ptr->error) = STATUS_GCODE_VALUE_WORD_MISSING;
			return EVENT_HANDLED;
		}

		if (ptr->words->p >= PWM_PINS_OFFSET && ptr->words->p < (DOUT_PINS_OFFSET + 50))
		{
			io_set_pinvalue(ptr->words->p, (uint8_t)CLAMP(0, ptr->words->s, 255));
		}

		*(ptr->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	return EVENT_CONTINUE;
}

#endif

DECL_MODULE(m42)
{
#ifdef ENABLE_PARSER_MODULES
	ADD_EVENT_LISTENER(gcode_parse, m42_parse);
	ADD_EVENT_LISTENER(gcode_exec, m42_exec);
#else
#warning "Parser extensions are not enabled. M42 code extension will not work."
#endif
}
