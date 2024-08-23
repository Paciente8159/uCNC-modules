/*
	Name: coordinate_table.c
	Description: Creates the coordinate table widget.

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
#include "../fonts/pixel_mono.h"

#include "src/cnc.h"
#include "src/modules/system_languages.h"

static const char* axis_labels[] = {
	"X", "Y", "Z",
#if (AXIS_COUNT > 3)
	"A",
#endif
#if (AXIS_COUNT > 4)
	"B",
#endif
#if (AXIS_COUNT > 5)
	"C",
#endif
};

static void coord_box_event_cb(lv_event_t *event)
{
	lv_draw_task_t *draw_task = lv_event_get_draw_task(event);
	lv_draw_dsc_base_t *base_dsc = (lv_draw_dsc_base_t*)draw_task->draw_dsc;

	if(base_dsc->part == LV_PART_ITEMS)
	{
    uint32_t row = base_dsc->id1;
    uint32_t col = base_dsc->id2;

		lv_draw_label_dsc_t *label_draw_dsc = lv_draw_task_get_label_dsc(draw_task);
		if(label_draw_dsc)
		{
			if(col == 0 || row == 0)
			{
				label_draw_dsc->align = LV_TEXT_ALIGN_LEFT;
			}
			else
			{
				label_draw_dsc->align = LV_TEXT_ALIGN_RIGHT;
			}
		}

		lv_draw_border_dsc_t *border_draw_dsc = lv_draw_task_get_border_dsc(draw_task);
		if(border_draw_dsc)
		{
			if(col == 0)
			{
				border_draw_dsc->side &= ~LV_BORDER_SIDE_LEFT;
			}
			if(row == 0)
			{
				border_draw_dsc->side &= ~LV_BORDER_SIDE_TOP;
			}
		}
  }
}

lv_obj_t *win9x_coordinate_table(lv_obj_t *parent)
{
	lv_obj_t *container = lv_obj_create(parent);
	lv_obj_set_pos(container, 70, 30);
	lv_obj_set_size(container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

	lv_obj_set_style_text_font(container, &font_pixel_mono_14pt, LV_PART_MAIN);

	lv_obj_add_style(container, &g_styles.container, LV_PART_MAIN);
	WIN9X_BORDER_PART_TWO(container, shadow_light);

	lv_obj_t *table = lv_table_create(container);

	lv_obj_add_style(table, &g_styles.table_item, LV_PART_ITEMS);
	lv_obj_set_style_align(table, LV_ALIGN_CENTER, LV_PART_ITEMS);

	lv_table_set_column_width(table, 0, 40);
	lv_table_set_column_width(table, 1, 108);
	lv_table_set_column_width(table, 2, 108);
	
	lv_table_set_cell_value(table, 0, 1, STR_COORD_WORKSPACE_SHORT);
	lv_table_set_cell_value(table, 0, 2, STR_COORD_MACHINE);

	for(int i = 0; i < MAX(AXIS_COUNT, 3); ++i)
	{
		lv_table_set_cell_value(table, i + 1, 0, axis_labels[i]);
	}

	lv_obj_add_event_cb(table, coord_box_event_cb, LV_EVENT_DRAW_TASK_ADDED, NULL);
	lv_obj_add_flag(table, LV_OBJ_FLAG_SEND_DRAW_TASK_EVENTS);

	return container;
}

void win9x_update_coordinate_table(lv_obj_t *coordinate_table)
{
	lv_obj_t *table = lv_obj_get_child(coordinate_table, 0);

	int32_t steppos[STEPPER_COUNT];
	itp_get_rt_position(steppos);

	float mpos[MAX(AXIS_COUNT, 3)];
	kinematics_apply_forward(steppos, mpos);
	kinematics_apply_reverse_transform(mpos);

	float wpos[MAX(AXIS_COUNT, 3)];
	for(uint8_t i = 0; i < MAX(AXIS_COUNT, 3); ++i)
		wpos[i] = mpos[i];
	parser_machine_to_work(wpos);

	char str[32];
	for(int i = 0; i < MAX(AXIS_COUNT, 3); ++i)
	{
	 	sprintf(str, "%4d.%03d", (int)wpos[i], ABS((int)(wpos[i] * 1000) % 1000));
		lv_table_set_cell_value(table, i + 1, 1, str);
		sprintf(str, "%4d.%03d", (int)mpos[i], ABS((int)(mpos[i] * 1000) % 1000));
		lv_table_set_cell_value(table, i + 1, 2, str);
	}
}

#endif

