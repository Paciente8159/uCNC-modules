/*
	Name: parser_g33.c
	Description: Implements a parser extension for LinuxCNC G33 for Ã‚ÂµCNC.

	Copyright: Copyright (c) JoÃƒÂ£o Martins
	Author: JoÃƒÂ£o Martins
	Date: 25/11/2022

	Ã‚ÂµCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	Ã‚ÂµCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../cnc.h"
#include "../encoder.h"
#include <stdint.h>
#include <stdbool.h>
#include <float.h>
#include <math.h>

#ifdef G33_ENCODER

#ifdef ENABLE_PARSER_MODULES

#if (UCNC_MODULE_VERSION < 11501 || UCNC_MODULE_VERSION > 99999)
#error "This module is not compatible with the current version of Ã‚ÂµCNC"
#endif

#ifndef AXIS_DIR_VECTORS
#ifdef ABC_INDEP_FEED_CALC
#define AXIS_DIR_VECTORS MIN(AXIS_COUNT, 3)
#else
#define AXIS_DIR_VECTORS AXIS_COUNT
#endif
#endif

#ifndef G33_INDEXES_PER_REV
#define G33_INDEXES_PER_REV 1U
#endif

#if (G33_INDEXES_PER_REV < 1)
#error "G33_INDEXES_PER_REV must be >= 1"
#endif

#ifndef G33_CORRECTION_GAIN
#if (G33_INDEXES_PER_REV > 1)
#define G33_CORRECTION_GAIN 0.25f
#else
#define G33_CORRECTION_GAIN 1.0f
#endif
#endif

// HW counter step-domain debug/test controls.
// Enable G33_HW_COUNTER_FEED_ONLY_TEST to prove that the RPM/virtual-index
// feed update alone is sane before adding phase-error correction.
// #define G33_HW_COUNTER_FEED_ONLY_TEST

#ifndef G33_CORRECTION_MAX_FRAC
// Clamp correction so a bad phase sample cannot collapse the feed to near zero.
// 0.25 means +/-25% of the base step rate per update.
#define G33_CORRECTION_MAX_FRAC 0.10f
#endif

#ifndef G33_DEBUG_EVERY_N
#define G33_DEBUG_EVERY_N 10U
#endif

#ifndef G33_MIN_INDEX_DELTA_US
// Ignore impossible index/virtual-index timing samples. A first virtual update can
// arrive very close to a physical index; using that tiny dt gives fake huge RPM.
#define G33_MIN_INDEX_DELTA_US 1000U
#endif

#ifndef G33_INDEX_WAIT_TIMEOUT_MS
#define G33_INDEX_WAIT_TIMEOUT_MS 3000U
#endif

#ifndef G33_MIN_AXIS_NORM
#define G33_MIN_AXIS_NORM 0.000001f
#endif

// enable this to use the encoder pulse as the feedback loop marker/trigger
//  #define G33_FEEDBACK_LOOP_USE_ENC_PULSE

// third mode: keep index/virtual-index hook only as update trigger,
// but calculate phase error from raw hardware counter encoder counts using a fixed
// steps-per-hardware-counter-count slope calculated once before G33 motion starts.
//  #define G33_FEEDBACK_LOOP_USE_HW_COUNTER

#if defined(G33_FEEDBACK_LOOP_USE_ENC_PULSE) && defined(G33_FEEDBACK_LOOP_USE_HW_COUNTER)
#error "Use only one G33 feedback loop mode"
#endif

// uncomment to allow data verbose of sync constants
// the message output is
// [MSG:<spindle index counter>:<expected_step_position>:<current_step_position>:<error>:<encoder_rpm>]
// #define G33_DEBUG

#define SYNC_DISABLED 0
#define SYNC_READY 1
#define SYNC_STARTING 2
#define SYNC_RUNNING 4
#define SYNC_UPDATED 8

static volatile int32_t itp_sync_step_counter;		// step distance counter for synched motions
static volatile uint8_t synched_motion_status;		// synched motion status/phase
static volatile bool g33_cycle_active;				// true while G33 is preparing, waiting index, or running
static volatile int32_t spindle_index_counter;		// spindle index pulse counter
static int32_t spindle_index_counter_start;			// spindle index pulse initial offset when motion starts
static volatile int32_t spindle_index_step_counter; // step distance counter when the spindle index pulses
static volatile int32_t spindle_index_time;			// index pulse timestamp in us
static volatile int32_t spindle_index_last_time;	// index pulse previous timestamp in us
static uint32_t steps_per_index;					// motion steps per index pulse
#ifdef G33_FEEDBACK_LOOP_USE_ENC_PULSE
static uint32_t update_loop_index_counter; // keeps the last update loop index counter
#endif
#ifdef G33_FEEDBACK_LOOP_USE_HW_COUNTER
static volatile int32_t spindle_hw_counter;          // raw HW counter/encoder count at the current virtual index event
static volatile int32_t spindle_hw_last_counter;     // raw HW counter/encoder count at the previous virtual index event
static volatile int32_t spindle_hw_motion_origin;    // raw HW counter/encoder count at the real G33 start/index phase
static volatile int32_t spindle_step_motion_origin;    // sync step counter at the same real G33 start/index phase
static int32_t steps_per_hw_count_q16;               // fixed motion slope: Z steps per one encoder/hardware counter count, Q16.16
static int32_t hw_phase_offset_steps;                  // learned fixed start delay: raw_expected - actual_step
static uint8_t hw_phase_offset_valid;                  // 0 until first real running virtual-index sample
static float g33_last_good_index_rpm;                    // protects feed from one late/early virtual poll
static int32_t g33_rpm_hw_last;                         // last HW counter sample used for smoothed RPM
static uint32_t g33_rpm_time_last;                      // timestamp of last HW counter RPM sample
static float g33_hw_rpm;                                // smoothed RPM derived from large HW counter deltas
#endif
static uint32_t motion_total_steps;
static float motion_total_distance;
static int32_t current_error;
static float rpm_to_stepfeed_constant;
static uint32_t enc_res;
static uint32_t g33_indexes_per_rev = G33_INDEXES_PER_REV;

static uint32_t g33_get_indexes_per_rev(uint32_t encoder_cpr)
{
#if (G33_ENCODER == ENC0) && defined(ENC0_VIRTUAL_INDEX) && ENC0_VIRTUAL_INDEX && defined(ENC0_VIRTUAL_INDEX_CPR) && (ENC0_VIRTUAL_INDEX_CPR > 0)
	uint32_t virtual_cpr = (uint32_t)ENC0_VIRTUAL_INDEX_CPR;
	return (encoder_cpr > virtual_cpr) ? MAX(1U, (encoder_cpr + (virtual_cpr >> 1)) / virtual_cpr) : 1U;
#else
	(void)encoder_cpr;
	return (uint32_t)G33_INDEXES_PER_REV;
#endif
}

bool g33_els_is_active(void)
{
	bool active;
	ATOMIC_CODEBLOCK
	{
		active = g33_cycle_active || synched_motion_status != SYNC_DISABLED;
	}
	return active;
}

bool g33_els_get_sync(int32_t *start_ec, uint32_t *cpr)
{
	bool active;
	ATOMIC_CODEBLOCK
	{
		active = g33_cycle_active || synched_motion_status != SYNC_DISABLED;
		if (start_ec)
		{
#ifdef G33_FEEDBACK_LOOP_USE_HW_COUNTER
			*start_ec = spindle_hw_motion_origin;
#else
			*start_ec = spindle_index_counter_start;
#endif
		}
		if (cpr)
		{
			*cpr = enc_res;
		}
	}
	return active;
}

#if (MCU == MCU_VIRTUAL_WIN)
// used with the virtual emulator to simulate pulses
void mcu_stimul_inputs(volatile VIRTUAL_MAP *virtualmap, uint64_t micros)
{
	static uint64_t last_stim = 0, last_stimsync = 0;
	uint64_t next_stim = last_stim + 120000;			 // 120RPM
	uint64_t next_stimsync = last_stimsync + 120000 / 4; // 120RPM

	if (micros >= next_stimsync)
	{
		last_stimsync = next_stimsync;
		virtualmap->inputs ^= 1; // index pin
		mcu_inputs_changed_cb();
	}

	if (micros >= next_stim)
	{
		last_stim = next_stim;
		virtualmap->inputs ^= 2; // index pin
		mcu_inputs_changed_cb();
	}
}
#endif

void itp_rt_stepcount_cb_handler(uint8_t stepbits, uint8_t itp_flags)
{
	if (itp_flags & ITP_SYNC)
	{
		if (itp_flags & ITP_CONST)
		{
			synched_motion_status |= SYNC_RUNNING;
		}
		else
		{
			synched_motion_status &= ~SYNC_RUNNING;
		}
		itp_sync_step_counter++;
	}
}

#ifdef G33_FEEDBACK_LOOP_USE_ENC_PULSE
#define _g33_enc_pulse_(X) enc##X##_pulse(void)
#define _g33_enc_pulse(X) _g33_enc_pulse_(X)
#define g33_enc_pulse(X) _g33_enc_pulse(X)

void g33_enc_pulse(G33_ENCODER)
{
	if (synched_motion_status >= SYNC_RUNNING)
	{
		spindle_index_step_counter = itp_sync_step_counter;
		synched_motion_status |= SYNC_UPDATED;
	}
}
#endif

void spindle_index_cb_handler(void)
{
	// This hook may be fired by a physical index, by an ESP32 hardware counter virtual-index
	// generator, or by another encoder backend. In hardware counter mode the hook is
	// only a timing/update trigger; spindle phase itself comes from raw encoder counts.
	uint32_t now = mcu_micros();
	int32_t index = spindle_index_counter;

#ifdef G33_FEEDBACK_LOOP_USE_HW_COUNTER
	int32_t hw_now = encoder_get_position(G33_ENCODER);
	spindle_hw_last_counter = spindle_hw_counter;
	spindle_hw_counter = hw_now;
#endif

	spindle_index_last_time = spindle_index_time;
	spindle_index_time = now;
	index++;

	switch (synched_motion_status)
	{
	case SYNC_READY:
		if (itp_is_empty())
		{
#ifdef G33_DEBUG
			static uint32_t g33_empty_start_last_print = 0;
			if ((mcu_millis() - g33_empty_start_last_print) > 500U)
			{
				g33_empty_start_last_print = mcu_millis();
#ifdef G33_FEEDBACK_LOOP_USE_HW_COUNTER
				proto_info("MSG:G33 defer start hw=%ld idx=%ld empty=1", hw_now, index);
#else
				proto_info("MSG:G33 defer start idx=%ld empty=1", index);
#endif
			}
#endif
			break;
		}
		// The aligned index/virtual-index starts synchronized motion.
		// For HW counter mode this is also the zero phase of this G33 move.
		itp_start(false);
#ifdef G33_DEBUG
#ifdef G33_FEEDBACK_LOOP_USE_HW_COUNTER
		proto_info("MSG:G33 start hw=%ld idx=%ld empty=%u",
		           hw_now, index, (unsigned)itp_is_empty());
#else
		proto_info("MSG:G33 start idx=%ld empty=%u",
		           index, (unsigned)itp_is_empty());
#endif
#endif
		synched_motion_status = SYNC_STARTING;
		index = spindle_index_counter_start;
#ifdef G33_FEEDBACK_LOOP_USE_HW_COUNTER
		spindle_hw_motion_origin = hw_now;
		spindle_step_motion_origin = itp_sync_step_counter;
		spindle_hw_last_counter = hw_now;
		spindle_hw_counter = hw_now;
#endif
#ifdef G33_FEEDBACK_LOOP_USE_ENC_PULSE
		encoder_reset_position(G33_ENCODER, index * enc_res); // syncs the pulse counter with the index counter
#endif
		break;
	default:
#ifndef G33_FEEDBACK_LOOP_USE_ENC_PULSE
		if (index > 0 && synched_motion_status >= SYNC_RUNNING)
		{
			synched_motion_status |= SYNC_UPDATED;
			// Store the step position at the same update event as the spindle sample.
			spindle_index_step_counter = itp_sync_step_counter;
		}
#endif
		break;
	}

	spindle_index_counter = index;
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
		g33_indexes_per_rev = g33_get_indexes_per_rev(enc_res);
		if (g33_indexes_per_rev < 1)
		{
			g33_indexes_per_rev = 1;
		}

		// Hard reset all G33 runtime state before attaching the index hook.
		// This prevents second-run stale dt/rpm samples like TOOL RPM 1.5.
		// reset virtual index too

		encoder_virtual_index_clear(G33_ENCODER);

		ATOMIC_CODEBLOCK
		{
			synched_motion_status = SYNC_DISABLED;
			spindle_index_counter = 0;
			spindle_index_counter_start = 0;
			spindle_index_step_counter = 0;
			spindle_index_time = 0;
			spindle_index_last_time = 0;
#ifdef G33_FEEDBACK_LOOP_USE_HW_COUNTER
			spindle_hw_counter = encoder_get_position(G33_ENCODER);
			spindle_hw_last_counter = spindle_hw_counter;
			spindle_hw_motion_origin = spindle_hw_counter;
			spindle_step_motion_origin = itp_sync_step_counter;
			hw_phase_offset_steps = 0;
			hw_phase_offset_valid = 0;
			g33_last_good_index_rpm = 0.0f;
			// Reset smooth HW-counter RPM tracking. These are file-scope globals used by
			// spindle_sync_update_loop(); local statics here would not reset the loop state.
			g33_rpm_hw_last = encoder_get_position(G33_ENCODER);
			g33_rpm_time_last = mcu_micros();
			g33_hw_rpm = 0.0f; // will be seeded after the initial index RPM measurement
#endif
		}

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

		// this code can be removed as the initial reading of the G33 RPM ensures the spindle is running

		// if (!ptr->block_data->motion_flags.bit.spindle_running)
		// {
		// 	*(ptr->error) = STATUS_SPINDLE_RPM_ERROR;
		// 	return EVENT_HANDLED;
		// }

		// // update tool
		// mc_update_tools(ptr->block_data);
		g33_cycle_active = true;

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
				g33_cycle_active = false;
				return EVENT_HANDLED;
			}
		}
#endif
		uint32_t t = 0, delta_t = 0;
#ifdef G33_FEEDBACK_LOOP_USE_HW_COUNTER
		float index_rpm = 0.0f;
		{
			uint16_t encoder_rpm = encoder_get_rpm(G33_ENCODER);
			if (encoder_rpm > 0)
			{
				index_rpm = (float)encoder_rpm;
			}
		}
		{
			int32_t hw_start = encoder_get_position(G33_ENCODER);
			uint32_t hw_time_start = mcu_micros();
			uint32_t hw_timeout = mcu_millis();
			uint32_t hw_time_dt = 0;
			int32_t hw_dt = 0;
			uint32_t hw_min_counts = enc_res / 20U;

			if (hw_min_counts < 20U)
			{
				hw_min_counts = 20U;
			}

#ifdef G33_DEBUG
			proto_info("MSG:G33 measuring hw rpm enc_res=%lu min_counts=%lu",
			           (unsigned long)enc_res, (unsigned long)hw_min_counts);
#endif
			do
			{
				if (!cnc_dotasks())
				{
					*(ptr->error) = STATUS_CRITICAL_FAIL;
					g33_cycle_active = false;
					return EVENT_HANDLED;
				}

				int32_t hw_now = encoder_get_position(G33_ENCODER);
				uint32_t hw_time_now = mcu_micros();
				hw_dt = hw_now - hw_start;
				hw_time_dt = hw_time_now - hw_time_start;

				if (hw_dt < 0)
				{
					hw_dt = -hw_dt;
				}
			} while ((hw_time_dt < 200000U || (uint32_t)hw_dt < hw_min_counts) && (mcu_millis() - hw_timeout) < 1500U);

			if (index_rpm < 1.0f && hw_time_dt && hw_dt)
			{
				index_rpm = ((float)hw_dt * 60000000.0f) / ((float)hw_time_dt * (float)enc_res);
			}
#ifdef G33_DEBUG
			proto_info("MSG:G33 hw rpm sample dt=%lu counts=%ld rpm=%f enc_rpm=%u idx_count=%ld",
			           (unsigned long)hw_time_dt, (long)hw_dt, index_rpm,
			           (unsigned)encoder_get_rpm(G33_ENCODER), (long)spindle_index_counter);
#endif
		}
#else
		uint32_t index_wait_start = mcu_millis();
		uint32_t index_wait_last_print = index_wait_start;

#ifdef G33_DEBUG
		proto_info("MSG:G33 waiting for index sample idx_rev=%lu enc_res=%lu",
		           (unsigned long)g33_indexes_per_rev, (unsigned long)enc_res);
#endif
		for (;;)
		{
			ATOMIC_CODEBLOCK
			{
				delta_t = spindle_index_time;
				t = spindle_index_last_time;
			}
			if (t)
			{
				uint32_t sample_dt = delta_t - t;
				if (sample_dt >= G33_MIN_INDEX_DELTA_US)
				{
					delta_t = sample_dt;
					break;
				}
			}
			if ((mcu_millis() - index_wait_start) > G33_INDEX_WAIT_TIMEOUT_MS)
			{
#ifdef G33_DEBUG
				proto_info("MSG:G33 index wait timeout t=%lu last=%lu count=%ld",
				           (unsigned long)delta_t, (unsigned long)t, (long)spindle_index_counter);
#endif
				*(ptr->error) = STATUS_SPINDLE_RPM_ERROR;
				g33_cycle_active = false;
				return EVENT_HANDLED;
			}
#ifdef G33_DEBUG
			if ((mcu_millis() - index_wait_last_print) > 1000U)
			{
				index_wait_last_print = mcu_millis();
				proto_info("MSG:G33 still waiting index t=%lu last=%lu count=%ld",
				           (unsigned long)delta_t, (unsigned long)t, (long)spindle_index_counter);
			}
#endif
			cnc_dotasks();
		}

		float index_rpm = 1000000.0f / ((float)delta_t * MIN_SEC_MULT * (float)g33_indexes_per_rev);
#endif

		// spindle speed ins not valid
		if (index_rpm < 1)
		{
			*(ptr->error) = STATUS_SPINDLE_RPM_ERROR;
			g33_cycle_active = false;
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
			if (normal_vect < G33_MIN_AXIS_NORM)
			{
				continue;
			}
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
			g33_cycle_active = false;
			return EVENT_HANDLED;
		}

		// calculates the expected number of steps per revolution
		float steps_per_rev = (float)total_steps / total_revs;
		steps_per_index = lroundf(steps_per_rev / (float)g33_indexes_per_rev);
#ifdef G33_FEEDBACK_LOOP_USE_HW_COUNTER
		steps_per_hw_count_q16 = (enc_res) ? (int32_t)lroundf((steps_per_rev * 65536.0f) / (float)enc_res) : 0;
#ifdef G33_DEBUG
		proto_info("MSG:G33 init total_steps=%lu total_revs=%f steps_rev=%f enc_res=%lu idx_rev=%lu spi=%lu q16=%ld rpm_const=%f feed=%f",
		           (unsigned long)total_steps, total_revs, steps_per_rev, (unsigned long)enc_res,
		           (unsigned long)g33_indexes_per_rev, (unsigned long)steps_per_index,
		           steps_per_hw_count_q16, rpm_to_stepfeed_constant, feed);
#endif
#endif

		ptr->block_data->feed = feed;
		ptr->block_data->motion_flags.bit.synched = 1;
		ptr->block_data->max_accel = max_accel;

		// convert feed to mm/s
		feed *= MIN_SEC_MULT;

		// The thread feed is given by:
		// vf = (RPM / 60) * K
		// and the thread position at any given time t(s) is expressed as
		// x = vf * t
		// on a linear acceleration the motion will ALWAYS stay behind because of the acceleration phase
		// the real thread path is given by:
		// xreal = (vf^2) / (2 * a) + vf * (t - tacc)
		// we can also express this as the ideal position version minus the acceleration triangle area (valid for constant acceleration)
		// xreal = vf * t - (vf^2) / (2 * a)
		// the error is expressed as
		// e = x - xreal = (vf^2) / (2 * a)
		// the thread correct position is a multiple of pitch K
		// the motion will always lag behind a bit. The acceleration can be tuned so that the lag is exctly P multiples of pitch K
		// e = P * K
		// replacing the value of error
		// (vf^2) / (2 * a) = P * K
		// solving this for acceleration we get
		// a = (vf^2) / (2 * P * K)
		// replacing vf from the first equation we get
		// a = (K * RPM^2) / (7200 * P)

		// calculate the minimum acceleration time
		if (!isfinite(max_accel) || max_accel <= 0.0f)
		{
			max_accel = g_settings.acceleration[AXIS_TOOL];
			if (!isfinite(max_accel) || max_accel <= 0.0f)
			{
				max_accel = 1.0f;
			}
		}

		float accel_time = feed / max_accel;
		// calculate the time per revolution
		float rev_time = 60 / index_rpm;

		// calculate the minimum amount of revs needed to reach the target speed
		float p_revs = ceilf(accel_time / rev_time);
		if (!isfinite(p_revs) || p_revs < 1.0f)
		{
			p_revs = 1.0f;
		}
		// calculate the new acceleration given an additional revolution to compensate for the lag
		float new_accel = ptr->words->ijk[2] * index_rpm * index_rpm / (p_revs * 7200);
		if (!isfinite(new_accel) || new_accel <= 0.0f)
		{
			new_accel = max_accel;
		}
		float g33_motion_accel = new_accel;
		ptr->block_data->max_accel = new_accel;

		spindle_index_counter_start = -(int32_t)lroundf(p_revs * (float)g33_indexes_per_rev);

		// resets indexes
		spindle_index_counter = 0;
#ifdef G33_FEEDBACK_LOOP_USE_HW_COUNTER
		spindle_hw_counter = encoder_get_position(G33_ENCODER);
		spindle_hw_last_counter = spindle_hw_counter;
		spindle_hw_motion_origin = spindle_hw_counter;
		spindle_step_motion_origin = itp_sync_step_counter;
		hw_phase_offset_steps = 0;
		hw_phase_offset_valid = 0;
		g33_last_good_index_rpm = index_rpm;
		// Seed smooth HW-counter RPM tracking with the already validated initial RPM.
		g33_rpm_hw_last = encoder_get_position(G33_ENCODER);
		g33_rpm_time_last = mcu_micros();
		g33_hw_rpm = index_rpm;
#endif
		spindle_index_time = 0;
		spindle_index_last_time = 0;
		itp_sync_step_counter = 0;

// resets the correction loop
#ifdef G33_FEEDBACK_LOOP_USE_ENC_PULSE
		update_loop_index_counter = 0;
#endif

		if (mc_line(ptr->target, ptr->block_data) != STATUS_OK)
		{
			*(ptr->error) = STATUS_CRITICAL_FAIL;
			g33_cycle_active = false;
			return EVENT_HANDLED;
		}

		// attach the stepcounter callback
		HOOK_ATTACH_CALLBACK(itp_rt_stepbits, itp_rt_stepcount_cb_handler);

		// Build the first synchronized segment before arming the index start.
		// cnc_dotasks() runs cnc_io_dotasks before itp_run(); if SYNC_READY is
		// armed first, an encoder index can call itp_start() while no segment is
		// available yet, leaving the move stuck in SYNC_STARTING.
#ifndef ENABLE_ITP_FEED_TASK
		for (uint8_t build_wait = 0; itp_is_empty() && !planner_buffer_is_empty() && build_wait < 10; build_wait++)
		{
			if (!cnc_dotasks())
			{
				*(ptr->error) = STATUS_CRITICAL_FAIL;
				g33_cycle_active = false;
				return EVENT_HANDLED;
			}
		}
#endif

#ifdef G33_DEBUG
		proto_info("MSG:G33 armed empty=%u planner=%u start_idx=%ld accel_revs=%f g33_accel=%f",
		           (unsigned)itp_is_empty(), (unsigned)!planner_buffer_is_empty(),
		           (long)spindle_index_counter_start, p_revs, g33_motion_accel);
#endif

		// flag the spindle index callback that it can start the threading motion
		synched_motion_status = SYNC_READY;

		// wait for the motion to end
		if (itp_sync() != STATUS_OK)
		{
			*(ptr->error) = STATUS_CRITICAL_FAIL;
			g33_cycle_active = false;
			return EVENT_HANDLED;
		}

		synched_motion_status = SYNC_DISABLED;
		g33_cycle_active = false;

// encoder_dettach_index_cb();
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
		if ((synched_motion_status >= SYNC_RUNNING))
		{
			float error = motion_total_distance * current_error;
			error /= (float)motion_total_steps;
			proto_printf("|Se:%f", error);
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
#ifdef G33_FEEDBACK_LOOP_USE_HW_COUNTER
		int32_t hw_counter_snapshot = 0;
		int32_t hw_origin_snapshot = 0;
		int32_t step_origin_snapshot = 0;
#endif
		// Get one coherent snapshot of the update event.
		// In normal/virtual-index mode index_counter is the software virtual slot.
		// In HW counter mode the software slot only triggers update timing; raw HW count is the phase ruler.
		ATOMIC_CODEBLOCK
		{
#ifdef G33_FEEDBACK_LOOP_USE_ENC_PULSE
			index_counter = encoder_get_position(G33_ENCODER);
#else
			index_counter = spindle_index_counter;
#endif
#ifdef G33_FEEDBACK_LOOP_USE_HW_COUNTER
			hw_counter_snapshot = spindle_hw_counter;
			hw_origin_snapshot = spindle_hw_motion_origin;
			step_origin_snapshot = spindle_step_motion_origin;
#endif
			synched_motion_status &= ~SYNC_UPDATED;
			index_step_counter = spindle_index_step_counter;
			delta_t = spindle_index_time;
			t = spindle_index_last_time;
		}

#ifdef G33_FEEDBACK_LOOP_USE_HW_COUNTER
		// Calculate smooth RPM from HW counter position deltas. State is file-scope
		// so G33 start can reset/seed it; otherwise the first sample after index can
		// compare against stale/zero values and report absurd RPM.
		int32_t hw_now = encoder_get_position(G33_ENCODER);
		uint32_t time_now = mcu_micros();

		int32_t hw_dt = hw_now - g33_rpm_hw_last;
		uint32_t time_dt = time_now - g33_rpm_time_last;

		if (hw_dt < 0)
			hw_dt = -hw_dt;

		if (g33_rpm_time_last && time_dt > 0 && hw_dt >= (g_settings.encoders_resolution[G33_ENCODER] / 10))
		{
			g33_hw_rpm =
				((float)hw_dt * 60000000.0f) /
				((float)time_dt * g_settings.encoders_resolution[G33_ENCODER]);
		}

		g33_rpm_hw_last = hw_now;
		g33_rpm_time_last = time_now;
#endif

#ifdef G33_FEEDBACK_LOOP_USE_ENC_PULSE
		delta_t = encoder_get_delta(G33_ENCODER) * g_settings.encoders_resolution[G33_ENCODER];
#else
		delta_t -= t;
#endif
		if (delta_t < G33_MIN_INDEX_DELTA_US)
		{
			return EVENT_CONTINUE;
		}
		if (!delta_t)
		{
			return EVENT_CONTINUE;
		}
#ifdef G33_FEEDBACK_LOOP_USE_HW_COUNTER
		float index_rpm = g33_hw_rpm;
#else
		float index_rpm = 1000000.0f / ((float)delta_t * MIN_SEC_MULT * (float)g33_indexes_per_rev);
#endif
#ifdef G33_FEEDBACK_LOOP_USE_HW_COUNTER
		// cnc_io_dotasks can occasionally poll late/early. hardware counter phase correction can
		// tolerate this, but the RPM-derived base feed should not jump violently.
		// Clamp a single bad timing sample to +/-25% around the previous good RPM
		// and then apply a light low-pass.
		if (g33_last_good_index_rpm > 1.0f)
		{
			float rpm_min = g33_last_good_index_rpm * 0.75f;
			float rpm_max = g33_last_good_index_rpm * 1.25f;
			if (index_rpm < rpm_min)
			{
				index_rpm = rpm_min;
			}
			else if (index_rpm > rpm_max)
			{
				index_rpm = rpm_max;
			}
			index_rpm = (g33_last_good_index_rpm * 0.75f) + (index_rpm * 0.25f);
		}
		g33_last_good_index_rpm = index_rpm;
#endif
		if (index_rpm < 1)
		{
			cnc_alarm(EXEC_ALARM_SPINDLE_SYNC_FAIL);
			return STATUS_CRITICAL_FAIL;
		}

		// Calculate the expected motion position.
		// Normal virtual-index mode: expected = virtual_slot * steps_per_virtual_slot.
		// hardware counter step-domain mode: expected sync steps are derived directly from
		// raw hardware counter delta using a fixed Q16.16 slope calculated before motion.
		// No mm coordinates, no kinematics conversion, no float in this loop.
		int32_t expected_position;
#ifdef G33_FEEDBACK_LOOP_USE_HW_COUNTER
		{
			// Fixed-origin hardware counter phase mode.
			// hardware counter origin is captured when G33 is armed/started. This gives a stable
			// spindle ruler, but the actual synced step stream starts later because the
			// planner waits for the aligned physical/virtual index and acceleration path.
			// Therefore the first real running sample learns one fixed phase offset:
			//     offset = raw_expected_steps - actual_steps
			// Later updates subtract only this constant and correct only drift.
			int32_t hw_delta = hw_counter_snapshot - hw_origin_snapshot;


			if (hw_delta < 0)
				hw_delta = -hw_delta;

			int64_t expected_q16 = ((int64_t)hw_delta * (int64_t)steps_per_hw_count_q16);
			int32_t raw_expected_position;

			// Round signed Q16.16 to integer steps and add the step origin captured at start.
			if (expected_q16 >= 0)
			{
				raw_expected_position = step_origin_snapshot + (int32_t)((expected_q16 + 32768) >> 16);
			}
			else
			{
				raw_expected_position = step_origin_snapshot - (int32_t)(((-expected_q16) + 32768) >> 16);
			}

			if (!hw_phase_offset_valid)
			{
				hw_phase_offset_steps = raw_expected_position - index_step_counter;
				hw_phase_offset_valid = 1;
			}

			expected_position = raw_expected_position - hw_phase_offset_steps;
		}
#else
		expected_position = index_counter * steps_per_index;
#ifdef G33_FEEDBACK_LOOP_USE_ENC_PULSE
		expected_position /= g_settings.encoders_resolution[G33_ENCODER];
#endif
#endif

		// if negative the axis are ahead of spindle and need to slow down
		// if positive the axis are behind the spindle and need to speed up.
		error = expected_position - index_step_counter;
		current_error = error;

		// #ifdef G33_DEBUG
		//  cnc_call_rt_command(CMD_CODE_REPORT);
		// #endif

		float base_step_rate = rpm_to_stepfeed_constant * index_rpm;
		float correction_step_rate = 0.0f;

#ifdef G33_FEEDBACK_LOOP_USE_HW_COUNTER
#ifndef G33_HW_COUNTER_FEED_ONLY_TEST
		correction_step_rate = ((float)error * G33_CORRECTION_GAIN);
		{
			float max_correction = base_step_rate * G33_CORRECTION_MAX_FRAC;
			if (correction_step_rate > max_correction)
			{
				correction_step_rate = max_correction;
			}
			else if (correction_step_rate < -max_correction)
			{
				correction_step_rate = -max_correction;
			}
		}
#endif
		// In hardware counter mode always update from the measured virtual-index period.
		// The phase correction is optional/clamped; the RPM feed part must first prove sane.
		itp_update_feed(base_step_rate + correction_step_rate);
#else
		if (error)
		{
#ifdef G33_FEEDBACK_LOOP_USE_ENC_PULSE
			correction_step_rate = error * g_settings.encoders_resolution[G33_ENCODER];
#else
			correction_step_rate = ((float)error * G33_CORRECTION_GAIN);
#endif
			// this updates the interpolator right on the next step and the current motion in the planner
			itp_update_feed(base_step_rate + correction_step_rate);
		}
#endif

#ifdef G33_DEBUG
		{
			static uint32_t debug_div = 0;
			debug_div++;
			if (debug_div >= G33_DEBUG_EVERY_N)
			{
				debug_div = 0;
#ifdef G33_FEEDBACK_LOOP_USE_HW_COUNTER
				/*proto_info("MSG:G33 hw=%ld org=%ld sorg=%ld dt=%lu rpm=%f base=%f corr=%f q16=%ld off=%ld exp=%ld real=%ld err=%ld",
			           hw_counter_snapshot, hw_origin_snapshot, step_origin_snapshot, (unsigned long)delta_t, index_rpm, base_step_rate, correction_step_rate,
			           steps_per_hw_count_q16, hw_phase_offset_steps, expected_position, index_step_counter, error);*/

				proto_info("MSG:G33 exp=%ld real=%ld err=%ld",
			             expected_position, index_step_counter, error);
