/*
	Name: tft_system_menu.c
	Description: TFT display for µCNC using lgvl. Integration functions with system menu

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 20-08-2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "src/cnc.h"
#include "src/modules/system_menu.h"
#include "src/modules/softspi.h"
#include "lvgl/lvgl.h"
#include "lvgl/src/drivers/display/st7796/lv_st7796.h"
#include "ui/ui.h"
#include "src/modules/touch_screen/touch_screen.h"

#ifndef MAX_TFT_LINES
#define MAX_TFT_LINES 6
#endif

/**
 * Helper functions
 */
static void io_states_str(char *buff)
{
	uint8_t controls = io_get_controls();
	uint8_t limits = io_get_limits();
	uint8_t probe = io_get_probe();
	uint8_t i = 0;
	if (CHECKFLAG(controls, (ESTOP_MASK | SAFETY_DOOR_MASK | FHOLD_MASK)) || CHECKFLAG(limits, LIMITS_MASK) || probe)
	{
		if (CHECKFLAG(controls, ESTOP_MASK))
		{
			buff[i++] = 'R';
		}

		if (CHECKFLAG(controls, SAFETY_DOOR_MASK))
		{
			buff[i++] = 'D';
		}

		if (CHECKFLAG(controls, FHOLD_MASK))
		{
			buff[i++] = 'H';
		}

		if (probe)
		{
			buff[i++] = 'P';
		}

		if (CHECKFLAG(limits, LINACT0_LIMIT_MASK))
		{
			buff[i++] = 'X';
		}

		if (CHECKFLAG(limits, LINACT1_LIMIT_MASK))
		{

#if ((AXIS_COUNT == 2) && defined(USE_Y_AS_Z_ALIAS))
			buff[i++] = 'Z';
#else
			buff[i++] = 'Y';
#endif
		}

		if (CHECKFLAG(limits, LINACT2_LIMIT_MASK))
		{
			buff[i++] = 'Z';
		}

		if (CHECKFLAG(limits, LINACT3_LIMIT_MASK))
		{
			buff[i++] = 'A';
		}

		if (CHECKFLAG(limits, LINACT4_LIMIT_MASK))
		{
			buff[i++] = 'B';
		}

		if (CHECKFLAG(limits, LINACT5_LIMIT_MASK))
		{
			buff[i++] = 'C';
		}
	}
}

static void render_status(char *buff)
{
	uint8_t state = cnc_get_exec_state(0xFF);
	uint8_t filter = 0x80;
	while (!(state & filter) && filter)
	{
		filter >>= 1;
	}

	state &= filter;
	if (cnc_has_alarm())
	{
		rom_strcpy(buff, MSG_STATUS_ALARM);
		ui_object_set_themeable_style_property(ui_idle_container_statusinfo, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_BG_COLOR,
																					 _ui_theme_color_alarm);
	}
	else if (mc_get_checkmode())
	{
		rom_strcpy(buff, MSG_STATUS_CHECK);
	}
	else
	{
		switch (state)
		{
		case EXEC_DOOR:
			rom_strcpy(buff, MSG_STATUS_DOOR);
			ui_object_set_themeable_style_property(ui_idle_container_statusinfo, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_BG_COLOR,
																						 _ui_theme_color_hold);
			break;
		case EXEC_KILL:
		case EXEC_UNHOMED:
			rom_strcpy(buff, MSG_STATUS_ALARM);
			ui_object_set_themeable_style_property(ui_idle_container_statusinfo, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_BG_COLOR,
																						 _ui_theme_color_alarm);
			break;
		case EXEC_HOLD:
			rom_strcpy(buff, MSG_STATUS_HOLD);
			ui_object_set_themeable_style_property(ui_idle_container_statusinfo, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_BG_COLOR,
																						 _ui_theme_color_hold);
			break;
		case EXEC_HOMING:
			rom_strcpy(buff, MSG_STATUS_HOME);
			ui_object_set_themeable_style_property(ui_idle_container_statusinfo, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_BG_COLOR,
																						 _ui_theme_color_button);
			break;
		case EXEC_JOG:
			rom_strcpy(buff, MSG_STATUS_JOG);
			ui_object_set_themeable_style_property(ui_idle_container_statusinfo, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_BG_COLOR,
																						 _ui_theme_color_button);
			break;
		case EXEC_RUN:
			rom_strcpy(buff, MSG_STATUS_RUN);
			ui_object_set_themeable_style_property(ui_idle_container_statusinfo, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_BG_COLOR,
																						 _ui_theme_color_button);
			break;
		default:
			rom_strcpy(buff, MSG_STATUS_IDLE);
			ui_object_set_themeable_style_property(ui_idle_container_statusinfo, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_BG_COLOR,
																						 _ui_theme_color_container);
			break;
		}
	}

	lv_label_set_text(ui_idle_label_statusvalue, buff);
}

