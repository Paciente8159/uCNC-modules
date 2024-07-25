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

	printf("Run %d tests, %d succesful, %d failed\n", total, successful, total - successful);
	return 0;
}
