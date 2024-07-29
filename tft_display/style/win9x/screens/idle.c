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

#include "../../support.h"

#ifdef TFT_STYLE_WIN9X

#include "../colors.h"
#include "../utility.h"

#include "../bitmaps/warning.h"
#include "../bitmaps/move.h"
#include "../bitmaps/zero.h"

#ifndef MAX_MODAL_GROUPS
#define MAX_MODAL_GROUPS 14
#endif

static void get_alarm_string(char* dest);

void idle_screen_dyn_draw(void);

GFX_DECL_SCREEN(idle, idle_screen_dyn_draw)
{
	GFX_SCREEN_HEADER();
	GFX_SET_FONT(FONT_MONO, 1);

	GFX_CLEAR(BASE_BACKGROUND);

	/* Top bar */
	GFX_CONTAINER_dynamic(0, 0, GFX_DISPLAY_WIDTH, 22, TOP_BAR);

	uint8_t state = cnc_get_exec_state(EXEC_RESET_LOCKED);
	GFX_TEXT_dynamic(GFX_DISPLAY_WIDTH - 19, 3, GFX_WHITE, FONT_SYMBOL, 2, state ? "\x01" : "\x02");

	uint8_t modes[MAX_MODAL_GROUPS];
	uint16_t feed, spindle;
	parser_get_modes(modes, &feed, &spindle);

	char str[64];
	sprintf(str, "G%d %s [%s] T%d",
					modes[6],
					modes[2] == 90 ? "Abs" : "Inc",
					modes[4] == 21 ? "mm" : "inch",
					modes[11]);

	GFX_TEXT_dynamic(2, 0, GFX_WHITE, FONT_SANS, 1, str);

	if(cnc_has_alarm())
	{
		GFX_RECT_dynamic(180, 0, 220, 22, GFX_RED);
		get_alarm_string(str);
		GFX_TEXT_dynamic(180 + GFX_CENTER_TEXT_OFFSET(220, FONT_SANS, 1, str), 0, GFX_RED, GFX_WHITE, FONT_SANS, 1, str);

		GFX_BITMAP_dynamic(182, 5, 14, 13, GFX_RED, GFX_WHITE, WarningSign_14x13);
		GFX_BITMAP_dynamic(384, 5, 14, 13, GFX_RED, GFX_WHITE, WarningSign_14x13);
	}

	BOX_OUTSET(0, 115, 50, 50, BASE_BACKGROUND);
	GFX_BITMAP(GFX_REL(2, 2), 20, 20, BASE_BACKGROUND, CHARCOAL, MoveBitmap_20x20, 2);

	BOX_OUTSET(0, 30, 50, 50, BASE_BACKGROUND);
	GFX_BITMAP(GFX_REL(4, 8), 12, 9, BASE_BACKGROUND, CHARCOAL, ZeroPosBitmap_12x9, 3);

	BOX_OUTSET(0, 200, 50, 50, BASE_BACKGROUND);

	/* Coordinates */
#define COORDINATE_BOX_X 70
#define COORDINATE_BOX_Y 30
#define COORDINATE_BOX_WIDTH 280
#define COORDINATE_BOX_HEIGHT 138

	BOX_INSET_dynamic(COORDINATE_BOX_X, COORDINATE_BOX_Y, COORDINATE_BOX_WIDTH, COORDINATE_BOX_HEIGHT, BOX_BACKGROUND);

	float mpos[MAX(AXIS_COUNT, 3)];
	int32_t steppos[STEPPER_COUNT];
	itp_get_rt_position(steppos);
	kinematics_apply_forward(steppos, mpos);
	kinematics_apply_reverse_transform(mpos);

	float wpos[MAX(AXIS_COUNT, 3)];
	for(uint8_t i = 0; i < MAX(AXIS_COUNT, 3); ++i)
		wpos[i] = mpos[i];
	parser_machine_to_work(wpos);

#define DRAW_AXIS(y, axis, idx, color) \
	GFX_TEXT(GFX_REL(7, y), color, axis); \
	sprintf(str, "%4d.%02d %4d.%02d", (int)wpos[idx], ABS((int)(wpos[idx] * 100) % 100), (int)mpos[idx], ABS((int)(mpos[idx] * 100) % 100)); \
	GFX_TEXT_dynamic(GFX_REL(57, y), GFX_BLACK, str);

	GFX_TEXT(GFX_REL(36, 7), GFX_BLACK, "Work");
	GFX_TEXT(GFX_REL(169, 7), GFX_BLACK, "Machine");

	GFX_RECT(GFX_REL(0, 7 + 26), COORDINATE_BOX_WIDTH - 6, 1, SEPARATOR);
	DRAW_AXIS(40, "X", 0, GFX_RED);
	GFX_RECT_dynamic(GFX_REL(0, 40 + 26), COORDINATE_BOX_WIDTH - 6, 1, SEPARATOR);
	DRAW_AXIS(73, "Y", 1, GFX_DARK_GREEN);
	GFX_RECT_dynamic(GFX_REL(0, 73 + 26), COORDINATE_BOX_WIDTH - 6, 1, SEPARATOR);
	DRAW_AXIS(106, "Z", 2, GFX_BLUE);

	GFX_RECT_dynamic(GFX_REL(29, 0), 1, COORDINATE_BOX_HEIGHT - 6, SEPARATOR);
	GFX_RECT_dynamic(GFX_REL(162, 0), 1, COORDINATE_BOX_HEIGHT - 6, SEPARATOR);

	/* Feed and Spindle */
#define FS_BOX_X 360
#define FS_BOX_Y 30
#define FS_BOX_WIDTH 110
#define FS_BOX_HEIGHT 70

	BOX_INSET_dynamic(FS_BOX_X, FS_BOX_Y, FS_BOX_WIDTH, FS_BOX_HEIGHT, BOX_BACKGROUND);

	GFX_TEXT(GFX_REL(6, 7), GFX_BLACK, "F");
	sprintf(str, "%4d", feed);
	GFX_TEXT_dynamic(GFX_REL(45, 7), GFX_BLACK, str);

	GFX_RECT_dynamic(GFX_REL(0, 32), FS_BOX_WIDTH - 6, 1, SEPARATOR);

	GFX_TEXT(GFX_REL(6, 37), GFX_BLACK, "S");
	sprintf(str, "%5d", spindle);
	GFX_TEXT_dynamic(GFX_REL(31, 37), GFX_BLACK, str);

	/* Status */
	state = cnc_get_exec_state(EXEC_ALLACTIVE);
	uint8_t filter = 0x80;
	while(!(state & filter) && filter)
		filter >>= 1;
	state &= filter;

	gfx_pixel_t bg_color = BOX_BACKGROUND;
	switch(state)
	{
		case EXEC_DOOR:
			rom_strcpy(str, MSG_STATUS_DOOR);
			bg_color = STATUS_COLOR_RED;
			break;
		case EXEC_KILL:
		case EXEC_UNHOMED:
		case EXEC_LIMITS:
			rom_strcpy(str, MSG_STATUS_ALARM);
			bg_color = STATUS_COLOR_RED;
			break;
		case EXEC_HOLD:
			rom_strcpy(str, MSG_STATUS_HOLD);
			bg_color = STATUS_COLOR_YELLOW;
			break;
		case EXEC_HOMING:
			rom_strcpy(str, MSG_STATUS_HOME);
			bg_color = STATUS_COLOR_GREEN;
			break;
		case EXEC_JOG:
			rom_strcpy(str, MSG_STATUS_JOG);
			bg_color = STATUS_COLOR_BLUE;
			break;
		case EXEC_RUN:
			rom_strcpy(str, MSG_STATUS_RUN);
			bg_color = STATUS_COLOR_GREEN;
			break;
		default:
			rom_strcpy(str, MSG_STATUS_IDLE);
			break;
	}

#define SCREEN_MAIN_STATUS_X 360
#define SCREEN_MAIN_STATUS_Y 110
#define SCREEN_MAIN_STATUS_WIDTH 110
#define SCREEN_MAIN_STATUS_HEIGHT 37

	BOX_INSET(SCREEN_MAIN_STATUS_X, SCREEN_MAIN_STATUS_Y, SCREEN_MAIN_STATUS_WIDTH, SCREEN_MAIN_STATUS_HEIGHT, bg_color);
	GFX_TEXT(GFX_REL(GFX_CENTER_TEXT_OFFSET(SCREEN_MAIN_STATUS_WIDTH, str), 7), bg_color, GFX_BLACK, str);
}

void idle_screen_dyn_draw(void)
{
	GFX_RENDER_AREA(idle, 0, 0, GFX_DISPLAY_WIDTH, 22); \
	GFX_RENDER_AREA(idle, COORDINATE_BOX_X + 60, COORDINATE_BOX_Y + 40, COORDINATE_BOX_WIDTH - 66, COORDINATE_BOX_HEIGHT - 46); \
	GFX_RENDER_AREA(idle, FS_BOX_X + 30, FS_BOX_Y + 3, FS_BOX_WIDTH - 36, FS_BOX_HEIGHT - 6);
	
	static uint8_t last_exec_state = 0;
	uint8_t state = cnc_get_exec_state(EXEC_ALLACTIVE);
	if(state != last_exec_state) {
		last_exec_state = state;

		gfx_invalidate();
		GFX_RENDER_AREA(idle, SCREEN_MAIN_STATUS_X, SCREEN_MAIN_STATUS_Y, SCREEN_MAIN_STATUS_WIDTH, SCREEN_MAIN_STATUS_HEIGHT);
	}
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

