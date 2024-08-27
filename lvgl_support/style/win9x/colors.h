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

const static lv_color_t bg_base = LV_COLOR_MAKE(229, 229, 229);
const static lv_color_t bg_box = LV_COLOR_MAKE(242, 242, 242);
const static lv_color_t bg_other = LV_COLOR_MAKE(220, 220, 220);

const static lv_color_t col_black = LV_COLOR_MAKE(0, 0, 0);
const static lv_color_t col_white = LV_COLOR_MAKE(255, 255, 255);
const static lv_color_t col_red = LV_COLOR_MAKE(255, 0, 0);

const static lv_color_t shadow_dark = LV_COLOR_MAKE(153, 153, 153);
const static lv_color_t shadow_light = LV_COLOR_MAKE(255, 255, 255);
const static lv_color_t shadow_darker = LV_COLOR_MAKE(102, 102, 102);

const static lv_color_t bg_title_bar = LV_COLOR_MAKE(0, 0, 178);
const static lv_color_t separator = LV_COLOR_MAKE(178, 178, 178);

const static lv_color_t charcoal = LV_COLOR_MAKE(45, 45, 45);

const static lv_color_t select_highlight = LV_COLOR_MAKE(204, 204, 204);

const static lv_color_t button_click = LV_COLOR_MAKE(204, 204, 204);

const static lv_color_t status_yellow = LV_COLOR_MAKE(255, 255, 34);
const static lv_color_t status_green = LV_COLOR_MAKE(136, 255, 136);
const static lv_color_t status_red = LV_COLOR_MAKE(255, 17, 17);
const static lv_color_t status_blue = LV_COLOR_MAKE(204, 204, 255);

#ifdef __cplusplus
}
#endif

#endif

