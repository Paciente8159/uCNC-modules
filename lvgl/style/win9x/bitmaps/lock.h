/*
	Name: lock.h
	Description: Bitmapped lock icons

	Copyright: Copyright (c) Patryk Mierzyński
	Author: Patryk Mierzyński
	Date: 03/08/2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the GNU General Public License for more details.
*/

#ifndef WIN9X_BITMAP_LOCK_H
#define WIN9X_BITMAP_LOCK_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef BITMAP_IMPL
static const uint8_t Lock_16x16[] = { 0, 0, 0, 0, 3, 192, 3, 192, 12, 48, 12, 48, 12, 48, 12, 48, 63, 252, 63, 252, 63, 60, 63, 60, 60, 252, 60, 252, 63, 252, 63, 252 };
static const uint8_t Unlock_16x16[] = { 0, 0, 0, 0, 60, 0, 60, 0, 195, 0, 195, 0, 195, 0, 195, 0, 15, 255, 15, 255, 15, 207, 15, 207, 15, 63, 15, 63, 15, 255, 15, 255 };

const lv_image_dsc_t Img_Locked = {
	.header = {
		.cf = LV_COLOR_FORMAT_A1,
		.w = 16,
		.h = 16,
	},
	.data = Lock_16x16,
	.data_size = sizeof(Lock_16x16),
};

const lv_image_dsc_t Img_Unlocked = {
	.header = {
		.cf = LV_COLOR_FORMAT_A1,
		.w = 16,
		.h = 16,
	},
	.data = Unlock_16x16,
	.data_size = sizeof(Unlock_16x16),
};

#else
extern const lv_image_dsc_t Img_Locked;
extern const lv_image_dsc_t Img_Unlocked;
#endif

#ifdef __cplusplus
}
#endif

#endif

