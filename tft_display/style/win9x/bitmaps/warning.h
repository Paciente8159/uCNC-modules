/*
	Name: warning.h
	Description: Bitmapped warning sign

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
#ifndef WIN9X_BITMAP_WARNING_H
#define WIN9X_BITMAP_WARNING_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef BITMAP_IMPL
static const uint8_t WarningSign_14x15[] = { 
	3, 0, 3, 0, 7, 128, 7, 128, 12, 192, 12, 192, 28, 224, 28, 224, 60, 240, 63, 240, 127, 248, 124, 248, 252, 252, 255, 252, 255, 252
};

const lv_image_dsc_t Img_Warning = {
	.header = {
		.cf = LV_COLOR_FORMAT_A1,
		.w = 14,
		.h = 15,
	},
	.data = WarningSign_14x15,
	.data_size = sizeof(WarningSign_14x15),
};
#else
extern const lv_image_dsc_t Img_Warning;
#endif

#ifdef __cplusplus
}
#endif

#endif

