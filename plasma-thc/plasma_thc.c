/*
	Name: palsma_thc.c
	Description: Implements Plasma THC for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: xx/xx/2023

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

// PLASMA conditions
// PLASMA conditions can be defined as any type of eval expression and will determine the behaviour of the plasma
// by default they evaluate to a pin state but

#if (UCNC_MODULE_VERSION > 010702)
#error "This module is not compatible with the current version of µCNC"
#endif

#ifndef PLASMA_UP
#define PLASMA_UP() mcu_get_input(DIN15)
#endif

#ifndef PLASMA_DOWN
#define PLASMA_DOWN() mcu_get_input(DIN14)
#endif

#ifndef PLASMA_ARC_OK
#define PLASMA_ARC_OK() mcu_get_input(DIN13)
#endif

#ifndef PLASMA_STEPPERS_MASK
#define PLASMA_STEPPERS_MASK 4
#endif

static bool plasma_thc_enabled;
static int8_t plasma_action;
typedef struct plasma_start_params_
{
	float probe_depth;	  // I
	float probe_feed;	  // J
	float retract_height; // R
	float cut_depth;	  // K
	float cut_feed;		  // F
	uint16_t dwell;		  // P*1000
	uint8_t retries;	  // L

} plasma_start_params_t;
static plasma_start_params_t plasma_start_params;

bool plasma_thc_probe_and_start(plasma_start_params_t start_params)
{
	plasma_thc_enabled = false;
	while (max_retries--)
	{
		// cutoff torch
		motion_data_t block = {0};
		block.motion_flags.bit.spindle_running = 0;
		mc_update_tools(&block);

		// get current position
		float pos[AXIS_COUNT];
		mc_get_position(pos);

		// modify target to probe depth
		pos[AXIS_Z] -= max_probe_depth;
		// probe feed speed
		block_data->feed = probe_feed;
		// similar to G38.2
		if (mc_probe(pos, 0, &block) == STATUS_PROBE_SUCCESS)
		{
			// modify target to probe depth
			mc_get_position(pos);
			pos[AXIS_Z] += max_probe_depth;
			block_data->feed = probe_feed * 0.5f; // half speed
			// similar to G38.4
			if (mc_probe(pos, 2, &block) == STATUS_PROBE_SUCCESS)
			{
				// modify target to torch start height
				mc_get_position(pos);
				pos[AXIS_Z] += retract_height;
				// rapid feed
				block.feed = FLT_MAX;
				mc_line(pos, &block);
				// turn torch on and wait before confirm the arc on signal
				block.motion_flags.bit.spindle_running = 1;
				block.dwell = dwell_ms;
				// updated tools and wait
				mc_dwell(&block);

				// confirm if arc is ok
				if (PLASMA_ARC_OK())
				{
					mc_get_position(pos);
					pos[AXIS_Z] -= cut_depth;
					// rapid feed
					block.feed = cut_feed;
					mc_line(pos, &block);
					cnc_set_exec_state(EXEC_HOLD);
					// restore the motion controller, planner and parser
					mc_restore();
					planner_restore();
					parser_sync_position();
					plasma_thc_enabled = true;
					cnc_clear_exec_state(EXEC_HOLD);
					// continues program
					return true;
				}
			}
		}
	}

	return false;
}

#ifdef ENABLE_RT_SYNC_MOTIONS
void itp_rt_stepbits(uint8_t *stepbits, uint8_t dirbits)
{
	if (!plasma_action)
	{
		return;
	}

	if (plasma_action > 0)
	{
		*stepbits |= PLASMA_STEPPERS_MASK;
		io_set_dirs(dirbits & ~PLASMA_STEPPERS_MASK);
		plasma_action--;
	}

	if (plasma_action < 0)
	{
		*stepbits |= PLASMA_STEPPERS_MASK;
		io_set_dirs(dirbits | PLASMA_STEPPERS_MASK);
		plasma_action--;
	}
}
#endif

#ifdef ENABLE_PARSER_MODULES
#define M103 EXTENDED_MCODE(103)

bool m103_parse(void *args);
bool m103_exec(void *args);

CREATE_EVENT_LISTENER(gcode_parse, m103_parse);
CREATE_EVENT_LISTENER(gcode_exec, m103_exec);

// this just parses and acceps the code
bool m103_parse(void *args)
{
	gcode_parse_args_t *ptr = (gcode_parse_args_t *)args;
	if (ptr->word == 'M' && ptr->code == 103)
	{
		if (ptr->cmd->group_extended != 0)
		{
			// there is a collision of custom gcode commands (only one per line can be processed)
			*(ptr->error) = STATUS_GCODE_MODAL_GROUP_VIOLATION;
			return EVENT_HANDLED;
		}
		// tells the gcode validation and execution functions this is custom code M42 (ID must be unique)
		ptr->cmd->group_extended = M103;
		*(ptr->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	// if this is not catched by this parser, just send back the error so other extenders can process it
	return EVENT_CONTINUE;
}

// this actually performs 2 steps in 1 (validation and execution)
bool m103_exec(void *args)
{
	gcode_exec_args_t *ptr = (gcode_exec_args_t *)args;
	if (ptr->cmd->group_extended == M103)
	{
		if (CHECKFLAG(ptr->cmd->words, (GCODE_WORD_I | GCODE_WORD_J | GCODE_WORD_K | GCODE_WORD_R | GCODE_WORD_P)) != (GCODE_WORD_I | GCODE_WORD_J | GCODE_WORD_K | GCODE_WORD_R | GCODE_WORD_P))
		{
			*(ptr->error) = STATUS_GCODE_VALUE_WORD_MISSING;
			return EVENT_HANDLED;
		}

		plasma_start_params.dwell = (uint16_t)(ptr->words->p * 1000);
		plasma_start_params.probe_depth = ptr->words->ijk[0];
		plasma_start_params.probe_feed = ptr->words->ijk[1];
		plasma_start_params.retract_height = ptr->words->r;
		if (CHECKFLAG(ptr->cmd->words, (GCODE_WORD_F)))
		{
			plasma_start_params.cut_feed = ptr->words->f;
		}
		plasma_start_params.cut_feed = (CHECKFLAG(ptr->cmd->words, (GCODE_WORD_L))) ? ptr->words->l : 1;

		if (plasma_thc_probe_and_start(plasma_start_params))
		{
			*(ptr->error) = STATUS_OK;
		}
		else
		{
			cnc_alarm(EXEC_ALARM_THC_FAILED_ARC_START);
		}
		return EVENT_HANDLED;
	}

	return EVENT_CONTINUE;
}

#endif

#ifdef ENABLE_MAIN_LOOP_MODULES
bool plasma_thc_update_loop(void *ptr)
{
	if (plasma_thc_enabled)
	{
		// arc lost
		// on arc lost the plasma must enter hold
		if (!(PLASMA_ARC_OK()))
		{
			// set hold and wait for motion to stop
			cnc_set_exec_state(EXEC_HOLD);
			itp_sync();
			// store planner and motion controll data away
			planner_store();
			mc_store();
			// reset planner and sync systems
			planner_clear();
			mc_sync_position();

			// clear the current hold state
			cnc_clear_exec_state(EXEC_HOLD);
		}

		if (PLASMA_UP())
		{
			// option 1 - modify the planner block
			// this assumes Z is not moving in this motion
			// planner_block_t *p = planner_get_block();
			// p->steps[2] = p->steps[p->main_stepper];
			// p->dirbits &= 0xFB;

			// option 2 - mask the step bits directly
			plasma_action += 1;
		}
		else if (PLASMA_DOWN())
		{
			// option 1 - modify the planner block
			// this assumes Z is not moving in this motion
			// planner_block_t *p = planner_get_block();
			// p->steps[2] = p->steps[p->main_stepper];
			// p->dirbits |= 4;

			// option 2 - mask the step bits directly
			plasma_action -= 1;
		}
		else
		{
			// option 1 - modify the planner block
			// this assumes Z is not moving in this motion
			// planner_block_t *p = planner_get_block();
			// p->steps[2] = 0;

			// option 2 - mask the step bits directly
			plasma_action = 0;
		}
	}
	return EVENT_CONTINUE;
}

CREATE_EVENT_LISTENER(cnc_dotasks, plasma_thc_update_loop);
#endif

DECL_MODULE(plasma_thc)
{
	plasma_thc_enabled = false;
#ifdef ENABLE_MAIN_LOOP_MODULES
	ADD_EVENT_LISTENER(cnc_dotasks, plasma_thc_update_loop);
#else
#warning "Main loop extensions are not enabled. Plasma THC will not work."
#endif
#ifndef ENABLE_MOTION_CONTROL_PLANNER_HIJACKING
#warning "Motion controller and planner hijacking option must be enabled. Plasma THC will not work."
#endif
#ifdef ENABLE_PARSER_MODULES

#else
#warning "Parser extensions are not enabled. Plasma THC will not work."
#endif
}
