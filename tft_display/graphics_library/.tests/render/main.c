#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "../../graphics_library.h"
#include <stdio.h>

GFX_INCLUDE_SCREEN(screen);
extern gfx_pixel_t test_driver_buffer[];

int main() {
	printf("Rendering screen to screen.png\n");

	GFX_RENDER_SCREEN(screen);
	printf("Screen rendered, writing to file...\n");

	stbi_write_png("screen.png", 480, 320, 4, test_driver_buffer, 480 * 4);

	printf("Performing second render\n");

	memset(test_driver_buffer, 0, 480 * 320 * 4);
	GFX_RENDER_SCREEN(screen);

	printf("Screen rendered, writing to file...\n");

	stbi_write_png("screen_dyn.png", 480, 320, 4, test_driver_buffer, 480 * 4);

	return 0;
}
