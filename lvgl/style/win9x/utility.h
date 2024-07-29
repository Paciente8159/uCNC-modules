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

#define TWO_TONE_BORDER(x, y, w, h, t, c1, c2) \
	GFX_RECT(x, y, (w) - (t), (t), c1); \
	GFX_RECT(x, y, (t), (h) - (t), c1); \
	GFX_RECT((x) + (w) - (t), (y) + (t), (t), (h) - (t), c2); \
	GFX_RECT((x) + (t), (y) + (h) - (t), (w) - (t), (t), c2);

#define BORDER_WIDTH 2

#define BOX_INSET(x, y, w, h, bg) \
	TWO_TONE_BORDER(x, y, w, h, BORDER_WIDTH, SHADOW_DARK, SHADOW_LIGHT); \
	GFX_CONTAINER((x) + BORDER_WIDTH, (y) + BORDER_WIDTH, (w) - BORDER_WIDTH * 2, (h) - BORDER_WIDTH * 2, bg);

#define BOX_INSET_dynamic(x, y, w, h, bg) \
	TWO_TONE_BORDER(x, y, w, h, BORDER_WIDTH, SHADOW_DARK, SHADOW_LIGHT); \
	GFX_CONTAINER_dynamic((x) + BORDER_WIDTH, (y) + BORDER_WIDTH, (w) - BORDER_WIDTH * 2, (h) - BORDER_WIDTH * 2, bg);

#define BOX_OUTSET(x, y, w, h, bg) \
	TWO_TONE_BORDER(x, y, w, h, BORDER_WIDTH, SHADOW_LIGHT, SHADOW_DARK); \
	GFX_CONTAINER((x) + BORDER_WIDTH, (y) + BORDER_WIDTH, (w) - BORDER_WIDTH * 2, (h) - BORDER_WIDTH * 2, bg);

#define BOX_OUTSET_dynamic(x, y, w, h, bg) \
	TWO_TONE_BORDER(x, y, w, h, BORDER_WIDTH, SHADOW_LIGHT, SHADOW_DARK); \
	GFX_CONTAINER_dynamic((x) + BORDER_WIDTH, (y) + BORDER_WIDTH, (w) - BORDER_WIDTH * 2, (h) - BORDER_WIDTH * 2, bg);

#define FONT_MONO &FreeMonoBold12pt7b
#define FONT_SANS &FreeSans9pt7b
#define FONT_SYMBOL &Symbols8x8

INCLUDE_FONT(FreeSans9pt7b);
INCLUDE_FONT(FreeMonoBold12pt7b);
INCLUDE_FONT(Symbols8x8);

#endif

