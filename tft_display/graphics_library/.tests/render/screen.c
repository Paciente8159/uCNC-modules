#include "../../graphics_library.h"

#include "support.h"

#define COORD_DECIMAL_DIGITS 3
#define COORD_FRACTIONAL_DIGITS 2
#define COORD_BOX_SIZE ((COORD_DECIMAL_DIGITS + COORD_FRACTIONAL_DIGITS + 2) * 15)

uint8_t CheckboxEmpty_9x9[] = { 28, 49, 144, 80, 24, 12, 5, 4, 198, 28, 0 };
uint8_t CheckboxChecked_9x9[] = { 28, 49, 144, 83, 153, 204, 229, 4, 198, 28, 0 };

GFX_DECL_ELEMENT(coordinate_box, uint16_t x, uint16_t y, bool sign, uint8_t decimal, uint8_t fraction, float value, int highlight_index) {
	GFX_SET_FONT(FONT_PIXEL_MONO, 2);

	uint16_t width = (decimal + fraction + (sign ? 1 : 0) + (fraction > 0 ? 1 : 0)) * 15 - 1;

	TWO_TONE_BORDER(x-2, y-2, width+4, 26+4, 2, SHADOW_DARK, SHADOW_LIGHT);
	GFX_CONTAINER(x, y, width, 26, BOX_BACKGROUND);

	uint16_t offset = 0;
	char str[2] = "0";

	int32_t divider = 1;
	for(int i = 0; i < fraction; ++i)
	{
		value *= 10;
		divider *= 10;
	}
	for(int i = 0; i < decimal - 1; ++i)
	{
		divider *= 10;
	}

	int32_t int_value = (int32_t)ABS(value);

	if(sign)
	{
		GFX_TEXT(GFX_REL(2, 4), GFX_BLACK, value >= 0 ? "+" : "-");
		offset = 15;
	}

	for(int i = 0; i < decimal; ++i)
	{
		uint8_t digit = (int32_t)(int_value / divider) % 10;
		str[0] = '0' + digit;
		divider /= 10;

		GFX_TEXT(GFX_REL(2+offset, 4), GFX_BLACK, str);
		if(offset > 0)
			GFX_RECT(GFX_REL(offset-1, 0), 1, 26, SEPARATOR);
		offset += 15;
	}
	if(fraction > 0) 
	{
		GFX_TEXT(GFX_REL(2+offset, 4), GFX_BLACK, ".");
		GFX_RECT(GFX_REL(offset-1, 0), 1, 26, SEPARATOR);
		offset += 15;
		for(int i = 0; i < fraction; ++i)
		{
			uint8_t digit = (int32_t)(int_value / divider) % 10;
			str[0] = '0' + digit;
			divider /= 10;

			GFX_TEXT(GFX_REL(2+offset, 4), GFX_BLACK, str);
			GFX_RECT(GFX_REL(offset-1, 0), 1, 26, SEPARATOR);
			offset += 15;
		}
	}
}

#define _HIGHLIGHT(x, y, w, h, highlight) \
	if(highlight) { \
		GFX_CONTAINER(x, y, w, h, GFX_COLOR(SELECT_HIGHLIGHT)); \
	} else { \
		GFX_SET_ORIGIN(x, y); \
	}

#define HIGHLIGHT(...) _HIGHLIGHT(__VA_ARGS__)

GFX_DECL_ELEMENT(list_selector, uint16_t *out_width, uint16_t *out_height, const char* header, const char **options, uint8_t option_count, uint8_t selected_index, int highlight_index)
{
	GFX_SET_FONT(FONT_PIXEL, 1);

	uint16_t width = 0;
	uint16_t height = option_count * 12 + 18;

	gfx_text_size(&width, 0, FONT_PIXEL, 1, header);
	width += 8;

	for(uint8_t i = 0; i < option_count; ++i)
	{
		uint16_t line_width;
		gfx_text_size(&line_width, 0, FONT_PIXEL, 1, header);
		line_width += 8;
		if(line_width > width)
			width = line_width;
	}

	TWO_TONE_BORDER(GFX_REL(-2, -2), width+4, height+4, 2, SHADOW_DARK, SHADOW_LIGHT);
	GFX_CONTAINER(GFX_REL(0, 0), width, height, BOX_BACKGROUND);

	GFX_TEXT(GFX_REL(4, 3), GFX_BLACK, header);
	GFX_RECT(GFX_REL(0, 14), width, 1, SEPARATOR);

	for(uint8_t i = 0; i < option_count; ++i)
	{
		// TODO: This macro is really bad
		GFX_PUSH_STATE();

		HIGHLIGHT(GFX_REL(0, 18 + i * 12 - 1), width, 12, i == highlight_index);

		// GFX_BITMAP(GFX_REL(4, 18 + i * 12), 9, 9, GFX_BACKGROUND, GFX_BLACK,
		// 				 i == selected_index ? CheckboxChecked_9x9 : CheckboxEmpty_9x9);
		// GFX_TEXT(GFX_REL(16, 18 + i * 12), GFX_BLACK, options[i]);
		GFX_BITMAP(GFX_REL(4, 1), 9, 9, GFX_BACKGROUND, GFX_BLACK,
						 i == selected_index ? CheckboxChecked_9x9 : CheckboxEmpty_9x9);
		GFX_TEXT(GFX_REL(16, 1), GFX_BLACK, options[i]);

		GFX_POP_STATE();
	}

	*out_width = width;
	*out_height = height;
}

