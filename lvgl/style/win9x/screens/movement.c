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

#include "../../config.h"

#ifdef TFT_STYLE_WIN9X

#include "lvgl.h"
#include "../colors.h"
#include "../styles.h"
#include "../fonts/pixel.h"
#include "../fonts/pixel_mono.h"
#include "../bitmaps/checkbox.h"
#include "../../../../../cnc.h"
#include "../elements.h"
#include "math.h"

#define COORDINATE_DECIMAL_DIGITS 3
#define COORDINATE_FRACTIONAL_DIGITS 3

#define COORD_BOX_SIZE ((COORDINATE_DECIMAL_DIGITS + COORDINATE_FRACTIONAL_DIGITS + 2) * 15)

static lv_obj_t *screen;
static lv_group_t *group;

static uint8_t movement_type = 0;
static uint8_t coordinate_type = 0;

typedef struct _cb_state
{
	uint32_t sign:1;
	uint32_t decimal:10;
	uint32_t fractional:10;
	uint32_t edited:1;
	uint32_t reserved:10;

	int multiplier;
} coord_box_state_t;

static coord_box_state_t coordinates[3];

static float coord_box_value(const coord_box_state_t *state)
{
	return ((float)state->decimal + (float)state->fractional * 0.001f) * (state->sign ? -1 : 1);
}

static void coordbox_table_cb(lv_event_t *event)
{
	lv_draw_task_t *draw_task = lv_event_get_draw_task(event);
	lv_draw_dsc_base_t *base_dsc = (lv_draw_dsc_base_t*)draw_task->draw_dsc;

	coord_box_state_t *state = (coord_box_state_t*)lv_event_get_user_data(event);

	if(base_dsc->part == LV_PART_ITEMS)
	{
    uint32_t col = base_dsc->id2;

		lv_draw_border_dsc_t *border_draw_dsc = lv_draw_task_get_border_dsc(draw_task);
		if(border_draw_dsc)
		{
			if(col == 0)
			{
				border_draw_dsc->side = LV_BORDER_SIDE_NONE;
			}
		}

		lv_draw_fill_dsc_t *fill_draw_dsc = lv_draw_task_get_fill_dsc(draw_task);
		if(fill_draw_dsc)
		{
			lv_obj_t *table = lv_event_get_target(event);
			lv_obj_t *root = lv_obj_get_parent(table);
			bool focused = lv_obj_get_state(root) & LV_STATE_FOCUSED;

			int digit = (COORDINATE_DECIMAL_DIGITS + 1) - col;
			int cursor = state->multiplier;
			if(cursor >= 0)
				cursor += 1;

			fill_draw_dsc->opa = digit == cursor && focused ? LV_OPA_COVER : LV_OPA_TRANSP;
		}
	}
}

static void set_coordinate_value(lv_obj_t *coordbox, float value)
{
	lv_obj_t *table = lv_obj_get_child(coordbox, 1);
	
	char str[10];
	sprintf(str, "%4d.%03d", (int)value, ABS((int)(value * 1000) % 1000));

	for(int i = 0; i < 8; ++i)
	{
		char c[2];
		c[0] = str[i];
		c[1] = 0;
		lv_table_set_cell_value(table, 0, i, c);
	}
}

static void clear_coordinate(lv_obj_t *coordbox)
{
	lv_obj_t *table = lv_obj_get_child(coordbox, 1);

	for(int i = 0; i < 8; ++i)
	{
		lv_table_set_cell_value(table, 0, i, "");
	}
}

static void coord_input_cb(lv_event_t *event)
{
	coord_box_state_t *state = (coord_box_state_t*)lv_event_get_user_data(event);
	lv_obj_t *root = lv_event_get_current_target(event);

	char c = lv_event_get_key(event);
	if(c >= '0' && c <= '9')
	{
		if(state->multiplier >= 0)
		{
			state->decimal = (state->decimal * 10 + (c - '0')) % 1000;
		}
		else
		{
			if(state->multiplier == -1)
				state->fractional = 0;
			state->fractional += (c - '0') * powf(10, 3 + state->multiplier);
			if(--state->multiplier < -3)
				state->multiplier = 0;
		}
		state->edited = 1;
	}
	else if(c == '-')
	{
		state->sign = !state->sign;
	}
	else if(c == '\b')
	{
		if(state->decimal == 0 && state->multiplier >= 0)
		{
			state->fractional = 0;
			state->edited = 0;
			state->multiplier = 0;
			clear_coordinate(root);
			return;
		}
		else if(state->multiplier >= 0)
		{
			state->decimal *= 0.1;
			state->multiplier -= 1;
			if(state->multiplier < 0)
				state->multiplier = 0;
		}
	}
	else if(c == '.')
	{
		state->multiplier = -1;
	}

	set_coordinate_value(root, coord_box_value(state));
}

