/*
	Name: style.h
	Description: Base of the style subsystem

	Copyright: Copyright (c) Patryk Mierzyński
	Author: Patryk Mierzyński
	Date: 29/07/2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the GNU General Public License for more details.
*/

#ifndef LVGL_STYLE_H
#define LVGL_STYLE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"

extern void style_init(lv_display_t *display, lv_indev_t *indev);

extern void style_startup();
extern void style_idle();
extern void style_popup(const char* text);
extern void style_alarm();

#ifdef __cplusplus
}
#endif

#endif

