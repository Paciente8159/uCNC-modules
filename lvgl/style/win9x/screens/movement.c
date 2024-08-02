/*
	Name: movement.c
	Description: Screen for moving into specified coordinates

	Copyright: Copyright (c) Patryk Mierzyński
	Author: Patryk Mierzyński
	Date: 30/07/2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the GNU General Public License for more details.
*/

#include "../../support.h"
#include "src/modules/system_menu.h"

#ifdef TFT_STYLE_WIN9X

#include "../utility.h"

#include "../bitmaps/checkbox.h"

#define COORD_BOX_SIZE ((COORDINATE_DECIMAL_DIGITS + COORDINATE_FRACTIONAL_DIGITS + 2) * 15)

static float position[3];
static uint8_t movement_type = 0;
static uint8_t coordinate_type = 0;

typedef struct assign_action_ {
	uint8_t *ptr;
	uint8_t value;
} assign_action_t;

static bool action_assign_value(uint8_t action, system_menu_item_t *item)
{
	assign_action_t *aarg = (assign_action_t*)item->argptr;
	if(action == SYSTEM_MENU_ACTION_SELECT)
	{
		*aarg->ptr = aarg->value;
		return true;
	}
	return false;
}

GFX_DECL_ELEMENT(coordinate_box, uint16_t x, uint16_t y, bool sign, uint8_t decimal, uint8_t fraction, float value, int highlight_index) {
	GFX_SET_FONT(FONT_PIXEL_MONO, 2);

	uint16_t width = (decimal + fraction + (sign ? 1 : 0) + (fraction > 0 ? 1 : 0)) * 15 - 1;

	TWO_TONE_BORDER(x-2, y-2, width+4, 26+4, 2, SHADOW_DARK, SHADOW_LIGHT);
	GFX_CONTAINER(x, y, width, 26, BOX_BACKGROUND);

	uint16_t offset = 0;
	char str[2] = "0";

	int32_t divider = 1;
	for(int i = 0; i < fraction; ++i)
	{
		value *= 10;
		divider *= 10;
	}
	for(int i = 0; i < decimal - 1; ++i)
	{
		divider *= 10;
	}

	int32_t int_value = (int32_t)ABS(value);

	if(sign)
	{
		GFX_TEXT(GFX_REL(2, 4), GFX_BLACK, value >= 0 ? "+" : "-");
		offset = 15;
	}

	for(int i = 0; i < decimal; ++i)
	{
		uint8_t digit = (int32_t)(int_value / divider) % 10;
		str[0] = '0' + digit;
		divider /= 10;

		GFX_TEXT(GFX_REL(2+offset, 4), GFX_BLACK, str);
		if(offset > 0)
			GFX_RECT(GFX_REL(offset-1, 0), 1, 26, SEPARATOR);
		offset += 15;
	}
	if(fraction > 0) 
	{
		GFX_TEXT(GFX_REL(2+offset, 4), GFX_BLACK, ".");
		GFX_RECT(GFX_REL(offset-1, 0), 1, 26, SEPARATOR);
		offset += 15;
		for(int i = 0; i < fraction; ++i)
		{
			uint8_t digit = (int32_t)(int_value / divider) % 10;
			str[0] = '0' + digit;
			divider /= 10;

			GFX_TEXT(GFX_REL(2+offset, 4), GFX_BLACK, str);
			GFX_RECT(GFX_REL(offset-1, 0), 1, 26, SEPARATOR);
			offset += 15;
		}
	}
}