static lv_obj_t *make_coordinate_box(lv_obj_t *parent, const char *label, coord_box_state_t *state)
{
	lv_obj_t *root = lv_obj_create(parent);
	lv_obj_set_size(root, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
	lv_obj_set_style_bg_opa(root, LV_OPA_TRANSP, LV_PART_MAIN);
	lv_obj_set_style_text_font(root, &font_pixel_mono_14pt, LV_PART_MAIN);

	lv_obj_set_style_bg_opa(root, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_FOCUSED);
	lv_obj_set_style_bg_color(root, select_highlight, LV_PART_MAIN | LV_STATE_FOCUSED);
	lv_obj_set_style_pad_all(root, 2, LV_PART_MAIN);

	lv_obj_t *labelob = lv_label_create(root);
	lv_label_set_text(labelob, label);
	lv_obj_set_pos(labelob, 0, 2 + 4);

	lv_obj_t *table = lv_table_create(root);
	lv_obj_set_pos(table, 40, 0);
	lv_obj_add_style(table, &g_styles.container, LV_PART_MAIN);
	WIN9X_BORDER_PART_TWO(table, shadow_light);

	for(int i = 0; i < COORDINATE_DECIMAL_DIGITS + COORDINATE_FRACTIONAL_DIGITS + 2; ++i)
		lv_table_set_column_width(table, i, 18);

	lv_obj_set_style_pad_left(table, 2, LV_PART_ITEMS);
	lv_obj_set_style_pad_top(table, 4, LV_PART_ITEMS);
	lv_obj_set_style_border_side(table, LV_BORDER_SIDE_LEFT, LV_PART_ITEMS);
	lv_obj_set_style_border_width(table, 1, LV_PART_ITEMS);
	lv_obj_set_style_border_color(table, separator, LV_PART_ITEMS);
	lv_obj_set_style_text_align(table, LV_TEXT_ALIGN_CENTER, LV_PART_ITEMS);
	lv_obj_set_style_bg_opa(table, LV_OPA_COVER, LV_PART_ITEMS);

	state->decimal = 0;
	state->fractional = 0;
	state->multiplier = 0;
	state->sign = 0;
	state->edited = 0;

	lv_obj_add_event_cb(table, coordbox_table_cb, LV_EVENT_DRAW_TASK_ADDED, state);
	lv_obj_add_flag(table, LV_OBJ_FLAG_SEND_DRAW_TASK_EVENTS | LV_OBJ_FLAG_EVENT_BUBBLE);
	lv_obj_remove_flag(table, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_CLICK_FOCUSABLE);

	lv_obj_add_event_cb(root, coord_input_cb, LV_EVENT_KEY, state);
	lv_obj_add_flag(root, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_CLICK_FOCUSABLE);

	lv_group_add_obj(group, root);

	return root;
}

void style_create_movement_screen()
{
	screen = lv_obj_create(NULL);
	lv_obj_set_style_bg_color(screen, bg_base, LV_PART_MAIN);
	lv_obj_set_style_text_font(screen, &font_pixel_bold_11pt, LV_PART_MAIN);

	group = lv_group_create();
	lv_group_set_default(group);

	// TODO: Grab the strings from language file
	{
		lv_obj_t *topbar = lv_obj_create(screen);
		lv_obj_set_style_bg_color(topbar, bg_title_bar, LV_PART_MAIN);
		lv_obj_set_style_text_color(topbar, col_white, LV_PART_MAIN);
		lv_obj_set_size(topbar, 480, 22);

		lv_obj_t *header = lv_label_create(topbar);
		lv_label_set_text(header, "Movement");
		lv_obj_set_align(header, LV_ALIGN_CENTER);
	}

	lv_obj_t *column = lv_obj_create(screen);
	lv_obj_set_layout(column, LV_LAYOUT_FLEX);
	lv_obj_set_flex_flow(column, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_style_text_font(column, &font_pixel_7pt, LV_PART_MAIN);
	lv_obj_set_style_text_color(column, col_black, LV_PART_MAIN);
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

	 	const char* opts_move[] = { "Rapid", "Linear" };
		win9x_radio_list(row, "Movement type", opts_move, 2, &movement_type);

		const char* opts_coord[] = { "Workspace", "Relative", "Machine" };
		win9x_radio_list(row, "Coordinate type", opts_coord, 3, &coordinate_type);
	}

	{
		lv_obj_t *header = lv_label_create(column);
		lv_label_set_text(header, "Coordinates");
		lv_obj_set_style_text_font(header, &font_pixel_14pt, LV_PART_MAIN);

		lv_obj_t *row = lv_obj_create(column);
		lv_obj_set_size(row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
		lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, LV_PART_MAIN);

		static int32_t rows[] = { 30, 30, 30, LV_GRID_TEMPLATE_LAST };
		static int32_t cols[] = { LV_SIZE_CONTENT, LV_SIZE_CONTENT, LV_GRID_TEMPLATE_LAST };
		lv_obj_set_grid_dsc_array(row, cols, rows);
		lv_obj_set_style_pad_row(row, 5, LV_PART_MAIN);
		lv_obj_set_style_pad_column(row, 5, LV_PART_MAIN);

		lv_obj_t *x = make_coordinate_box(row, "X", &coordinates[0]);
		lv_obj_set_grid_cell(x, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 0, 1);
		lv_obj_t *y = make_coordinate_box(row, "Y", &coordinates[1]);
		lv_obj_set_grid_cell(y, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 1, 1);
		lv_obj_t *z = make_coordinate_box(row, "Z", &coordinates[2]);
		lv_obj_set_grid_cell(z, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 2, 1);
	}

	{
		lv_obj_t *row = lv_obj_create(column);
		lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, LV_PART_MAIN);
		lv_obj_set_style_pad_all(row, 2, LV_PART_MAIN);

		lv_obj_set_layout(row, LV_LAYOUT_FLEX);
		lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
		lv_obj_set_style_pad_column(row, 5, LV_PART_MAIN);
		lv_obj_set_flex_align(row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

		lv_obj_t *back = win9x_button(row, "Back");
		lv_obj_t *next = win9x_button(row, "Move");
	}

	lv_group_set_default(NULL);
}

#endif

