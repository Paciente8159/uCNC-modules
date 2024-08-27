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
#include "config.h"

#ifdef GUI_STYLE_WIN9X

#include "lvgl.h"

#define BITMAP_IMPL
#include "bitmaps/lock.h"
#include "bitmaps/warning.h"
#include "bitmaps/checkbox.h"
#include "bitmaps/move.h"
#include "bitmaps/zero.h"
#include "bitmaps/menu.h"
#include "bitmaps/close.h"
#include "bitmaps/dropdown_arrow.h"

#include "styles.h"
#include "colors.h"

#include "fonts/pixel.h"

extern void style_create_startup_screen();
extern void style_create_idle_screen();
extern void style_create_movement_screen();
extern void style_create_jog_screen();

win9x_styles g_styles;
lv_obj_t* g_current_screen = 0;

void style_init(lv_display_t *display)
{
	lv_theme_t *theme = lv_theme_simple_init(display);
	lv_display_set_theme(display, theme);


	lv_style_init(&g_styles.screen);
	lv_style_set_bg_color(&g_styles.screen, bg_base);
	lv_style_set_text_font(&g_styles.screen, &font_pixel_bold_11pt);
	lv_style_set_text_color(&g_styles.screen, col_black);


	lv_style_init(&g_styles.button);
	lv_style_set_bg_color(&g_styles.button, bg_base);

	lv_style_set_border_color(&g_styles.button, shadow_light);
	lv_style_set_border_side(&g_styles.button, LV_BORDER_SIDE_FULL);
	lv_style_set_border_width(&g_styles.button, 2);
	lv_style_set_image_recolor(&g_styles.button, col_black);


	lv_style_init(&g_styles.text_button);
	lv_style_set_text_color(&g_styles.text_button, col_black);
	lv_style_set_text_font(&g_styles.text_button, &font_pixel_7pt);

	lv_style_set_pad_ver(&g_styles.text_button, 3);
	lv_style_set_pad_hor(&g_styles.text_button, 6);


	lv_style_set_outline_opa(&g_styles.outline, LV_OPA_COVER);
	lv_style_set_outline_color(&g_styles.outline, col_black);
	lv_style_set_outline_width(&g_styles.outline, 1);


	lv_style_init(&g_styles.container);
	lv_style_set_bg_color(&g_styles.container, bg_box);
	lv_style_set_bg_opa(&g_styles.container, LV_OPA_COVER);

	lv_style_set_border_color(&g_styles.container, shadow_dark);
	lv_style_set_border_side(&g_styles.container, LV_BORDER_SIDE_FULL);
	lv_style_set_border_width(&g_styles.container, 2);

	lv_style_set_text_color(&g_styles.container, col_black);


	lv_style_init(&g_styles.table_item);
	lv_style_set_bg_opa(&g_styles.table_item, LV_OPA_TRANSP);
	lv_style_set_border_color(&g_styles.table_item, separator);
	lv_style_set_border_width(&g_styles.table_item, 1);
	lv_style_set_border_side(&g_styles.table_item, LV_BORDER_SIDE_FULL);
	lv_style_set_pad_all(&g_styles.table_item, 4);


	style_create_startup_screen();
	style_create_idle_screen();
#if MOVEMENT_MENU
	style_create_movement_screen();
#endif
#if CUSTOM_JOG_MENU
	style_create_jog_screen();
#endif
}

#endif

