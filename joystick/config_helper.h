/*
	Name: config_helper.h
	Description: Helper file for the joystick module.

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
#ifndef JOYSTICK_CONFIG_HELPER_H
#define JOYSTICK_CONFIG_HELPER_H

#ifdef __cplusplus
extern "C"
{
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

// Other configuration options
#ifndef JOYSTICK_LINEAR_RESPONSE
#define JOYSTICK_LINEAR_RESPONSE 0
#endif

#ifndef JOYSTICK_ALLOW_MULTI_AXIS_JOG
#define JOYSTICK_ALLOW_MULTI_AXIS_JOG 0
#endif

#ifndef JOYSTICK_DEAD_ZONE
#define JOYSTICK_DEAD_ZONE 0.05
#endif

#ifndef JOYSTICK_DISABLE_DEAD_ZONE
#define JOYSTICK_DISABLE_DEAD_ZONE 0
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
#define JOYSTICK_MAP_AXIS(axis) __helper__(JOYSTICK_AXIS_LETTER_, axis,)

#ifdef __cplusplus
}
#endif

#endif

