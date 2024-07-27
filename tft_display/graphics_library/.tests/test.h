/*
	Name: test.h
	Description: Helper file to make testing easier

	Copyright: Copyright (c) Patryk Mierzyński
	Author: Patryk Mierzyński
	Date: 27/07/2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef TEST_H
#define TEST_H

#include <stdbool.h>
#include <stdio.h>

#define DECL_TEST(name) bool test_##name()

#define TEST_SCREEN(x, y, w, h, pixels) { \
	uint16_t invalid = check_screen(x, y, w, h, pixels); \
	if(invalid != 0xFFFF) { \
		uint8_t i_x = invalid & 0xFF; \
		uint8_t i_y = (invalid >> 8) & 0xFF; \
		printf("Unexpected pixel (value %d) found at (%d, %d), expected (%d). Aborting test.\n", get_pixel((x) + i_x, (y) + i_y), i_x, i_y, pixels[i_x + i_y * (w)]); \
		return false; \
	} \
}

#define DUMP_SCREEN(x, y, w, h) { \
	printf("Screen area from (%d, %d) to (%d, %d)\n", x, y, (x) + (w), (y) + (h)); \
	for(int j = 0; j < (h); ++j) { \
		for(int i = 0; i < (w); ++i) { \
			printf("%d ", get_pixel((x) + i, (y) + j)); \
		} \
		printf("\n"); \
	} \
}

#endif
