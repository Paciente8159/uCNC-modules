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