static void append_modal_state(char *buffer, char word, uint8_t code, uint8_t mantissa)
{
	char tmp[32];
	memset(tmp, 0, sizeof(tmp));
	if (mantissa)
	{
		sprintf(tmp, "%c%d.%d", word, code, mantissa);
	}
	else
	{
		sprintf(tmp, "%c%d", word, code);
	}

	strcat(buffer, tmp);
}

/**
 *
 * Sytem menu implementations
 *
 */
static int8_t item_line;
static int8_t item_line_value;
static lv_obj_t *modal_popup;
#ifndef STR_PRESS_RESET
#define STR_PRESS_RESET "Press to reset"
#endif
// reset button event for alarm message box
static void reset_event_cb(lv_event_t *e)
{
	cnc_call_rt_command(CMD_CODE_RESET);
}

void system_menu_render_header(const char *__s)
{
	// udpate header lables
	lv_label_set_text(ui_navigate_label_headerlabel, __s);
	lv_label_set_text(ui_edit_label_headerlabel, __s);
	item_line = 0;
	item_line_value = 0;
	if (modal_popup)
	{
		lv_msgbox_close(modal_popup);
		modal_popup = NULL;
	}
}

bool system_menu_render_menu_item_filter(uint8_t item_index)
{
	static uint8_t menu_top = 0;
	uint8_t current_index = MAX(0, g_system_menu.current_index);
	uint8_t max_lines = MAX_TFT_LINES;

	uint8_t top = menu_top;
	if ((top + max_lines) <= current_index)
	{
		// advance menu top
		menu_top = top = (current_index - max_lines + 1);
	}

	if (top > current_index)
	{
		// rewind menu top
		menu_top = top = current_index;
	}

	return ((top <= item_index) && (item_index < (top + max_lines)));
}

void system_menu_item_render_label(uint8_t render_flags, const char *label)
{
	int8_t line = item_line;
	item_line = line + 1;

	char str[SYSTEM_MENU_MAX_STR_LEN];
	memset(str, 0, sizeof(str));
	if (label)
	{
		strcpy(str, label);
	}

	switch (line)
	{
	case 0:
		lv_label_set_text(ui_navigate_label_menuitemlabel0, str);
		lv_obj_set_state(ui_navigate_container_menuitem0, LV_STATE_FOCUSED, (render_flags & SYSTEM_MENU_MODE_SELECT));
		break;
	case 1:
		lv_label_set_text(ui_navigate_label_menuitemlabel1, str);
		lv_obj_set_state(ui_navigate_container_menuitem1, LV_STATE_FOCUSED, (render_flags & SYSTEM_MENU_MODE_SELECT));
		break;
	case 2:
		lv_label_set_text(ui_navigate_label_menuitemlabel2, str);
		lv_obj_set_state(ui_navigate_container_menuitem2, LV_STATE_FOCUSED, (render_flags & SYSTEM_MENU_MODE_SELECT));
		break;
	case 3:
		lv_label_set_text(ui_navigate_label_menuitemlabel3, str);
		lv_obj_set_state(ui_navigate_container_menuitem3, LV_STATE_FOCUSED, (render_flags & SYSTEM_MENU_MODE_SELECT));
		break;
	case 4:
		lv_label_set_text(ui_navigate_label_menuitemlabel4, str);
		lv_obj_set_state(ui_navigate_container_menuitem4, LV_STATE_FOCUSED, (render_flags & SYSTEM_MENU_MODE_SELECT));
		break;
	case 5:
		lv_label_set_text(ui_navigate_label_menuitemlabel5, str);
		lv_obj_set_state(ui_navigate_container_menuitem5, LV_STATE_FOCUSED, (render_flags & SYSTEM_MENU_MODE_SELECT));
		break;
	}
}

