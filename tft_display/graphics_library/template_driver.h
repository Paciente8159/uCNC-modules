/*
	Name: template_driver.h
	Description: Example display driver for use with the graphics library

	Copyright: Copyright (c) Patryk Mierzyński
	Author: Patryk Mierzyński
	Date: 20/07/2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/
#ifndef GFX_TEMPLATE_DRIVER_H
#define GFX_TEMPLATE_DRIVER_H

#ifdef __cplusplus
extern "C"
{
#endif

#error "This file is just a template, do not include it in your project!"

#include <stdint.h>

// Define the display dimensions
#define GFX_DISPLAY_WIDTH 480
#define GFX_DISPLAY_HEIGHT 320

// Define the pixel variable type
typedef uint16_t gfx_pixel_t;

// Define a macro to encode standard RGB values into the pixel type
// This macro is expected to take in 8 bit values (0-255)
#define GFX_COLOR_INTERNAL(r, g, b) ((((r) * 31 / 255) << 10) | (((g) * 31 / 255) << 5) | ((b) * 31 / 255))

// Define the blit method used for displaying pixels onto the screen
#define GFX_BLIT(x, y, w, h, pixels) display_blit(x, y, w, h, pixels);

// Notifies the graphics library that a driver has been included
#define __GFX_DRIVER

#ifdef __cplusplus
}
#endif

#endif

