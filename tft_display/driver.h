/*
	Name: driver.h
	Description: Defines which driver should be used for the TFT module and configures it

	Copyright: Copyright (c) Patryk Mierzyński
	Author: Patryk Mierzyński
	Date: 03/08/2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the GNU General Public License for more details.
*/

#ifndef TFT_DRIVER_H
#define TFT_DRIVER_H

#ifdef __cplusplus
extern "C"
{
#endif

// Used for RPi type screens
// #define TFT_ALWAYS_16BIT
// #include "driver/ILI9486.h"


// Use LVGL driver
#define TFT_LV_DRIVER 1

// Specify display resolution
#define TFT_DISPLAY_WIDTH 320
#define TFT_DISPLAY_HEIGHT 240

// Set display rotation
#define TFT_DISPLAY_ROTATION LV_DISPLAY_ROTATION_0

// Set other optional flags
#define TFT_DISPLAY_FLAGS 0

#ifdef __cplusplus
}
#endif

#endif

