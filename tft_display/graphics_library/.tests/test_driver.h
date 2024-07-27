/*
	Name: test_driver.h
	Description: Graphics library for µCNC
		Driver used for unit tests

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

#ifndef GRAPHICS_LIBRARY_TEST_DRIVER_H
#define GRAPHICS_LIBRARY_TEST_DRIVER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#define GFX_DISPLAY_WIDTH 100
#define GFX_DISPLAY_HEIGHT 100

#undef GFX_RENDER_BUFFER_SIZE

#if defined(GFX_TEST_ROW_BY_ROW)
#define GFX_RENDER_BUFFER_SIZE (GFX_DISPLAY_WIDTH)
#elif defined(GFX_TEST_ALL_AT_ONCE)
#define GFX_RENDER_BUFFER_SIZE (GFX_DISPLAY_WIDTH * GFX_DISPLAY_HEIGHT)
#elif defined(GFX_TEST_PIXEL_BY_PIXEL)
#define GFX_RENDER_BUFFER_SIZE 1
#endif

typedef uint8_t gfx_pixel_t;

#define GFX_COLOR_INTERNAL(r, g, b) ((((r) * 3 / 255) << 4) | (((g) * 3 / 255) << 2) | ((b) * 3 / 255))

extern void blit(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const gfx_pixel_t* pixels);
#define GFX_BLIT(x, y, w, h, pixels) blit(x, y, w, h, pixels);

extern uint16_t check_screen(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const gfx_pixel_t* expected);
extern gfx_pixel_t get_pixel(uint8_t x, uint8_t y);

#define __GFX_DRIVER

#ifdef __cplusplus
}
#endif

#endif

