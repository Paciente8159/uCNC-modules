/*
	Name: joystick.c
	Description: Adds support for jogging using an analog joystick.

	Copyright: Copyright (c) Patryk Mierzyński
	Author: Patryk Mierzyński
	Date: 24/08/2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the GNU General Public License for more details.
*/

#include "joystick.h"

#if (UCNC_MODULE_VERSION < 10800 || UCNC_MODULE_VERSION > 99999)
#error "This module is not compatible with the current version of µCNC"
#endif

#if defined(JOYSTICK_INPUT_0_AXIS) || defined(JOYSTICK_INPUT_1_AXIS) || defined(JOYSTICK_INPUT_2_AXIS) || \
	defined(JOYSTICK_INPUT_3_AXIS) || defined(JOYSTICK_INPUT_4_AXIS) || defined(JOYSTICK_INPUT_5_AXIS)

#include "src/utils.h"
#include "config_helper.h"

#if JOYSTICK_AXIS_COUNT == 0
#error "Joystick axis count should not be zero at this point"
#endif

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

extern float joystick_virtual_input_0(void);
extern float joystick_virtual_input_1(void);
extern float joystick_virtual_input_2(void);
extern float joystick_virtual_input_3(void);
extern float joystick_virtual_input_4(void);
extern float joystick_virtual_input_5(void);

static float joystick_read_axis(uint8_t index)
{
	switch(index)
	{
		case 0:
#if ASSERT_PIN(JOYSTICK_INPUT_0)
			return (float)((int16_t)io_get_analog(JOYSTICK_INPUT_0) - 512) / 512;
#else
			return joystick_virtual_input_0();
#endif
#if JOYSTICK_AXIS_COUNT > 1
		case 1:
#if ASSERT_PIN(JOYSTICK_INPUT_1)
			return (float)((int16_t)io_get_analog(JOYSTICK_INPUT_1) - 512) / 512;
#else
			return joystick_virtual_input_1();
#endif
#endif
#if JOYSTICK_AXIS_COUNT > 2
		case 2:
#if ASSERT_PIN(JOYSTICK_INPUT_2)
			return (float)((int16_t)io_get_analog(JOYSTICK_INPUT_2) - 512) / 512;
#else
			return joystick_virtual_input_2();
#endif
#endif
#if JOYSTICK_AXIS_COUNT > 3
		case 3:
#if ASSERT_PIN(JOYSTICK_INPUT_3)
			return (float)((int16_t)io_get_analog(JOYSTICK_INPUT_3) - 512) / 512;
#else
			return joystick_virtual_input_3();
#endif
#endif
#if JOYSTICK_AXIS_COUNT > 4
		case 4:
#if ASSERT_PIN(JOYSTICK_INPUT_4)
			return (float)((int16_t)io_get_analog(JOYSTICK_INPUT_4) - 512) / 512;
#else
			return joystick_virtual_input_4();
#endif
#endif
#if JOYSTICK_AXIS_COUNT > 5
		case 5:
#if ASSERT_PIN(JOYSTICK_INPUT_5)
			return (float)((int16_t)io_get_analog(JOYSTICK_INPUT_5) - 512) / 512;
#else
			return joystick_virtual_input_5();
#endif
#endif
	}

	return 0;
}

static FORCEINLINE float joystick_transform_axis(float value)
{
	return fast_flt_pow2(value);
}

static float joystick_process_axis(uint8_t axis_index)
{
#if JOYSTICK_DISABLE_DEAD_ZONE && JOYSTICK_LINEAR_RESPONSE
	// Shortcut
	return joystick_read_axis(axis_index);
#else
	float raw = joystick_read_axis(axis_index);

	// Work with absolute values (sign gets applied at the end)
	bool sign = raw < 0;
	if(sign)
	{
		raw = -raw;
	}

#if !JOYSTICK_DISABLE_DEAD_ZONE
	if(raw < JOYSTICK_DEAD_ZONE)
	{
		return 0;
	}

	// Remap values outside of dead zone back to the <0.0; 1.0> range
	raw -= JOYSTICK_DEAD_ZONE;
	raw *= 1.0 / (1.0 - JOYSTICK_DEAD_ZONE);
#endif

#if JOYSTICK_LINEAR_RESPONSE
	return sign ? -raw : raw;
#else
	float transformed = joystick_transform_axis(raw);
	return sign ? -transformed : transformed;
#endif
#endif
}

#ifdef DECL_SERIAL_STREAM
#if JOYSTICK_ALLOW_MULTI_AXIS_JOG
// "$J=G91F????.???\r" is 16 characters, add 9 bytes for each enabled axis
#define JOYSTICK_BUFFER_SIZE (16 + JOYSTICK_AXIS_COUNT * 9)
#else
// "$J=G91X???.???F????.???\r" is 24 characters long so 32 is more than enough for single axis jogging.
#define JOYSTICK_BUFFER_SIZE 32
#endif