void system_menu_item_render_arg(uint8_t render_flags, const char *value)
{
	if (!CHECKFLAG(g_system_menu.flags, (SYSTEM_MENU_MODE_EDIT | SYSTEM_MENU_MODE_SIMPLE_EDIT)))
	{
		char str[SYSTEM_MENU_MAX_STR_LEN];
		memset(str, 0, sizeof(str));
		if (value)
		{
			strcpy(str, value);
		}

		int8_t line = item_line_value;
		item_line_value = line + 1;
		switch (line)
		{
		case 0:
			lv_label_set_text(ui_navigate_label_menuitemvalue0, str);
			break;
		case 1:
			lv_label_set_text(ui_navigate_label_menuitemvalue1, str);
			break;
		case 2:
			lv_label_set_text(ui_navigate_label_menuitemvalue2, str);
			break;
		case 3:
			lv_label_set_text(ui_navigate_label_menuitemvalue3, str);
			break;
		case 4:
			lv_label_set_text(ui_navigate_label_menuitemvalue4, str);
			break;
		case 5:
			lv_label_set_text(ui_navigate_label_menuitemvalue5, str);
			break;
		}

		if ((render_flags & SYSTEM_MENU_MODE_SELECT))
		{ // also set text of edit box with the current selected item
			lv_textarea_set_text(ui_edit_textarea_vareditinput, str);
		}
	}
}

void system_menu_render_nav_back(bool is_hover)
{
	if (is_hover)
	{
		lv_obj_add_state(ui_navigate_button_btnclose, LV_STATE_FOCUSED);
		lv_obj_add_state(ui_edit_button_btnclose, LV_STATE_FOCUSED);
		lv_obj_add_state(ui_jog_button_btnclose, LV_STATE_FOCUSED);
	}
	else
	{
		lv_obj_remove_state(ui_navigate_button_btnclose, LV_STATE_FOCUSED);
		lv_obj_remove_state(ui_edit_button_btnclose, LV_STATE_FOCUSED);
		lv_obj_remove_state(ui_jog_button_btnclose, LV_STATE_FOCUSED);
	}
}

void system_menu_render_footer(void)
{
	if (CHECKFLAG(g_system_menu.flags, (SYSTEM_MENU_MODE_EDIT | SYSTEM_MENU_MODE_SIMPLE_EDIT)))
	{
		lv_disp_load_scr(ui_edit);
	}
	else
	{
		lv_disp_load_scr(ui_navigate);
		while (item_line < MAX_TFT_LINES)
		{
			system_menu_item_render_label(0, "");
		}
		while (item_line_value < MAX_TFT_LINES)
		{
			system_menu_item_render_arg(0, "");
		}
	}
}

void system_menu_render_startup(void)
{
	lv_label_set_text(ui_startup_label_versioninfo, "uCNC v" CNC_VERSION);
	lv_disp_load_scr(ui_startup);
}

