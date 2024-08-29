
#include "../../cnc.h"
#include "touch_screen.h"
#include <stdint.h>
#include <stdbool.h>
#include "../softspi.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * SPI bus
 */
// if defined this will override the bus configuration
#ifndef TOUCH_SCREEN_SPI_FREQ
// #define TOUCH_SCREEN_SPI_FREQ 1000000UL
#endif

	// Global variables
	static uint16_t touch_screen_width;
	static uint16_t touch_screen_height;
	static softspi_port_t *touch_spi;
	static uint8_t touch_screen_flags;
	static uint8_t touch_cs_pin;
	static uint8_t touch_penirq_pin;

	static FORCEINLINE void touch_screen_corr(uint16_t *x, uint16_t *y)
	{
		if (touch_screen_flags & TOUCH_SWAP_XY)
		{
			int16_t swap_tmp;
			swap_tmp = *x;
			*x = *y;
			*y = swap_tmp;
		}

		if ((*x) > TOUCH_SCREEN_ADC_MIN)
			(*x) -= TOUCH_SCREEN_ADC_MIN;
		else
			(*x) = 0;

		if ((*y) > TOUCH_SCREEN_ADC_MIN)
			(*y) -= TOUCH_SCREEN_ADC_MIN;
		else
			(*y) = 0;

		(*x) = (uint32_t)((uint32_t)(*x) * touch_screen_width) /
					 (TOUCH_SCREEN_ADC_MAX - TOUCH_SCREEN_ADC_MIN);

		(*y) = (uint32_t)((uint32_t)(*y) * touch_screen_height) /
					 (TOUCH_SCREEN_ADC_MAX - TOUCH_SCREEN_ADC_MIN);

		if (touch_screen_flags & TOUCH_INVERT_X)
		{
			(*x) = touch_screen_width - (*x);
		}

		if (touch_screen_flags & TOUCH_INVERT_Y)
		{
			(*y) = touch_screen_height - (*y);
		}
	}

	void touch_screen_init(softspi_port_t *spiport, uint16_t width, uint16_t height, uint8_t flags, uint8_t cs_pin, uint8_t penirq_pin)
	{
		spi_config_t conf = spiport->spiconfig;
		touch_spi = spiport;
#ifdef TOUCH_SCREEN_SPI_FREQ
		softspi_config(touch_spi, conf, TOUCH_SCREEN_SPI_FREQ);
#endif
		touch_screen_width = width;
		touch_screen_height = height;
		touch_cs_pin = cs_pin;
		touch_penirq_pin = penirq_pin;
		touch_screen_flags = flags;

		softspi_start(touch_spi);
		io_set_pinvalue(touch_cs_pin, false);
		softspi_xmit(touch_spi, TOUCH_SCREEN_READ_Y);
		softspi_xmit16(touch_spi, 0);
		io_set_pinvalue(touch_cs_pin, true);
		softspi_stop(touch_spi);
	}

	bool touch_screen_is_touching() { return (io_get_pinvalue(touch_penirq_pin) < 1); }

	void touch_screen_get_position(uint16_t *px, uint16_t *py)
	{
		if (!touch_screen_is_touching())
		{
			*px = *py = 0xffff;
			return;
		}

		uint8_t buf = 0;

		uint16_t x = 0xFFFF;
		uint16_t y = 0xFFFF;

		softspi_start(touch_spi);
		io_set_pinvalue(touch_cs_pin, false);

		softspi_xmit(touch_spi, TOUCH_SCREEN_READ_X); /*Start x read*/

		buf = softspi_xmit(touch_spi, 0); /*Read x MSB*/
		x = buf << 8;
		buf = softspi_xmit(touch_spi, TOUCH_SCREEN_READ_Y); /*Until x LSB converted y command can be sent*/
		x += buf;

		buf = softspi_xmit(touch_spi, 0); /*Read y MSB*/
		y = buf << 8;

		buf = softspi_xmit(touch_spi, 0); /*Read y LSB*/
		y += buf;

		/*Normalize Data*/
		x = x >> 3;
		y = y >> 3;

		touch_screen_corr(&x, &y);

		io_set_pinvalue(touch_cs_pin, true);
		softspi_stop(touch_spi);

		*px = x;
		*py = y;
	}

#ifdef __cplusplus
}
#endif