#else
				proto_info("MSG:G33 idx=%ld dt=%lu rpm=%f base=%f corr=%f exp=%ld real=%ld err=%ld",
				           index_counter, (unsigned long)delta_t, index_rpm, base_step_rate, correction_step_rate, expected_position, index_step_counter, error);
#endif
			}
		}
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
#ifdef G33_FEEDBACK_LOOP_USE_HW_COUNTER
				float index_rpm = g33_hw_rpm;
#else
				float index_rpm = 1000000.0f / ((float)delta_t * MIN_SEC_MULT * (float)g33_indexes_per_rev);
#endif
#ifdef G33_FEEDBACK_LOOP_USE_HW_COUNTER
				{
					int32_t hw_now = encoder_get_position(G33_ENCODER);
					uint32_t now = mcu_micros();
					int32_t hw_dt = hw_now - g33_rpm_hw_last;
					uint32_t time_dt = now - g33_rpm_time_last;
					if (hw_dt < 0)
					{
						hw_dt = -hw_dt;
					}
					float hw_rpm = (time_dt && enc_res) ? (((float)hw_dt * 60000000.0f) / ((float)time_dt * (float)enc_res)) : 0.0f;
					proto_info("MSG:G33 wait st=%u idxrpm=%f hwrpm=%f hw=%ld empty=%u",
					           (unsigned)synched_motion_status, index_rpm, hw_rpm,
					           hw_now, (unsigned)itp_is_empty());
				}
#else
				proto_info("MSG:G33 wait st=%u rpm=%f empty=%u",
				           (unsigned)synched_motion_status, index_rpm,
				           (unsigned)itp_is_empty());
#endif
			}
			prev_print = mcu_millis();
		}
#endif
	}

	return EVENT_CONTINUE;
}

#ifdef G33_USE_CNC_DOTASKS
CREATE_EVENT_LISTENER(cnc_dotasks, spindle_sync_update_loop);
#else
CREATE_EVENT_LISTENER(cnc_io_dotasks, spindle_sync_update_loop);
#endif
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
	ADD_EVENT_LISTENER(proto_status, g33_proto_status);
#ifdef G33_USE_CNC_DOTASKS
	ADD_EVENT_LISTENER(cnc_dotasks, spindle_sync_update_loop);
#else
	ADD_EVENT_LISTENER(cnc_io_dotasks, spindle_sync_update_loop);
#endif
#else
#error "Main loop extensions are not enabled. G33 code extension will not work."
#endif
#ifndef ENABLE_RT_SYNC_MOTIONS
#error "ENABLE_RT_SYNC_MOTIONS must be enabled to allow realtime step counting in sync motions."
#endif
}

#else

DECL_MODULE(g33)
{
}

#endif