void system_menu_render_idle(void)
{
	//  starts from the bottom up
	float axis[MAX(AXIS_COUNT, 3)];
	int32_t steppos[STEPPER_COUNT];
	itp_get_rt_position(steppos);
	kinematics_apply_forward(steppos, axis);
	kinematics_apply_reverse_transform(axis);
	char buffer[SYSTEM_MENU_MAX_STR_LEN];

	memset(buffer, 0, sizeof(buffer));
	render_status(buffer);

#if (AXIS_COUNT >= 6)
	memset(buffer, 0, sizeof(buffer));
	system_menu_flt_to_str(buffer, axis[5]);
	lv_label_set_text(ui_comp_get_child(ui_idle_axisinfo_axisinfoc, UI_COMP_CONTAINER_AXISINFO_LABEL_AXISVALUE), buffer);
#else
	lv_obj_set_width(ui_idle_axisinfo_axisinfoc, 0);
#endif
#if (AXIS_COUNT >= 5)
	memset(buffer, 0, sizeof(buffer));
	system_menu_flt_to_str(buffer, axis[4]);
	lv_label_set_text(ui_comp_get_child(ui_idle_axisinfo_axisinfob, UI_COMP_CONTAINER_AXISINFO_LABEL_AXISVALUE), buffer);
#else
	lv_obj_set_width(ui_idle_axisinfo_axisinfob, 0);
#endif
#if (AXIS_COUNT >= 4)
	memset(buffer, 0, sizeof(buffer));
	system_menu_flt_to_str(buffer, axis[3]);
	lv_label_set_text(ui_comp_get_child(ui_idle_axisinfo_axisinfoa, UI_COMP_CONTAINER_AXISINFO_LABEL_AXISVALUE), buffer);
#else
	lv_obj_set_height(ui_idle_container_container9, 0); // hide the full container
#endif
#if (AXIS_COUNT >= 3)
	memset(buffer, 0, sizeof(buffer));
	system_menu_flt_to_str(buffer, axis[2]);
	lv_label_set_text(ui_comp_get_child(ui_idle_axisinfo_axisinfoz, UI_COMP_CONTAINER_AXISINFO_LABEL_AXISVALUE), buffer);
#else
	lv_obj_set_width(ui_idle_axisinfo_axisinfoz, 0);
#endif
#if (AXIS_COUNT >= 2)
	memset(buffer, 0, sizeof(buffer));
	system_menu_flt_to_str(buffer, axis[1]);
	lv_label_set_text(ui_comp_get_child(ui_idle_axisinfo_axisinfoy, UI_COMP_CONTAINER_AXISINFO_LABEL_AXISVALUE), buffer);
#else
	lv_obj_set_width(ui_idle_axisinfo_axisinfoy, 0);
#endif
#if (AXIS_COUNT >= 1)
	memset(buffer, 0, sizeof(buffer));
	system_menu_flt_to_str(buffer, axis[0]);
	lv_label_set_text(ui_comp_get_child(ui_idle_axisinfo_axisinfox, UI_COMP_CONTAINER_AXISINFO_LABEL_AXISVALUE), buffer);
#endif

	memset(buffer, 0, sizeof(buffer));
	io_states_str(buffer);
	lv_label_set_text(ui_idle_label_switchvalue, buffer);

	memset(buffer, 0, sizeof(buffer));
	system_menu_int_to_str(buffer, (int32_t)itp_get_rt_feed());
	lv_label_set_text(ui_idle_label_feedvalue, buffer);

	// Tool
	uint8_t modalgroups[14];
	uint16_t feed;
	uint16_t spindle;
	parser_get_modes(modalgroups, &feed, &spindle);
	memset(buffer, 0, sizeof(buffer));
	system_menu_int_to_str(buffer, modalgroups[11]);
	lv_label_set_text(ui_idle_label_toolnumbervalue, buffer);
	// Realtime tool speed
	memset(buffer, 0, sizeof(buffer));
	system_menu_int_to_str(buffer, tool_get_speed());
	lv_label_set_text(ui_idle_label_toolspeedvalue, buffer);

	/**shoe some modal states */
	memset(buffer, 0, sizeof(buffer));
	append_modal_state(buffer, 'G', modalgroups[0], modalgroups[12]);
	for (uint8_t i = 1; i < 7; i++)
	{
		append_modal_state(buffer, 'G', modalgroups[i], 0);
	}

	append_modal_state(buffer, 'M', modalgroups[8], 0);
#ifdef ENABLE_COOLANT
	if (modalgroups[9] == M9)
	{
		append_modal_state(buffer, 'M', 9, 0);
	}
#ifndef M7_SAME_AS_M8
	if (modalgroups[9] & M7)
	{
		append_modal_state(buffer, 'M', 7, 0);
	}
#endif
	if (modalgroups[9] & M8)
	{
		append_modal_state(buffer, 'M', 8, 0);
	}
#else
	// permanent M9
	append_modal_state(buffer, 'M', 9, 0);
#endif

	lv_label_set_text(ui_idle_label_modalmodesvalue, buffer);

	// unlock button image
	if (cnc_get_exec_state(EXEC_UNHOMED))
	{
		lv_image_set_scale(ui_idle_image_image8, 256);
		lv_image_set_scale(ui_idle_image_image17, 0);
	}
	else
	{
		lv_image_set_scale(ui_idle_image_image8, 0);
		lv_image_set_scale(ui_idle_image_image17, 256);
	}

	// hold resume button image
	if (cnc_get_exec_state(EXEC_HOLD))
	{
		lv_image_set_scale(ui_idle_image_image12, 256);
		lv_image_set_scale(ui_idle_image_image7, 0);
	}
	else
	{
		lv_image_set_scale(ui_idle_image_image12, 0);
		lv_image_set_scale(ui_idle_image_image7, 256);
	}

	// lv_obj_invalidate(ui_idle);
	lv_disp_load_scr(ui_idle);
}

