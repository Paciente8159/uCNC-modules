/*
	Name: elements.c
	Description: Element implementations

	Copyright: Copyright (c) Patryk Mierzyński
	Author: Patryk Mierzyński
	Date: 31/07/2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the GNU General Public License for more details.
*/
#include "../support.h"

#ifdef TFT_STYLE_WIN9X

#include "utility.h"

BUTTON_DEF() {
	GFX_SET_FONT(FONT_PIXEL, 1);

	TWO_TONE_BORDER(GFX_REL(0, 0), width, height, 2, SHADOW_LIGHT, SHADOW_DARK);
	GFX_RECT(GFX_REL(2, 2), width - 4, height - 4, BASE_BACKGROUND);
	GFX_TEXT(GFX_REL(GFX_CENTER_TEXT_OFFSET(width - 4, text) + 2, (height - 15) / 2 + 2), GFX_BLACK, text);
}

#endif

