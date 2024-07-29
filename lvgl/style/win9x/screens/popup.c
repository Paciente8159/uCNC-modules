/*
	Name: popup.c
	Description: Popup screen in style of Win9x

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

#include "../../support.h"

#ifdef TFT_STYLE_WIN9X

#include "../colors.h"
#include "../utility.h"

struct PopupScreenArgument {
	uint16_t width, height;
	const char *text;
};

GFX_DECL_SCREEN(popup)
{
	GFX_SCREEN_HEADER();
	GFX_SET_FONT(FONT_SANS, 1);

#define POPUP_WIDTH GFX_SCREEN_ARG(struct PopupScreenArgument)->width
#define POPUP_HEIGHT GFX_SCREEN_ARG(struct PopupScreenArgument)->height
#define POPUP_X ((GFX_DISPLAY_WIDTH - POPUP_WIDTH) / 2)
#define POPUP_Y ((GFX_DISPLAY_HEIGHT - POPUP_HEIGHT) / 2)

	TWO_TONE_BORDER(POPUP_X - 2, POPUP_Y - 2, POPUP_WIDTH + 4, POPUP_HEIGHT + 4, 2, GFX_WHITE, SHADOW_DARKER);

	// Title bar
	GFX_CONTAINER(POPUP_X, POPUP_Y, POPUP_WIDTH, 22, GFX_COLOR(0x0000b0));
	GFX_TEXT(POPUP_X + 4, POPUP_Y + 1, GFX_WHITE, "Attention");

	BOX_INSET(50, 50, 20, 20, BOX_BACKGROUND);
	BOX_OUTSET(50, 80, 20, 20, BOX_BACKGROUND);

	// Body
	GFX_CONTAINER(POPUP_X, POPUP_Y + 22, POPUP_WIDTH, POPUP_HEIGHT - 22, BASE_BACKGROUND);

	GFX_TEXT(GFX_REL(5, 5), GFX_BLACK, GFX_SCREEN_ARG(struct PopupScreenArgument)->text);
}

#endif

