/*
	Name: test_text.c
	Description: Graphics library for µCNC
		Test the GFX_TEXT function

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

INCLUDE_FONT(TestFont);

GFX_DECL_SCREEN(simple_text_screen) {
	GFX_SCREEN_HEADER();
	GFX_CLEAR(2);
	GFX_TEXT(0, 0,	0, 1, &TestFont, 1, "\x01");
}

GFX_DECL_SCREEN(simple_text_screen_2) {
	GFX_SCREEN_HEADER();
	GFX_CLEAR(2);
	GFX_TEXT(0, 0,	0, 1, &TestFont, 1, "\x01\x01");
}

GFX_DECL_SCREEN(simple_text_screen_3) {
	GFX_SCREEN_HEADER();
	GFX_CLEAR(2);
	GFX_TEXT(0, 0,	0, 1, &TestFont, 1, "\x02");
}

GFX_DECL_SCREEN(simple_text_screen_4) {
	GFX_SCREEN_HEADER();
	GFX_CLEAR(2);
	GFX_TEXT(0, 0,	0, 1, &TestFont, 2, "\x01");
}

DECL_TEST(simple_text)
{
	GFX_RENDER_SCREEN(simple_text_screen);

	gfx_pixel_t expected[] = {
		0, 0, 2, 2,
		0, 0, 2, 2,
		1, 0, 2, 2,
		0, 0, 2, 2,
		0, 0, 2, 2,
		2, 2, 2, 2,
	};
	
	TEST_SCREEN(0, 0, 4, 6, expected);

	return true;
}

DECL_TEST(simple_text_2)
{
	GFX_RENDER_SCREEN(simple_text_screen_2);

	gfx_pixel_t expected[] = {
		0, 0, 0, 0, 2,
		0, 0, 0, 0, 2,
		1, 0, 1, 0, 2,
		0, 0, 0, 0, 2,
		0, 0, 0, 0, 2,
		2, 2, 2, 2, 2,
	};

	TEST_SCREEN(0, 0, 5, 6, expected);

	return true;
}

DECL_TEST(simple_text_3)
{
	GFX_RENDER_SCREEN(simple_text_screen_3);

	gfx_pixel_t expected[] = {
		0, 0, 0, 0, 2,
		0, 1, 0, 0, 2,
		0, 0, 1, 0, 2,
		0, 1, 0, 0, 2,
		0, 0, 0, 0, 2,
		2, 2, 2, 2, 2,
	};

	TEST_SCREEN(0, 0, 5, 6, expected);

	return true;
}

DECL_TEST(simple_text_4)
{
	GFX_RENDER_SCREEN(simple_text_screen_4);

	gfx_pixel_t expected[] = {
		0, 0, 0, 0, 2,
		0, 0, 0, 0, 2,
		0, 0, 0, 0, 2,
		0, 0, 0, 0, 2,
		1, 1, 0, 0, 2,
		1, 1, 0, 0, 2,
		0, 0, 0, 0, 2,
		0, 0, 0, 0, 2,
		0, 0, 0, 0, 2,
		0, 0, 0, 0, 2,
		2, 2, 2, 2, 2,
	};

	TEST_SCREEN(0, 0, 5, 11, expected);

	return true;
}

