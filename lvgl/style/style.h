/*
	Name: style.h
	Description: Base of the style subsystem

	Copyright: Copyright (c) Patryk Mierzyński
	Author: Patryk Mierzyński
	Date: 29/07/2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the GNU General Public License for more details.
*/

#ifndef LVGL_STYLE_H
#define LVGL_STYLE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"
#include "src/modules/system_menu.h"

extern void style_init(lv_display_t *display);

extern void style_startup();
extern void style_idle();
extern void style_popup(const char* text);
extern void style_alarm();

typedef struct _style_item_arg
{
	const char *text;
	uint8_t render_flags;
	uint16_t item_index;
	bool must_rebuild;
} list_menu_item_arg_t;

extern void style_list_menu_header(lv_obj_t *screen, const char *header);
extern void style_list_menu_item_label(lv_obj_t *screen, list_menu_item_arg_t *arg);
extern void style_list_menu_item_value(lv_obj_t *screen, list_menu_item_arg_t *arg);
extern void style_list_menu_nav_back(lv_obj_t *screen, bool is_hover, bool rebuild);
extern void style_list_menu_footer(lv_obj_t *screen);

extern void style_list_menu(lv_obj_t *screen);

#ifdef __cplusplus
}
#endif

#endif