DECL_BUFFER(uint8_t, joystick_buffer, JOYSTICK_BUFFER_SIZE);

static uint8_t js_getc()
{
	uint8_t c;
	BUFFER_DEQUEUE(joystick_buffer, &c);
	return c;
}

static uint8_t js_available()
{
	return BUFFER_READ_AVAILABLE(joystick_buffer);
}

static void js_clear()
{
	BUFFER_CLEAR(joystick_buffer);
}

static void js_putc(char c)
{
	BUFFER_ENQUEUE(joystick_buffer, &c);
}

static bool js_empty()
{
	return BUFFER_EMPTY(joystick_buffer);
}

DECL_SERIAL_STREAM(joystick_serial_stream, js_getc, js_available, js_clear, NULL, NULL);
#endif

#ifdef ENABLE_MAIN_LOOP_MODULES
static bool joystick_jog = false;
static uint32_t joystick_last_update;

static FORCEINLINE void joystick_update()
{
	// Only jog if idle or jogging
	uint8_t state = cnc_get_exec_state(EXEC_ALLACTIVE);
	if((state != EXEC_IDLE && !(state & EXEC_JOG)) || cnc_has_alarm())
		return;
	if(!(state & EXEC_JOG))
	{
		// Jog finished, set flag to false
		joystick_jog = false;
	}
#if JOYSTICK_ALLOW_MULTI_AXIS_JOG
	float axis_values[JOYSTICK_AXIS_COUNT];
	float max_value = 0;

	for(uint8_t i = 0; i < JOYSTICK_AXIS_COUNT; ++i)
	{
		float axis_value = joystick_process_axis(i);
		axis_values[i] = axis_value;

		float abs_value = ABS(axis_value);
		if(abs_value > max_value)
			max_value = abs_value;
	}

	if(max_value == 0)
	{
		// Rest position, stop jogging
		if(joystick_jog)
		{
			joystick_jog = false;
			cnc_call_rt_command(CMD_CODE_JOG_CANCEL);
		}
		return;
	}

	// Only send new command if planner buffer has less than 3 commands
	if(!js_empty() || PLANNER_BUFFER_SIZE - planner_get_buffer_freeblocks() >= 3)
		return;

	float feed = max_value * JOYSTICK_MAX_FEED_RATE;
	float distance = feed * ((float)JOYSTICK_DELTA_TIME_MS / 1000) * (1.0 / 60.0);

	float square_sum = 0;
	for(uint8_t i = 0; i < JOYSTICK_AXIS_COUNT; ++i)
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
	for(uint8_t i = 0; i < JOYSTICK_AXIS_COUNT; ++i)
	{
		if(axis_values[i] == 0)
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
	int8_t index = -1;
	float value;

	// Find which axis to use
	for(uint8_t i = 0; i < JOYSTICK_AXIS_COUNT; ++i)
	{
		float axis_value = joystick_process_axis(i);
		if(axis_value != 0)
		{
			if(index == -1)
			{
				// First non-zero axis
				value = axis_value;
				index = i;
			}
			else
			{
				// Multi axis jog detected, stop.
				cnc_call_rt_command(CMD_CODE_JOG_CANCEL);
				joystick_jog = false;
				return;
			}
		}
	}

	if(index == -1)
	{
		// Rest position, stop jogging
		if(joystick_jog)
		{
			joystick_jog = false;
			cnc_call_rt_command(CMD_CODE_JOG_CANCEL);
		}
		return;
	}

	// Only send new command if planner buffer has less than 3 commands
	if(!js_empty() || PLANNER_BUFFER_SIZE - planner_get_buffer_freeblocks() >= 3)
		return;

	float feed = ABS(value) * JOYSTICK_MAX_FEED_RATE;
	float distance = feed * ((float)JOYSTICK_DELTA_TIME_MS / 1000) * (1.0 / 60.0);

	// Header
	js_putc('$');
	js_putc('J');
	js_putc('=');
	js_putc('G');
	js_putc('9');
	js_putc('1');

	// Axis letter
	js_putc(joystick_axis_map[index]);
	// Movement distance
	print_flt(js_putc, value < 0 ? -distance : distance);

	// Feed rate
	js_putc('F');
	print_flt(js_putc, feed);

	js_putc('\r');
	joystick_jog = true;
#endif
}

static bool joystick_dotasks(void *arg)
{
	// To keep the planner buffer filled we update at twice the delta time frequency.
	if(mcu_millis() - joystick_last_update < (JOYSTICK_DELTA_TIME_MS / 2))
		return EVENT_CONTINUE;

	joystick_update();

	joystick_last_update = mcu_millis();
	return EVENT_CONTINUE;
}

CREATE_EVENT_LISTENER(cnc_dotasks, joystick_dotasks);
#endif

DECL_MODULE(joystick)
{
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

