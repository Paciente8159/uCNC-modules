/*
	Name: zero.h
	Description: Bitmapped icon for zero axis position button

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
#ifndef WIN9X_BITMAP_ZERO_H
#define WIN9X_BITMAP_ZERO_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef BITMAP_IMPL
static const uint8_t ZeroX_36x27[] = { 224, 0, 56, 0, 0, 224, 0, 56, 0, 0, 224, 0, 56, 0, 0, 224, 0, 56, 0, 0, 240, 0, 120, 0, 0, 248, 0, 248, 0, 0, 124, 1, 240, 0, 0, 62, 3, 224, 0, 0, 31, 7, 192, 0, 0, 15, 143, 128, 0, 0, 7, 223, 0, 0, 0, 3, 254, 0, 0, 0, 1, 252, 0, 31, 128, 0, 248, 0, 63, 192, 1, 252, 0, 127, 224, 3, 254, 0, 249, 240, 7, 223, 0, 240, 240, 15, 143, 128, 224, 112, 31, 7, 192, 224, 112, 62, 3, 224, 224, 112, 124, 1, 240, 224, 112, 248, 0, 248, 224, 112, 240, 0, 120, 240, 240, 224, 0, 56, 249, 240, 224, 0, 56, 127, 224, 224, 0, 56, 63, 192, 224, 0, 56, 31, 128 };
static const uint8_t ZeroY_36x27[] = { 224, 0, 56, 0, 0, 224, 0, 56, 0, 0, 224, 0, 56, 0, 0, 224, 0, 56, 0, 0, 240, 0, 120, 0, 0, 248, 0, 248, 0, 0, 124, 1, 240, 0, 0, 62, 3, 224, 0, 0, 31, 7, 192, 0, 0, 15, 143, 128, 0, 0, 7, 223, 0, 0, 0, 3, 254, 0, 0, 0, 1, 252, 0, 31, 128, 0, 248, 0, 63, 192, 0, 112, 0, 127, 224, 0, 112, 0, 249, 240, 0, 112, 0, 240, 240, 0, 112, 0, 224, 112, 0, 112, 0, 224, 112, 0, 112, 0, 224, 112, 0, 112, 0, 224, 112, 0, 112, 0, 224, 112, 0, 112, 0, 240, 240, 0, 112, 0, 249, 240, 0, 112, 0, 127, 224, 0, 112, 0, 63, 192, 0, 112, 0, 31, 128 };
static const uint8_t ZeroZ_36x27[] = { 255, 255, 248, 0, 0, 255, 255, 248, 0, 0, 255, 255, 248, 0, 0, 0, 0, 56, 0, 0, 0, 0, 120, 0, 0, 0, 0, 248, 0, 0, 0, 1, 240, 0, 0, 0, 3, 224, 0, 0, 0, 7, 192, 0, 0, 0, 15, 128, 0, 0, 0, 31, 0, 0, 0, 0, 62, 0, 0, 0, 0, 124, 0, 31, 128, 0, 248, 0, 63, 192, 1, 240, 0, 127, 224, 3, 224, 0, 249, 240, 7, 192, 0, 240, 240, 15, 128, 0, 224, 112, 31, 0, 0, 224, 112, 62, 0, 0, 224, 112, 124, 0, 0, 224, 112, 248, 0, 0, 224, 112, 240, 0, 0, 240, 240, 224, 0, 0, 249, 240, 255, 255, 248, 127, 224, 255, 255, 248, 63, 192, 255, 255, 248, 31, 128 };

const lv_image_dsc_t Img_ZeroX = {
	.header = {
		.magic = LV_IMAGE_HEADER_MAGIC,
		.cf = LV_COLOR_FORMAT_A1,
		.w = 36,
		.h = 27,
		.stride = 5,
	},
	.data = ZeroX_36x27,
	.data_size = sizeof(ZeroX_36x27),
};
const lv_image_dsc_t Img_ZeroY = {
	.header = {
		.magic = LV_IMAGE_HEADER_MAGIC,
		.cf = LV_COLOR_FORMAT_A1,
		.w = 36,
		.h = 27,
		.stride = 5,
	},
	.data = ZeroY_36x27,
	.data_size = sizeof(ZeroY_36x27),
};
const lv_image_dsc_t Img_ZeroZ = {
	.header = {
		.magic = LV_IMAGE_HEADER_MAGIC,
		.cf = LV_COLOR_FORMAT_A1,
		.w = 36,
		.h = 27,
		.stride = 5,
	},
	.data = ZeroZ_36x27,
	.data_size = sizeof(ZeroZ_36x27),
};
#else
extern const lv_image_dsc_t Img_ZeroX;
extern const lv_image_dsc_t Img_ZeroY;
extern const lv_image_dsc_t Img_ZeroZ;
#endif

#ifdef __cplusplus
}
#endif

#endif