void system_menu_render_jog(uint8_t flags)
{
	char buffer[SYSTEM_MENU_MAX_STR_LEN];
	lv_roller_get_selected_str(ui_jog_roller_jogaxis, buffer, SYSTEM_MENU_MAX_STR_LEN);
	buffer[1] = ':';
	buffer[2] = 0;

	uint8_t jog_axis = MIN(AXIS_COUNT, lv_roller_get_selected(ui_jog_roller_jogaxis));
	float axis[MAX(AXIS_COUNT, 3)];
	int32_t steppos[STEPPER_COUNT];
	itp_get_rt_position(steppos);
	kinematics_apply_forward(steppos, axis);
	kinematics_apply_reverse_transform(axis);
	lv_label_set_text(ui_jog_label_jogaxislabel, buffer);
	memset(buffer, 0, sizeof(buffer));
	system_menu_flt_to_str(buffer, axis[jog_axis]);
	lv_label_set_text(ui_jog_label_jogaxisvalue, buffer);
	if (cnc_get_exec_state(EXEC_JOG))
	{
		// force redraw update
		g_system_menu.flags |= SYSTEM_MENU_MODE_DELAYED_REDRAW;
		system_menu_action_timeout(SYSTEM_MENU_REDRAW_IDLE_MS);
	}
	lv_disp_load_scr(ui_jog);
}

void system_menu_render_modal_popup(const char *__s)
{
	if (!modal_popup)
	{
		modal_popup = lv_msgbox_create(NULL);
		lv_msgbox_add_title(modal_popup, "Info");
		lv_msgbox_add_text(modal_popup, __s);
	}
}

void system_menu_render_alarm(void)
{
	lv_disp_load_scr(ui_alarm);
	
	char buff[SYSTEM_MENU_MAX_STR_LEN];
	rom_strcpy(buff, __romstr__("ALARM "));
	uint8_t alarm = cnc_get_alarm();
	system_menu_int_to_str(&buff[6], alarm);

	lv_label_set_text(ui_alarm_label_alarmcode, buff);

	memset(buff, 0, sizeof(buff));

	switch (alarm)
	{
	case 1:
		rom_strcpy(buff, __romstr__(STR_ALARM_1));
		break;
	case 2:
		rom_strcpy(buff, __romstr__(STR_ALARM_2));
		break;
	case 3:
		rom_strcpy(buff, __romstr__(STR_ALARM_3));
		break;
	case 4:
		rom_strcpy(buff, __romstr__(STR_ALARM_4));
		break;
	case 5:
		rom_strcpy(buff, __romstr__(STR_ALARM_5));
		break;
	case 6:
		rom_strcpy(buff, __romstr__(STR_ALARM_6));
		break;
	case 7:
		rom_strcpy(buff, __romstr__(STR_ALARM_7));
		break;
	case 8:
		rom_strcpy(buff, __romstr__(STR_ALARM_8));
		break;
	case 9:
		rom_strcpy(buff, __romstr__(STR_ALARM_9));
		break;
	case 10:
		rom_strcpy(buff, __romstr__(STR_ALARM_10));
		break;
	case 11:
		rom_strcpy(buff, __romstr__(STR_ALARM_11));
		break;
	case 12:
		rom_strcpy(buff, __romstr__(STR_ALARM_12));
		break;
	case 13:
		rom_strcpy(buff, __romstr__(STR_ALARM_13));
		break;
	default:
		rom_strcpy(buff, __romstr__(STR_ALARM_0));
		break;
	}

	lv_label_set_text(ui_alarm_label_alarmmessage, buff);
}

/**
 * LVGL button events
 * */

#include "ui/ui.h"

