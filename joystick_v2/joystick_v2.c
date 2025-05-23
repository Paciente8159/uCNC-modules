/*
	Name: joystick_v2.c
	Description: Adds support for jogging using an analog joystick. Based on the joystick module by Patryk Mierzyński

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 23-05-2025

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the GNU General Public License for more details.
*/

#include "../../cnc.h"

#if (UCNC_MODULE_VERSION < 11201 || UCNC_MODULE_VERSION > 99999)
#error "This module is not compatible with the current version of µCNC"
#endif

// Pin definitions
#ifndef JOYSTICK_INPUT_0
#define JOYSTICK_INPUT_0 UNDEF_PIN
#endif

#ifndef JOYSTICK_INPUT_1
#define JOYSTICK_INPUT_1 UNDEF_PIN
#endif

#ifndef JOYSTICK_INPUT_2
#define JOYSTICK_INPUT_2 UNDEF_PIN
#endif

#ifndef JOYSTICK_INPUT_3
#define JOYSTICK_INPUT_3 UNDEF_PIN
#endif

#ifndef JOYSTICK_INPUT_4
#define JOYSTICK_INPUT_4 UNDEF_PIN
#endif

#ifndef JOYSTICK_INPUT_5
#define JOYSTICK_INPUT_5 UNDEF_PIN
#endif

// Axis mapping
#ifndef JOYSTICK_INPUT_0_AXIS
#define JOYSTICK_INPUT_0_AXIS 255
#endif

#ifndef JOYSTICK_INPUT_1_AXIS
#define JOYSTICK_INPUT_1_AXIS 255
#endif

#ifndef JOYSTICK_INPUT_2_AXIS
#define JOYSTICK_INPUT_2_AXIS 255
#endif

#ifndef JOYSTICK_INPUT_3_AXIS
#define JOYSTICK_INPUT_3_AXIS 255
#endif

#ifndef JOYSTICK_INPUT_4_AXIS
#define JOYSTICK_INPUT_4_AXIS 255
#endif

#ifndef JOYSTICK_INPUT_5_AXIS
#define JOYSTICK_INPUT_5_AXIS 255
#endif

// Axis count
#ifndef JOYSTICK_AXIS_COUNT
#if JOYSTICK_INPUT_5_AXIS != 255
#define JOYSTICK_AXIS_COUNT 6
#elif JOYSTICK_INPUT_4_AXIS != 255
#define JOYSTICK_AXIS_COUNT 5
#elif JOYSTICK_INPUT_3_AXIS != 255
#define JOYSTICK_AXIS_COUNT 4
#elif JOYSTICK_INPUT_2_AXIS != 255
#define JOYSTICK_AXIS_COUNT 3
#elif JOYSTICK_INPUT_1_AXIS != 255
#define JOYSTICK_AXIS_COUNT 2
#elif JOYSTICK_INPUT_0_AXIS != 255
#define JOYSTICK_AXIS_COUNT 1
#else
#define JOYSTICK_AXIS_COUNT 0
#endif
#endif

// Other configuration options
#ifndef JOYSTICK_LINEAR_RESPONSE
#define JOYSTICK_LINEAR_RESPONSE true
#endif

#ifndef JOYSTICK_ALLOW_MULTI_AXIS_JOG
#define JOYSTICK_ALLOW_MULTI_AXIS_JOG 0
#endif

#ifndef JOYSTICK_DEAD_ZONE
#define JOYSTICK_DEAD_ZONE 0.0f
#endif

#ifndef JOYSTICK_MAX_FEED_RATE
#define JOYSTICK_MAX_FEED_RATE 100
#endif

#ifndef JOYSTICK_DELTA_TIME_MS
#define JOYSTICK_DELTA_TIME_MS 50
#endif

// Other macros
#define JOYSTICK_AXIS_LETTER_0 'X'
#define JOYSTICK_AXIS_LETTER_1 'Y'
#define JOYSTICK_AXIS_LETTER_2 'Z'
#define JOYSTICK_AXIS_LETTER_3 'A'
#define JOYSTICK_AXIS_LETTER_4 'B'
#define JOYSTICK_AXIS_LETTER_5 'C'

#define __helper_ex__(left, mid, right) left##mid##right
#define __helper__(left, mid, right) __helper_ex__(left, mid, right)
#define JOYSTICK_MAP_AXIS(axis) __helper__(JOYSTICK_AXIS_LETTER_, axis, )

#if JOYSTICK_AXIS_COUNT != 0

// set the joystick sensitivity (value between 1 and 100)
#ifndef JOYSTICK_SENSITIVITY
#define JOYSTICK_SENSITIVITY 10
#endif
#if (JOYSTICK_SENSITIVITY < 1) || (JOYSTICK_SENSITIVITY > 100)
#error "Joystick sensitivity value should be between 1 and 100"
#endif
#define JOYSTICK_SENSITIVITY_REV ((float)(1.0f / JOYSTICK_SENSITIVITY))

