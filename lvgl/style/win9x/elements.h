/*
	Name: elements.h
	Description: Predefined components for the GUI

	Copyright: Copyright (c) Patryk Mierzyński
	Author: Patryk Mierzyński
	Date: 06/08/2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the GNU General Public License for more details.
*/

#ifndef WIN9X_ELEMENTS_H
#define WIN9X_ELEMENTS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"

extern lv_obj_t *win9x_radio_list(lv_obj_t *parent, const char *header, const char **items, int item_count, uint8_t *index_ptr);
extern lv_obj_t *win9x_button(lv_obj_t *parent, const char *text);

#define STYLE_LOAD_SCREEN(screen_name) \
void style_##screen_name() { \
	extern lv_obj_t *g_current_screen; \
	if(g_current_screen != screen) \
	{ \
		g_current_screen = screen; \
		lv_screen_load(screen); \
extern void lvgl_set_indev_group(lv_group_t*); \
		lvgl_set_indev_group(NULL); \
	} \
}

#define STYLE_LOAD_SCREEN_UPDATE(screen_name) \
void style_##screen_name() { \
	extern lv_obj_t *g_current_screen; \
	if(g_current_screen != screen) \
	{ \
		g_current_screen = screen; \
		lv_screen_load(screen); \
extern void lvgl_set_indev_group(lv_group_t*); \
		lvgl_set_indev_group(NULL); \
	} \
	screen_name##_update(); \
}

#define STYLE_LOAD_SCREEN_WITH_GROUP(screen_name) \
void style_##screen_name() { \
	extern lv_obj_t *g_current_screen; \
	if(g_current_screen != screen) \
	{ \
		g_current_screen = screen; \
		lv_screen_load(screen); \
extern void lvgl_set_indev_group(lv_group_t*); \
		lvgl_set_indev_group(group); \
	} \
}

#define STYLE_LOAD_SCREEN_WITH_GROUP_ENTER(screen_name) \
void style_##screen_name() { \
	extern lv_obj_t *g_current_screen; \
	if(g_current_screen != screen) \
	{ \
		g_current_screen = screen; \
		enter(); \
		lv_screen_load(screen); \
extern void lvgl_set_indev_group(lv_group_t*); \
		lvgl_set_indev_group(group); \
	} \
}

#define STYLE_LOAD_SCREEN_UPDATE_WITH_GROUP(screen_name) \
void style_##screen_name() { \
	extern lv_obj_t *g_current_screen; \
	if(g_current_screen != screen) \
	{ \
		g_current_screen = screen; \
		lv_screen_load(screen); \
extern void lvgl_set_indev_group(lv_group_t*); \
		lvgl_set_indev_group(group); \
	} \
	screen_name##_update(); \
}

#define STYLE_LOAD_SCREEN_UPDATE_WITH_GROUP_ENTER(screen_name) \
void style_##screen_name() { \
	extern lv_obj_t *g_current_screen; \
	if(g_current_screen != screen) \
	{ \
		g_current_screen = screen; \
		enter(); \
		lv_screen_load(screen); \
extern void lvgl_set_indev_group(lv_group_t*); \
		lvgl_set_indev_group(group); \
	} \
	screen_name##_update(); \
}

#ifdef __cplusplus
}
#endif

#endif

