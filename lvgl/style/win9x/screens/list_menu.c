/*
	Name: list_menu.c
	Description: List menu builder

	Copyright: Copyright (c) Patryk Mierzyński
	Author: Patryk Mierzyński
	Date: 09/08/2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the GNU General Public License for more details.
*/

#include "../config.h"

#ifdef GUI_STYLE_WIN9X

#include "lvgl.h"
#include "src/modules/system_menu.h"

#include "../colors.h"
#include "../styles.h"
#include "../fonts/pixel.h"
#include "../fonts/pixel_mono.h"
#include "../bitmaps/close.h"
#include "../../style.h"
#include "../../../lvgl_support.h"

static lv_obj_t *item_list;
static lv_obj_t *last_item_entry;

void style_list_menu_header(lv_obj_t *screen, const char *header)
{
	lv_obj_set_style_bg_color(screen, bg_base, LV_PART_MAIN);
	lv_obj_set_style_text_font(screen, &font_pixel_bold_11pt, LV_PART_MAIN);

	lv_obj_t *topbar = lv_obj_create(screen);
	lv_obj_set_size(topbar, 480, 22);
	lv_obj_set_style_bg_color(topbar, bg_title_bar, LV_PART_MAIN);
	lv_obj_set_style_text_color(topbar, col_white, LV_PART_MAIN);

	lv_obj_t *label = lv_label_create(topbar);
	lv_obj_center(label);
	lv_label_set_text(label, header);

	item_list = lv_obj_create(screen);
	lv_obj_set_pos(item_list, 0, 32);
	lv_obj_set_size(item_list, LV_PCT(100), 320 - 32);
	lv_obj_set_layout(item_list, LV_LAYOUT_FLEX);
	lv_obj_set_flex_flow(item_list, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_style_bg_opa(item_list, LV_OPA_TRANSP, LV_PART_MAIN);
	lv_obj_set_style_text_color(item_list, col_black, LV_PART_MAIN);
}

void style_list_menu_nav_back(lv_obj_t *screen, bool is_hover, bool rebuild)
{
	lv_obj_t *back;
	if(rebuild)
	{
		back = lv_obj_create(lv_obj_get_child(screen, 0));
		lv_obj_add_style(back, &g_styles.button, LV_PART_MAIN);
		lv_obj_set_size(back, 18, 18);
		WIN9X_BORDER_PART_TWO(back, shadow_dark);

		lv_obj_set_align(back, LV_ALIGN_RIGHT_MID);
		lv_obj_set_pos(back, -2, 0);

		lv_obj_set_style_bg_color(back, select_highlight, LV_PART_MAIN | LV_STATE_FOCUSED);

		lv_obj_t *image = lv_image_create(back);
		lv_obj_center(image);
		lv_image_set_src(image, &Img_Close);
		lv_obj_set_style_image_recolor(image, col_black, LV_PART_MAIN);
	}
	else
	{
		lv_obj_t *topbar = lv_obj_get_child(screen, 0);
		back = lv_obj_get_child(topbar, 1);
	}

	if(is_hover && !lv_obj_has_state(back, LV_STATE_FOCUSED))
		lv_obj_add_state(back, LV_STATE_FOCUSED);
	else if(!is_hover && lv_obj_has_state(back, LV_STATE_FOCUSED))
		lv_obj_remove_state(back, LV_STATE_FOCUSED);
}

void style_list_menu_item_label(lv_obj_t *screen, list_menu_item_arg_t *arg)
{
	lv_obj_t *entry, *label;
	if(arg->must_rebuild)
	{
		last_item_entry = lv_obj_create(item_list);

		lv_obj_set_size(last_item_entry, LV_PCT(100), LV_SIZE_CONTENT);
		lv_obj_set_style_pad_ver(last_item_entry, 2, LV_PART_MAIN);
		lv_obj_set_style_pad_hor(last_item_entry, 10, LV_PART_MAIN);

		lv_color_t bg_col = bg_base;
		if(lv_obj_get_index(last_item_entry) % 2 == 0)
		{
			bg_col = bg_box;
		}
		lv_obj_set_style_bg_color(last_item_entry, bg_col, LV_PART_MAIN);
		lv_obj_set_style_bg_color(last_item_entry, select_highlight, LV_PART_MAIN | LV_STATE_FOCUSED);

		label = lv_label_create(last_item_entry);
		lv_obj_set_align(label, LV_ALIGN_LEFT_MID);
		lv_obj_set_pos(label, 0, 0);

		entry = last_item_entry;
	}
	else
	{
		entry = lv_obj_get_child(item_list, arg->item_index);
		label = lv_obj_get_child(entry, 0);
	}
	
	if(strcmp(lv_label_get_text(label), arg->text))
	{
		// Set text if changed
		lv_label_set_text(label, arg->text);
	}

	if((arg->render_flags & SYSTEM_MENU_MODE_SELECT) && !lv_obj_has_state(entry, LV_STATE_FOCUSED))
		lv_obj_add_state(entry, LV_STATE_FOCUSED);
	else if(!(arg->render_flags & SYSTEM_MENU_MODE_SELECT) && lv_obj_has_state(entry, LV_STATE_FOCUSED))
		lv_obj_remove_state(entry, LV_STATE_FOCUSED);
}

void style_list_menu_item_value(lv_obj_t *screen, list_menu_item_arg_t *arg)
{
	lv_obj_t *value, *cursor;
	if(arg->must_rebuild)
	{
		value = lv_label_create(last_item_entry);
		lv_obj_set_pos(value, 150, 0);

		if(arg->render_flags & SYSTEM_MENU_MODE_EDIT)
		{
			lv_obj_set_style_text_font(value, &font_pixel_mono_14pt, LV_PART_MAIN);

			cursor = lv_obj_create(last_item_entry);
			lv_obj_set_size(cursor, 12, 2);
			lv_obj_set_style_bg_color(cursor, col_black, LV_PART_MAIN);
		}
	}
	else
	{
		lv_obj_t *entry = lv_obj_get_child(item_list, arg->item_index);
		value = lv_obj_get_child(entry, 1);
		cursor = lv_obj_get_child(entry, 2);
	}

	if(strcmp(lv_label_get_text(value), arg->text))
	{
		// Set text if changed
		lv_label_set_text(value, arg->text);
	}

	if(arg->render_flags & SYSTEM_MENU_MODE_EDIT)
	{
		const system_menu_item_t *item = system_menu_get_current_item();
		uint8_t vartype = (uint8_t)VARG_CONST(item->action_arg);

		uint8_t text_len = strlen(arg->text);
		int8_t mult = g_system_menu.current_multiplier;
		if(mult >= 0)
		{
			if(vartype == VAR_TYPE_FLOAT && mult >= 3)
				mult += 1;
			int8_t offset = text_len - mult - 1;
			lv_obj_set_pos(cursor, 150 + offset * 12, 20);

			lv_obj_remove_flag(cursor, LV_OBJ_FLAG_HIDDEN);
		}
		else
		{
			lv_obj_add_flag(cursor, LV_OBJ_FLAG_HIDDEN);
		}
	}
}

void style_list_menu(lv_obj_t *screen)
{
	extern lv_obj_t *g_current_screen;
	if(g_current_screen != screen)
	{
		g_current_screen = screen;
		lv_screen_load(screen);
		lvgl_set_indev_group(NULL);
	}
}

#endif

