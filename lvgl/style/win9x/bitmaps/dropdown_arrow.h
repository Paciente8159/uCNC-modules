/*
	Name: dropdown_arrow.h
	Description: Bitmapped dropdown list arrow icon

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
#ifndef WIN9X_BITMAP_DROPDOWN_ARROW_H
#define WIN9X_BITMAP_DROPDOWN_ARROW_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef BITMAP_IMPL
static const uint8_t Arrow_8x4[] = { 255, 126, 60, 24 };
const lv_image_dsc_t Img_DropdownArrow = {
	.header = {
		.magic = LV_IMAGE_HEADER_MAGIC,
		.cf = LV_COLOR_FORMAT_A1,
		.w = 8,
		.h = 4,
		.stride = 1,
	},
	.data = Arrow_8x4,
	.data_size = sizeof(Arrow_8x4),
};
#else
extern const lv_image_dsc_t Img_DropdownArrow;
#endif

#ifdef __cplusplus
}
#endif

#endif

