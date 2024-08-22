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

static void switch_indicator_border(lv_event_t *event)
{
	lv_layer_t *layer = lv_event_get_layer(event);
	lv_obj_t *sw = lv_event_get_target(event);
	bool checked = lv_obj_has_state(sw, LV_STATE_CHECKED);

	int32_t x = sw->coords.x1;
	int32_t y = sw->coords.y1;

	if(checked)
		x += 10;
	lv_area_t area = {
		x + 2, y + 2,
		x + 13, y + 13,
	};


	lv_draw_rect_dsc_t dsc = {};
	dsc.bg_opa = LV_OPA_TRANSP;
	dsc.border_color = shadow_dark;
	dsc.border_width = 2;
	dsc.border_opa = LV_OPA_COVER;
	dsc.border_side = LV_BORDER_SIDE_RIGHT | LV_BORDER_SIDE_BOTTOM;
	lv_draw_rect(layer, &dsc, &area);
}

lv_obj_t *win9x_switch(lv_obj_t *parent)
{
	lv_obj_t *sw = lv_switch_create(parent);

	lv_obj_set_size(sw, 26, 16);

	lv_obj_add_style(sw, &g_styles.container, LV_PART_MAIN);
	lv_obj_set_style_border_color(sw, shadow_darker, LV_PART_MAIN);
	lv_obj_set_style_bg_color(sw, bg_other, LV_PART_MAIN);
	WIN9X_BORDER_PART_TWO(sw, shadow_light);

	lv_obj_set_style_bg_opa(sw, LV_OPA_TRANSP, LV_PART_INDICATOR);
	lv_obj_set_style_pad_all(sw, -2, LV_PART_KNOB);
	lv_obj_set_style_bg_color(sw, bg_base, LV_PART_KNOB);

	lv_obj_set_style_border_width(sw, 2, LV_PART_KNOB);
	lv_obj_set_style_border_color(sw, shadow_light, LV_PART_KNOB);
	lv_obj_set_style_border_opa(sw, LV_OPA_COVER, LV_PART_KNOB);

	lv_obj_add_style(sw, &g_styles.outline, LV_PART_MAIN | LV_STATE_FOCUSED);

	lv_obj_add_event_cb(sw, switch_indicator_border, LV_EVENT_DRAW_MAIN_END, NULL);
	return sw;
}

#endif

