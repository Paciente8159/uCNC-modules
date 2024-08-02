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

#include "bitmaps/checkbox.h"

BUTTON_DEF() {
	GFX_SET_FONT(FONT_PIXEL, 1);

	if(highlight)
		BORDER(GFX_REL(-1, -1), width+2, height+2, 1, GFX_BLACK);
	TWO_TONE_BORDER(GFX_REL(0, 0), width, height, 2, SHADOW_LIGHT, SHADOW_DARK);
	GFX_RECT(GFX_REL(2, 2), width - 4, height - 4, BASE_BACKGROUND);
	GFX_TEXT(GFX_REL(GFX_CENTER_TEXT_OFFSET(width - 4, text) + 2, (height - 15) / 2 + 2), GFX_BLACK, text);
}

LIST_SELECTOR_DEF()
{
	GFX_SET_FONT(FONT_PIXEL, 1);

	uint16_t width = 0;
	uint16_t height = option_count * 12 + 18;

	gfx_text_size(&width, 0, FONT_PIXEL, 1, header);
	width += 8;

	for(uint8_t i = 0; i < option_count; ++i)
	{
		uint16_t line_width;
		gfx_text_size(&line_width, 0, FONT_PIXEL, 1, header);
		line_width += 8;
		if(line_width > width)
			width = line_width;
	}

	TWO_TONE_BORDER(GFX_REL(-2, -2), width+4, height+4, 2, SHADOW_DARK, SHADOW_LIGHT);
	GFX_CONTAINER(GFX_REL(0, 0), width, height, BOX_BACKGROUND);

	GFX_TEXT(GFX_REL(4, 3), GFX_BLACK, header);
	GFX_RECT(GFX_REL(0, 14), width, 1, SEPARATOR);

	for(uint8_t i = 0; i < option_count; ++i)
	{
		// TODO: This macro is really bad
		GFX_PUSH_STATE();

		HIGHLIGHT(GFX_REL(0, 18 + i * 12 - 1), width, 12, i == highlight_index);

		// GFX_BITMAP(GFX_REL(4, 18 + i * 12), 9, 9, GFX_BACKGROUND, GFX_BLACK,
		// 				 i == selected_index ? CheckboxChecked_9x9 : CheckboxEmpty_9x9);
		// GFX_TEXT(GFX_REL(16, 18 + i * 12), GFX_BLACK, options[i]);
		GFX_BITMAP(GFX_REL(4, 1), 9, 9, GFX_BACKGROUND, GFX_BLACK,
						 i == selected_index ? CheckboxChecked_9x9 : CheckboxEmpty_9x9);
		GFX_TEXT(GFX_REL(16, 1), GFX_BLACK, options[i]);

		GFX_POP_STATE();
	}

	*out_width = width;
	*out_height = height;
}

#endif