BUTTON_DEF() {
	GFX_SET_FONT(FONT_PIXEL, 1);

	if(highlight)
		BORDER(GFX_REL(-1, -1), width+2, height+2, 1, GFX_BLACK);
	TWO_TONE_BORDER(GFX_REL(0, 0), width, height, 2, SHADOW_LIGHT, SHADOW_DARK);
	GFX_RECT(GFX_REL(2, 2), width - 4, height - 4, BASE_BACKGROUND);
	GFX_TEXT(GFX_REL(GFX_CENTER_TEXT_OFFSET(width - 4, text) + 2, (height - 15) / 2 + 2), GFX_BLACK, text);
}

GFX_DECL_SCREEN(screen)
{
	GFX_SCREEN_HEADER();
	GFX_SET_FONT(FONT_PIXEL, 2);

	GFX_CLEAR(BASE_BACKGROUND);

	GFX_CONTAINER(0, 0, GFX_DISPLAY_WIDTH, 24, TOP_BAR);

	char str[64];

	rom_strcpy(str, __romstr__(STR_MOVEMENT));
	GFX_TEXT(GFX_CENTER_TEXT_OFFSET(GFX_DISPLAY_WIDTH, str), 2, GFX_WHITE, str);

	uint16_t w, h;
	uint16_t x = 10, y = 24 + 10;

	// TODO: Grab the strings from language file
	GFX_SET_ORIGIN(x, y);
	const char* opts_move[] = { "Rapid", "Linear" };
	GFX_DRAW_ELEMENT(list_selector, &w, &h, "Movement type", opts_move, 2, 0, g_system_menu.current_index);

	x += w + 14;
	GFX_SET_ORIGIN(x, y);
	const char* opts_coord[] = { "Workspace", "Relative", "Machine" };
	GFX_DRAW_ELEMENT(list_selector, &w, &h, "Coordinate type", opts_coord, 3, 0, g_system_menu.current_index - 2);
	
	GFX_SET_BACKGROUND(BASE_BACKGROUND);

	GFX_SET_ORIGIN(200, 24+10+2)
	GFX_TEXT(GFX_REL(0, 0), GFX_BLACK, "Feed");
	GFX_DRAW_ELEMENT(coordinate_box, GFX_REL(54, -2), false, 4, 0, 0, -1);

	GFX_SET_ORIGIN(0, 100);
	TWO_TONE_BORDER(GFX_REL(0, 0), GFX_DISPLAY_WIDTH, GFX_DISPLAY_HEIGHT - 100, 2, SHADOW_LIGHT, SHADOW_DARK);

	GFX_TEXT(GFX_REL(10, 8), GFX_BLACK, "Coordinates");

	HIGHLIGHT(GFX_REL(13, 8+25), COORD_BOX_SIZE + 55, 34, g_system_menu.current_index == 5);
	GFX_BITMAP(GFX_REL(2, 8), 9, 9, GFX_BACKGROUND, GFX_BLACK, CheckboxEmpty_9x9, 2);
	GFX_TEXT(GFX_REL(28, 8), GFX_BLACK, FONT_PIXEL_MONO, 2, "X");
	GFX_DRAW_ELEMENT(coordinate_box, GFX_REL(52, 4), true, COORD_DECIMAL_DIGITS, COORD_FRACTIONAL_DIGITS, -123.54, -1);
	GFX_BACKGROUND = BASE_BACKGROUND;
	
	HIGHLIGHT(GFX_REL(0, 34), COORD_BOX_SIZE + 55, 34, g_system_menu.current_index == 6);
	GFX_BITMAP(GFX_REL(2, 8), 9, 9, GFX_BACKGROUND, GFX_BLACK, CheckboxEmpty_9x9, 2);
	GFX_TEXT(GFX_REL(28, 8), GFX_BLACK, FONT_PIXEL_MONO, 2, "Y");
	GFX_DRAW_ELEMENT(coordinate_box, GFX_REL(52, 4), true, COORD_DECIMAL_DIGITS, COORD_FRACTIONAL_DIGITS, -123.54, -1);
	GFX_BACKGROUND = BASE_BACKGROUND;

	HIGHLIGHT(GFX_REL(0, 34), COORD_BOX_SIZE + 55, 34, g_system_menu.current_index == 7);
	GFX_BITMAP(GFX_REL(2, 8), 9, 9, GFX_BACKGROUND, GFX_BLACK, CheckboxEmpty_9x9, 2);
	GFX_TEXT(GFX_REL(28, 8), GFX_BLACK, FONT_PIXEL_MONO, 2, "Z");
	GFX_DRAW_ELEMENT(coordinate_box, GFX_REL(52, 4), true, COORD_DECIMAL_DIGITS, COORD_FRACTIONAL_DIGITS, -123.54, -1);
	GFX_BACKGROUND = BASE_BACKGROUND;

	GFX_SET_ORIGIN(10, GFX_DISPLAY_HEIGHT - 35);
	GFX_DRAW_ELEMENT(win9x_button, 50, 25, "Back", g_system_menu.current_index == 8);
	
	GFX_SET_ORIGIN(65, GFX_DISPLAY_HEIGHT - 35);
	GFX_DRAW_ELEMENT(win9x_button, 50, 25, "Move", g_system_menu.current_index == 9);
}

