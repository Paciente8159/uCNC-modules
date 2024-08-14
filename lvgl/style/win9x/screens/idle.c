/*
	Name: idle.c
	Description: Idle screen in style of Win9x

	Copyright: Copyright (c) Patryk Mierzyński
	Author: Patryk Mierzyński
	Date: 27/07/2024

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
#include "../bitmaps/lock.h"
#include "../bitmaps/warning.h"
#include "../bitmaps/move.h"
#include "../bitmaps/zero.h"
#include "../bitmaps/menu.h"
#include "../fonts/pixel.h"
#include "../fonts/pixel_mono.h"
#include "../../../lvgl_support.h"

#ifndef MAX_MODAL_GROUPS
#define MAX_MODAL_GROUPS 14
#endif

static void get_alarm_string(char* dest);

static lv_obj_t *screen;
static lv_group_t *group;
static lv_group_t *zero_btn_group;

static lv_obj_t *side_buttons_blocks[2];

static struct _topbar {
	lv_obj_t *state_text;
	lv_obj_t *lock_icon;

	lv_obj_t *alarm_box;
	lv_obj_t *alarm_text;
} g_topbar;

static struct _posbox {
	lv_obj_t *coordinate_table;
	lv_obj_t *feed_spindle_table;
	lv_obj_t *state_box;
	lv_obj_t *state_label;
} g_state;

static const char* axis_labels[] = {
	"X", "Y", "Z"
};

static void update_topbar()
{
	static uint8_t old_state = -1;
	uint8_t state = cnc_get_exec_state(EXEC_RESET_LOCKED);
	if(state != old_state)
	{
		lv_image_set_src(g_topbar.lock_icon, state ? &Img_Locked : &Img_Unlocked);
		old_state = state;
	}

	uint8_t modes[MAX_MODAL_GROUPS];
	uint16_t feed, spindle;
	parser_get_modes(modes, &feed, &spindle);

	static uint32_t old_modes = -1;
	uint32_t new_modes = (modes[6]) | (modes[2] << 8) | (modes[4] << 16) | (modes[11] << 24);
	if(old_modes != new_modes)
	{
		char str[32];
		sprintf(str, "G%d %s [%s] T%d",
						modes[6],
						modes[2] == 90 ? "Abs" : "Inc",
						modes[4] == 21 ? "mm" : "inch",
						modes[11]);

		lv_label_set_text(g_topbar.state_text, str);
		old_modes = new_modes;
	}

	static uint8_t old_alarm = -1;
	uint8_t new_alarm = cnc_get_alarm();
	if(old_alarm != new_alarm)
	{
		char str[64];
		if(cnc_has_alarm())
		{
			get_alarm_string(str);
			lv_label_set_text(g_topbar.alarm_text, str);

			lv_obj_remove_flag(g_topbar.alarm_box, LV_OBJ_FLAG_HIDDEN);
		}
		else
		{
			lv_obj_add_flag(g_topbar.alarm_box, LV_OBJ_FLAG_HIDDEN);
		}
		old_alarm = new_alarm;
	}
}

static void update_coordinates()
{
	if(!cnc_get_exec_state(EXEC_RUN))
		return;

	float mpos[MAX(AXIS_COUNT, 3)];
	int32_t steppos[STEPPER_COUNT];
	itp_get_rt_position(steppos);
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
		lv_table_set_cell_value(g_state.coordinate_table, i + 1, 1, str);
		sprintf(str, "%4d.%03d", (int)mpos[i], ABS((int)(mpos[i] * 1000) % 1000));
		lv_table_set_cell_value(g_state.coordinate_table, i + 1, 2, str);
	}
}

static void update_feed_spindle()
{
	uint8_t modes[MAX_MODAL_GROUPS];
	uint16_t feed, spindle;
	parser_get_modes(modes, &feed, &spindle);

	char str[16];

	static uint16_t old_feed = -1, old_spindle = -1;
	if(old_feed != feed)
	{
		sprintf(str, "%4d", feed);
		lv_table_set_cell_value(g_state.feed_spindle_table, 0, 1, str);
		old_feed = feed;
	}

	if(old_spindle != spindle)
	{
		sprintf(str, "%5d", spindle);
		lv_table_set_cell_value(g_state.feed_spindle_table, 1, 1, str);
		old_spindle = spindle;
	}
}

static void update_state()
{
	static uint8_t old_state_2 = 0;
	uint8_t state = cnc_get_exec_state(EXEC_ALLACTIVE);
	if(state == old_state_2)
		return;
	old_state_2 = state;

	uint8_t filter = 0x80;
	while(!(state & filter) && filter)
		filter >>= 1;
	state &= filter;

	char str[64];

	lv_color_t bg_color = bg_box;
	switch(state)
	{
		case EXEC_DOOR:
			rom_strcpy(str, MSG_STATUS_DOOR);
			bg_color = status_red;
			break;
		case EXEC_KILL:
		case EXEC_UNHOMED:
		case EXEC_LIMITS:
			rom_strcpy(str, MSG_STATUS_ALARM);
			bg_color = status_red;
			break;
		case EXEC_HOLD:
			rom_strcpy(str, MSG_STATUS_HOLD);
			bg_color = status_yellow;
			break;
		case EXEC_HOMING:
			rom_strcpy(str, MSG_STATUS_HOME);
			bg_color = status_green;
			break;
		case EXEC_JOG:
			rom_strcpy(str, MSG_STATUS_JOG);
			bg_color = status_blue;
			break;
		case EXEC_RUN:
			rom_strcpy(str, MSG_STATUS_RUN);
			bg_color = status_green;
			break;
		default:
			rom_strcpy(str, MSG_STATUS_IDLE);
			break;
	}

	lv_obj_set_style_bg_color(g_state.state_box, bg_color, LV_PART_MAIN);
	lv_label_set_text(g_state.state_label, str);
}

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

static void feed_box_event_cb(lv_event_t *event)
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
			if(col == 0)
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
			border_draw_dsc->side = LV_BORDER_SIDE_TOP;
			if(row == 0)
			{
				border_draw_dsc->side = LV_BORDER_SIDE_NONE;
			}
		}
	}
}

static lv_obj_t *side_button(lv_obj_t *parent, int32_t y, const void *img, lv_group_t *group)
{
	lv_obj_t *btn = lv_button_create(parent);
	lv_obj_set_pos(btn, 0, y);
	lv_obj_set_size(btn, 50, 50);
	lv_obj_add_style(btn, &g_styles.button, 0);
	WIN9X_BORDER_PART_TWO(btn, shadow_dark);

	lv_obj_t *btn_img = lv_image_create(btn);
	lv_obj_center(btn_img);
	lv_image_set_src(btn_img, img);
	lv_obj_set_style_image_recolor(btn_img, charcoal, LV_PART_MAIN);

	lv_obj_set_style_outline_opa(btn, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_FOCUSED);
	lv_obj_set_style_outline_color(btn, col_black, LV_PART_MAIN | LV_STATE_FOCUSED);
	lv_obj_set_style_outline_width(btn, 1, LV_PART_MAIN | LV_STATE_FOCUSED);

	lv_group_add_obj(group, btn);

	return btn;
}

static void side_buttons_zero(lv_event_t *event)
{
	lv_obj_remove_flag(side_buttons_blocks[1], LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag(side_buttons_blocks[0], LV_OBJ_FLAG_HIDDEN);
	lvgl_set_indev_group(zero_btn_group);
}

static void side_buttons_back(lv_event_t *event)
{
	lv_obj_remove_flag(side_buttons_blocks[0], LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag(side_buttons_blocks[1], LV_OBJ_FLAG_HIDDEN);
	lvgl_set_indev_group(group);
}

static void zero_axis(lv_event_t *event)
{
	uint8_t axis = *(uint8_t*)lv_event_get_user_data(event);

	char cmd[] = "G10 L20 P0 ?0\r";
	switch(axis)
	{
		case 0:
			cmd[11] = 'X';
			break;
		case 1:
			cmd[11] = 'Y';
			break;
		case 2:
			cmd[11] = 'Z';
			break;
		default:
			break;
	}

	if(cmd[11] != '?')
	{
		// Valid axis selected
		if(system_menu_send_cmd(cmd) != STATUS_OK)
		{
			char buffer[32];
			rom_strcpy((char *)buffer, __romstr__(STR_CMD_NOTSENT));
			system_menu_show_modal_popup(SYSTEM_MENU_MODAL_POPUP_MS, buffer);
		}
	}

	side_buttons_back(event);
}

void style_create_idle_screen()
{
	screen = lv_obj_create(NULL);
	lv_obj_set_style_bg_color(screen, bg_base, LV_PART_MAIN);
	lv_obj_set_style_text_font(screen, &font_pixel_bold_11pt, LV_PART_MAIN);

	group = lv_group_create();
	zero_btn_group = lv_group_create();

	{
		lv_obj_t *topbar = lv_obj_create(screen);
		lv_obj_set_style_bg_color(topbar, bg_title_bar, LV_PART_MAIN);
		lv_obj_set_style_text_color(topbar, col_white, LV_PART_MAIN);
		lv_obj_set_size(topbar, 480, 22);

		g_topbar.state_text = lv_label_create(topbar);
		lv_obj_set_align(g_topbar.state_text, LV_ALIGN_LEFT_MID);
		lv_obj_set_pos(g_topbar.state_text, 2, 0);

		g_topbar.lock_icon = lv_image_create(topbar);
		lv_obj_set_align(g_topbar.lock_icon, LV_ALIGN_RIGHT_MID);
		lv_obj_set_pos(g_topbar.lock_icon, -3, 0);
		lv_obj_set_style_image_recolor(g_topbar.lock_icon, col_white, LV_PART_MAIN);

		g_topbar.alarm_box = lv_obj_create(topbar);
		lv_obj_set_pos(g_topbar.alarm_box, 180, 0);
		lv_obj_set_size(g_topbar.alarm_box, 220, 22);
		lv_obj_set_style_bg_color(g_topbar.alarm_box, col_red, LV_PART_MAIN);

		lv_obj_t *warn_1 = lv_image_create(g_topbar.alarm_box);
		lv_image_set_src(warn_1, &Img_Warning);
		lv_obj_set_style_image_recolor(warn_1, col_white, LV_PART_MAIN);
		lv_obj_set_align(warn_1, LV_ALIGN_LEFT_MID);
		lv_obj_set_pos(warn_1, 3, 0);

		lv_obj_t *warn_2 = lv_image_create(g_topbar.alarm_box);
		lv_image_set_src(warn_2, &Img_Warning);
		lv_obj_set_style_image_recolor(warn_2, col_white, LV_PART_MAIN);
		lv_obj_set_align(warn_2, LV_ALIGN_RIGHT_MID);
		lv_obj_set_pos(warn_2, -3, 0);

		g_topbar.alarm_text = lv_label_create(g_topbar.alarm_box);
		lv_obj_set_align(g_topbar.alarm_text, LV_ALIGN_CENTER);

		update_topbar();
	}

	{
		{
			lv_obj_t *buttons_def = lv_obj_create(screen);
			lv_obj_set_style_bg_opa(buttons_def, LV_OPA_TRANSP, LV_PART_MAIN);
			lv_obj_set_size(buttons_def, LV_SIZE_CONTENT + 1, LV_SIZE_CONTENT + 1);

			lv_obj_t *btn1 = side_button(buttons_def,  30, &Img_ZeroX, group);
			lv_obj_add_event_cb(btn1, side_buttons_zero, LV_EVENT_PRESSED, NULL);

			lv_obj_t *btn2 = side_button(buttons_def, 115, &Img_Move, group);
#if MOVEMENT_MENU
			static cb_goto_arg_t btn2_arg = { 10 };
#else
			static cb_goto_arg_t btn2_arg = { 7 };
#endif
			lv_obj_add_event_cb(btn2, lvgl_callback_goto, LV_EVENT_PRESSED, &btn2_arg);

			lv_obj_t *btn3 = side_button(buttons_def, 200, &Img_Menu, group);
			static cb_goto_arg_t btn3_arg = { 1 };
			lv_obj_add_event_cb(btn3, lvgl_callback_goto, LV_EVENT_PRESSED, &btn3_arg);

			side_buttons_blocks[0] = buttons_def;
		}

		{
			lv_obj_t *buttons_zero = lv_obj_create(screen);
			lv_obj_set_style_bg_opa(buttons_zero, LV_OPA_TRANSP, LV_PART_MAIN);
			lv_obj_set_size(buttons_zero, LV_SIZE_CONTENT + 1, LV_SIZE_CONTENT + 1);
			lv_obj_add_flag(buttons_zero, LV_OBJ_FLAG_HIDDEN);

			lv_obj_t *btn1 = side_button(buttons_zero,  30, &Img_ZeroX, zero_btn_group);
			static uint8_t x = 0;
			lv_obj_add_event_cb(btn1, zero_axis, LV_EVENT_PRESSED, &x);

			lv_obj_t *btn2 = side_button(buttons_zero, 115, &Img_ZeroY, zero_btn_group);
			static uint8_t y = 1;
			lv_obj_add_event_cb(btn2, zero_axis, LV_EVENT_PRESSED, &y);

			lv_obj_t *btn3 = side_button(buttons_zero, 200, &Img_ZeroZ, zero_btn_group);
			static uint8_t z = 2;
			lv_obj_add_event_cb(btn3, zero_axis, LV_EVENT_PRESSED, &z);

			lv_obj_t *back = win9x_button(buttons_zero, "Back");
			lv_obj_set_width(back, 50);
			lv_obj_set_pos(back, 0, 285);
			lv_group_add_obj(zero_btn_group, back);
			lv_obj_add_event_cb(back, side_buttons_back, LV_EVENT_PRESSED, NULL);

			lv_group_focus_obj(btn1);
			side_buttons_blocks[1] = buttons_zero;
		}
	}

	{
		lv_obj_t *container = lv_obj_create(screen);
		lv_obj_set_pos(container, 70, 30);
		lv_obj_set_size(container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

		lv_obj_set_style_text_font(container, &font_pixel_mono_14pt, LV_PART_MAIN);
		lv_obj_set_style_text_color(container, col_black, LV_PART_MAIN);

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

		for(int i = 0; i < 3; ++i)
		{
			lv_table_set_cell_value(table, i + 1, 0, axis_labels[i]);
		}

		lv_obj_add_event_cb(table, coord_box_event_cb, LV_EVENT_DRAW_TASK_ADDED, NULL);
		lv_obj_add_flag(table, LV_OBJ_FLAG_SEND_DRAW_TASK_EVENTS);

		g_state.coordinate_table = table;
		update_coordinates();
	}

	{
		lv_obj_t *container = lv_obj_create(screen);
		lv_obj_set_pos(container, 350, 30);
		lv_obj_set_size(container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

		lv_obj_set_style_text_font(container, &font_pixel_mono_14pt, LV_PART_MAIN);
		lv_obj_set_style_text_color(container, col_black, LV_PART_MAIN);

		lv_obj_add_style(container, &g_styles.container, LV_PART_MAIN);
		WIN9X_BORDER_PART_TWO(container, shadow_light);

		lv_obj_t *table = lv_table_create(container);

		lv_obj_add_style(table, &g_styles.table_item, LV_PART_ITEMS);

		lv_table_set_cell_value(table, 0, 0, "F");
		lv_table_set_cell_value(table, 1, 0, "S");

		lv_table_set_column_width(table, 0, 30);
		lv_table_set_column_width(table, 1, 80);

		lv_obj_add_event_cb(table, feed_box_event_cb, LV_EVENT_DRAW_TASK_ADDED, NULL);
		lv_obj_add_flag(table, LV_OBJ_FLAG_SEND_DRAW_TASK_EVENTS);

		g_state.feed_spindle_table = table;
		update_feed_spindle();
	}

	{
		lv_obj_t *container = lv_obj_create(screen);
		lv_obj_set_pos(container, 350, 110);
		lv_obj_set_size(container, 110, LV_SIZE_CONTENT);

		lv_obj_set_style_text_font(container, &font_pixel_mono_14pt, LV_PART_MAIN);
		lv_obj_set_style_text_color(container, col_black, LV_PART_MAIN);

		lv_obj_set_style_pad_all(container, 4, LV_PART_MAIN);

		lv_obj_add_style(container, &g_styles.container, LV_PART_MAIN);
		WIN9X_BORDER_PART_TWO(container, shadow_light);

		g_state.state_box = container;

		lv_obj_t *label = lv_label_create(container);
		lv_obj_set_align(label, LV_ALIGN_CENTER);

		g_state.state_label = label;
		update_state();
	}

	system_menu_set_action_callback(0, NULL);
}

static void idle_update()
{
	update_topbar();
	update_coordinates();
	update_feed_spindle();
	update_state();
}

STYLE_LOAD_SCREEN_UPDATE_WITH_GROUP(idle);

void style_alarm()
{
	style_idle();
}

static void get_alarm_string(char* dest)
{
	switch (cnc_get_alarm())
	{
		case 1:
			rom_strcpy(dest, __romstr__(STR_ALARM_1));
			break;
		case 2:
			rom_strcpy(dest, __romstr__(STR_ALARM_2));
			break;
		case 3:
			rom_strcpy(dest, __romstr__(STR_ALARM_3));
			break;
		case 4:
			rom_strcpy(dest, __romstr__(STR_ALARM_4));
			break;
		case 5:
			rom_strcpy(dest, __romstr__(STR_ALARM_5));
			break;
		case 6:
			rom_strcpy(dest, __romstr__(STR_ALARM_6));
			break;
		case 7:
			rom_strcpy(dest, __romstr__(STR_ALARM_7));
			break;
		case 8:
			rom_strcpy(dest, __romstr__(STR_ALARM_8));
			break;
		case 9:
			rom_strcpy(dest, __romstr__(STR_ALARM_9));
			break;
		case 10:
			rom_strcpy(dest, __romstr__(STR_ALARM_10));
			break;
		case 11:
			rom_strcpy(dest, __romstr__(STR_ALARM_11));
			break;
		case 12:
			rom_strcpy(dest, __romstr__(STR_ALARM_12));
			break;
		case 13:
			rom_strcpy(dest, __romstr__(STR_ALARM_13));
			break;
		default:
			rom_strcpy(dest, __romstr__(STR_ALARM_0));
			break;
	}
}

#endif

