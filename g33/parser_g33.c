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
#include <math.h>

#ifdef ENABLE_PARSER_MODULES

#if (UCNC_MODULE_VERSION < 11501 || UCNC_MODULE_VERSION > 99999)
#error "This module is not compatible with the current version of µCNC"
#endif

#ifndef AXIS_DIR_VECTORS
#ifdef ABC_INDEP_FEED_CALC
#define AXIS_DIR_VECTORS MIN(AXIS_COUNT, 3)
#else
#define AXIS_DIR_VECTORS AXIS_COUNT
#endif
#endif

#ifndef G33_ENCODER
#error "G33 requires to have an assigned encoder"
#endif

#ifndef G33_SYNCHRONIZATION_SPEED
#define G33_SYNCHRONIZATION_SPEED 8
#endif
// enable this to use the encoder pulse as the feedback loop marker/trigger
//  #define G33_FEEDBACK_LOOP_USE_ENC_PULSE

// uncomment to allow data verbose of sync constants
// the message output is
// [MSG:<spindle index counter>:<expected_step_position>:<current_step_position>:<error>:<encoder_rpm>]
// #define G33_DEBUG

#ifndef G33_RPM_SMOOTHING
#define G33_RPM_SMOOTHING
#endif
#ifndef G33_RPM_SMOOTHING_FACTOR
#define G33_RPM_SMOOTHING_FACTOR 0.2f
#endif
#ifndef G33_ERROR_DEADBAND
#define G33_ERROR_DEADBAND 3
#endif

#define SYNC_DISABLED 0
#define SYNC_READY 1
#define SYNC_STARTING 2
#define SYNC_RUNNING 4
#define SYNC_UPDATED 8

static volatile int32_t itp_sync_step_counter;		// step distance counter for synched motions
static volatile uint8_t synched_motion_status;		// synched motion status/phase
static volatile int32_t spindle_index_counter;		// spindle index pulse counter
static volatile int32_t spindle_index_step_counter; // step distance counter when the spindle index pulses
static volatile int32_t spindle_index_time;			// index pulse timestamp in us
static volatile int32_t spindle_index_last_time;	// index pulse previous timestamp in us
static uint32_t steps_per_index;					// motion steps per index pulse
static uint32_t motion_total_steps;
static float motion_total_distance;
static int32_t current_error;
static float rpm_to_stepfeed_constant;
static uint32_t enc_res;
float g33_pitch_k;
#ifdef G33_RPM_SMOOTHING
static float filtered_rpm = 0.0f; // smoothed spindle RPM
#endif

// Counts the stepper pulses and signals when the synchronization rampup is done.
void itp_rt_stepcount_cb_handler(uint8_t stepbits, uint8_t itp_flags)
{
	if (itp_flags & ITP_SYNC)
	{
		synched_motion_status |= SYNC_RUNNING;
		itp_sync_step_counter++;
	}
}

#define _g33_enc_pulse_(X) enc##X##_pulse(void)
#define _g33_enc_pulse(X) _g33_enc_pulse_(X)
#define g33_enc_pulse(X) _g33_enc_pulse(X)

// Counts the steps while executing G33. Changes the sync status to running when a synchronization pulse is received.
void g33_enc_pulse(G33_ENCODER)
{
	if (synched_motion_status > SYNC_DISABLED) // only count pulses if a sync motion is active
	{
		spindle_index_step_counter = itp_sync_step_counter; // get the step count at the time of the encoder pulse
		synched_motion_status |= SYNC_UPDATED;				// signal the update loop that it can update the feedrate based on the new error
	}
}

// Starts the G33 at the receive of an index pulse.
void spindle_index_cb_handler(void)
{
	// this measures the amount of time it took to do X full turns of the tool
	// allow to measure RPM (using the index pin instead of the encoder)
	uint32_t now = mcu_micros();
	spindle_index_last_time = spindle_index_time;
	spindle_index_time = now;
	spindle_index_counter++;
	switch (synched_motion_status)
	{
	case SYNC_READY:
		// Waiting for the index pulse is done. Start synchronized motion
		itp_start(false);
		synched_motion_status = SYNC_STARTING;
		spindle_index_counter = 0;				// reset index counter at the start of the motion
		encoder_reset_position(G33_ENCODER, 0); // reset the synchronisation pulse counter at the start of the motion
		break;
	}
}

