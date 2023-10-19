/*
	Name: parser_m80_m81.c
	Description: Implements a parser extension for Marlin M80 (PSU ON) and M81 (PSU OFF) for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 22/05/2022

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

#if (UCNC_MODULE_VERSION != 10800)
#error "This module is not compatible with the current version of µCNC"
#endif

// setup PSU pins and value to enable the PSU
#ifndef PSU_PIN
#define PSU_PIN DOUT31
#endif
#ifndef PSU_ON
#define PSU_ON true
#endif

// this ID must be unique for each code
#define M80 EXTENDED_MCODE(80)
#define M81 EXTENDED_MCODE(81)

bool m80_m81_parse(void *args);
bool m80_m81_exec(void *args);

CREATE_EVENT_LISTENER(gcode_parse, m80_m81_parse);
CREATE_EVENT_LISTENER(gcode_exec, m80_m81_exec);

// this just parses and acceps the code
bool m80_m81_parse(void *args)
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
		case 80:
		case 81:
			ptr->cmd->group_extended = EXTENDED_MCODE(ptr->code);
			*(ptr->error) = STATUS_OK;
			return EVENT_HANDLED;
		}
	}

	// if this is not catched by this parser, just send back the error so other extenders can process it
	return EVENT_CONTINUE;
}

// this actually performs 2 steps in 1 (validation and execution)
bool m80_m81_exec(void *args)
{
	gcode_exec_args_t *ptr = (gcode_exec_args_t *)args;

	switch (ptr->cmd->group_extended)
	{
	case M80:
		io_set_pinvalue(PSU_PIN, PSU_ON);
		*(ptr->error) = STATUS_OK;
		return EVENT_HANDLED;
	case M81:
		io_set_pinvalue(PSU_PIN, !PSU_ON);
		*(ptr->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	return EVENT_CONTINUE;
}

#endif

DECL_MODULE(m80_m81)
{
#ifdef ENABLE_PARSER_MODULES
	ADD_EVENT_LISTENER(gcode_parse, m80_m81_parse);
	ADD_EVENT_LISTENER(gcode_exec, m80_m81_exec);
#else
#warning "Parser extensions are not enabled. M80 and M81 code extension will not work."
#endif
}
