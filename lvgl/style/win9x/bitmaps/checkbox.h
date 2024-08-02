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

#ifdef BITMAP_IMPL
const uint8_t CheckboxEmpty_9x9[] = { 28, 49, 144, 80, 24, 12, 5, 4, 198, 28, 0 };
const uint8_t CheckboxChecked_9x9[] = { 28, 49, 144, 83, 153, 204, 229, 4, 198, 28, 0 };
#else
extern const uint8_t CheckboxEmpty_9x9[];
extern const uint8_t CheckboxChecked_9x9[];
#endif