#ifdef G33_INDEX_PIN
CREATE_EVENT_LISTENER(input_change, spindle_index_cb_handler);
#endif

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

		enc_res = ((uint32_t)g_settings.encoders_resolution[G33_ENCODER]);

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

#ifdef TOOL_WAIT_FOR_SPEED
		// wait for spindle to reach the desired speed
		uint16_t programmed_speed = ptr->block_data->spindle;
		uint16_t at_speed_threshold = lroundf(TOOL_WAIT_FOR_SPEED_MAX_ERROR * 0.01f * programmed_speed);

		// wait for tool at speed
		uint32_t start_spindle_time = mcu_millis();
		while (ABS(programmed_speed - encoder_get_rpm(G33_ENCODER)) > at_speed_threshold)
		{
			if (!cnc_dotasks() || (mcu_millis() - start_spindle_time) > (DELAY_ON_RESUME_SPINDLE * 1000))
			{
				*(ptr->error) = STATUS_SPINDLE_RPM_ERROR;
				return EVENT_HANDLED;
			}
		}
#endif
		uint32_t t = 0, delta_t = 0;

		for (;;) // wait for index pulse.
		{
			ATOMIC_CODEBLOCK
			{
				delta_t = spindle_index_time;
				t = spindle_index_last_time;
			}
			if (t)
			{
				break;
			}
			cnc_dotasks();
		}
		// calculate the RPM.
		delta_t -= t;
		float index_rpm = 1000000.0f / ((float)delta_t * MIN_SEC_MULT);
		filtered_rpm = index_rpm;

		// spindle speed too slow not valid
		if (index_rpm < 1)
		{
			*(ptr->error) = STATUS_SPINDLE_RPM_ERROR;
			return EVENT_HANDLED;
		}

		// Start the synchronization process by synchronizing the Z-axis to the spindle using a calculated motion ramp up.
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
		motion_total_distance = line_dist;
		float inv_dist = fast_flt_inv(line_dist);

		// determines the normalized direction vector
		// and the maximum acceleration
		float max_feed = FLT_MAX;
		float max_accel = FLT_MAX;

		for (uint8_t i = 0; i < AXIS_DIR_VECTORS; i++)
		{
			float normal_vect = dir_vect[i] * inv_dist;
			dir_vect[i] = normal_vect;
			normal_vect = ABS(normal_vect);
			// denormalize max feed rate for each axis
			float denorm_param = fast_flt_div(g_settings.max_feed_rate[i], normal_vect);
			max_feed = MIN(max_feed, denorm_param);
			max_feed = MIN(max_feed, F_STEP_MAX);
			denorm_param = fast_flt_div(g_settings.acceleration[i], normal_vect);
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

		g33_pitch_k = ptr->words->ijk[2];

		// from this the factor to convert from RPM to step feed can be obtained
		// step rate = rpm_to_stepfeed_constant * RPM
		rpm_to_stepfeed_constant = ptr->words->ijk[2] * total_steps * MIN_SEC_MULT / line_dist;

		// calculates the feedrate based in the K factor and the programmed spindle RPM
		// spindle is in Rev/min and K is in units(mm) per Rev Rev/min * mm/Rev = mm/min
		float total_revs = line_dist / ptr->words->ijk[2];
		float feed = ptr->words->ijk[2] * index_rpm;
		if (feed > max_feed)
		{
			*(ptr->error) = STATUS_MAX_STEP_RATE_EXCEEDED;
			return EVENT_HANDLED;
		}

		// calculates the expected number of steps per revolution
		float steps_per_rev = (float)total_steps / total_revs;
		steps_per_index = lroundf(steps_per_rev*100.0f); // Calcualte factor 100 higher to reduce rounding errors in the integer math of the synchronization process.

		ptr->block_data->feed = feed;
		ptr->block_data->motion_flags.bit.synched = 1;
		ptr->block_data->max_accel = max_accel;

		// convert feed to mm/s
		feed *= MIN_SEC_MULT;

		spindle_index_counter = 0;
		itp_sync_step_counter = 0;

		if (mc_line(ptr->target, ptr->block_data) != STATUS_OK)
		{
			*(ptr->error) = STATUS_CRITICAL_FAIL;
			synched_motion_status = SYNC_DISABLED; // clear the motion status after a soft reset to prevent wrong feedrate updates if a sync motion was active during the reset
			return EVENT_HANDLED;
		}

		// attach the stepcounter callback
		HOOK_ATTACH_CALLBACK(itp_rt_stepbits, itp_rt_stepcount_cb_handler);

		// flag the spindle index callback that it can start the threading motion
		synched_motion_status = SYNC_READY;

		// wait for the motion to end
		if (itp_sync() != STATUS_OK)
		{
			*(ptr->error) = STATUS_CRITICAL_FAIL;
			synched_motion_status = SYNC_DISABLED;
			return EVENT_HANDLED;
		}
		synched_motion_status = SYNC_DISABLED;

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
		HOOK_RELEASE(itp_rt_stepbits);

		*(ptr->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	return EVENT_CONTINUE;
}

#endif

#ifdef ENABLE_MAIN_LOOP_MODULES
bool g33_proto_status(void *args)
{
	if ((g_settings.status_report_mask & 4))
	{
		if ((synched_motion_status > SYNC_DISABLED))
		{
			float error = motion_total_distance * current_error; // calculate the error in mm by multiplying the step error by the distance per step, which is the total distance divided by the total steps
			error /= (float)motion_total_steps;
			proto_printf("|Se:%f", error);
			// proto_printf("|Se:%f;Stat:%d;icnt:%d;scnt:%d", error, synched_motion_status, spindle_index_counter, encoder_get_position(G33_ENCODER));
		}
	}

	return EVENT_CONTINUE;
}
CREATE_EVENT_LISTENER(proto_status, g33_proto_status);

bool spindle_sync_update_loop(void *ptr)
{
	if ((synched_motion_status & SYNC_UPDATED))
	{
		int32_t error, index_step_counter, index_counter;
		uint32_t t = 0, delta_t = 0;
		
		// gets a snapshot of the current spindle index position, and the step position at the time of the index pulse
		ATOMIC_CODEBLOCK
		{
			synchronization_counter = encoder_get_position(G33_ENCODER); // get the sync pulse count.
			synched_motion_status &= ~SYNC_UPDATED;						 // clear the update flag
			index_step_counter = spindle_index_step_counter;			 // get the step count at the time of the index pulse
			delta_t = spindle_index_time;								 // get the time of the index pulse
			t = spindle_index_last_time;								 // get the time of the previous index pulse
		}
		delta_t = encoder_get_delta(G33_ENCODER) * g_settings.encoders_resolution[G33_ENCODER]; //
		float index_rpm = 1000000.0f / ((float)delta_t * MIN_SEC_MULT);							// calculate the current RPM based in the time between index pulses
		if (index_rpm < 1)
		{
			cnc_alarm(EXEC_ALARM_SPINDLE_SYNC_FAIL);
			return EVENT_HANDLED;
		}

// add RPM smoothing factor to dampen RPM variation effects
#ifdef G33_RPM_SMOOTHING
		filtered_rpm = G33_RPM_SMOOTHING_FACTOR * index_rpm + (1.0f - G33_RPM_SMOOTHING_FACTOR) * filtered_rpm;
		index_rpm = filtered_rpm;
				// calculate the spindle position
		int32_t expected_position = index_counter * steps_per_index;
#else
		// calculate the target position based on the actual spindle position
		int32_t expected_position = (index_counter * steps_per_index) / 100.0f; // Correct for th 100 times to high steps per index value.
#endif


#ifdef G33_FEEDBACK_LOOP_USE_ENC_PULSE

		expected_position /= g_settings.encoders_resolution[G33_ENCODER];
		// #endif

		// if negative the axis are ahead of spindle and need to slow down
		// if positive the axis are behind the spindle and need to speed up.
		error = expected_position - index_step_counter;

#if G33_ERROR_DEADBAND > 0
		if (abs(error) < G33_ERROR_DEADBAND)
			error = 0;
#endif
		float gain = G33_SYNCHRONIZATION_GAIN / (1.0f + g33_pitch_k);

		current_error = error;

		// #ifdef G33_DEBUG
		//  cnc_call_rt_command(CMD_CODE_REPORT);
		// #endif

		if (error)
		{
			float new_step_rate = rpm_to_stepfeed_constant * index_rpm;
#ifdef G33_FEEDBACK_LOOP_USE_ENC_PULSE
			new_step_rate += error * gain * g_settings.encoders_resolution[G33_ENCODER];
#else
			new_step_rate += error * gain;
#endif
			// this updates the interpolator right on the next step and the current motion in the planner
			itp_update_feed(new_step_rate);
		}

#ifdef G33_DEBUG
		proto_info("MSG:Spindle turns %ld, expected pos %ld, real pos %ld, error: %ld, rpm %f", index_counter, expected_position, index_step_counter, error, index_rpm);
#endif
	}
	else if (synched_motion_status)
	{
#ifdef G33_DEBUG
		static uint32_t prev_print = 0;
		uint32_t elapsed = mcu_millis() - prev_print;
		if (elapsed > 1000)
		{
			uint32_t t = 0, delta_t = 0;
			ATOMIC_CODEBLOCK
			{
				delta_t = spindle_index_time;
				t = spindle_index_last_time;
			}
			delta_t -= t;
			if (delta_t)
			{
				float index_rpm = 1000000.0f / ((float)delta_t * MIN_SEC_MULT);
				proto_info("MSG:G33 TOOL RPM %f", index_rpm);
			}
			prev_print = mcu_millis();
		}
#endif
	}

	return EVENT_CONTINUE;
}

CREATE_EVENT_LISTENER(cnc_dotasks, spindle_sync_update_loop);
#endif

#if (MCU == MCU_VIRTUAL_WIN)

void emulate_tool_encoder(volatile VIRTUAL_MAP *virtualmap, uint64_t micros)
{
	static uint64_t last_pulse = 0;
	static uint64_t last_index = 0;

	uint32_t rpm = tool_get_setpoint();
	if (synched_motion_status >= SYNC_RUNNING)
	{
		// emulate a 20% rpm drop
		rpm = (uint32_t)(0.8f * rpm);
	}
	uint32_t maxrpm = g_settings.spindle_max_rpm;

	if (rpm == 0 || maxrpm == 0)
		return; // spindle parado → sem pulsos

	// clamp para evitar valores absurdos
	if (rpm > maxrpm)
		rpm = maxrpm;

	/*
		Math:
		1 RPM period = 60.000.000us / RPM
		pulse_interval = period
		index_interval = period * g_settings.encoders_resolution[G33_ENCODER]
	*/
	uint64_t period_us = 30000000ULL / rpm;

	uint64_t index_interval = period_us;
	uint64_t pulse_interval = period_us / g_settings.encoders_resolution[G33_ENCODER];

	uint64_t next_pulse = last_pulse + pulse_interval;
	uint64_t next_index = last_index + index_interval;

	// index pulse
	if (micros >= next_index)
	{
		last_index = next_index;
		virtualmap->inputs ^= 2; // index pin
		mcu_inputs_changed_cb();
	}

	// main pulse
	if (micros >= next_pulse)
	{
		last_pulse = next_pulse;
		virtualmap->inputs ^= 1; // pulse pin
		mcu_inputs_changed_cb();
	}
}
#endif

DECL_MODULE(g33)
{
#if (MCU == MCU_VIRTUAL_WIN)
	mcu_stimul_inputs = emulate_tool_encoder;
#endif
	synched_motion_status = SYNC_DISABLED; // clear the motion status after a soft reset to prevent wrong feedrate updates if a sync motion was active during the reset
#ifdef ENABLE_PARSER_MODULES
	ADD_EVENT_LISTENER(gcode_parse, g33_parse);
	ADD_EVENT_LISTENER(gcode_exec, g33_exec);
#else
#error "Parser extensions are not enabled. G33 code extension will not work."
#endif
#ifdef ENABLE_MAIN_LOOP_MODULES
	ADD_EVENT_LISTENER(proto_status, g33_proto_status);
	ADD_EVENT_LISTENER(cnc_dotasks, spindle_sync_update_loop);
#else
#error "Main loop extensions are not enabled. G33 code extension will not work."
#endif
#ifndef ENABLE_RT_SYNC_MOTIONS
#error "ENABLE_RT_SYNC_MOTIONS must be enabled to allow realtime step counting in sync motions."
#endif
}
