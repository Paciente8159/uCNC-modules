/*
	Name: parser_g5.c
	Description: Implements a parser extension for LinuxCNC G5 and G5.1 for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 25/11/2022

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

#ifndef G5_MAX_SEGMENT_LENGTH
#define G5_MAX_SEGMENT_LENGTH 1.0f
#endif

// this ID must be unique for each code
#define G5 5

uint8_t g5_parse(void *args, bool *handled);
uint8_t g5_exec(void *args, bool *handled);

CREATE_EVENT_LISTENER(gcode_parse, g5_parse);
CREATE_EVENT_LISTENER(gcode_exec, g5_exec);

static bool is_chained_g5;
static float next_x;
static float next_y;
//#define SPLINE_USE_CASTELJAUS

#ifdef SPLINE_USE_CASTELJAUS
// De Casteljau's algorithm
#define spline_interpol(a, b, t) ((1 - t) * a + t * b)
#endif

float quadratic_spline_interpol(const float a, const float b, const float c, const float t)
{
#ifdef SPLINE_USE_CASTELJAUS
	// 6 multiplications plus 6 additions
	float k = spline_interpol(a, b, t);
	float l = spline_interpol(b, c, t);
	return spline_interpol(k, l, t);
#else
	// optimized version 6 multiplications plus 5 additions
	float t_sqr = t * t;
	float k = t_sqr;				 // t^2
	float p = c * k;				 // P = P2 * (t^2)
	k = fast_flt_mul2((t - k));		 // -2t^2 + 2t
	p += b * k;						 // P = P2 * (t^2) + P1 * (-2t^2 + 2t)
	k = fast_flt_div2((-k)) - t + 1; // t^2 - 2t + 1
	return (p + k * a);				 // P = P2 * (t^2) + P1 * (-2t^2 + 2t) + P0 * (t^2 - 2t + 1)
#endif
}

float cubic_spline_interpol(const float a, const float b, const float c, const float d, const float t)
{
#ifdef SPLINE_USE_CASTELJAUS
	// 12 multiplications plus 12 additions
	float k = spline_interpol(a, b, t);
	float l = spline_interpol(b, c, t);
	float m = spline_interpol(c, d, t);
	return quadratic_spline_interpol(k, l, m, t);
#else
	// optimized version (9 multiplications plus 8 additions)
	float t_sqr = t * t;							 // t^2
	float k = t_sqr * t;							 // t^3
	float p = d * k;								 // P = P3 * (t^3)
	k = 3.0f * (t_sqr - k);							 // -3t^3 + 3t^2
	p += k * c;										 // P = P3 * (t^3) + P2 * (-3t^3 + 3t^2)
	k = -3.0f * (t_sqr - t) - k;					 // 3t^3 - 6t^2 + 3t
	p += k * b;										 // P = P3 * (t^3) + P2 * (-3t^3 + 3t^2) + P1 * (3t^3 - 6t^2 + 3t)
	k = ((-1.0f / 3.0f) * k) - fast_flt_mul2(t) + 1; // -t^3 + 3t^2 - 3t + 1
	return (p + k * a);								 // P = P3 * (t^3) + P2 * (-3t^3 + 3t^2) + P1 * (3t^3 - 6t^2 + 3t) + P0 * (-t^3 + 3t^2 - 3t + 1)
#endif
}

// this just parses and accepts the code
uint8_t g5_parse(void *args, bool *handled)
{
	gcode_parse_args_t *ptr = (gcode_parse_args_t *)args;
	if (ptr->word == 'G' && ptr->code == 5)
	{
		// stops event propagation
		*handled = true;
		if (ptr->cmd->group_extended != 0 || CHECKFLAG(ptr->cmd->groups, GCODE_GROUP_MOTION))
		{
			// there is a collision of custom gcode commands (only one per line can be processed)
			return STATUS_GCODE_MODAL_GROUP_VIOLATION;
		}
		// checks if it's G5 or G5.1
		// check mantissa
		uint8_t mantissa = (uint8_t)lroundf(((ptr->value - ptr->code) * 100.0f));
		if (mantissa < 10)
		{
			mantissa = 255; // set an invalid value
		}
		else
		{
			mantissa /= 10;
		}
		// checks if this is a chained G5 or G5.1 motion
		is_chained_g5 = (ptr->new_state->groups.motion == G5);
		ptr->new_state->groups.motion = G5;
		ptr->new_state->groups.motion_mantissa = mantissa;
		SETFLAG(ptr->cmd->groups, GCODE_GROUP_MOTION);
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
		p3_x = ptr->target[AXIS_X];
		p2_x = p3_x + ptr->words->p;

		p0_y = current[AXIS_Y];
		p1_y = p0_y + p1_y_offset;
		p3_y = ptr->target[AXIS_Y];
		p2_y = p3_y + ptr->words->d; // d contains the value of q since d and q do not co-exist in any gcode

		// determine t step by figuring the max straight distance between control points
		// and then split them into a predefined fixed segment
		// not sure if this strategy is reliable but I'll use it for now
		float max_len = fast_flt_sqrt((fast_flt_pow2((p0_x - p1_x)) + fast_flt_pow2((p0_y - p1_y))));
		max_len = MAX(max_len, fast_flt_sqrt((fast_flt_pow2((p1_x - p2_x)) + fast_flt_pow2((p1_y - p2_y)))));
		max_len = MAX(max_len, fast_flt_sqrt((fast_flt_pow2((p2_x - p3_x)) + fast_flt_pow2((p2_y - p3_y)))));

		float t_inc = G5_MAX_SEGMENT_LENGTH / max_len;
		float next[AXIS_COUNT];

		uint8_t error;

		for (float t = t_inc; t < 1.0f; t += t_inc)
		{
			next[AXIS_X] = cubic_spline_interpol(p0_x, p1_x, p2_x, p3_x, t);
			next[AXIS_Y] = cubic_spline_interpol(p0_y, p1_y, p2_y, p3_y, t);
			for (uint8_t i = AXIS_Z; i < AXIS_COUNT; i++)
			{
				next[i] = t * (ptr->target[i] - current[i]);
			}

			error = mc_line(next, ptr->block_data);
			if (error != STATUS_OK)
			{
				return error;
			}
		}

		// ensure last motion to target;
		return mc_line(ptr->target, ptr->block_data);
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
