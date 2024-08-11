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

#include "../../config.h"

#ifdef GUI_STYLE_WIN9X

#include "lvgl.h"

#include "../colors.h"
#include "../fonts/pixel.h"

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

void style_list_menu_nav_back(lv_obj_t *screen, bool is_hover)
{

}

void style_list_menu_item_label(lv_obj_t *screen, const char *label)
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
	
	lv_obj_t *labelob = lv_label_create(last_item_entry);
	lv_label_set_text(labelob, label);
	lv_obj_set_pos(labelob, 0, 0);
}

void style_list_menu_item_value(lv_obj_t *screen, const char *label)
{
	if(label != NULL)
	{
		lv_obj_t *valueob = lv_label_create(last_item_entry);
		lv_label_set_text(valueob, label);
		lv_obj_set_pos(valueob, 150, 0);
	}
}

void style_list_menu_footer(lv_obj_t *screen)
{

}

lv_obj_t *style_list_menu_get_item(lv_obj_t *screen, uint16_t index)
{
	lv_obj_t *list = lv_obj_get_child(screen, 1);
	return lv_obj_get_child(list, index);
}

lv_obj_t *style_list_menu_get_item_label(lv_obj_t *item_entry)
{
	return lv_obj_get_child(item_entry, 0);
}

lv_obj_t *style_list_menu_get_item_value(lv_obj_t *item_entry)
{
	return lv_obj_get_child(item_entry, 1);
}

#endif

