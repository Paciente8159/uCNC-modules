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

#ifndef TFT_DISPLAY_H
#error "Please do not include this file manually"
#endif

#define BOX_INSET(x, y, w, h, bg) \
	GFX_RECT(x, y, w, 3, BORDER_DARK); \
	GFX_RECT(x, (y) + 3, 3, (h) - 3, BORDER_DARK); \
	GFX_RECT((x) + (w) - 3, (y) + 3, 3, (h) - 3, BORDER_LIGHT); \
	GFX_RECT((x) + 3, (y) + (h) - 3, (w) - 3, 3, BORDER_LIGHT); \
	GFX_RECT((x) + 3, (y) + 3, (w) - 6, (h) - 6, bg); \
	__gfx_rel_x = x; __gfx_rel_y = y; __gfx_last_background = bg;

#define BOX_OUTSET(x, y, w, h, bg) \
	GFX_RECT(x, y, w, 3, BORDER_LIGHT); \
	GFX_RECT(x, (y) + 3, 3, (h) - 3, BORDER_LIGHT); \
	GFX_RECT((x) + (w) - 3, (y) + 3, 3, (h) - 3, BORDER_DARK); \
	GFX_RECT((x) + 3, (y) + (h) - 3, (w) - 3, 3, BORDER_DARK); \
	GFX_RECT((x) + 3, (y) + 3, (w) - 6, (h) - 6, bg); \
	__gfx_rel_x = x; __gfx_rel_y = y; __gfx_last_background = bg;

#define BOX_INSET_dynamic(x, y, w, h, bg) \
	GFX_RECT(x, y, w, 3, BORDER_DARK); \
	GFX_RECT(x, (y) + 3, 3, (h) - 3, BORDER_DARK); \
	GFX_RECT((x) + (w) - 3, (y) + 3, 3, (h) - 3, BORDER_LIGHT); \
	GFX_RECT((x) + 3, (y) + (h) - 3, (w) - 3, 3, BORDER_LIGHT); \
	GFX_RECT_dynamic((x) + 3, (y) + 3, (w) - 6, (h) - 6, bg); \
	__gfx_rel_x = x; __gfx_rel_y = y; __gfx_last_background = bg;

#define BOX_OUTSET_dynamic(x, y, w, h, bg) \
	GFX_RECT(x, y, w, 3, BORDER_LIGHT); \
	GFX_RECT(x, (y) + 3, 3, (h) - 3, BORDER_LIGHT); \
	GFX_RECT((x) + (w) - 3, (y) + 3, 3, (h) - 3, BORDER_DARK); \
	GFX_RECT((x) + 3, (y) + (h) - 3, (w) - 3, 3, BORDER_DARK); \
	GFX_RECT_dynamic((x) + 3, (y) + 3, (w) - 6, (h) - 6, bg); \
	__gfx_rel_x = x; __gfx_rel_y = y; __gfx_last_background = bg;

static void get_alarm_string(char* dest)
{
	switch (cnc_get_alarm())
	{
		case 1:
			rom_strcpy(dest, __romstr__(STR_ALARM_1));
			break;
		case 2:
			rom_strcpy(dest, __romstr__(STR_ALARM_2));
			break;
		case 3:
			rom_strcpy(dest, __romstr__(STR_ALARM_3));
			break;
		case 4:
			rom_strcpy(dest, __romstr__(STR_ALARM_4));
			break;
		case 5:
			rom_strcpy(dest, __romstr__(STR_ALARM_5));
			break;
		case 6:
			rom_strcpy(dest, __romstr__(STR_ALARM_6));
			break;
		case 7:
			rom_strcpy(dest, __romstr__(STR_ALARM_7));
			break;
		case 8:
			rom_strcpy(dest, __romstr__(STR_ALARM_8));
			break;
		case 9:
			rom_strcpy(dest, __romstr__(STR_ALARM_9));
			break;
		case 10:
			rom_strcpy(dest, __romstr__(STR_ALARM_10));
			break;
		case 11:
			rom_strcpy(dest, __romstr__(STR_ALARM_11));
			break;
		case 12:
			rom_strcpy(dest, __romstr__(STR_ALARM_12));
			break;
		case 13:
			rom_strcpy(dest, __romstr__(STR_ALARM_13));
			break;
		default:
			rom_strcpy(dest, __romstr__(STR_ALARM_0));
			break;
	}
}

