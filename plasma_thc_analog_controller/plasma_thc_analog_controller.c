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

#ifndef PLASMA_THC_VOLTAGE_INPUT
#define PLASMA_THC_VOLTAGE_INPUT ANALOG0
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

static pid_data_t plasma_thc_pid;
static float thc_up_down;
static float thc_setpoint;

#ifdef ENABLE_SETTINGS_MODULES
// using settings helper macros
#define PLASMA_THC_ANALOG_SETTING_ID 1010
DECL_EXTENDED_SETTING(PLASMA_THC_ANALOG_SETTING_ID, &plasma_thc_pid.k, float, 3, protocol_send_gcode_setting_line_flt);
#endif

void plasma_thc_update(void *args)
{
#if ASSERT_PIN(PLASMA_THC_VOLTAGE_INPUT)
	float thc_value = io_get_analog(PLASMA_THC_VOLTAGE_INPUT);
	thc_value *= PLASMA_THC_ANALOG_RANGE;
	pid_compute(&plasma_thc_pid, &thc_up_down, thc_setpoint, thc_value, HZ_TO_MS(PLASMA_THC_VOLTAGE_SAMPLERATE));
#endif
	return EVENT_CONTINUE;
}
CREATE_EVENT_LISTENER(cnc_dotasks, plasma_thc_update);

// overrides plasma_thc_up
uint8_t plasma_thc_up(void)
{
	// probably set threshold via M command depending on the material
	return (thc_up_down < -0.5f);
}

// overrides plasma_thc_down
uint8_t plasma_thc_down(void)
{
	// probably set threshold via M command depending on the material
	return (thc_up_down > 0.5f);
}

// plasma_thc_arc_ok
uint8_t plasma_thc_arc_ok(void)
{
	// probably set threshold via M command depending on the material
	return (thc_up_down < 0.7f);
}

DECL_MODULE(web_pendant)
{
#ifdef ENABLE_SETTINGS_MODULES
	EXTENDED_SETTING_INIT(PLASMA_THC_ANALOG_SETTING_ID, &plasma_thc_pid.k);
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
