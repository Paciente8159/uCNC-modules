/*
	Name: parser_m67_m68.c
	Description: Implements a parser extension for LinuxCNC M67-M68 for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 31/05/2022

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
#define M67 EXTENDED_MCODE(67)
#define M68 EXTENDED_MCODE(68)

bool m67_m68_parse(void *args);
bool m67_m68_exec(void *args);

CREATE_EVENT_LISTENER(gcode_parse, m67_m68_parse);
CREATE_EVENT_LISTENER(gcode_exec, m67_m68_exec);

// this just parses and acceps the code
bool m67_m68_parse(void *args)
{
	gcode_parse_args_t *ptr = (gcode_parse_args_t *)args;

	if (ptr->word == 'M' && (ptr->code == 67 || ptr->code == 68))
	{
		if (ptr->cmd->group_extended != 0)
		{
			// there is a collision of custom gcode commands (only one per line can be processed)
			*(ptr->error) = STATUS_GCODE_MODAL_GROUP_VIOLATION;
			return EVENT_HANDLED;
		}

		// tells the gcode validation and execution functions this is custom code M42 (ID must be unique)
		ptr->cmd->group_extended = (ptr->code == 67) ? M67 : M68;
		*(ptr->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	if (ptr->cmd->group_extended == M67 || ptr->cmd->group_extended == M68)
	{
		if (ptr->word == 'E')
		{
			ptr->cmd->words |= GCODE_WORD_A;
			ptr->words->xyzabc[3] = ptr->value;
			*(ptr->error) = STATUS_OK;
			return EVENT_HANDLED;
		}
	}

	// if this is not catched by this parser, just send back the error so other extenders can process it
	return EVENT_CONTINUE;
}

// this actually performs 2 steps in 1 (validation and execution)
bool m67_m68_exec(void *args)
{
	gcode_exec_args_t *ptr = (gcode_exec_args_t *)args;

	if (ptr->cmd->group_extended == M67 || ptr->cmd->group_extended == M68)
	{
		if (ptr->words->l == 0)
		{
			*(ptr->error) = STATUS_GCODE_VALUE_WORD_MISSING;
			return EVENT_HANDLED;
		}

		if (ptr->words->xyzabc[3] < 0)
		{
			*(ptr->error) = STATUS_NEGATIVE_VALUE;
			return EVENT_HANDLED;
		}

		uint8_t analogoutput = (uint8_t)truncf(ptr->words->xyzabc[3]) + PWM_PINS_OFFSET;
		*(ptr->error) = STATUS_INVALID_STATEMENT;

		if (ptr->cmd->group_extended == M67)
		{
			itp_sync();
		}

		if (analogoutput < 16)
		{
			io_set_pinvalue(analogoutput, (uint8_t)CLAMP(0, ptr->words->d, 255));
			*(ptr->error) = STATUS_OK;
		}
		else
		{
			// let this propagate
			return EVENT_CONTINUE;
		}

		return EVENT_HANDLED;
	}

	return EVENT_CONTINUE;
}

#endif

DECL_MODULE(m67_m68)
{
#ifdef ENABLE_PARSER_MODULES
	ADD_EVENT_LISTENER(gcode_parse, m67_m68_parse);
	ADD_EVENT_LISTENER(gcode_exec, m67_m68_exec);
#else
#warning "Parser extensions are not enabled. M67-M68 code extension will not work."
#endif
}
