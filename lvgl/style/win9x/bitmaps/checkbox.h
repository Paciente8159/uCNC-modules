/*
	Name: checkbox.h
	Description: Bitmapped checkbox

	Copyright: Copyright (c) Patryk Mierzyński
	Author: Patryk Mierzyński
	Date: 01/08/2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the GNU General Public License for more details.
*/
#ifndef WIN9X_BITMAP_CHECKBOX_H
#define WIN9X_BITMAP_CHECKBOX_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef BITMAP_IMPL
static const uint8_t CheckboxEmpty_9x9[] = { 28, 0, 99, 0, 65, 0, 128, 128, 128, 128, 128, 128, 65, 0, 99, 0, 28, 0 };
static const uint8_t CheckboxChecked_9x9[] = { 28, 0, 99, 0, 65, 0, 156, 128, 156, 128, 156, 128, 65, 0, 99, 0, 28, 0 };

const lv_img_dsc_t Img_CheckboxEmpty = {
	.header = {
		.cf = LV_COLOR_FORMAT_A1,
		.w = 9,
		.h = 9,
	},
	.data = CheckboxEmpty_9x9,
	.data_size = sizeof(CheckboxEmpty_9x9),
};

const lv_img_dsc_t Img_CheckboxChecked = {
	.header = {
		.cf = LV_COLOR_FORMAT_A1,
		.w = 9,
		.h = 9,
	},
	.data = CheckboxChecked_9x9,
	.data_size = sizeof(CheckboxChecked_9x9),
};
#else
extern const lv_img_dsc_t Img_CheckboxEmpty;
extern const lv_img_dsc_t Img_CheckboxChecked;
#endif

#endif