static const char joystick_axis_map[JOYSTICK_AXIS_COUNT] = {
		JOYSTICK_MAP_AXIS(JOYSTICK_INPUT_0_AXIS),
#if JOYSTICK_AXIS_COUNT > 1
		JOYSTICK_MAP_AXIS(JOYSTICK_INPUT_1_AXIS),
#endif
#if JOYSTICK_AXIS_COUNT > 2
		JOYSTICK_MAP_AXIS(JOYSTICK_INPUT_2_AXIS),
#endif
#if JOYSTICK_AXIS_COUNT > 3
		JOYSTICK_MAP_AXIS(JOYSTICK_INPUT_3_AXIS),
#endif
#if JOYSTICK_AXIS_COUNT > 4
		JOYSTICK_MAP_AXIS(JOYSTICK_INPUT_4_AXIS),
#endif
#if JOYSTICK_AXIS_COUNT > 5
		JOYSTICK_MAP_AXIS(JOYSTICK_INPUT_5_AXIS),
#endif
};

__attribute__((weak)) float joystick_virtual_input(uint8_t index)
{
	return 0;
}

static float joystick_read_axis(uint8_t index)
{
	switch (index)
	{
	case 0:
#if ASSERT_PIN(JOYSTICK_INPUT_0)
		return (float)((int16_t)io_get_analog(JOYSTICK_INPUT_0) - 512) / 512;
#else
		return joystick_virtual_input(0);
#endif
#if JOYSTICK_AXIS_COUNT > 1
	case 1:
#if ASSERT_PIN(JOYSTICK_INPUT_1)
		return (float)((int16_t)io_get_analog(JOYSTICK_INPUT_1) - 512) / 512;
#else
		return joystick_virtual_input(1);
#endif
#endif
#if JOYSTICK_AXIS_COUNT > 2
	case 2:
#if ASSERT_PIN(JOYSTICK_INPUT_2)
		return (float)((int16_t)io_get_analog(JOYSTICK_INPUT_2) - 512) / 512;
#else
		return joystick_virtual_input(2);
#endif
#endif
#if JOYSTICK_AXIS_COUNT > 3
	case 3:
#if ASSERT_PIN(JOYSTICK_INPUT_3)
		return (float)((int16_t)io_get_analog(JOYSTICK_INPUT_3) - 512) / 512;
#else
		return joystick_virtual_input(3);
#endif
#endif
#if JOYSTICK_AXIS_COUNT > 4
	case 4:
#if ASSERT_PIN(JOYSTICK_INPUT_4)
		return (float)((int16_t)io_get_analog(JOYSTICK_INPUT_4) - 512) / 512;
#else
		return joystick_virtual_input(4);
#endif
#endif
#if JOYSTICK_AXIS_COUNT > 5
	case 5:
#if ASSERT_PIN(JOYSTICK_INPUT_5)
		return (float)((int16_t)io_get_analog(JOYSTICK_INPUT_5) - 512) / 512;
#else
		return joystick_virtual_input(5);
#endif
#endif
	}

	return 0;
}

// overridable non-linear transformation function
__attribute__((weak)) float joystick_transform_axis(float value)
{
	return fast_flt_pow2(value);
}

static float joystick_process_axis(uint8_t axis_index)
{
	if (JOYSTICK_DEAD_ZONE == 0)
	{
#if JOYSTICK_LINEAR_RESPONSE
		return joystick_read_axis(axis_index);
#else
		return joystick_transform_axis(joystick_read_axis(axis_index));
#endif
	}

	float raw = CLAMP(-1.0f, joystick_read_axis(axis_index), 1.0f);

	// Work with absolute values (sign gets applied at the end)
	bool sign = raw < 0;
	if (sign)
	{
		raw = -raw;
	}

	if (raw < JOYSTICK_DEAD_ZONE)
	{
		return 0;
	}

	// Remap values outside of dead zone back to the <0.0; 1.0> range
	raw = (raw - JOYSTICK_DEAD_ZONE) / (1.0f - JOYSTICK_DEAD_ZONE);

#if JOYSTICK_SENSITIVITY > 1
	uint8_t aprox_raw = (uint8_t)(JOYSTICK_SENSITIVITY * raw);
	raw = (JOYSTICK_SENSITIVITY_REV * (float)aprox_raw);
#endif

#if JOYSTICK_LINEAR_RESPONSE
	return sign ? -raw : raw;
#else
	float transformed = joystick_transform_axis(raw);
	return sign ? -transformed : transformed;
#endif
}

static uint8_t joystick_dummy_stream_getc()
{
	return EOL;
}

static uint8_t joystick_dummy_stream_available()
{
	return 1;
}

static void joystick_dummy_stream_clear()
{
}

#ifdef ENABLE_MAIN_LOOP_MODULES
static uint32_t joystick_last_update;

