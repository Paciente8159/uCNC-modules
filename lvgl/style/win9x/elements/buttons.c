/*
	Name: buttons.c
	Description: Creates various button elements.

	Copyright: Copyright (c) Patryk Mierzyński
	Author: Patryk Mierzyński
	Date: 20/08/2024

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

#include "../elements.h"
#include "../styles.h"
#include "../colors.h"
#include "../fonts/pixel.h"

#include "../bitmaps/close.h"

lv_obj_t *win9x_close_button(lv_obj_t *parent)
{
	lv_obj_t *back = lv_obj_create(parent);
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
	return back;
}

lv_obj_t *win9x_button(lv_obj_t *parent, const char *text)
{
	lv_obj_t *btn = lv_button_create(parent);
	lv_obj_add_style(btn, &g_styles.button, LV_PART_MAIN);
	lv_obj_add_style(btn, &g_styles.text_button, LV_PART_MAIN);
	lv_obj_add_style(btn, &g_styles.outline, LV_PART_MAIN | LV_STATE_FOCUSED);
	lv_obj_set_size(btn, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
	WIN9X_BORDER_PART_TWO(btn, shadow_dark);

	lv_obj_t *label = lv_label_create(btn);
	lv_label_set_text(label, text);
	lv_obj_center(label);

	return btn;
}

#endif

