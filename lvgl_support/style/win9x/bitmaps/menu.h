/*
	Name: menu.h
	Description: Bitmapped icon for main menu button

	Copyright: Copyright (c) Patryk Mierzyński
	Author: Patryk Mierzyński
	Date: 09/08/2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the GNU General Public License for more details.
*/
#ifndef WIN9X_BITMAP_MENU_H
#define WIN9X_BITMAP_MENU_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef BITMAP_IMPL
static const uint8_t Menu_36x24[] = { 255, 255, 255, 255, 240, 255, 255, 255, 255, 240, 255, 255, 255, 255, 240, 255, 255, 255, 255, 240, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 240, 255, 255, 255, 255, 240, 255, 255, 255, 255, 240, 255, 255, 255, 255, 240, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 240, 255, 255, 255, 255, 240, 255, 255, 255, 255, 240, 255, 255, 255, 255, 240 };
const lv_image_dsc_t Img_Menu = {
	.header = {
		.magic = LV_IMAGE_HEADER_MAGIC,
		.cf = LV_COLOR_FORMAT_A1,
		.w = 36,
		.h = 24,
		.stride = 5,
	},
	.data = Menu_36x24,
	.data_size = sizeof(Menu_36x24),
};
#else
extern const lv_image_dsc_t Img_Menu;
#endif

#ifdef __cplusplus
}
#endif

#endif

