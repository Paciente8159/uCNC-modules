/*
	Name: number_input.c
	Description: Creates the number input field.

	Copyright: Copyright (c) Patryk Mierzyński
	Author: Patryk Mierzyński
	Date: 21/08/2024

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
#include "../fonts/pixel_mono.h"

#include "src/modules/system_menu.h"

static FORCEINLINE int int_to_string(uint32_t value, char *str)
{
	int length = 0;
	do
	{
		char digit = (value % 10) + '0';
		*(str + length++) = digit;
		value /= 10;
	} while(value > 0);
	// Reverse the digits
	for(int i = 0; i < length / 2; ++i)
	{
		char buf = str[i];
		str[i] = str[length - i - 1];
		str[length - i - 1] = buf;
	}
	str[length] = 0;
	return length;
}

static void value_to_string(const void *valuePtr, uint8_t valueType, char *str)
{
	uint32_t valueI;

#define HANDLE_SIGN(type) \
	if(*(const type*)valuePtr < 0) \
	{ \
		valueI = -(*(const type*)valuePtr); \
		*str++ = '-'; \
	}

	switch(valueType)
	{
		case VAR_TYPE_INT8:
			HANDLE_SIGN(int8_t);
			// Fallthrough
		case VAR_TYPE_UINT8:
			valueI = *(const uint8_t*)valuePtr;
			goto case_int;

		case VAR_TYPE_INT16:
			HANDLE_SIGN(int16_t);
			// Fallthrough
		case VAR_TYPE_UINT16:
			valueI = *(const uint16_t*)valuePtr;
			goto case_int;

		case VAR_TYPE_INT32:
			HANDLE_SIGN(int32_t);
			// Fallthrough
		case VAR_TYPE_UINT32:
			valueI = *(const uint32_t*)valuePtr;
		case_int:
			int_to_string(valueI, str);
			break;
		case VAR_TYPE_BOOLEAN:
			str[0] = *(const bool*)valuePtr == 0 ? '0' : '1';
			str[1] = 0;
			break;
		case VAR_TYPE_FLOAT:
			{
				float valueF = *(const float*)valuePtr;
				// Sign
				if(valueF < 0)
				{
					*str++ = '-';
					valueF = -valueF;
				}
				// Decimal part
				valueI = (uint32_t)valueF;
				str += int_to_string(valueI, str);
				*str++ = '.';
				// Fractional part (3 digits)
				valueI = (valueF - valueI) * 1000;
				for(int i = 0; i < 3; ++i)
				{
					char digit = (valueI % 10) + '0';
					str[2 - i] = digit;
					valueI /= 10;
				}
			}
			break;
	}

#undef HANDLE_SIGN
}

static void key_cb(lv_event_t *event)
{
	lv_obj_t *spinbox = lv_event_get_target_obj(event);
	char key = lv_event_get_key(event);

	if(key >= '0' && key <= '9')
	{
		int32_t step = lv_spinbox_get_step(spinbox);
		int32_t value = lv_spinbox_get_value(spinbox);

		int32_t masked = (value % (step * 10)) - (value % step);
		value = value - masked + (key - '0') * step;
		lv_spinbox_set_value(spinbox, value);

		lv_spinbox_step_prev(spinbox);
	}
}

lv_obj_t *win9x_number_input(lv_obj_t *parent)
{
	lv_obj_t *value = lv_spinbox_create(parent);
	lv_obj_add_style(value, &g_styles.container, LV_PART_MAIN);
	lv_obj_set_style_text_font(value, &font_pixel_mono_14pt, LV_PART_MAIN);
	WIN9X_BORDER_PART_TWO(value, shadow_light);
	lv_obj_set_style_pad_all(value, 4, LV_PART_MAIN);
	lv_obj_set_style_text_align(value, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);

	lv_obj_set_style_bg_opa(value, LV_OPA_TRANSP, LV_PART_CURSOR);
	lv_obj_set_style_border_side(value, LV_BORDER_SIDE_BOTTOM, LV_PART_CURSOR);
	lv_obj_set_style_border_color(value, col_black, LV_PART_CURSOR);
	lv_obj_set_style_border_width(value, 2, LV_PART_CURSOR | LV_STATE_FOCUSED);
	lv_obj_set_style_border_width(value, 0, LV_PART_CURSOR);

	lv_obj_set_size(value, LV_PCT(100), LV_SIZE_CONTENT);
	lv_obj_set_pos(value, 0, 20);

	lv_obj_add_event_cb(value, key_cb, LV_EVENT_KEY, NULL);
	return value;
}

#endif

