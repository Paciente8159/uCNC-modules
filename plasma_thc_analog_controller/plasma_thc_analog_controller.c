/*
	Name: plasma_thc_analog_controller.c
	Description: Implements a plasma THC analog controller for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 14-02-2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../cnc.h"
#include "../pid.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#ifdef ENABLE_PLASMA_THC

#define PLASMA_FRONTEND_ANALOG 1
#define PLASMA_FRONTEND_FREQ 2

#ifndef PLASMA_FRONTEND_TYPE
#define PLASMA_FRONTEND_ANALOG
#endif

#if (PLASMA_FRONTEND_TYPE == PLASMA_FRONTEND_ANALOG)
#ifndef PLASMA_THC_VOLTAGE_INPUT
#define PLASMA_THC_VOLTAGE_INPUT ANALOG0
#endif
#elif (PLASMA_FRONTEND_TYPE == PLASMA_FRONTEND_FREQ)
// assign one encoder to act as a frequency counter
#ifndef PLASMA_THC_FREQ_INPUT
#define PLASMA_THC_FREQ_INPUT ENC0
#endif
#endif

#ifndef PLASMA_THC_VOLTAGE_RANGE
#define PLASMA_THC_VOLTAGE_RANGE 300
#endif

#ifndef PLASMA_THC_VOLTAGE_SAMPLERATE
#define PLASMA_THC_VOLTAGE_SAMPLERATE 100
#endif

#define PLASMA_THC_ANALOG_RANGE ((float)PLASMA_THC_VOLTAGE_RANGE / 1024.0f)

#if (UCNC_MODULE_VERSION < 10801 || UCNC_MODULE_VERSION > 99999)
#error "This module is not compatible with the current version of µCNC"
#endif

typedef struct arc_params_t
{
	float voltage_conversion;
	float voltage_offset;
	float units_per_volt;
	float arc_ok_upper_limit;
	float arc_ok_lower_limit;
#if (PLASMA_FRONTEND_TYPE == PLASMA_FRONTEND_FREQ)
	float input_freq_range;
#endif
} arc_params_t;

static pid_data_t plasma_thc_pid;
static float thc_up_down;
static arc_params_t thc_arc_params;

#ifdef ENABLE_SETTINGS_MODULES
// using settings helper macros
#define PLASMA_THC_PID_SETTING_ID 352
DECL_EXTENDED_SETTING(PLASMA_THC_PID_SETTING_ID, &plasma_thc_pid.k, float, 3, protocol_send_gcode_setting_line_flt);
#define PLASMA_THC_PID_SETTING_ID 353
DECL_EXTENDED_SETTING(PLASMA_THC_PID_SETTING_ID, &plasma_thc_pid.k, float, 3, protocol_send_gcode_setting_line_flt);
#endif

#if (PLASMA_FRONTEND_TYPE == PLASMA_FRONTEND_ANALOG)
float get_thc_arc_voltage(void)
{
	float value = thc_arc_params.voltage_conversion;
	value *= (float)io_get_analog(PLASMA_THC_VOLTAGE_INPUT);
	return (value - thc_arc_params.voltage_offset);
}
#elif (PLASMA_FRONTEND_TYPE == PLASMA_FRONTEND_FREQ)
float get_thc_arc_voltage(void)
{
	static uint32_t last_time = 0;
	uint32_t elapsed = mcu_micros() - last_time;
	last_time = elapsed;

	int32_t count = encoder_get_position(PLASMA_THC_FREQ_INPUT);
	encoder_reset_position(PLASMA_THC_FREQ_INPUT, 0);

	float value = thc_arc_params.voltage_conversion;
	float value *= ABS(count);
	value *= 0.000001f * elapsed; //units per second
	return (value - thc_arc_params.voltage_offset);
}
#endif

void plasma_thc_update(void *args)
{
#if ASSERT_PIN(PLASMA_THC_VOLTAGE_INPUT)
	float thc_value = get_thc_arc_voltage();
	float thc_setpoint = planner_get_spindle_speed(1); // gets the plasma setpoint adjusted to the current override value
	pid_compute(&plasma_thc_pid, &thc_up_down, thc_setpoint, thc_value, HZ_TO_MS(PLASMA_THC_VOLTAGE_SAMPLERATE));
	
#endif
	return EVENT_CONTINUE;
}
CREATE_EVENT_LISTENER(cnc_dotasks, plasma_thc_update);

// overrides plasma_thc_up
uint8_t plasma_thc_up(void)
{
	float thc_value = get_thc_arc_voltage();
	// probably set threshold via M command depending on the material
	return (thc_arc_params.arc_ok_lower_limit > thc_value);
}

// overrides plasma_thc_down
uint8_t plasma_thc_down(void)
{
	// probably set threshold via M command depending on the material
	return (thc_arc_params.arc_ok_upper_limit < thc_value);
}

// plasma_thc_arc_ok
uint8_t plasma_thc_arc_ok(void)
{
	float thc_value = get_thc_arc_voltage();

	return ((thc_arc_params.arc_ok_lower_limit > thc_value) && (thc_arc_params.arc_ok_upper_limit < thc_value));
}

DECL_MODULE(web_pendant)
{
#ifdef ENABLE_SETTINGS_MODULES
	EXTENDED_SETTING_INIT(PLASMA_THC_PID_SETTING_ID, &plasma_thc_pid.k);
#else
#error "Settings extensions are not enabled. Your module will not work."
#endif

#ifdef ENABLE_MAIN_LOOP_MODULES
	ADD_EVENT_LISTENER(cnc_dotasks, plasma_thc_update);
#else
#error "Main loop extensions are not enabled. Your module will not work."
#endif
}

#endif
