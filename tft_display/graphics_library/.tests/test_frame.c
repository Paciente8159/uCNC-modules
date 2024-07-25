/*
	Name: test_frame.c
	Description: Graphics library for µCNC
		Test the GFX_FRAME function

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

GFX_DECL_SCREEN(simple_frame_screen) {
	GFX_CLEAR(0);
	GFX_FRAME(1, 1, 3, 3, 1, 0, 0x01);
}

DECL_TEST(simple_frame)
{
	GFX_RENDER_SCREEN(simple_frame_screen);

	uint8_t expected[16] = {
		0, 0, 0, 0,
		0, 1, 1, 1,
		0, 1, 0, 1,
		0, 1, 1, 1,
	};
	TEST_SCREEN(0, 0, 4, 4, expected);

	return true;
}

