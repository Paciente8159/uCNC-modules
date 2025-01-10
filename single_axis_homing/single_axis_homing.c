/*
	This is and example of how to add a custom Grbl system command extension to µCNC
	Extension system commands can be added simply by adding an event listener to the core parser

	GCode/MCode execution is done in 3 steps:
		- Parsing - the code is read from the stream and it's parsed generating a new machine state
		- Validation - the new machine is validated against some NIST/RS274 rules for incompatibilities
		- Execution - The new state is executed and the active machine state is updated

	Every unrecognized system command can be intercepted before being discarded as an invalid command
	There is an event that can be hooked to do this.
*/

#include "../../cnc.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/**
 * @brief	System command extensions depend on the ENABLE_PARSER_MODULES option
 * 			Check if this option is defined or not
 */
#ifdef ENABLE_PARSER_MODULES

/**
 * @brief	Check if your current module is up to date with the current core version of module
 */
#if (UCNC_MODULE_VERSION < 10800 || UCNC_MODULE_VERSION > 99999)
#error "This module is not compatible with the current version of µCNC"
#endif

/**
 * @brief Create a function to parse your custom MCode. All event functions are declared as uint8_t <function>(void* args, bool* handle)
 *
 * @param args		is a pointer to a set of arguments to be passed to the event handler. In the case of the grbl_cmd event it's a struct of type gcode_parse_args_t define the following way
 * 					typedef struct grbl_cmd_args_
 *					{
 *						uint8_t *error;		// the current parser error code
 *						unsigned char *cmd; // pointer to a string with the system command read
 *						uint8_t len; 		// length of the command string
 *						char next_char;		// the value of the next char in the buffer
 *					} grbl_cmd_args_t;
 * @return bool 	a boolean that tells the handler if the event should continue to propagate through additional listeners or is handled by the current listener an should stop propagation
 */
