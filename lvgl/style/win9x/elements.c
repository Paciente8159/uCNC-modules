#include "../config.h"

#ifdef GUI_STYLE_WIN9X

#include "elements.h"
#include "styles.h"
#include "colors.h"
#include "fonts/pixel.h"

#include "bitmaps/checkbox.h"

static void radio_list_cb(lv_event_t *event)
{
	uint8_t *index_ptr = (uint8_t*)lv_event_get_user_data(event);

	lv_obj_t *clicked_obj = lv_event_get_target(event);
	lv_obj_t *root = lv_obj_get_parent(clicked_obj);
	lv_obj_t *old_obj = lv_obj_get_child(root, *index_ptr + 1);

	lv_obj_remove_state(old_obj, LV_STATE_CHECKED);
	lv_obj_add_state(clicked_obj, LV_STATE_CHECKED);
	*index_ptr = lv_obj_get_index(clicked_obj) - 1;
}

lv_obj_t *win9x_radio_list(lv_obj_t *parent, const char *header, const char **items, int item_count, uint8_t *index_ptr)
{
	lv_obj_t *root = lv_obj_create(parent);
	lv_obj_add_style(root, &g_styles.container, LV_PART_MAIN);
	WIN9X_BORDER_PART_TWO(root, shadow_light);

	lv_obj_set_size(root, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

	lv_obj_set_layout(root, LV_LAYOUT_FLEX);
	lv_obj_set_flex_flow(root, LV_FLEX_FLOW_COLUMN);

	lv_obj_t *headerlb = lv_label_create(root);
	lv_label_set_text(headerlb, header);
	
	lv_obj_set_style_pad_all(headerlb, 3, LV_PART_MAIN);
	lv_obj_set_style_pad_bottom(headerlb, 1, LV_PART_MAIN);

	lv_obj_set_style_border_width(headerlb, 1, LV_PART_MAIN);
	lv_obj_set_style_border_side(headerlb, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN);
	lv_obj_set_style_border_color(headerlb, separator, LV_PART_MAIN);

	for(int i = 0; i < item_count; ++i)
	{
		lv_obj_t *checkbox = lv_checkbox_create(root);
		lv_checkbox_set_text(checkbox, items[i]);

		lv_obj_set_style_pad_all(checkbox, 2, LV_PART_MAIN);
		if(i != item_count - 1)
			lv_obj_set_style_pad_bottom(checkbox, 0, LV_PART_MAIN);

		if(*index_ptr == i)
			lv_obj_add_state(checkbox, LV_STATE_CHECKED);

		lv_obj_set_style_bg_opa(checkbox, LV_OPA_TRANSP, LV_PART_INDICATOR);
		lv_obj_set_style_bg_opa(checkbox, LV_OPA_TRANSP, LV_PART_INDICATOR | LV_STATE_CHECKED);
		lv_obj_set_style_pad_right(checkbox, 5, LV_PART_INDICATOR);

		lv_obj_set_style_bg_image_src(checkbox, &Img_CheckboxEmpty, LV_PART_INDICATOR);
		lv_obj_set_style_bg_image_recolor(checkbox, col_black, LV_PART_INDICATOR);

		lv_obj_set_style_bg_image_src(checkbox, &Img_CheckboxChecked, LV_PART_INDICATOR | LV_STATE_CHECKED);
		lv_obj_set_style_bg_image_recolor(checkbox, col_black, LV_PART_INDICATOR | LV_STATE_CHECKED);

		lv_obj_set_style_bg_opa(checkbox, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_FOCUSED);
		lv_obj_set_style_bg_color(checkbox, select_highlight, LV_PART_MAIN | LV_STATE_FOCUSED);

		lv_obj_add_event_cb(checkbox, radio_list_cb, LV_EVENT_VALUE_CHANGED, index_ptr);

		lv_group_add_obj(lv_group_get_default(), checkbox);
	}

	return root;
}

lv_obj_t *win9x_button(lv_obj_t *parent, const char *text)
{
	lv_obj_t *btn = lv_button_create(parent);
	lv_obj_add_style(btn, &g_styles.button, LV_PART_MAIN);
	lv_obj_set_size(btn, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
	WIN9X_BORDER_PART_TWO(btn, shadow_dark);

	lv_obj_t *label = lv_label_create(btn);
	lv_label_set_text(label, text);
	lv_obj_center(label);

	lv_obj_set_style_text_color(label, col_black, LV_PART_MAIN);
	lv_obj_set_style_text_font(label, &font_pixel_7pt, LV_PART_MAIN);

	lv_obj_set_style_pad_ver(btn, 3, LV_PART_MAIN);
	lv_obj_set_style_pad_hor(btn, 6, LV_PART_MAIN);

	lv_obj_set_style_outline_opa(btn, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_FOCUSED);
	lv_obj_set_style_outline_color(btn, col_black, LV_PART_MAIN | LV_STATE_FOCUSED);
	lv_obj_set_style_outline_width(btn, 1, LV_PART_MAIN | LV_STATE_FOCUSED);

	lv_obj_set_style_bg_color(btn, button_click, LV_PART_MAIN | LV_STATE_PRESSED);

	return btn;
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

