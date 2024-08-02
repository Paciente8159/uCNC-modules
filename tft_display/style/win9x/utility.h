/*
	Name: utility.h
	Description: Utility functions used in the GUI

	Copyright: Copyright (c) Patryk Mierzyński
	Author: Patryk Mierzyński
	Date: 27/07/2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the GNU General Public License for more details.
*/
#ifndef WIN9X_UTILITY_H
#define WIN9X_UTILITY_H

#include "colors.h"
#include "config.h"

#define _TWO_TONE_BORDER(x, y, w, h, t, c1, c2) \
	GFX_RECT(x, y, (w) - (t), (t), c1); \
	GFX_RECT(x, y, (t), (h) - (t), c1); \
	GFX_RECT((x) + (w) - (t), (y) + (t), (t), (h) - (t), c2); \
	GFX_RECT((x) + (t), (y) + (h) - (t), (w) - (t), (t), c2);

#define TWO_TONE_BORDER(...) { _TWO_TONE_BORDER(__VA_ARGS__) }

#define _BORDER(x, y, w, h, t, c) \
	GFX_RECT(x, y, (w) - (t), (t), c); \
	GFX_RECT(x, y, (t), (h) - (t), c); \
	GFX_RECT((x) + (w) - (t), (y) + (t), (t), (h) - (t), c); \
	GFX_RECT((x) + (t), (y) + (h) - (t), (w) - (t), (t), c);

#define BORDER(...) { _BORDER(__VA_ARGS__) }

#define _HIGHLIGHT(x, y, w, h, highlight) \
	if(highlight) { \
		GFX_CONTAINER(x, y, w, h, GFX_COLOR(SELECT_HIGHLIGHT)); \
	} else { \
		GFX_SET_ORIGIN(x, y); \
	}

#define HIGHLIGHT(...) _HIGHLIGHT(__VA_ARGS__)

#define BUTTON_DEF() GFX_DECL_ELEMENT(win9x_button, uint16_t width, uint16_t height, const char *text, bool highlight)
extern BUTTON_DEF();

#define LIST_SELECTOR_DEF() GFX_DECL_ELEMENT(win9x_list_selector, const char* header, const char **options, uint8_t option_count, uint8_t selected_index, int highlight_index, uint16_t *out_width, uint16_t *out_height)
extern LIST_SELECTOR_DEF();

#define BORDER_WIDTH 2

#define BOX_INSET(x, y, w, h, bg) { \
	TWO_TONE_BORDER(x, y, w, h, BORDER_WIDTH, SHADOW_DARK, SHADOW_LIGHT); \
	GFX_CONTAINER((x) + BORDER_WIDTH, (y) + BORDER_WIDTH, (w) - BORDER_WIDTH * 2, (h) - BORDER_WIDTH * 2, bg); }

#define BOX_INSET_dynamic(x, y, w, h, bg) { \
	TWO_TONE_BORDER(x, y, w, h, BORDER_WIDTH, SHADOW_DARK, SHADOW_LIGHT); \
	GFX_CONTAINER_dynamic((x) + BORDER_WIDTH, (y) + BORDER_WIDTH, (w) - BORDER_WIDTH * 2, (h) - BORDER_WIDTH * 2, bg); }

#define BOX_OUTSET(x, y, w, h, bg) { \
	TWO_TONE_BORDER(x, y, w, h, BORDER_WIDTH, SHADOW_LIGHT, SHADOW_DARK); \
	GFX_CONTAINER((x) + BORDER_WIDTH, (y) + BORDER_WIDTH, (w) - BORDER_WIDTH * 2, (h) - BORDER_WIDTH * 2, bg); }

#define BOX_OUTSET_dynamic(x, y, w, h, bg) { \
	TWO_TONE_BORDER(x, y, w, h, BORDER_WIDTH, SHADOW_LIGHT, SHADOW_DARK); \
	GFX_CONTAINER_dynamic((x) + BORDER_WIDTH, (y) + BORDER_WIDTH, (w) - BORDER_WIDTH * 2, (h) - BORDER_WIDTH * 2, bg); }

#define FONT_MONO &FreeMonoBold12pt7b
#define FONT_SANS &FreeSans9pt7b
#define FONT_SYMBOL &Symbols8x8
#define FONT_PIXEL &PixelFont
#define FONT_PIXEL_MONO &PixelFontMono

INCLUDE_FONT(FreeSans9pt7b);
INCLUDE_FONT(FreeMonoBold12pt7b);
INCLUDE_FONT(Symbols8x8);
INCLUDE_FONT(PixelFont);
INCLUDE_FONT(PixelFontMono);

#endif

