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

#include "../cnc.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef ENABLE_PARSER_MODULES

#ifndef UCNC_MODULE_VERSION_1_5_0_PLUS
#error "This module is not compatible with the current version of µCNC"
#endif

// if all conventions changes this must be updated
#define PWM0_ID 24
#define DOUT0_ID 46

// this ID must be unique for each code
#define M42 EXTENDED_MCODE(42)

uint8_t m42_parse(void *args, bool *handled);
uint8_t m42_exec(void *args, bool *handled);

CREATE_EVENT_LISTENER(gcode_parse, m42_parse);
CREATE_EVENT_LISTENER(gcode_exec, m42_exec);

// this just parses and acceps the code
uint8_t m42_parse(void *args, bool *handled)
{
	gcode_parse_args_t *ptr = (gcode_parse_args_t *)args;
	if (ptr->word == 'M' && ptr->code == 42)
	{
		// stops event propagation
		*handled = true;
		if (ptr->cmd->group_extended != 0)
		{
			// there is a collision of custom gcode commands (only one per line can be processed)
			return STATUS_GCODE_MODAL_GROUP_VIOLATION;
		}
		// tells the gcode validation and execution functions this is custom code M42 (ID must be unique)
		ptr->cmd->group_extended = M42;
		return STATUS_OK;
	}

	// if this is not catched by this parser, just send back the error so other extenders can process it
	return ptr->error;
}

// this actually performs 2 steps in 1 (validation and execution)
uint8_t m42_exec(void *args, bool *handled)
{
	gcode_exec_args_t *ptr = (gcode_exec_args_t *)args;
	if (ptr->cmd->group_extended == M42)
	{
		// stops event propagation
		*handled = true;
		if (CHECKFLAG(ptr->cmd->words, (GCODE_WORD_S | GCODE_WORD_P)) != (GCODE_WORD_S | GCODE_WORD_P))
		{
			return STATUS_GCODE_VALUE_WORD_MISSING;
		}

		if (ptr->words->p >= PWM0_ID && ptr->words->p < DOUT0_ID)
		{
			io_set_pwm(ptr->words->p, (uint8_t)CLAMP(0, ptr->words->s, 255));
		}

		if (ptr->words->p >= DOUT0_ID && ptr->words->p < (DOUT0_ID + 32))
		{
			io_set_output(ptr->words->p, (ptr->words->s != 0));
		}

		return STATUS_OK;
	}

	return STATUS_GCODE_EXTENDED_UNSUPPORTED;
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
