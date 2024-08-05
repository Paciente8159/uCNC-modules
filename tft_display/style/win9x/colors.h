/*
	Name: colors.h
	Description: Color definitions for the GUI

	Copyright: Copyright (c) Patryk Mierzyński
	Author: Patryk Mierzyński
	Date: 28/07/2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the GNU General Public License for more details.
*/

#ifndef WIN9X_COLORS_H
#define WIN9X_COLORS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"
#include "misc/lv_color.h"

const static lv_color_t bg_base = { 229, 229, 229 };
const static lv_color_t bg_box = { 242, 242, 242 };

const static lv_color_t col_black = { 0, 0, 0 };
const static lv_color_t col_white = { 255, 255, 255 };
const static lv_color_t col_red = { 0, 0, 255 };

const static lv_color_t shadow_dark = { 153, 153, 153 };
const static lv_color_t shadow_light = { 255, 255, 255 };
const static lv_color_t shadow_darker = { 102, 102, 102 };

const static lv_color_t bg_title_bar = { 178, 0, 0 };
const static lv_color_t separator = { 178, 178, 178 };

const static lv_color_t charcoal = { 45, 45, 45 };

const static lv_color_t select_highlight = { 204, 204, 204 };

const static lv_color_t status_yellow = { 34, 255, 255 };
const static lv_color_t status_green = { 136, 255, 136 };
const static lv_color_t status_red = { 17, 17, 255 };
const static lv_color_t status_blue = { 255, 204, 204 };

#ifdef __cplusplus
}
#endif

#endif