static uint8_t str_to_float(char *str, float *value)
{
	uint32_t intval = 0;
	uint8_t fpcount = 0;
	float rhs = 0;
	uint8_t result = NUMBER_UNDEF;

	uint8_t c = *str++;

	if (c == '-' || c == '+')
	{
		if (c == '-')
		{
			result |= NUMBER_ISNEGATIVE;
		}
		c = *str++;
	}

	for (;;)
	{
		uint8_t digit = (uint8_t)c - 48;
		if (digit <= 9)
		{
			intval = fast_int_mul10(intval) + digit;
			if (fpcount)
			{
				fpcount++;
			}

			result |= NUMBER_OK;
		}
		else if (c == '.' && !fpcount)
		{
			fpcount++;
			result |= NUMBER_ISFLOAT;
		}
		else
		{
			if (!(result & NUMBER_OK))
			{
				return NUMBER_UNDEF;
			}
			break;
		}

		c = *str++;
	}

	rhs = (float)intval;
	if (fpcount)
	{
		fpcount--;
	}

	do
	{
		if (fpcount >= 2)
		{
			rhs *= 0.01f;
			fpcount -= 2;
		}

		if (fpcount >= 1)
		{
			rhs *= 0.1f;
			fpcount -= 1;
		}

	} while (fpcount != 0);

	*value = (result & NUMBER_ISNEGATIVE) ? -rhs : rhs;

	return result;
}

static void send_jog_cmd(bool reverse)
{
	if (serial_freebytes() > SYSTEM_MENU_MAX_STR_LEN)
	{
		char buffer[SYSTEM_MENU_MAX_STR_LEN];
		memset(buffer, 0, SYSTEM_MENU_MAX_STR_LEN);
		rom_strcpy((char *)buffer, __romstr__("$J=G91"));

		char str[SYSTEM_MENU_MAX_STR_LEN];
		memset(str, 0, SYSTEM_MENU_MAX_STR_LEN);
		lv_roller_get_selected_str(ui_jog_roller_jogaxis, str, SYSTEM_MENU_MAX_STR_LEN);
		strcat(buffer, str);
		memset(str, 0, SYSTEM_MENU_MAX_STR_LEN);
		lv_roller_get_selected_str(ui_jog_roller_jogdist, str, SYSTEM_MENU_MAX_STR_LEN);
		if (reverse)
		{
			strcat(buffer, "-");
		}
		strcat(buffer, str);
		strcat(buffer, "F");
		memset(str, 0, SYSTEM_MENU_MAX_STR_LEN);
		lv_roller_get_selected_str(ui_jog_roller_jogfeed, str, SYSTEM_MENU_MAX_STR_LEN);
		strcat(buffer, str);
		strcat(buffer, "\n");

		system_menu_send_cmd(buffer);
	}
	// force redraw update
	g_system_menu.flags |= SYSTEM_MENU_MODE_DELAYED_REDRAW;
	system_menu_action_timeout(SYSTEM_MENU_REDRAW_IDLE_MS);
}

void touch_btn_jog_cb(lv_event_t *e)
{
	// Your code here
	system_menu_goto(SYSTEM_MENU_ID_JOG);
}

void touch_btn_settings_cb(lv_event_t *e)
{
	// Your code here
	system_menu_action(SYSTEM_MENU_ACTION_SELECT);
}

void touch_btn_prev_cb(lv_event_t *e)
{
	// Your code here
	system_menu_action(SYSTEM_MENU_ACTION_PREV);
}

void touch_btn_next_cb(lv_event_t *e)
{
	// Your code here
	system_menu_action(SYSTEM_MENU_ACTION_NEXT);
}

void touch_btn_enter_cb(lv_event_t *e)
{
	// Your code here
	system_menu_action(SYSTEM_MENU_ACTION_SELECT);
}

void touch_btn_close_cb(lv_event_t *e)
{
	// Your code here
	g_system_menu.current_index = -1;
	system_menu_action(SYSTEM_MENU_ACTION_SELECT);
}

void touch_btn_unlock_cb(lv_event_t *e)
{
	if (serial_freebytes() > SYSTEM_MENU_MAX_STR_LEN)
	{
		char buffer[SYSTEM_MENU_MAX_STR_LEN];
		memset(buffer, 0, SYSTEM_MENU_MAX_STR_LEN);
		rom_strcpy((char *)buffer, __romstr__("$X\r"));

		if (system_menu_send_cmd(buffer) != STATUS_OK)
		{
			rom_strcpy((char *)buffer, __romstr__(STR_CMD_NOTSENT));
			system_menu_show_modal_popup(SYSTEM_MENU_MODAL_POPUP_MS, buffer);
		}
	}
}

void touch_btn_hold_cb(lv_event_t *e)
{
	// Your code here
	if (cnc_get_exec_state(EXEC_HOLD))
	{
		cnc_call_rt_command(CMD_CODE_CYCLE_START);
	}
	else
	{
		cnc_call_rt_command(CMD_CODE_FEED_HOLD);
	}
}

