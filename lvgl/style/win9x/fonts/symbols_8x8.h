/*
	Name: symbols_8x8.h
	Description: Font containing only special symbols for use in a GUI

	Copyright: Copyright (c) Patryk Mierzyński
	Author: Patryk Mierzyński
	Date: 26/07/2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the GNU General Public License for more details.
*/

const uint8_t Symbols8x8Bitmap[] = {
	/*  0: */ 0x31, 0x24, 0xBF, 0xEF, 0x7F, 0xC0,
	/*  6: */ 0x60, 0x90, 0x90, 0x3F, 0x3B, 0x37, 0x3F,
	/* 13: */ 0x3C, 0x42, 0x81, 0x81, 0x81, 0x81, 0x42, 0x3C,
	/* 21: */ 0x3C, 0x6A, 0xD5, 0xAA, 0xD5, 0xAA, 0x56, 0x3C,
	/* 29: */ 0x3C, 0x7E, 0xFF, 0xFF, 0xFF, 0xFF, 0x7E, 0x3C,
};

const struct BitmapFontGlyph Symbols8x8Glyphs[] = {
	{  0,  6, 7,  8,  1, -7 }, // 0x01 - Locked symbol
	{  6,  8, 7,  8,  0, -7 }, // 0x02 - Unlocked symbol
	{ 13,  8, 8,  8,  0, -8 }, // 0x03 - Indicator empty
	{ 21,  8, 8,  8,  0, -8 }, // 0x04 - Indicator half-filled
	{ 29,  8, 8,  8,  0, -8 }, // 0x05 - Indicator full
};

const struct BitmapFont Symbols8x8 = {
	Symbols8x8Bitmap,
	Symbols8x8Glyphs,
	0x01, 0x05, 8, 8
};