static FORCEINLINE void joystick_update()
{
	static bool has_stream_handle = false;

	// Only jog if idle or jogging
	uint8_t state = cnc_get_exec_state(EXEC_ALLACTIVE);
	if ((state != EXEC_IDLE && !(state & EXEC_JOG)) || cnc_has_alarm())
	{
		return;
	}

	int8_t index = -1;
	float value = 0;
	float values[AXIS_COUNT];

	// Find which axis to use (the one with the higher value wins)
	// This means the the most "intentional" joystick change wins
	for (uint8_t i = 0; i < JOYSTICK_AXIS_COUNT; ++i)
	{
		float axis_value = joystick_process_axis(i);
		if (axis_value != 0)
		{
			if (axis_value > value)
			{
				value = axis_value;
				index = i;
			}
		}
		values[i] = axis_value;
	}

	if (index == -1)
	{
		// Rest position, stop jogging
		if (has_stream_handle)
		{
			cnc_call_rt_command(CMD_CODE_JOG_CANCEL);
			grbl_stream_change(NULL); // release the stream
			has_stream_handle = false;
		}
		return;
	}

#if JOYSTICK_ALLOW_MULTI_AXIS_JOG
	float axis_values[JOYSTICK_AXIS_COUNT];
	float max_value = 0;

	for (uint8_t i = 0; i < JOYSTICK_AXIS_COUNT; ++i)
	{
		float axis_value = joystick_process_axis(i);
		axis_values[i] = axis_value;

		float abs_value = ABS(axis_value);
		if (abs_value > max_value)
			max_value = abs_value;
	}

	if (max_value == 0)
	{
		// Rest position, stop jogging
		if (joystick_jog)
		{
			joystick_jog = false;
			cnc_call_rt_command(CMD_CODE_JOG_CANCEL);
		}
		return;
	}

	// Only send new command if planner buffer has less than 3 commands
	if (!js_empty() || PLANNER_BUFFER_SIZE - planner_get_buffer_freeblocks() >= 3)
		return;

	float feed = max_value * JOYSTICK_MAX_FEED_RATE;
	float distance = feed * ((float)JOYSTICK_DELTA_TIME_MS / 1000) * (1.0 / 60.0);

	float square_sum = 0;
	for (uint8_t i = 0; i < JOYSTICK_AXIS_COUNT; ++i)
		square_sum += fast_flt_pow2(axis_values[i]);
	float vec_inv_length = fast_flt_invsqrt(square_sum);

	// Header
	js_putc('$');
	js_putc('J');
	js_putc('=');
	js_putc('G');
	js_putc('9');
	js_putc('1');

	// Append every non-zero axis
	for (uint8_t i = 0; i < JOYSTICK_AXIS_COUNT; ++i)
	{
		if (axis_values[i] == 0)
			continue;
		float axis_distance = axis_values[i] * distance * vec_inv_length;
		js_putc(joystick_axis_map[i]);
		print_flt(js_putc, axis_distance);
	}

	// Feed rate
	js_putc('F');
	print_flt(js_putc, feed);

	js_putc('\r');
	joystick_jog = true;
#else
	
	// If does not hold control of the protocol stream tries to grab it
	if (!has_stream_handle)
	{
		bool controls_stream = grbl_stream_readonly(joystick_dummy_stream_getc, joystick_dummy_stream_available, joystick_dummy_stream_clear);
		if (!controls_stream)
		{
			return;
		}

		// gained control hover the stream
		has_stream_handle = controls_stream;
	}

	float feed = ABS(value) * JOYSTICK_MAX_FEED_RATE;
	float distance = 1;

	if (!planner_buffer_is_full())
	{
		float target[AXIS_COUNT];
		target[index] = value;
		motion_data_t block = {0};
		block.feed = feed;
		block.spindle = planner_get_spindle_speed(1);
		block.motion_flags.bit.spindle_running = (block.spindle != 0) ? 1 : 0;
		mc_incremental_jog(target, &block);
	}

#endif
}

static bool joystick_dotasks(void *arg)
{
	// To keep the planner buffer filled we update at twice the delta time frequency.
	if (mcu_millis() - joystick_last_update < (JOYSTICK_DELTA_TIME_MS / 2))
	{
		return EVENT_CONTINUE;
	}

	joystick_update();

	joystick_last_update = mcu_millis();
	return EVENT_CONTINUE;
}

CREATE_EVENT_LISTENER(cnc_dotasks, joystick_dotasks);
#endif

DECL_MODULE(joystick)
{
	// inital read to fix adc read diff
	for (uint8_t i = 0; i < JOYSTICK_AXIS_COUNT; ++i)
	{
		joystick_process_axis(i);
	}
#ifdef DECL_SERIAL_STREAM
	BUFFER_INIT(uint8_t, joystick_buffer, JOYSTICK_BUFFER_SIZE);
	serial_stream_register(&joystick_serial_stream);
#endif

#ifdef ENABLE_MAIN_LOOP_MODULES
	ADD_EVENT_LISTENER(cnc_dotasks, joystick_dotasks);
#else
#warning "Main loop extensions are not enabled, joystick module will not function properly!"
#endif
}

#endif
