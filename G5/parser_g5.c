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

#ifndef UCNC_MODULE_VERSION_1_5_0_PLUS
#error "This module is not compatible with the current version of µCNC"
#endif

// this ID must be unique for each code
#define G5 18
#define G5_1 19

uint8_t g5_parse(void *args, bool *handled);
uint8_t g5_exec(void *args, bool *handled);

CREATE_EVENT_LISTENER(gcode_parse, g5_parse);
CREATE_EVENT_LISTENER(gcode_exec, g5_exec);

static bool is_chained_g5;
static float next_x;
static float next_y;

// De Casteljau's algorithm
#define spline_interpol(a, b, t) ((1 - t) * a + t * b)

float cubic_spline_interpol(const float a, const float b, const float c, const float d, const float t)
{
	
}

// this just parses and accepts the code
uint8_t g5_parse(void *args, bool *handled)
{
	gcode_parse_args_t *ptr = (gcode_parse_args_t *)args;
	if (ptr->word == 'G' && ptr->code == 5)
	{
		// stops event propagation
		*handled = true;
		if (ptr->cmd->group_extended != 0)
		{
			// there is a collision of custom gcode commands (only one per line can be processed)
			return STATUS_GCODE_MODAL_GROUP_VIOLATION;
		}
		// checks if it's G5 or G5.1
		uint8_t mantissa = (uint8_t)lroundf((ptr->value - ptr->code) * 100.0f);
		// checks if this is a chained G5 or G5.1 motion
		is_chained_g5 = (ptr->new_state->groups.motion == G5 || ptr->new_state->groups.motion == G5_1);
		switch (mantissa)
		{
		case 0:
			ptr->new_state->groups.motion = G5;
			break;
		case 1:
			ptr->new_state->groups.motion = G5_1;
			break;
		default:
			return STATUS_GCODE_UNSUPPORTED_COMMAND;
		}

		ptr->cmd->group_extended = EXTENDED_MOTION_GCODE(5);
		return STATUS_OK;
	}

	// if this is not catched by this parser, just send back the error so other extenders can process it
	return ptr->error;
}

// this actually performs 2 steps in 1 (validation and execution)
uint8_t g5_exec(void *args, bool *handled)
{
	gcode_exec_args_t *ptr = (gcode_exec_args_t *)args;
	if (ptr->cmd->group_extended == EXTENDED_MOTION_GCODE(5))
	{
		// stops event propagation
		*handled = true;

		if (CHECKFLAG(ptr->cmd->words, (GCODE_WORD_I | GCODE_WORD_J)) != (GCODE_WORD_I | GCODE_WORD_J))
		{
			// it's an error if both I and J is not explicitly defined or if they are omitted without a previous G5 command 
			if (!is_chained_g5 || CHECKFLAG(ptr->cmd->words, (GCODE_WORD_I | GCODE_WORD_J)))
			{
				return STATUS_GCODE_VALUE_WORD_MISSING;
			}
		}

		float current[AXIS_COUNT];
		float p0_x, p1_x, p2_x, p3_x;
		float p0_y, p1_y, p2_y, p3_y;
		float p1_x_offset = (!is_chained_g5) ? ptr->words->ijk[AXIS_X] : -next_x;
		float p1_y_offset = (!is_chained_g5) ? ptr->words->ijk[AXIS_Y] : -next_y;
		
		mc_get_position(current);

		p0_x = current[AXIS_X];
		p1_x = p0_x + p1_x_offset;
		p3_x = ptr->words->xyzabc[AXIS_X];
		p2_x = p3_x + ptr->words->p;

		p0_y = current[AXIS_Y];
		p1_y = p0_y + p1_y_offset;
		p3_y = ptr->words->xyzabc[AXIS_Y];
		p2_y = p3_y + ptr->words->d; // d contains the value of q since d and q do not co-exist in any gcode


		return STATUS_OK;
	}

	return STATUS_GCODE_EXTENDED_UNSUPPORTED;
}

#endif

DECL_MODULE(g5)
{
#ifdef ENABLE_PARSER_MODULES
	ADD_EVENT_LISTENER(gcode_parse, g5_parse);
	ADD_EVENT_LISTENER(gcode_exec, g5_exec);
#else
#warning "Parser extensions are not enabled. G5 and G5.1 code extension will not work."
#endif
}
