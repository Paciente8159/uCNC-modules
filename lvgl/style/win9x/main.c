/*
	Name: main.c
	Description: Main file of the Win9x GUI style

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
#include "../config.h"

#ifdef TFT_STYLE_WIN9X

#include "lvgl.h"

#define BITMAP_IMPL
#include "bitmaps/lock.h"
#include "bitmaps/warning.h"
#include "bitmaps/checkbox.h"

#include "styles.h"
#include "colors.h"

extern void style_create_startup_screen();
extern void style_create_idle_screen();
extern void style_create_movement_screen();

win9x_styles g_styles;
lv_obj_t* g_current_screen = 0;
lv_indev_t *g_indev = 0;

void style_init(lv_display_t *display, lv_indev_t *indev)
{
	lv_theme_t *theme = lv_theme_simple_init(display);
	lv_display_set_theme(display, theme);
	g_indev = indev;


	lv_style_init(&g_styles.button);
	lv_style_set_bg_color(&g_styles.button, bg_base);

	lv_style_set_border_color(&g_styles.button, shadow_light);
	lv_style_set_border_side(&g_styles.button, LV_BORDER_SIDE_FULL);
	lv_style_set_border_width(&g_styles.button, 2);


	lv_style_init(&g_styles.container);
	lv_style_set_bg_color(&g_styles.container, bg_box);
	lv_style_set_bg_opa(&g_styles.container, LV_OPA_COVER);

	lv_style_set_border_color(&g_styles.container, shadow_dark);
	lv_style_set_border_side(&g_styles.container, LV_BORDER_SIDE_FULL);
	lv_style_set_border_width(&g_styles.container, 2);


	lv_style_init(&g_styles.table_item);
	lv_style_set_bg_opa(&g_styles.table_item, LV_OPA_TRANSP);
	lv_style_set_border_color(&g_styles.table_item, separator);
	lv_style_set_border_width(&g_styles.table_item, 1);
	lv_style_set_border_side(&g_styles.table_item, LV_BORDER_SIDE_FULL);
	lv_style_set_pad_all(&g_styles.table_item, 4);


	style_create_startup_screen();
	style_create_idle_screen();
	style_create_movement_screen();
}

void win9x_two_color_border_draw(lv_event_t *event)
{
	lv_layer_t *layer = lv_event_get_layer(event);
	lv_obj_t *target = lv_event_get_target_obj(event);

	lv_draw_rect_dsc_t dsc = {};
	dsc.bg_opa = LV_OPA_TRANSP;
	dsc.border_color = *(lv_color_t*)lv_event_get_user_data(event);
	dsc.border_width = lv_obj_get_style_border_width(target, LV_PART_MAIN);
	dsc.border_opa = LV_OPA_COVER;
	dsc.border_side = LV_BORDER_SIDE_RIGHT | LV_BORDER_SIDE_BOTTOM;
	lv_draw_rect(layer, &dsc, &target->coords);
}

#endif