bool single_axis_homing_system_cmd(void *args)
{
	// this is just to cast the void* args to the used struct by the parse event
	grbl_cmd_args_t *ptr = (grbl_cmd_args_t *)args;
	float target[AXIS_COUNT];

	strupr((char *)ptr->cmd);

#if AXIS_X_HOMING_MASK != 0
	if (!strcmp((char *)ptr->cmd, "HX"))
	{
		if (mc_home_axis(AXIS_X_HOMING_MASK, LINACT0_LIMIT_MASK))
		{
			return KINEMATIC_HOMING_ERROR_X;
		}

		cnc_unlock(true);
		motion_data_t block_data = {0};
		mc_get_position(target);
		target[AXIS_X] += ((g_settings.homing_dir_invert_mask & (1 << AXIS_X)) ? -g_settings.homing_offset : g_settings.homing_offset);
		block_data.feed = g_settings.homing_fast_feed_rate;
		block_data.spindle = 0;
		block_data.dwell = 0;
		// starts offset and waits to finnish
		mc_line(target, &block_data);
		itp_sync();

		// system command should return GRBL_SYSTEM_CMD_EXTENDED
		*(ptr->error) = GRBL_SYSTEM_CMD_EXTENDED;
		return EVENT_HANDLED;
	}
#endif
#if AXIS_Y_HOMING_MASK != 0
	if (!strcmp((char *)ptr->cmd, "HY"))
	{
		if (mc_home_axis(AXIS_Y_HOMING_MASK, LINACT1_LIMIT_MASK))
		{
			return KINEMATIC_HOMING_ERROR_Y;
		}

		cnc_unlock(true);
		motion_data_t block_data = {0};
		mc_get_position(target);
		target[AXIS_Y] += ((g_settings.homing_dir_invert_mask & (1 << AXIS_Y)) ? -g_settings.homing_offset : g_settings.homing_offset);
		block_data.feed = g_settings.homing_fast_feed_rate;
		block_data.spindle = 0;
		block_data.dwell = 0;
		// starts offset and waits to finnish
		mc_line(target, &block_data);
		itp_sync();

		// system command should return GRBL_SYSTEM_CMD_EXTENDED
		*(ptr->error) = GRBL_SYSTEM_CMD_EXTENDED;
		return EVENT_HANDLED;
	}
#endif
#if AXIS_Z_HOMING_MASK != 0
	if (!strcmp((char *)ptr->cmd, "HZ"))
	{
		if (mc_home_axis(AXIS_Z_HOMING_MASK, LINACT2_LIMIT_MASK))
		{
			return KINEMATIC_HOMING_ERROR_Z;
		}

		cnc_unlock(true);
		motion_data_t block_data = {0};
		mc_get_position(target);
		target[AXIS_Z] += ((g_settings.homing_dir_invert_mask & (1 << AXIS_Z)) ? -g_settings.homing_offset : g_settings.homing_offset);
		block_data.feed = g_settings.homing_fast_feed_rate;
		block_data.spindle = 0;
		block_data.dwell = 0;
		// starts offset and waits to finnish
		mc_line(target, &block_data);
		itp_sync();

		// system command should return GRBL_SYSTEM_CMD_EXTENDED
		*(ptr->error) = GRBL_SYSTEM_CMD_EXTENDED;
		return EVENT_HANDLED;
	}
#endif
#if AXIS_A_HOMING_MASK != 0
	if (!strcmp((char *)ptr->cmd, "HA"))
	{
		if (mc_home_axis(AXIS_A_HOMING_MASK, LINACT3_LIMIT_MASK))
		{
			return KINEMATIC_HOMING_ERROR_A;
		}

		cnc_unlock(true);
		motion_data_t block_data = {0};
		mc_get_position(target);
		target[AXIS_A] += ((g_settings.homing_dir_invert_mask & (1 << AXIS_A)) ? -g_settings.homing_offset : g_settings.homing_offset);
		block_data.feed = g_settings.homing_fast_feed_rate;
		block_data.spindle = 0;
		block_data.dwell = 0;
		// starts offset and waits to finnish
		mc_line(target, &block_data);
		itp_sync();

		// system command should return GRBL_SYSTEM_CMD_EXTENDED
		*(ptr->error) = GRBL_SYSTEM_CMD_EXTENDED;
		return EVENT_HANDLED;
	}
#endif
#if AXIS_B_HOMING_MASK != 0
	if (!strcmp((char *)ptr->cmd, "HB"))
	{
		if (mc_home_axis(AXIS_B_HOMING_MASK, LINACT4_LIMIT_MASK))
		{
			return KINEMATIC_HOMING_ERROR_B;
		}

		cnc_unlock(true);
		motion_data_t block_data = {0};
		mc_get_position(target);
		target[AXIS_B] += ((g_settings.homing_dir_invert_mask & (1 << AXIS_B)) ? -g_settings.homing_offset : g_settings.homing_offset);
		block_data.feed = g_settings.homing_fast_feed_rate;
		block_data.spindle = 0;
		block_data.dwell = 0;
		// starts offset and waits to finnish
		mc_line(target, &block_data);
		itp_sync();

		// system command should return GRBL_SYSTEM_CMD_EXTENDED
		*(ptr->error) = GRBL_SYSTEM_CMD_EXTENDED;
		return EVENT_HANDLED;
	}
#endif
#if AXIS_C_HOMING_MASK != 0
	if (!strcmp((char *)ptr->cmd, "HC"))
	{
		if (mc_home_axis(AXIS_C_HOMING_MASK, LINACT5_LIMIT_MASK))
		{
			return KINEMATIC_HOMING_ERROR_C;
		}

		cnc_unlock(true);
		motion_data_t block_data = {0};
		mc_get_position(target);
		target[AXIS_C] += ((g_settings.homing_dir_invert_mask & (1 << AXIS_C)) ? -g_settings.homing_offset : g_settings.homing_offset);
		block_data.feed = g_settings.homing_fast_feed_rate;
		block_data.spindle = 0;
		block_data.dwell = 0;
		// starts offset and waits to finnish
		mc_line(target, &block_data);
		itp_sync();

		// system command should return GRBL_SYSTEM_CMD_EXTENDED
		*(ptr->error) = GRBL_SYSTEM_CMD_EXTENDED;
		return EVENT_HANDLED;
	}
#endif

	// just return an error to the handler telling this is an invalid command
	return EVENT_CONTINUE;
}

/**
 * @brief 	Create an event listener object an attach our custom code parser handler.
 * 			in this case we are adding a listener to the 'grbl_cmd' EVENT
 *
 */
CREATE_EVENT_LISTENER(grbl_cmd, mycustom_system_cmd);

#endif

/**
 * @brief 	Declarates a new module and adds the event listeners.
 * 			Again this should check the if the appropriate module option is enabled
 * 			To add this module you just neet to call LOAD_MODULE(mycustom_system_cmd_module); from inside the core code
 */
DECL_MODULE(single_axis_homing)
{
#ifdef ENABLE_PARSER_MODULES
	// Makes the event handler 'mycustom_system_cmd' listen to the event 'grbl_cmd'
	ADD_EVENT_LISTENER(grbl_cmd, single_axis_homing_system_cmd);
#else
// just a warning in case you disabled the PARSER_MODULES option on build
#warning "Parser extensions are not enabled. Single Axis Command code extension will not work."
#endif
}
