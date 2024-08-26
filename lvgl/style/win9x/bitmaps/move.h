/*
	Name: move.h
	Description: Bitmapped icon for movement screen

	Copyright: Copyright (c) Patryk Mierzyński
	Author: Patryk Mierzyński
	Date: 08/08/2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the GNU General Public License for more details.
*/
#ifndef WIN9X_BITMAP_MOVE_H
#define WIN9X_BITMAP_MOVE_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef BITMAP_IMPL
static const uint8_t Move_42x42[] = { 0, 0, 12, 0, 0, 0, 0, 0, 30, 0, 0, 0, 0, 0, 63, 0, 0, 0, 0, 0, 127, 128, 0, 0, 0, 0, 255, 192, 0, 0, 0, 0, 255, 192, 0, 0, 0, 0, 30, 0, 0, 0, 0, 0, 30, 0, 0, 0, 0, 0, 30, 0, 0, 0, 0, 0, 30, 0, 0, 0, 0, 0, 30, 0, 0, 0, 0, 0, 30, 0, 0, 0, 0, 0, 30, 0, 0, 0, 0, 0, 30, 0, 0, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 0, 0, 0, 12, 0, 28, 0, 0, 0, 14, 0, 60, 0, 63, 0, 15, 0, 127, 254, 63, 31, 255, 128, 255, 254, 63, 31, 255, 192, 255, 254, 63, 31, 255, 192, 127, 254, 63, 31, 255, 128, 60, 0, 63, 0, 15, 0, 28, 0, 0, 0, 14, 0, 12, 0, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 30, 0, 0, 0, 0, 0, 30, 0, 0, 0, 0, 0, 30, 0, 0, 0, 0, 0, 30, 0, 0, 0, 0, 0, 30, 0, 0, 0, 0, 0, 30, 0, 0, 0, 0, 0, 30, 0, 0, 0, 0, 0, 30, 0, 0, 0, 0, 0, 30, 0, 0, 0, 0, 0, 255, 192, 0, 0, 0, 0, 255, 192, 0, 0, 0, 0, 127, 128, 0, 0, 0, 0, 63, 0, 0, 0, 0, 0, 30, 0, 0, 0, 0, 0, 12, 0, 0, 0 };
const lv_image_dsc_t Img_Move = {
	.header = {
		.magic = LV_IMAGE_HEADER_MAGIC,
		.cf = LV_COLOR_FORMAT_A1,
		.w = 42,
		.h = 42,
		.stride = 6,
	},
	.data = Move_42x42,
	.data_size = sizeof(Move_42x42),
};
#else
extern const lv_image_dsc_t Img_Move;
#endif

#ifdef __cplusplus
}
#endif

#endif

