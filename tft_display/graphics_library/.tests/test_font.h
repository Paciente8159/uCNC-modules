/*
	Name: test_font.h
	Description: Graphics library for µCNC
    Font used for unit testing

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

#include "../fonts/font.h"

const uint8_t TestFontBitmaps[] = {
  /* 0: */ 0x80,
	/* 1: */ 0x98,
};

const struct BitmapFontGlyph TestFontGlyphs[] = {
	{ 0, 1, 1, 2, 0, -1 }, // 0x01
	{ 1, 2, 3, 4, 1, -2 }, // 0x02
};

const struct BitmapFont TestFont = {
  TestFontBitmaps,
  TestFontGlyphs,
  0x01, 0x02, 5, 3
};