void touch_btn_home_cb(lv_event_t *e)
{
	// Your code here
	if (serial_freebytes() > SYSTEM_MENU_MAX_STR_LEN)
	{
		char buffer[SYSTEM_MENU_MAX_STR_LEN];
		memset(buffer, 0, SYSTEM_MENU_MAX_STR_LEN);
		rom_strcpy((char *)buffer, __romstr__("$H\r"));

		if (system_menu_send_cmd(buffer) != STATUS_OK)
		{
			rom_strcpy((char *)buffer, __romstr__(STR_CMD_NOTSENT));
			system_menu_show_modal_popup(SYSTEM_MENU_MODAL_POPUP_MS, buffer);
		}
	}
}

void touch_kb_ready_ev(lv_event_t *e)
{
	// Your code here
	const char *value = lv_textarea_get_text(ui_edit_textarea_vareditinput);
	const system_menu_item_t *item = system_menu_get_current_item();
	uint8_t vartype = (uint8_t)VARG_CONST(item->render_arg);
	float fvalue = 0;
	if (str_to_float(value, &fvalue) == NUMBER_UNDEF)
	{
		fvalue = 0;
	}
	switch (vartype)
	{
	case VAR_TYPE_BOOLEAN:
		(*(bool *)item->argptr) = (fvalue != 0);
		break;
	case VAR_TYPE_INT8:
	case VAR_TYPE_UINT8:
		(*(uint8_t *)item->argptr) = CLAMP(0, (uint8_t)fvalue, 0xFF);
		break;
	case VAR_TYPE_INT16:
	case VAR_TYPE_UINT16:
		(*(uint16_t *)item->argptr) = CLAMP(0, (uint16_t)fvalue, 0xFFFF);
		break;
	case VAR_TYPE_INT32:
	case VAR_TYPE_UINT32:
		(*(uint32_t *)item->argptr) = CLAMP(0, (uint32_t)fvalue, 0xFFFFFFFF);
		break;
	case VAR_TYPE_FLOAT:
		(*(float *)item->argptr) = CLAMP(__FLT_MIN__, (float)fvalue, __FLT_MAX__);
		break;
	}

	if (CHECKFLAG(g_system_menu.flags, SYSTEM_MENU_MODE_EDIT))
	{
		// do a navback
		g_system_menu.current_index = -1;
		system_menu_action(SYSTEM_MENU_ACTION_SELECT);
	}
	else if (CHECKFLAG(g_system_menu.flags, SYSTEM_MENU_MODE_SIMPLE_EDIT))
	{
		g_system_menu.flags ^= SYSTEM_MENU_MODE_SIMPLE_EDIT;
	}
}

static uint8_t is_jog_longpress;
void touch_btn_jogplus_cb(lv_event_t *e)
{
	// Your code here
	uint8_t i = is_jog_longpress;
	is_jog_longpress = MAX(i, i + 1);
	send_jog_cmd(false);
}

void touch_btn_jogcancel_repeat_cb(lv_event_t *e)
{
	// Your code here
	if (is_jog_longpress > 1)
	{
		cnc_call_rt_command(CMD_CODE_JOG_CANCEL);
	}
	is_jog_longpress = 0;
	// force redraw update
	g_system_menu.flags |= SYSTEM_MENU_MODE_DELAYED_REDRAW;
	system_menu_action_timeout(SYSTEM_MENU_REDRAW_IDLE_MS);
}

void touch_btn_jogcancel_cb(lv_event_t *e)
{
	// Your code here
	cnc_call_rt_command(CMD_CODE_JOG_CANCEL);
	is_jog_longpress = 0;
	// force redraw update
	g_system_menu.flags |= SYSTEM_MENU_MODE_DELAYED_REDRAW;
	system_menu_action_timeout(SYSTEM_MENU_REDRAW_IDLE_MS);
}

void touch_btn_jogminus_cb(lv_event_t *e)
{
	// Your code here
	uint8_t i = is_jog_longpress;
	is_jog_longpress = MAX(i, i + 1);
	send_jog_cmd(true);
}

void touch_btn_reset_cb(lv_event_t *e)
{
	// Your code here
	cnc_call_rt_command(CMD_CODE_RESET);
}
