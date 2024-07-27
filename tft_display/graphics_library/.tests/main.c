/*
	Name: main.c
	Description: Graphics library for µCNC. Unit tests.

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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "test_font.h"

#define RUN_TEST(name) \
extern bool test_##name();\
++total; \
if(test_##name()) { \
	++successful; \
	printf("Test '%s' succesful\n", #name); \
} else { \
	printf("Test '%s' failed\n", #name); \
}

int main() {
	int total = 0, successful = 0;
	RUN_TEST(simple_clear);
	RUN_TEST(simple_rect);
	RUN_TEST(simple_frame);
	RUN_TEST(simple_text);
	RUN_TEST(simple_text_2);
	RUN_TEST(simple_text_3);
	RUN_TEST(simple_text_4);
	RUN_TEST(simple_bitmap);
	RUN_TEST(simple_bitmap_2);
	RUN_TEST(palette_bitmap);
	RUN_TEST(palette_bitmap_2);

	printf("Run %d tests, %d succesful, %d failed\n", total, successful, total - successful);
	return 0;
}
