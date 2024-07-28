#include "../../graphics_library.h"

#include "support.h"

const uint8_t WarningSign_14x13[] = { 3, 0, 18, 0, 72, 2, 16, 11, 64, 76, 129, 50, 8, 196, 32, 17, 12, 36, 48, 160, 1, 255, 252 };

void get_alarm_string(char* dest)
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

GFX_DECL_SCREEN(screen)
{
	GFX_SCREEN_HEADER();
	GFX_SET_FONT(FONT_MONO, 1);

	GFX_CLEAR(BASE_BACKGROUND);

	/* Top bar */
	GFX_CONTAINER(0, 0, GFX_DISPLAY_WIDTH, 22, TOP_BAR);

	uint8_t state = cnc_get_exec_state(EXEC_RESET_LOCKED);
	GFX_TEXT(GFX_DISPLAY_WIDTH - 19, 3, GFX_WHITE, FONT_SYMBOL, 2, state ? "\x01" : "\x02");

	uint8_t modes[MAX_MODAL_GROUPS];
	uint16_t feed, spindle;
	parser_get_modes(modes, &feed, &spindle);

	char str[64];
	sprintf(str, "G%d %s [%s] T%d",
					modes[6],
					modes[2] == 90 ? "Abs" : "Inc",
					modes[4] == 21 ? "mm" : "inch",
					modes[11]);

	GFX_TEXT(2, 0, GFX_WHITE, FONT_SANS, 1, str);

	if(cnc_has_alarm())
	{
		GFX_RECT(180, 0, 220, 22, GFX_RED);
		get_alarm_string(str);
		GFX_TEXT(180 + GFX_CENTER_TEXT_OFFSET(220, FONT_SANS, 1, str), 0, GFX_RED, GFX_WHITE, FONT_SANS, 1, str);

		GFX_BITMAP(182, 5, 14, 13, GFX_RED, GFX_WHITE, WarningSign_14x13);
		GFX_BITMAP(384, 5, 14, 13, GFX_RED, GFX_WHITE, WarningSign_14x13);
	}

	BOX_OUTSET(0, 30, 50, 50, BOX_BACKGROUND);
	GFX_BITMAP(GFX_REL(9, 11), 15, 14, BOX_BACKGROUND, CHARCOAL, ZeroPosBitmap_15x14, 2);

	BOX_OUTSET(0, 115, 50, 50, BOX_BACKGROUND);
	GFX_BITMAP(GFX_REL(5, 5), 20, 20, BOX_BACKGROUND, CHARCOAL, MoveBitmap_20x20, 2);

	BOX_OUTSET(0, 200, 50, 50, BOX_BACKGROUND);

	/* Coordinates */
#define COORDINATE_BOX_WIDTH 280
#define COORDINATE_BOX_HEIGHT 138

	BOX_INSET_dynamic(70, 30, COORDINATE_BOX_WIDTH, COORDINATE_BOX_HEIGHT, BOX_BACKGROUND);

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
	GFX_TEXT(GFX_REL(10, y), color, axis); \
	sprintf(str, "%4d.%02d %4d.%02d", (int)wpos[idx], ABS((int)(wpos[idx] * 100) % 100), (int)mpos[idx], ABS((int)(mpos[idx] * 100) % 100)); \
	GFX_TEXT_dynamic(GFX_REL(60, y), GFX_BLACK, str);

	GFX_TEXT(GFX_REL(39, 10), GFX_BLACK, "Work");
	GFX_TEXT(GFX_REL(172, 10), GFX_BLACK, "Machine");

	GFX_RECT(GFX_REL(3, 10 + 26), COORDINATE_BOX_WIDTH - 6, 1, SEPARATOR);
	DRAW_AXIS(43, "X", 0, GFX_RED);
	GFX_RECT_dynamic(GFX_REL(3, 43 + 26), COORDINATE_BOX_WIDTH - 6, 1, SEPARATOR);
	DRAW_AXIS(76, "Y", 1, GFX_DARK_GREEN);
	GFX_RECT_dynamic(GFX_REL(3, 76 + 26), COORDINATE_BOX_WIDTH - 6, 1, SEPARATOR);
	DRAW_AXIS(109, "Z", 2, GFX_BLUE);

	GFX_RECT_dynamic(GFX_REL(32, 3), 1, COORDINATE_BOX_HEIGHT - 6, SEPARATOR);
	GFX_RECT_dynamic(GFX_REL(165, 3), 1, COORDINATE_BOX_HEIGHT - 6, SEPARATOR);

	/* Feed and Spindle */
#define FS_BOX_WIDTH 110
#define FS_BOX_HEIGHT 70

	BOX_INSET_dynamic(360, 30, 110, 70, BOX_BACKGROUND);

	GFX_TEXT(GFX_REL(9, 10), GFX_BLACK, "F");
	sprintf(str, "%4d", feed);
	GFX_TEXT_dynamic(GFX_REL(48, 10), GFX_BLACK, str);

	GFX_RECT_dynamic(GFX_REL(3, 35), FS_BOX_WIDTH - 6, 1, SEPARATOR);

	GFX_TEXT(GFX_REL(9, 40), GFX_BLACK, "S");
	sprintf(str, "%5d", spindle);
	GFX_TEXT_dynamic(GFX_REL(34, 40), GFX_BLACK, str);

	/* Status */
	state = cnc_get_exec_state(EXEC_ALLACTIVE);
	uint8_t filter = 0x80;
	while(!(state & filter) && filter)
		filter >>= 1;
	state &= filter;

	#define STATUS_COLOR_YELLOW GFX_COLOR(0xffff22)
	#define STATUS_COLOR_GREEN GFX_COLOR(0x88ff88)
	#define STATUS_COLOR_RED GFX_COLOR(0xff1111)
	#define STATUS_COLOR_BLUE GFX_COLOR(0xccccff)

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

	BOX_INSET(360, 110, 110, 37, bg_color);
	GFX_TEXT(GFX_REL(GFX_CENTER_TEXT_OFFSET(110, str), 8), bg_color, GFX_BLACK, str);
}
