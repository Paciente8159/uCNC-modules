/*
	Name: parser_g33.c
	Description: Implements a parser extension for LinuxCNC G33 for µCNC.

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
#include "../encoder.h"
#include <stdint.h>
#include <stdbool.h>
#include <float.h>

#ifdef ENABLE_PARSER_MODULES

#if (UCNC_MODULE_VERSION < 11501 || UCNC_MODULE_VERSION > 99999)
#error "This module is not compatible with the current version of µCNC"
#endif

#ifndef G33_ENCODER
#error "G33 requires to have an assigned encoder"
#endif

#ifdef G33_INDEX_PIN
#define G33_INDEX_MASK (1 << (G33_INDEX_PIN - DIN_PINS_OFFSET))
#ifndef ENABLE_IO_MODULES
#error "G33 requires ENABLE_IO_MODULES to use a custom index pin"
#endif
#if (G33_INDEX_PIN < DIN_PINS_OFFSET || G33_INDEX_PIN > (DIN_PINS_OFFSET + 7))
#error "G33 index pin must be an interruptable pin DIN0-DIN7"
#endif
#endif

// uncomment to allow data verbose of sync constants
// the message output is
// [MSG:<tool_step_position>:<spindle index counter>:<spindle_step_position>:<encoder_rpm>]
// #define G33_DEBUG

// #define RPM_PPR_INV (1.0f / (float)RPM_PPR)

#define SYNC_DISABLED 0
#define SYNC_READY 1
#define SYNC_STARTING 2
#define SYNC_RUNNING 4

static volatile int32_t itp_sync_step_counter;
static volatile uint8_t synched_motion_status;
static volatile int32_t spindle_index_counter;
static uint32_t steps_per_index;
#ifndef G33_REPLACE_FP_OPERATION_IN_ISR
static float steps_per_index_inv;
#else
static uint64_t steps_per_index_inv_fixed;
#endif
static uint32_t motion_total_steps;
static float rpm_to_stepfeed_constant;
#ifdef G33_INDEX_PIN
static bool g33_wait_for_sync;
#endif

void itp_rt_stepcount_cb_handler(uint8_t stepbits, uint8_t itp_flags)
{
	if (itp_flags & ITP_SYNC)
	{
		itp_sync_step_counter++;
	}
}

#ifdef G33_INDEX_PIN
bool spindle_index_cb_handler(void *args)
#else
void spindle_index_cb_handler(void)
#endif
{
#ifdef G33_INDEX_PIN
	if (g33_wait_for_sync)
	{
		uint8_t *inputs = (uint8_t *)args;
		uint8_t pinstate = inputs[0] & inputs[1];
		if (pinstate & G33_INDEX_MASK)
		{
#endif
			switch (synched_motion_status)
			{
			case SYNC_READY:
				itp_start(false);
				synched_motion_status = SYNC_STARTING;
				break;
			case SYNC_STARTING:
				if (itp_sync_ready())
				{
#ifndef G33_REPLACE_FP_OPERATION_IN_ISR
					spindle_index_counter = lroundf(steps_per_index_inv * itp_sync_step_counter);
#else
			int32_t counter = itp_sync_step_counter;
			int64_t prod = (int64_t)counter * (int64_t)steps_per_index_inv_fixed;
			prod += (prod >= 0) ? (int64_t)(1 << 31) : -(int64_t)(1 << 31);
			spindle_index_counter = (int32_t)(prod >> 32);
#endif
					synched_motion_status = SYNC_RUNNING;
				}
				break;
			case SYNC_RUNNING:
				spindle_index_counter++;
				break;
			}
#ifdef G33_INDEX_PIN
		}
	}
	return EVENT_CONTINUE;
#endif
}

CREATE_EVENT_LISTENER(input_change, spindle_index_cb_handler);

// this ID must be unique for each code
#define G33 33

bool g33_parse(void *args);
bool g33_exec(void *args);

CREATE_EVENT_LISTENER(gcode_parse, g33_parse);
CREATE_EVENT_LISTENER(gcode_exec, g33_exec);

// this just parses and accepts the code
bool g33_parse(void *args)
{
	gcode_parse_args_t *ptr = (gcode_parse_args_t *)args;
	if (ptr->word == 'G' && ptr->code == 33)
	{
		// stops event propagation
		if (ptr->cmd->group_extended != 0 || CHECKFLAG(ptr->cmd->groups, GCODE_GROUP_MOTION))
		{
			// there is a collision of custom gcode commands (only one per line can be processed)
			*(ptr->error) = STATUS_GCODE_MODAL_GROUP_VIOLATION;
			return EVENT_HANDLED;
		}
		// checks if it's G5 or G5.1
		// check mantissa
		uint8_t mantissa = (uint8_t)lroundf(((ptr->value - ptr->code) * 100.0f));

		if (mantissa != 0)
		{
			*(ptr->error) = STATUS_GCODE_UNSUPPORTED_COMMAND;
			return EVENT_HANDLED;
		}

		ptr->new_state->groups.motion = G33;
		ptr->new_state->groups.motion_mantissa = 0;
		SETFLAG(ptr->cmd->groups, GCODE_GROUP_MOTION);
		ptr->cmd->group_extended = EXTENDED_MOTION_GCODE(33);
		*(ptr->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	// if this is not catched by this parser, just send back the error so other extenders can process it
	return EVENT_CONTINUE;
}

// this actually performs 2 steps in 1 (validation and execution)
bool g33_exec(void *args)
{
	gcode_exec_args_t *ptr = (gcode_exec_args_t *)args;
	if (ptr->cmd->group_extended == EXTENDED_MOTION_GCODE(33))
	{
		if (!CHECKFLAG(ptr->cmd->words, GCODE_XYZ_AXIS))
		{
			// it's an error no axis word is specified
			*(ptr->error) = STATUS_GCODE_NO_AXIS_WORDS;
			return EVENT_HANDLED;
		}

		if (!CHECKFLAG(ptr->cmd->words, GCODE_WORD_K))
		{
			// it's an error no distance per rev word is specified
			*(ptr->error) = STATUS_GCODE_VALUE_WORD_MISSING;
			return EVENT_HANDLED;
		}

		// syncs motions and sets spindle
		if (mc_update_tools(ptr->block_data) != STATUS_OK)
		{
			*(ptr->error) = STATUS_CRITICAL_FAIL;
			return EVENT_HANDLED;
		}

		if (!ptr->block_data->motion_flags.bit.spindle_running)
		{
			*(ptr->error) = STATUS_SPINDLE_STOPPED;
			return EVENT_HANDLED;
		}

		// update tool
		mc_update_tools(ptr->block_data);

#ifdef TOOL_WAIT_FOR_SPEED
		// wait for spindle to reach the desired speed
		uint16_t programmed_speed = ptr->block_data->spindle;
		uint16_t at_speed_threshold = lroundf(TOOL_WAIT_FOR_SPEED_MAX_ERROR * 0.01f * programmed_speed);

		// wait for tool at speed
		uint32_t start_spindle_time = mcu_millis();
		while (ABS(programmed_speed - tool_get_speed()) > at_speed_threshold)
		{
			if (!cnc_dotasks() || (mcu_millis() - start_spindle_time) > (DELAY_ON_RESUME_SPINDLE * 1000))
			{
				*(ptr->error) = STATUS_SPINDLE_STOPPED;
				return EVENT_HANDLED;
			}
		}
#endif

		float average_rpm = 0;
		for (uint8_t i = 10; i != 0; i--)
		{
			cnc_delay_ms(100);
			average_rpm += (float)encoder_get_rpm(G33_ENCODER);
		}

		average_rpm *= 0.1f;
		// spindle speed ins not valid
		if (average_rpm < 1)
		{
			*(ptr->error) = STATUS_SPINDLE_STOPPED;
			return EVENT_HANDLED;
		}

		// gets the starting point
		float prev_target[AXIS_COUNT];
		mc_get_position(prev_target);
		kinematics_apply_transform(prev_target);
		int32_t prev_step_pos[STEPPER_COUNT];
		kinematics_apply_inverse(prev_target, prev_step_pos);

		// gets the exit point (copies to prevent modifying target vector)
		float line_dist = 0;
		float dir_vect[AXIS_COUNT];
		memcpy(dir_vect, ptr->target, sizeof(dir_vect));
		kinematics_apply_transform(dir_vect);
		int32_t next_step_pos[STEPPER_COUNT];
		kinematics_apply_inverse(dir_vect, next_step_pos);

		// calculates amount of motion vector
		for (uint8_t i = AXIS_COUNT; i != 0;)
		{
			i--;
			dir_vect[i] -= prev_target[i];
			line_dist += dir_vect[i] * dir_vect[i];
		}

		line_dist = sqrtf(line_dist);

		// determines the normalized direction vector
		// and the maximum acceleration
		float max_accel = FLT_MAX;
		for (uint8_t i = AXIS_COUNT; i != 0;)
		{
			i--;
			dir_vect[i] /= line_dist;
			// denormalize max feed rate for each axis
			float denorm_param = g_settings.acceleration[i] / ABS(dir_vect[i]);
			max_accel = MIN(max_accel, denorm_param);
		}

		// calculates the total number of steps in the motion
		uint32_t total_steps = 0;
		for (uint8_t i = AXIS_TO_STEPPERS; i != 0;)
		{
			i--;
			int32_t steps = next_step_pos[i] - prev_step_pos[i];

			steps = ABS(steps);
			if (total_steps < (uint32_t)steps)
			{
				total_steps = steps;
			}
		}

		motion_total_steps = total_steps;

		// from this the factor to convert from RPM to step feed can be obtained
		// step rate = rpm_to_stepfeed_constant * RPM
		rpm_to_stepfeed_constant = ptr->words->ijk[2] * total_steps * MIN_SEC_MULT / line_dist;

		// calculates the feedrate based in the K factor and the programmed spindle RPM
		// spindle is in Rev/min and K is in units(mm) per Rev Rev/min * mm/Rev = mm/min
		float total_revs = line_dist / ptr->words->ijk[2];
		float feed = ptr->words->ijk[2] * average_rpm;
		ptr->block_data->feed = feed;
		ptr->block_data->motion_flags.bit.synched = 1;

		// convert feed to mm/s
		feed *= MIN_SEC_MULT;

		// resets indexes
		spindle_index_counter = 0;
		itp_sync_step_counter = 0;

		// calculates the expected number of steps per revolution
		float steps_per_rev = (float)motion_total_steps / total_revs;
		steps_per_index = lroundf(steps_per_rev);
#ifndef G33_REPLACE_FP_OPERATION_IN_ISR
		steps_per_index_inv = 1.0f / steps_per_index;
#else
		steps_per_index_inv_fixed = ((uint64_t)1 << 32) / (uint64_t)steps_per_index;
#endif

		if (mc_line(ptr->target, ptr->block_data) != STATUS_OK)
		{
			*(ptr->error) = STATUS_CRITICAL_FAIL;
			return EVENT_HANDLED;
		}

#ifdef G33_INDEX_PIN
		g33_wait_for_sync = true;
#else
// attach the index event callback
#if (G33_ENCODER == ENC0)
		HOOK_ATTACH_CALLBACK(enc0_index, spindle_index_cb_handler);
#elif (G33_ENCODER == ENC1)
		HOOK_ATTACH_CALLBACK(enc1_index, spindle_index_cb_handler);
#elif (G33_ENCODER == ENC2)
		HOOK_ATTACH_CALLBACK(enc2_index, spindle_index_cb_handler);
#elif (G33_ENCODER == ENC3)
		HOOK_ATTACH_CALLBACK(enc3_index, spindle_index_cb_handler);
#elif (G33_ENCODER == ENC4)
		HOOK_ATTACH_CALLBACK(enc4_index, spindle_index_cb_handler);
#elif (G33_ENCODER == ENC5)
		HOOK_ATTACH_CALLBACK(enc5_index, spindle_index_cb_handler);
#elif (G33_ENCODER == ENC6)
		HOOK_ATTACH_CALLBACK(enc6_index, spindle_index_cb_handler);
#elif (G33_ENCODER == ENC7)
		HOOK_ATTACH_CALLBACK(enc7_index, spindle_index_cb_handler);
#endif
#endif
		// attach the stepcounter callback
		HOOK_ATTACH_CALLBACK(itp_rt_stepbits, itp_rt_stepcount_cb_handler);

		// flag the spindle index callback that it can start the threading motion
		synched_motion_status = SYNC_READY;

		// wait for the motion to end
		if (itp_sync() != STATUS_OK)
		{
			*(ptr->error) = STATUS_CRITICAL_FAIL;
			return EVENT_HANDLED;
		}

		synched_motion_status = SYNC_DISABLED;

// encoder_dettach_index_cb();
#ifdef G33_INDEX_PIN
		g33_wait_for_sync = false;
#else
#if (G33_ENCODER == ENC0)
		HOOK_RELEASE(enc0_index);
#elif (G33_ENCODER == ENC1)
		HOOK_RELEASE(enc1_index);
#elif (G33_ENCODER == ENC2)
		HOOK_RELEASE(enc2_index);
#elif (G33_ENCODER == ENC3)
		HOOK_RELEASE(enc3_index);
#elif (G33_ENCODER == ENC4)
		HOOK_RELEASE(enc4_index);
#elif (G33_ENCODER == ENC5)
		HOOK_RELEASE(enc5_index);
#elif (G33_ENCODER == ENC6)
		HOOK_RELEASE(enc6_index);
#elif (G33_ENCODER == ENC7)
		HOOK_RELEASE(enc7_index);
#endif
#endif
		HOOK_RELEASE(itp_rt_stepbits);

		*(ptr->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	return EVENT_CONTINUE;
}

#endif

#ifdef ENABLE_MAIN_LOOP_MODULES
bool spindle_sync_update_loop(void *ptr)
{
	if ((synched_motion_status == SYNC_RUNNING))
	{
		float rpm = (float)encoder_get_rpm(G33_ENCODER);

		if (rpm < 1)
		{
			protocol_send_feedback(__romstr__("SPINDLE STOPPED ERROR"));
			cnc_alarm(EXEC_ALARM_SPINDLE_SYNC_FAIL);
			return STATUS_CRITICAL_FAIL;
		}

#ifndef RPM_SYNC_UPDATE_ON_INDEX_ONLY
		int32_t error, rt_pulse_counter, spindle_pulse_counter, index_counter;
		ATOMIC_CODEBLOCK
		{
			index_counter = spindle_index_counter;
			rt_pulse_counter = itp_sync_step_counter;
		}

		// calculate the spindle position
		float counts = (float)index_counter + ((float)encoder_get_position(G33_ENCODER) / g_settings.encoders_resolution[G33_ENCODER]);
		spindle_pulse_counter = lroundf(counts * steps_per_index);

		// if negative the axis are ahead of spindle and need to slow down
		// if positive the axis are behind the spindle and need to speed up.
		error = spindle_pulse_counter - rt_pulse_counter;

		// this updates the interpolator right on the next step and the current motion in the planner
		itp_update_feed(((rpm * rpm_to_stepfeed_constant) + error));

#ifdef G33_DEBUG
		protocol_send_string(MSG_START);
		serial_print_int(rt_pulse_counter);
		serial_putc(':');
		serial_print_int(index_counter);
		serial_putc(':');
		serial_print_int(spindle_pulse_counter);
		serial_putc(':');
		serial_print_int(error);
		serial_putc(':');
		serial_print_flt(rpm);
		protocol_send_string(MSG_END);
#endif
#endif
	}

	return EVENT_CONTINUE;
}

CREATE_EVENT_LISTENER(cnc_dotasks, spindle_sync_update_loop);
#endif

DECL_MODULE(g33)
{
#ifdef ENABLE_PARSER_MODULES
	ADD_EVENT_LISTENER(gcode_parse, g33_parse);
	ADD_EVENT_LISTENER(gcode_exec, g33_exec);
#else
#error "Parser extensions are not enabled. G33 code extension will not work."
#endif
#ifdef ENABLE_MAIN_LOOP_MODULES
	ADD_EVENT_LISTENER(cnc_dotasks, spindle_sync_update_loop);
#else
#error "Main loop extensions are not enabled. G33 code extension will not work."
#endif
#ifdef G33_INDEX_PIN
	ADD_EVENT_LISTENER(input_change, spindle_index_cb_handler);
#endif
#ifndef ENABLE_RT_SYNC_MOTIONS
#error "ENABLE_RT_SYNC_MOTIONS must be enabled to allow realtime step counting in sync motions."
#endif
}