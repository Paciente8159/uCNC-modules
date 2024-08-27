/*
	Name: popup.c
	Description: Modal popup screen

	Copyright: Copyright (c) Patryk Mierzyński
	Author: Patryk Mierzyński
	Date: 11/08/2024

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
#include "src/cnc.h"
#include "src/modules/system_menu.h"

#include "../colors.h"
#include "../styles.h"
#include "../elements.h"
#include "../fonts/pixel.h"
#include "../../../lvgl_support.h"

static lv_obj_t *last_popup = NULL;

void style_popup(const char *text)
{
	lv_obj_t *box = lv_obj_create(lv_layer_top());
	lv_obj_set_size(box, LV_PCT(50), LV_SIZE_CONTENT);
	lv_obj_center(box);

	lv_obj_set_style_border_color(box, shadow_light, LV_PART_MAIN);
	lv_obj_set_style_border_width(box, 2, LV_PART_MAIN);
	lv_obj_set_style_border_side(box, LV_BORDER_SIDE_FULL, LV_PART_MAIN);
	WIN9X_BORDER_PART_TWO(box, shadow_dark);

	lv_obj_t *topbar = lv_obj_create(box);
	lv_obj_set_size(topbar, LV_PCT(100), 22);
	lv_obj_set_style_bg_color(topbar, bg_title_bar, LV_PART_MAIN);
	lv_obj_set_style_text_color(topbar, col_white, LV_PART_MAIN);
	lv_obj_set_style_text_font(topbar, &font_pixel_bold_11pt, LV_PART_MAIN);

	lv_obj_t *title = lv_label_create(topbar);
	lv_obj_set_pos(title, 7, 4);

	char str[32];
	rom_strcpy(str, __rom__(STR_POPUP_TITLE));
	lv_label_set_text(title, str);

	lv_obj_t *body = lv_obj_create(box);
	lv_obj_set_style_text_color(body, col_black, LV_PART_MAIN);
	lv_obj_set_style_text_font(body, &font_pixel_7pt, LV_PART_MAIN);
	lv_obj_set_style_bg_opa(body, LV_OPA_TRANSP, LV_PART_MAIN);
	lv_obj_set_pos(body, 0, 22);
	lv_obj_set_style_pad_all(body, 7, LV_PART_MAIN);
	lv_obj_set_size(body, LV_PCT(100), LV_SIZE_CONTENT);

	lv_obj_t *text_object = lv_label_create(body);
	lv_label_set_text(text_object, text);

	last_popup = box;
}

void style_remove_popup()
{
	if(last_popup != NULL)
	{
		lv_obj_delete(last_popup);
		last_popup = NULL;
	}
}

#endif

