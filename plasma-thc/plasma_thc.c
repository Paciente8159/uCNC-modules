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

#ifndef PLASMA_UP
#define PLASMA_UP() mcu_get_input(DIN15)
#endif

#ifndef PLASMA_DOWN
#define PLASMA_DOWN() mcu_get_input(DIN14)
#endif

#ifndef PLASMA_ARC_OK
#define PLASMA_ARC_OK() mcu_get_input(DIN13)
#endif

static bool plasma_thc_enabled;

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
			// get current position
			float pos[AXIS_COUNT];
			mc_get_position(pos);

			// clear the current hold
			cnc_clear_exec_state(EXEC_HOLD);
			// modify target to probe depth
			// pos[AXIS_Z] = something
			motion_data_t block = {0};
			// cutoff torch
			block.motion_flags.bit.spindle_running = 0;
			mc_update_tools(&block);

			// probe feed speed
			// block_data->feed = feed;
			// similar to G38.2
			if (mc_probe(pos, 0, &block) == STATUS_PROBE_SUCCESS)
			{
				// modify target to probe depth
				// pos[AXIS_Z] = something
				// similar to G38.4
				if (mc_probe(pos, 2, &block) == STATUS_PROBE_SUCCESS)
				{
					// modify target to torch start height
					// pos[AXIS_Z] = something
					// rapid feed
					block.feed = FLT_MAX;
					mc_line(pos, &block);
					// turn torch on and plunge
					block.feed = FLT_MAX;
					block.motion_flags.bit.spindle_running = 1;
					// block.dwell = some dwell time;
					// pos[AXIS_Z] = something
					mc_line(pos, &block);
					// wait to finnish
					itp_sync();

					// after the torch on dwell and plunge it's ready to continue if arc on is ok
					if (PLASMA_ARC_OK())
					{
						cnc_set_exec_state(EXEC_HOLD);
						// restore the motion controller, planner and parser
						mc_restore();
						planner_restore();
						parser_sync_position();

						cnc_clear_exec_state(EXEC_HOLD);
						// continues
					}
				}
			}
		}

		if (PLASMA_UP())
		{
			planner_block_t *p = planner_get_block();
			p->steps[2] = p->steps[p->main_stepper];
			p->dirbits &= 0xFB;
		}
		else if (PLASMA_DOWN())
		{
			planner_block_t *p = planner_get_block();
			p->steps[2] = p->steps[p->main_stepper];
			p->dirbits |= 4;
		}
		else
		{
			planner_block_t *p = planner_get_block();
			p->steps[2] = 0;
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
