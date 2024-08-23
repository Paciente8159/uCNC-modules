/*
	Name: jog.c
	Description: Custom jogging screen

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

#include "../config.h"

#if defined(GUI_STYLE_WIN9X) && CUSTOM_JOG_MENU

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

static lv_obj_t *coordinate_table;
static lv_obj_t *axis_select;

static void jog_update()
{
	win9x_update_coordinate_table(coordinate_table);
}

STYLE_LOAD_SCREEN_UPDATE_WITH_GROUP(jog);
static void jog_render(uint8_t flags)
{
	style_jog();
}

static void edit_float_cb(lv_event_t *event)
{
	float *value = (float*)lv_event_get_user_data(event);
	lv_obj_t *spinbox = lv_event_get_target_obj(event);

	*value = (float)lv_spinbox_get_value(spinbox) / 1000;
}

static lv_obj_t *editable_value(lv_obj_t *parent, const char *nameStr, float *valuePtr)
{
	lv_obj_t *container = lv_obj_create(parent);
	lv_obj_set_size(container, 120, LV_SIZE_CONTENT);
	lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, LV_PART_MAIN);

	lv_obj_t *name = lv_label_create(container);
	lv_label_set_text(name, nameStr);

	lv_obj_t *value = win9x_number_input(container);
	lv_spinbox_set_digit_step_direction(value, LV_DIR_LEFT);
	lv_spinbox_set_digit_format(value, 7, 4);
	lv_spinbox_set_range(value, 0, 9999999);
	lv_spinbox_set_value(value, *valuePtr * 1000);
	lv_obj_add_event_cb(value, edit_float_cb, LV_EVENT_VALUE_CHANGED, valuePtr);

	lv_group_add_obj(group, value);
	return container;
}

static char *jog_axis_opts = "X\nY\nZ"
#if (AXIS_COUNT > 3)
	"\nA"
#endif
#if (AXIS_COUNT > 4)
	"\nB"
#endif
#if (AXIS_COUNT > 5)
	"\nC"
#endif
;

static void jog_pos_cb()
{
	if(cnc_get_exec_state(EXEC_JOG | EXEC_RUN) || cnc_has_alarm())
		return;

	char buffer[48];

	char axis;
	lv_dropdown_get_selected_str(axis_select, &axis, 1);

	int distI = (int)g_system_menu_jog_distance;
	int feedI = (int)g_system_menu_jog_feed;
	sprintf(buffer, "$J=G91%c%d.%03dF%d.%03d\r",
		axis,
		distI, (int)((g_system_menu_jog_distance - distI) * 1000),
		feedI, (int)((g_system_menu_jog_feed - feedI) * 1000));

	system_menu_send_cmd(buffer);
}

static void jog_neg_cb()
{
	if(cnc_get_exec_state(EXEC_JOG | EXEC_RUN) || cnc_has_alarm())
		return;

	char buffer[48];

	char axis;
	lv_dropdown_get_selected_str(axis_select, &axis, 1);

	int distI = (int)g_system_menu_jog_distance;
	int feedI = (int)g_system_menu_jog_feed;
	sprintf(buffer, "$J=G91%c-%d.%03dF%d.%03d\r",
		axis,
		distI, (int)((g_system_menu_jog_distance - distI) * 1000),
		feedI, (int)((g_system_menu_jog_feed - feedI) * 1000));

	system_menu_send_cmd(buffer);
}

void style_create_jog_screen()
{
	screen = win9x_screen();
	lv_obj_set_layout(screen, LV_LAYOUT_GRID);
	lv_obj_set_style_pad_column(screen, 10, LV_PART_MAIN);
	lv_obj_set_style_pad_row(screen, 10, LV_PART_MAIN);

	static int32_t rows[] = { 22, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_FR(100), LV_GRID_TEMPLATE_LAST };
	static int32_t columns[] = { 0, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST };
	lv_obj_set_style_grid_row_dsc_array(screen, rows, LV_PART_MAIN);
	lv_obj_set_style_grid_column_dsc_array(screen, columns, LV_PART_MAIN);

	group = lv_group_create();

	{
		lv_obj_t *topbar = lv_obj_create(screen);
		lv_obj_set_style_bg_color(topbar, bg_title_bar, LV_PART_MAIN);
		lv_obj_set_style_text_color(topbar, col_white, LV_PART_MAIN);
		lv_obj_set_size(topbar, LV_PCT(100), 22);

		lv_obj_set_grid_cell(topbar, LV_GRID_ALIGN_START, 0, 4, LV_GRID_ALIGN_START, 0, 1);

		lv_obj_t *header = lv_label_create(topbar);

		lv_label_set_text(header, STR_JOG);
		lv_obj_set_align(header, LV_ALIGN_CENTER);

		lv_obj_t *back = win9x_close_button(topbar);
		lv_obj_add_event_cb(back, lvgl_callback_back, LV_EVENT_CLICKED, NULL);
		lv_group_add_obj(group, back);
	}

	{
		lv_obj_t *container = win9x_coordinate_table(screen);
		lv_obj_set_grid_cell(container, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 1, 2);
		coordinate_table = container;
	}

	{
		lv_obj_t *container = editable_value(screen, STR_JOG_FEED, &g_system_menu_jog_feed);
		lv_obj_set_grid_cell(container, LV_GRID_ALIGN_START, 2, 1, LV_GRID_ALIGN_START, 1, 1);
	}

	{
		lv_obj_t *container = editable_value(screen, STR_JOG_DIST, &g_system_menu_jog_distance);
		lv_obj_set_grid_cell(container, LV_GRID_ALIGN_START, 2, 1, LV_GRID_ALIGN_START, 2, 1);
	}

	{
		lv_obj_t *container = lv_obj_create(screen);
		lv_obj_set_grid_cell(container, LV_GRID_ALIGN_START, 3, 1, LV_GRID_ALIGN_START, 1, 2);
		lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, LV_PART_MAIN);
		lv_obj_set_size(container, LV_SIZE_CONTENT + 1, LV_SIZE_CONTENT + 1);
		lv_obj_set_layout(container, LV_LAYOUT_FLEX);
		lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
		lv_obj_set_flex_align(container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
		lv_obj_set_style_pad_all(container, 1, LV_PART_MAIN);
		lv_obj_set_style_pad_row(container, 5, LV_PART_MAIN);

		lv_obj_set_style_pad_left(container, 10, LV_PART_MAIN);
		lv_obj_set_style_pad_top(container, 20, LV_PART_MAIN);

		lv_obj_t *jogPos = win9x_button(container, "+");
		lv_group_add_obj(group, jogPos);
		lv_obj_add_event_cb(jogPos, jog_pos_cb, LV_EVENT_CLICKED, NULL);

		lv_obj_t *axis = win9x_dropdown(container);
		lv_dropdown_set_options_static(axis, jog_axis_opts);
		lv_obj_set_style_text_font(axis, &font_pixel_14pt, LV_PART_MAIN);
		lv_obj_set_width(axis, 40);
		lv_group_add_obj(group, axis);
		axis_select = axis;

		lv_obj_t *jogNeg = win9x_button(container, "-");
		lv_group_add_obj(group, jogNeg);
		lv_obj_add_event_cb(jogPos, jog_neg_cb, LV_EVENT_CLICKED, NULL);
	}

	system_menu_set_render_callback(SYSTEM_MENU_ID_JOG, jog_render);
}

#endif

