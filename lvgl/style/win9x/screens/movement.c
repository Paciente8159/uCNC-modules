/*
	Name: movement.c
	Description: Screen for moving into specified coordinates

	Copyright: Copyright (c) Patryk Mierzyński
	Author: Patryk Mierzyński
	Date: 30/07/2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the GNU General Public License for more details.
*/

#include "../config.h"

#if defined(GUI_STYLE_WIN9X) && MOVEMENT_MENU

#include "lvgl.h"
#include "math.h"
#include "src/cnc.h"
#include "src/modules/system_menu.h"

#include "../colors.h"
#include "../styles.h"
#include "../fonts/pixel.h"
#include "../fonts/pixel_mono.h"
#include "../bitmaps/checkbox.h"
#include "../elements.h"
#include "../../../lvgl_support.h"

static lv_obj_t *screen;
static lv_group_t *group;

static uint8_t coordinate_type = 0;

static char axis[] = { 'X', 'Y', 'Z' };

static lv_obj_t *spinboxes[3];
static lv_obj_t *switches[3];

static lv_obj_t *make_coordinate_box(lv_obj_t *parent, const char *label)
{
	lv_obj_t *root = lv_obj_create(parent);
	lv_obj_set_size(root, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
	lv_obj_set_style_bg_opa(root, LV_OPA_TRANSP, LV_PART_MAIN);
	lv_obj_set_style_text_font(root, &font_pixel_mono_14pt, LV_PART_MAIN);

	lv_obj_set_style_pad_all(root, 2, LV_PART_MAIN);

	lv_obj_t *sw = win9x_switch(root);
	lv_obj_set_pos(sw, 0, 8);
	lv_group_add_obj(group, sw);

	lv_obj_t *labelob = lv_label_create(root);
	lv_label_set_text(labelob, label);
	lv_obj_set_pos(labelob, 35, 2 + 4);

	lv_obj_t *input = win9x_number_input(root);
	lv_obj_set_pos(input, 65, 0);
	lv_obj_set_width(input, 120);

	lv_spinbox_set_digit_format(input, 6, 3);
	lv_spinbox_set_range(input, -999999, 999999);

	lv_group_add_obj(group, input);
	return root;
}

static void feedbox_edit_cb(lv_event_t *event)
{
	lv_obj_t *spinbox = lv_event_get_target(event);
	g_system_menu_jog_feed = (float)lv_spinbox_get_value(spinbox) / 1000;
}

static lv_obj_t *make_feed_box(lv_obj_t *parent)
{
	lv_obj_t *root = lv_obj_create(parent);
	lv_obj_set_size(root, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
	lv_obj_set_style_bg_opa(root, LV_OPA_TRANSP, LV_PART_MAIN);
	lv_obj_set_style_text_font(root, &font_pixel_mono_14pt, LV_PART_MAIN);

	lv_obj_set_style_pad_all(root, 2, LV_PART_MAIN);

	lv_obj_t *labelob = lv_label_create(root);
	lv_label_set_text(labelob, STR_FEED);
	lv_obj_set_pos(labelob, 0, 2 + 4);

	lv_obj_t *input = win9x_number_input(root);
	lv_obj_set_pos(input, 60, 0);
	lv_obj_set_width(input, 120);

	lv_spinbox_set_digit_format(input, 7, 4);
	lv_spinbox_set_range(input, 0, 9999999);

	lv_spinbox_set_value(input, (int32_t)(g_system_menu_jog_feed * 1000));

	lv_obj_add_event_cb(input, feedbox_edit_cb, LV_EVENT_VALUE_CHANGED, NULL);

	lv_group_add_obj(group, input);

	return root;
}

static void reset_form()
{
	for(int i = 0; i < 3; ++i)
	{
		lv_obj_remove_state(switches[i], LV_STATE_CHECKED);
		lv_spinbox_set_value(spinboxes[i], 0);
	}
}

static void move_to_coord(lv_event_t *event)
{
	if(lvgl_serial_write_available() < 64)
		return;

	lvgl_serial_putc('$');
	lvgl_serial_putc('J');
	lvgl_serial_putc('=');
	lvgl_serial_putc('G');

	switch(coordinate_type)
	{
		case 0:
			lvgl_serial_putc('9');
			lvgl_serial_putc('0');
			break;
		case 1:
			lvgl_serial_putc('9');
			lvgl_serial_putc('1');
			break;
		case 2:
			lvgl_serial_putc('5');
			lvgl_serial_putc('3');
			break;
	}

	// Append axis which were modified
	for(uint8_t i = 0; i < 3; ++i)
	{
		if(!lv_obj_has_state(switches[i], LV_STATE_CHECKED))
			continue;
		lvgl_serial_putc(axis[i]);
		int32_t value = lv_spinbox_get_value(spinboxes[i]);
		print_flt(lvgl_serial_putc, (float)value / 1000);
	}

	// Append feed rate and finish the command
	lvgl_serial_putc('F');
	print_flt(lvgl_serial_putc, g_system_menu_jog_feed);
	lvgl_serial_putc('\r');

	system_menu_goto(SYSTEM_MENU_ID_IDLE);
}

static void movement_render(uint8_t flags);

void style_create_movement_screen()
{
	screen = win9x_screen();

	group = lv_group_create();

	{
		lv_obj_t *topbar = lv_obj_create(screen);
		lv_obj_set_style_bg_color(topbar, bg_title_bar, LV_PART_MAIN);
		lv_obj_set_style_text_color(topbar, col_white, LV_PART_MAIN);
		lv_obj_set_size(topbar, 480, 22);

		lv_obj_t *header = lv_label_create(topbar);

		char str[32];
		rom_strcpy(str, __rom__(STR_MOVEMENT));
		lv_label_set_text(header, str);
		lv_obj_set_align(header, LV_ALIGN_CENTER);
	}

	lv_obj_t *column = lv_obj_create(screen);
	lv_obj_set_layout(column, LV_LAYOUT_FLEX);
	lv_obj_set_flex_flow(column, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_style_text_font(column, &font_pixel_7pt, LV_PART_MAIN);
	lv_obj_set_style_bg_opa(column, LV_OPA_TRANSP, LV_PART_MAIN);
	lv_obj_set_flex_align(column, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);

	lv_obj_set_align(column, LV_ALIGN_TOP_MID);
	lv_obj_set_pos(column, 0, 32);
	lv_obj_set_size(column, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

	lv_obj_set_style_pad_row(column, 10, LV_PART_MAIN);

	{
		lv_obj_t *row = lv_obj_create(column);
		lv_obj_set_size(row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
		lv_obj_set_layout(row, LV_LAYOUT_FLEX);
		lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
		lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, LV_PART_MAIN);
		lv_obj_set_style_pad_column(row, 10, LV_PART_MAIN);

		lv_group_set_default(group);
		const char* opts_coord[] = { STR_COORD_WORKSPACE, STR_COORD_RELATIVE, STR_COORD_MACHINE };
		win9x_radio_list(row, STR_COORDINATE_TYPE, opts_coord, 3, &coordinate_type);
		lv_group_set_default(NULL);

		make_feed_box(row);
	}

	{
		lv_obj_t *header = lv_label_create(column);
		lv_label_set_text(header, STR_COORDINATES);
		lv_obj_set_style_text_font(header, &font_pixel_14pt, LV_PART_MAIN);

		lv_obj_t *row = lv_obj_create(column);
		lv_obj_set_size(row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
		lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, LV_PART_MAIN);

		static int32_t rows[] = { 30, 30, 30, LV_GRID_TEMPLATE_LAST };
		static int32_t cols[] = { LV_SIZE_CONTENT, LV_SIZE_CONTENT, LV_GRID_TEMPLATE_LAST };
		lv_obj_set_grid_dsc_array(row, cols, rows);
		lv_obj_set_style_pad_row(row, 5, LV_PART_MAIN);
		lv_obj_set_style_pad_column(row, 5, LV_PART_MAIN);

		lv_obj_t *x = make_coordinate_box(row, "X");
		lv_obj_set_grid_cell(x, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 0, 1);
		switches[0] = lv_obj_get_child(x, 0);
		spinboxes[0] = lv_obj_get_child(x, 2);
		
		lv_obj_t *y = make_coordinate_box(row, "Y");
		lv_obj_set_grid_cell(y, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 1, 1);
		switches[1] = lv_obj_get_child(y, 0);
		spinboxes[1] = lv_obj_get_child(y, 2);

		lv_obj_t *z = make_coordinate_box(row, "Z");
		lv_obj_set_grid_cell(z, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 2, 1);
		switches[2] = lv_obj_get_child(z, 0);
		spinboxes[2] = lv_obj_get_child(z, 2);
	}

	{
		lv_obj_t *row = lv_obj_create(column);
		lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, LV_PART_MAIN);
		lv_obj_set_style_pad_all(row, 2, LV_PART_MAIN);

		lv_obj_set_layout(row, LV_LAYOUT_FLEX);
		lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
		lv_obj_set_style_pad_column(row, 5, LV_PART_MAIN);
		lv_obj_set_flex_align(row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

		lv_obj_t *back = win9x_button(row, STR_BACK);
		lv_group_add_obj(group, back);
		lv_obj_add_event_cb(back, lvgl_callback_back, LV_EVENT_CLICKED, NULL);

		lv_obj_t *move = win9x_button(row, STR_MOVE);
		lv_group_add_obj(group, move);
		lv_obj_add_event_cb(move, move_to_coord, LV_EVENT_CLICKED, NULL);
	}

	// Declare system menu screen
	DECL_DYNAMIC_MENU(10, 1, movement_render, NULL);
}

static void enter()
{
	reset_form();
}

static STYLE_LOAD_SCREEN_WITH_GROUP_ENTER(movement);

static void movement_render(uint8_t flags)
{
	style_movement();
}

#endif

