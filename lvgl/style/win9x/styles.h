/*
	Name: styles.h
	Description: Predefined styles used in the GUI

	Copyright: Copyright (c) Patryk Mierzyński
	Author: Patryk Mierzyński
	Date: 04/08/2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the GNU General Public License for more details.
*/

#ifndef WIN9X_STYLES_H
#define WIN9X_STYLES_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"

typedef struct _styles {
	lv_style_t button;
	lv_style_t container;
	lv_style_t table_item;
} win9x_styles;

typedef struct _two_color_border {
	lv_color_t color1;
	lv_color_t color2;
	uint8_t width;
} win9x_two_color_border_t;

extern void win9x_two_color_border_draw(lv_event_t *event);
#define WIN9X_BORDER_PART_TWO(obj, color) lv_obj_add_event_cb(obj, win9x_two_color_border_draw, LV_EVENT_DRAW_MAIN_END, (lv_color_t*)&(color));

extern win9x_styles g_styles;

#ifdef __cplusplus
}
#endif

#endif

