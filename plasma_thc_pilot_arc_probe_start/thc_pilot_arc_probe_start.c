/*
	Name: thc_pilot_arc_probe_start.c
	Description: Implements Plasma THC probe and start action to take advantage of a torch with pilot arc for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 31-03-2025

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

#if (UCNC_MODULE_VERSION < 11201 || UCNC_MODULE_VERSION > 99999)
#error "This module is not compatible with the current version of µCNC"
#endif

#ifdef PROBE_ENABLE_CUSTOM_CALLBACK
extern void plasma_thc_arc_onff(uint8_t value);
extern uint8_t plasma_thc_up(void);
extern uint8_t plasma_thc_down(void);
extern uint8_t plasma_thc_arc_ok(void);

bool plasma_thc_pilot_arc_probe()
{
	return (bool)(plasma_thc_arc_ok() || plasma_thc_down());
}

bool plasma_thc_probe_and_start(void)
{
	static bool plasma_starting = false;
	if (plasma_starting)
	{
		// prevent reentrancy
		return false;
	}

	plasma_starting = true;
	uint8_t ret = plasma_start_params.retries;
	cnc_store_motion();

	// wait for cycle start
	while (cnc_get_exec_state(EXEC_HOLD))
	{
		cnc_dotasks();
	}

	// re-route the probe signal to the custom check callback
	// this allows to use the arc ok and thc down signals has a probing signal
	io_probe_custom_get = plasma_thc_pilot_arc_probe;

	while (ret--)
	{
		// cutoff torch
		// temporary disable
		CLEARFLAG(plasma_thc_state, PLASMA_THC_ACTIVE);
		motion_data_t block = {0};
		block.motion_flags.bit.spindle_running = 0;
		mc_update_tools(&block);

		// Turn the pilot on
		plasma_thc_arc_onff(1);

		// get current position
		float pos[AXIS_COUNT];
		mc_get_position(pos);

		// modify target to probe depth
		pos[AXIS_TOOL] += plasma_start_params.probe_depth;
		// probe feed speed
		block.feed = plasma_start_params.probe_feed;
		// similar to G38.2
		if (mc_probe(pos, 0, &block) == STATUS_PROBE_SUCCESS)
		{
			// reset the probe signal
			io_probe_custom_get = NULL;
			// modify target to torch start height
			mc_get_position(pos);
			pos[AXIS_TOOL] += plasma_start_params.retract_height;
			// rapid feed
			block.feed = FLT_MAX;
			mc_line(pos, &block);
			// turn torch on and wait before confirm the arc on signal and form puddle
			block.motion_flags.bit.spindle_running = 1;
			block.dwell = plasma_start_params.dwell;
			// updated tools and wait
			mc_dwell(&block);

			// confirm if arc is ok
			if (plasma_thc_arc_ok())
			{
				mc_get_position(pos);
				pos[AXIS_TOOL] -= plasma_start_params.cut_depth;
				// rapid feed
				block.feed = plasma_start_params.cut_feed;
				mc_line(pos, &block);
				// enable plasma mode
				SETFLAG(plasma_thc_state, PLASMA_THC_ACTIVE);
				// continues program
				plasma_starting = false;
				cnc_restore_motion();
				return true;
			}
		}
	}

	// reset the probe signal
	io_probe_custom_get = NULL;
	cnc_restore_motion();
	plasma_starting = false;
	return false;
}
#else
#warning "Plasma THC Pilot arc Probe and Start module requires PROBE_ENABLE_CUSTOM_CALLBACK to be enabled. The module is disabled."
#endif
