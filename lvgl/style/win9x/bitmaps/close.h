/*
	Name: close.h
	Description: Bitmapped close button

	Copyright: Copyright (c) Patryk Mierzyński
	Author: Patryk Mierzyński
	Date:11/08/2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the GNU General Public License for more details.
*/
#ifndef WIN9X_BITMAP_CLOSE_H
#define WIN9X_BITMAP_CLOSE_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef BITMAP_IMPL
static const uint8_t Close_10x10[] = { 192, 192, 225, 192, 115, 128, 63, 0, 30, 0, 30, 0, 63, 0, 115, 128, 225, 192, 192, 192 };
const lv_image_dsc_t Img_Close = {
	.header = {
		.magic = LV_IMAGE_HEADER_MAGIC,
		.cf = LV_COLOR_FORMAT_A1,
		.w = 10,
		.h = 10,
		.stride = 2,
	},
	.data = Close_10x10,
	.data_size = sizeof(Close_10x10),
};
#else
extern const lv_image_dsc_t Img_Close;
#endif

#ifdef __cplusplus
}
#endif

#endif