GFX_DECL_SCREEN(movement)
{
	GFX_SCREEN_HEADER();
	GFX_SET_FONT(FONT_PIXEL, 2);

	GFX_CLEAR(BASE_BACKGROUND);

	GFX_CONTAINER(0, 0, GFX_DISPLAY_WIDTH, 24, TOP_BAR);

	char str[64];

	rom_strcpy(str, __romstr__(STR_MOVEMENT));
	GFX_TEXT(GFX_CENTER_TEXT_OFFSET(GFX_DISPLAY_WIDTH, str), 2, GFX_WHITE, str);

	uint16_t w, h;
	uint16_t x = 10, y = 24 + 10;

	// TODO: Grab the strings from language file
	GFX_SET_ORIGIN(x, y);
	const char* opts_move[] = { "Rapid", "Linear" };
	GFX_DRAW_ELEMENT(win9x_list_selector, "Movement type", opts_move, 2, 0, g_system_menu.current_index, &w, &h);

	x += w + 14;
	GFX_SET_ORIGIN(x, y);
	const char* opts_coord[] = { "Workspace", "Relative", "Machine" };
	GFX_DRAW_ELEMENT(win9x_list_selector, "Coordinate type", opts_coord, 3, 0, g_system_menu.current_index - 2, &w, &h);
	
	GFX_SET_BACKGROUND(BASE_BACKGROUND);

	GFX_SET_ORIGIN(200, 24+10+2)
	GFX_TEXT(GFX_REL(0, 0), GFX_BLACK, "Feed");
	GFX_DRAW_ELEMENT(coordinate_box, GFX_REL(54, -2), false, 4, 0, 0, -1);

	GFX_SET_ORIGIN(0, 100);
	TWO_TONE_BORDER(GFX_REL(0, 0), GFX_DISPLAY_WIDTH, GFX_DISPLAY_HEIGHT - 100, 2, SHADOW_LIGHT, SHADOW_DARK);

	GFX_TEXT(GFX_REL(10, 8), GFX_BLACK, "Coordinates");

	HIGHLIGHT(GFX_REL(13, 8+25), COORD_BOX_SIZE + 55, 34, g_system_menu.current_index == 5);
	GFX_BITMAP(GFX_REL(2, 8), 9, 9, GFX_BACKGROUND, GFX_BLACK, CheckboxEmpty_9x9, 2);
	GFX_TEXT(GFX_REL(28, 8), GFX_BLACK, FONT_PIXEL_MONO, 2, "X");
	GFX_DRAW_ELEMENT(coordinate_box, GFX_REL(52, 4), true, COORDINATE_DECIMAL_DIGITS, COORDINATE_FRACTIONAL_DIGITS, -123.54, -1);
	GFX_BACKGROUND = BASE_BACKGROUND;
	
	HIGHLIGHT(GFX_REL(0, 34), COORD_BOX_SIZE + 55, 34, g_system_menu.current_index == 6);
	GFX_BITMAP(GFX_REL(2, 8), 9, 9, GFX_BACKGROUND, GFX_BLACK, CheckboxEmpty_9x9, 2);
	GFX_TEXT(GFX_REL(28, 8), GFX_BLACK, FONT_PIXEL_MONO, 2, "Y");
	GFX_DRAW_ELEMENT(coordinate_box, GFX_REL(52, 4), true, COORDINATE_DECIMAL_DIGITS, COORDINATE_FRACTIONAL_DIGITS, -123.54, -1);
	GFX_BACKGROUND = BASE_BACKGROUND;

	HIGHLIGHT(GFX_REL(0, 34), COORD_BOX_SIZE + 55, 34, g_system_menu.current_index == 7);
	GFX_BITMAP(GFX_REL(2, 8), 9, 9, GFX_BACKGROUND, GFX_BLACK, CheckboxEmpty_9x9, 2);
	GFX_TEXT(GFX_REL(28, 8), GFX_BLACK, FONT_PIXEL_MONO, 2, "Z");
	GFX_DRAW_ELEMENT(coordinate_box, GFX_REL(52, 4), true, COORDINATE_DECIMAL_DIGITS, COORDINATE_FRACTIONAL_DIGITS, -123.54, -1);
	GFX_BACKGROUND = BASE_BACKGROUND;

	GFX_SET_ORIGIN(10, GFX_DISPLAY_HEIGHT - 35);
	GFX_DRAW_ELEMENT(win9x_button, 50, 25, "Back", g_system_menu.current_index == 8);
	
	GFX_SET_ORIGIN(65, GFX_DISPLAY_HEIGHT - 35);
	GFX_DRAW_ELEMENT(win9x_button, 50, 25, "Move", g_system_menu.current_index == 9);
}

static bool action_back(uint8_t action, system_menu_item_t *item)
{
	if(action == SYSTEM_MENU_ACTION_SELECT)
	{
		return true;
	}
	return false;
}

static bool action_move(uint8_t action, system_menu_item_t *item)
{
	if(action == SYSTEM_MENU_ACTION_SELECT)
	{
		return true;
	}
	return false;
}

void movement_screen_render(uint8_t flags)
{
	GFX_RENDER_SCREEN(movement);
}

void movement_screen_init()
{
	DECL_DYNAMIC_MENU(10, 1, movement_screen_render, 0);

	const static assign_action_t radio_rapid_arg = { &movement_type, 0 };
	const static assign_action_t radio_linear_arg = { &movement_type, 1 };
	DECL_MENU_ACTION(10, radio_rapid, 0, action_assign_value, &radio_rapid_arg);
	DECL_MENU_ACTION(10, radio_linear, 0, action_assign_value, &radio_linear_arg);

	const static assign_action_t radio_workspace_arg = { &coordinate_type, 0 };
	const static assign_action_t radio_relative_arg = { &coordinate_type, 1 };
	const static assign_action_t radio_machine_arg = { &coordinate_type, 2 };
	DECL_MENU_ACTION(10, radio_workspace, 0, action_assign_value, &radio_workspace_arg);
	DECL_MENU_ACTION(10, radio_relative, 0, action_assign_value, &radio_relative_arg);
	DECL_MENU_ACTION(10, radio_machine, 0, action_assign_value, &radio_machine_arg);

	DECL_MENU_VAR(10, x_pos, 0, &position[0], VAR_TYPE_FLOAT);
	DECL_MENU_VAR(10, y_pos, 0, &position[1], VAR_TYPE_FLOAT);
	DECL_MENU_VAR(10, z_pos, 0, &position[2], VAR_TYPE_FLOAT);

	DECL_MENU_ACTION(10, back, 0, action_back, 0);
	DECL_MENU_ACTION(10, move, 0, action_move, 0);
}

#endif

