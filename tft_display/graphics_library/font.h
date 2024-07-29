/*
	Name: font.h
	Description: Font support for graphics library

	Copyright: Copyright (c) Patryk Mierzyński
	Author: Patryk Mierzyński
	Date: 29/07/2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/
#ifndef GRAPHICS_LIBRARY_FONT_H
#define GRAPHICS_LIBRARY_FONT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

struct BitmapFontGlyph {
	uint16_t bfg_bitmapOffset;
	uint8_t bfg_width;
	uint8_t bfg_height;
	uint8_t bfg_xAdvance;
	int8_t bfg_xOffset;
	int8_t bfg_yOffset;
};

struct BitmapFont {
	const uint8_t* bf_bitmap;
	const struct BitmapFontGlyph* bf_glyphs;

	uint8_t bf_firstChar;
	uint8_t bf_lastChar;

	uint8_t bf_yAdvance;
	uint8_t bf_lineHeight;
};

#define INCLUDE_FONT(font) extern const struct BitmapFont font

#ifdef __cplusplus
}
#endif

#endif
