/*
	Name: test_clear.c
	Description: Graphics library for µCNC
		Test the GFX_CLEAR function

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

GFX_DECL_SCREEN(simple_clear_screen) {
	GFX_SCREEN_HEADER();
	GFX_CLEAR(0xf0);
}

DECL_TEST(simple_clear)
{
	GFX_RENDER_SCREEN(simple_clear_screen);

	uint8_t expected[9];
	for(int i = 0; i < 9; ++i) expected[i] = 0xf0;
	TEST_SCREEN(0, 0, 3, 3, expected);

	return true;
}

