/*
	Name: startup.c
	Description: Startup screen

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
#include "../../config.h"

#ifdef GUI_STYLE_WIN9X

#include "lvgl.h"
#include "../colors.h"
#include "../elements.h"
#include "../bitmaps/logo.h"

static lv_obj_t *screen;

void style_create_startup_screen()
{
	screen = lv_obj_create(NULL);

	lv_obj_set_style_bg_color(screen, col_black, LV_PART_MAIN);

	lv_obj_t *logo = lv_img_create(screen);
	lv_image_set_src(logo, &Img_Logo);
	lv_obj_set_pos(logo, 180, 100);
	lv_obj_set_size(logo, 120, 120);
}

STYLE_LOAD_SCREEN(startup);

#endif

