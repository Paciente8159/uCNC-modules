/*
	Name: test_bitmap.c
	Description: Graphics library for µCNC
		Test the GFX_BITMAP function

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

#include "../graphics_library.h"
#include "test.h"

static const uint8_t bitmap[] = {
	0x96, 0x29
};

GFX_DECL_SCREEN(simple_bitmap_screen) {
	GFX_SCREEN_HEADER();

	GFX_CLEAR(2);
	GFX_BITMAP(0, 0, 4, 4, 0, 1, bitmap);
}

GFX_DECL_SCREEN(simple_bitmap_screen_2) {
	GFX_SCREEN_HEADER();

	GFX_CLEAR(2);
	GFX_BITMAP(0, 0, 4, 4, 0, 1, bitmap, 2);
}

DECL_TEST(simple_bitmap)
{
	GFX_RENDER_SCREEN(simple_bitmap_screen);

	gfx_pixel_t expected[] = {
		1, 0, 0, 1, 2,
		0, 1, 1, 0, 2,
		0, 0, 1, 0, 2,
		1, 0, 0, 1, 2,
		2, 2, 2, 2, 2,
	};
	TEST_SCREEN(0, 0, 5, 5, expected);

	return true;
}

DECL_TEST(simple_bitmap_2)
{
	GFX_RENDER_SCREEN(simple_bitmap_screen_2);

	gfx_pixel_t expected[] = {
		1, 1, 0, 0, 0, 0, 1, 1, 2,
		1, 1, 0, 0, 0, 0, 1, 1, 2,
		0, 0, 1, 1, 1, 1, 0, 0, 2,
		0, 0, 1, 1, 1, 1, 0, 0, 2,
		0, 0, 0, 0, 1, 1, 0, 0, 2,
		0, 0, 0, 0, 1, 1, 0, 0, 2,
		1, 1, 0, 0, 0, 0, 1, 1, 2,
		1, 1, 0, 0, 0, 0, 1, 1, 2,
		2, 2, 2, 2, 2, 2, 2, 2, 2,
	};
	TEST_SCREEN(0, 0, 9, 9, expected);

	return true;
}

