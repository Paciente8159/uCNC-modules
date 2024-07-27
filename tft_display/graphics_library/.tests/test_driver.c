/*
	Name: test_driver.c
	Description: Graphics library for µCNC
		Driver used for unit tests

	Copyright: Copyright (c) Patryk Mierzyński
	Author: Patryk Mierzyński
	Date: 27/07/2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "test_driver.h"
#include <stdbool.h>

#include <stdio.h>

gfx_pixel_t buffer[GFX_DISPLAY_WIDTH * GFX_DISPLAY_HEIGHT];

void blit(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const gfx_pixel_t* pixels) {
	for(int i = 0; i < w; ++i) {
		for(int j = 0; j < h; ++j) {
			buffer[(x + i) + (y + j) * GFX_DISPLAY_WIDTH] = pixels[i + j * w];
		}
	}
}

uint16_t check_screen(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const gfx_pixel_t* expected) {
	for(uint16_t i = 0; i < w; ++i) {
		for(uint16_t j = 0; j < h; ++j) {
			if(buffer[(x + i) + (y + j) * GFX_DISPLAY_WIDTH] != expected[i + j * w]) {
				return i + (j << 8);
			}
		}
	}
	return 0xFFFF;
}

gfx_pixel_t get_pixel(uint8_t x, uint8_t y) {
	return buffer[x + y * GFX_DISPLAY_WIDTH];
}
