#ifndef TOUCH_SCREEN_TOUCH_H
#define TOUCH_SCREEN_TOUCH_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include "src/cnc.h"
#include "src/modules/softspi.h"

/**
 * Parameters for calibration.
 */
#ifndef TOUCH_SCREEN_ADC_MIN
#define TOUCH_SCREEN_ADC_MIN 200
#endif
#ifndef TOUCH_SCREEN_ADC_MAX
#define TOUCH_SCREEN_ADC_MAX 3800
#endif
/**
 * Read parameters
 */
#define TOUCH_SCREEN_DFR_MODE 3
#define TOUCH_SCREEN_SER_MODE 4

#define TOUCH_SCREEN_READ_X 0xD0
#define TOUCH_SCREEN_READ_Y 0x90

#define TOUCH_INVERT_Y 0x80
#define TOUCH_INVERT_X 0x40
#define TOUCH_SWAP_XY 0x20

	void touch_screen_init(softspi_port_t *spiport, uint16_t width, uint16_t height, uint8_t flags, uint8_t cs_pin, uint8_t penirq_pin);
	bool touch_screen_is_touching();
	void touch_screen_get_position(uint16_t *px, uint16_t *py);

#ifdef __cplusplus
}
#endif
#endif