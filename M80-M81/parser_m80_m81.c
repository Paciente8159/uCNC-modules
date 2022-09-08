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

#ifndef UCNC_MODULE_VERSION_1_5_0_PLUS
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

uint8_t m80_m81_parse(void *args, bool *handled);
uint8_t m80_m81_exec(void *args, bool *handled);

CREATE_EVENT_LISTENER(gcode_parse, m80_m81_parse);
CREATE_EVENT_LISTENER(gcode_exec, m80_m81_exec);

// this just parses and acceps the code
uint8_t m80_m81_parse(void *args, bool *handled)
{
	gcode_parse_args_t *ptr = (gcode_parse_args_t *)args;

	if (ptr->word == 'M')
	{
		if (ptr->cmd->group_extended != 0)
		{
			// stops event propagation
			*handled = true;
			// there is a collision of custom gcode commands (only one per line can be processed)
			return STATUS_GCODE_MODAL_GROUP_VIOLATION;
		}

		switch (ptr->code)
		{
		case 80:
		case 81:
			// stops event propagation
			*handled = true;
			ptr->cmd->group_extended = EXTENDED_MCODE_BASE + ptr->code;
			return STATUS_OK;
		}
	}

	// if this is not catched by this parser, just send back the error so other extenders can process it
	return ptr->error;
}

// this actually performs 2 steps in 1 (validation and execution)
uint8_t m80_m81_exec(void *args, bool *handled)
{
	gcode_exec_args_t *ptr = (gcode_exec_args_t *)args;

	switch (ptr->cmd->group_extended)
	{
	case M80:
		// stops event propagation
		*handled = true;
		io_set_output(PSU_PIN, PSU_ON);
		return STATUS_OK;
	case M81:
		// stops event propagation
		*handled = true;
		io_set_output(PSU_PIN, !PSU_ON);
		return STATUS_OK;
	}

	return STATUS_GCODE_EXTENDED_UNSUPPORTED;
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
