/*
	Name: tft_display.c
	Description: TFT display panel support for µCNC

	Copyright: Copyright (c) Patryk Mierzyński
	Author: Patryk Mierzyński
	Date: 20/07/2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the GNU General Public License for more details.
*/

#ifdef ENABLE_TFT

#include "tft_display.h"
#include "../softspi.h"
#include "src/module.h"

#include "graphics_library/fonts/freemonobold12pt7b.h"

#ifndef TFT_LCD_CS
#define TFT_LCD_CS DOUT0
#endif

#ifndef TFT_LCD_RS
#define TFT_LCD_RS DOUT1
#endif

#ifndef TFT_SPI_PORT
#define TFT_SPI_PORT MCU_SPI
#endif

#ifndef TFT_SPI_FREQ
#define TFT_SPI_FREQ 5000000
#endif

/**
 * Question: Why is this macro needed?
 * Answer:
 *	 I'm not sure about other display but when using the Waveshare 3.5" RPi LCD (A)
 *	 I had an issue where sometimes the display would work and other times it would
 *	 stay completely white. This happens because of a race condition.
 *	 This display uses a couple of ICs (2 shift registers, a counter and an inverter)
 *	 to provide access to its LCD driver using a 16 bit parallel bus. While it removes
 *	 reading capabilities it does provide a slightly faster write cycle (I think). The
 *	 counter counts clock cycles and is used to trigger the write signal of the LCD driver
 *	 after it reaches 16. This also means that every transfer has to be 16 bit. When testing
 *	 the code with a STM32F411 Blackpill it turned out that the Data/Command control line was
 *	 getting reset (to 1) before the SPI hardware finished returning the clock line to 0. This
 *	 caused issues with the counter as it is activated on a falling edge of the clock, and instead
 *	 of writing to the command register the code was writing to the data register which broke everything.
 *	 However this didn't happen when the SPI clock was faster because the hardware was done with the
 *	 clock before the CPU executed the I/O code. I hope this story helps someone who has a simillar
 *	 problem and saves them the time I have wasted on this issue.
 *
 * This macro must delay code execution by atleast 1 SPI clock cycle, sometimes
 * if the clock is fast enough that delay is not needed.
 */

#define TFT_CLK_SETTLE_DELAY() mcu_delay_us(1)

void tft_start()
{
	softspi_config_t conf = { 0 };
	conf.spi.mode = 0;
	conf.spi.enable_dma = 1;
	softspi_config(TFT_SPI_PORT, conf, TFT_SPI_FREQ);
	softspi_start(TFT_SPI_PORT);
}

void tft_stop()
{
	softspi_stop(TFT_SPI_PORT);
}

void tft_command(uint8_t cmd)
{
	io_clear_output(TFT_LCD_CS);
	io_clear_output(TFT_LCD_RS);

#ifdef TFT_ALWAYS_16BIT
	softspi_xmit(TFT_SPI_PORT, 0);
#endif
	softspi_xmit(TFT_SPI_PORT, cmd);

	TFT_CLK_SETTLE_DELAY();
	io_set_output(TFT_LCD_RS);
	io_set_output(TFT_LCD_CS);
}

void tft_data(uint8_t data)
{
	io_clear_output(TFT_LCD_CS);

#ifdef TFT_ALWAYS_16BIT
	softspi_xmit(TFT_SPI_PORT, 0);
#endif
	softspi_xmit(TFT_SPI_PORT, data);

	TFT_CLK_SETTLE_DELAY();
	io_set_output(TFT_LCD_CS);
}

void tft_bulk_data(const uint8_t *data, uint16_t len)
{
	io_clear_output(TFT_LCD_CS);

	softspi_bulk_xmit(TFT_SPI_PORT, data, 0, len);
	
	TFT_CLK_SETTLE_DELAY();
	io_set_output(TFT_LCD_CS);
}

void tft_blit(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const gfx_pixel_t *data)
{
	// Set columns
	TFT_CAS(x, x + w);
	// Set rows
	TFT_RAS(y, y + h);

	// Send pixels
	TFT_MEMWR();
	tft_bulk_data((const uint8_t*)data, w * h * sizeof(uint16_t));
}

#define BASE_BACKGROUND GFX_COLOR(229, 229, 229)
#define BORDER_DARK GFX_COLOR(153, 153, 153)
#define BORDER_LIGHT GFX_COLOR(204, 204, 204)
#define BOX_BACKGROUND GFX_COLOR(242, 242, 242)
#define TOP_BAR GFX_COLOR(0, 0, 178)

#define BOX_INSET(x, y, w, h) \
	GFX_RECT(x, y, w, 3, BORDER_DARK); \
	GFX_RECT(x, (y) + 3, 3, (h) - 3, BORDER_DARK); \
	GFX_RECT((x) + (w) - 3, (y) + 3, 3, (h) - 3, BORDER_LIGHT); \
	GFX_RECT((x) + 3, (y) + (h) - 3, (w) - 3, 3, BORDER_LIGHT); \
	GFX_RECT((x) + 3, (y) + 3, (w) - 6, (h) - 6, BOX_BACKGROUND);

#define BOX_OUTSET(x, y, w, h) \
	GFX_RECT(x, y, w, 3, BORDER_LIGHT); \
	GFX_RECT(x, (y) + 3, 3, (h) - 3, BORDER_LIGHT); \
	GFX_RECT((x) + (w) - 3, (y) + 3, 3, (h) - 3, BORDER_DARK); \
	GFX_RECT((x) + 3, (y) + (h) - 3, (w) - 3, 3, BORDER_DARK); \
	GFX_RECT((x) + 3, (y) + 3, (w) - 6, (h) - 6, BOX_BACKGROUND);

GFX_DECL_SCREEN(main)
{
	GFX_CLEAR(BASE_BACKGROUND);

	GFX_RECT(0, 0, GFX_DISPLAY_WIDTH, 20, TOP_BAR);

	BOX_OUTSET(0, 30, 50, 50);
	BOX_OUTSET(0, 115, 50, 50);
	BOX_OUTSET(0, 200, 50, 50);

	BOX_INSET(70, 30, 200, 111);

	GFX_TEXT(80,	40, BOX_BACKGROUND, GFX_BLACK, "X");
	GFX_TEXT(80,	75, BOX_BACKGROUND, GFX_BLACK, "Y");
	GFX_TEXT(80, 110, BOX_BACKGROUND, GFX_BLACK, "Z");
	
	GFX_TEXT(145,  40, BOX_BACKGROUND, GFX_BLACK, "000.00");
	GFX_TEXT(145,  75, BOX_BACKGROUND, GFX_BLACK, "000.00");
	GFX_TEXT(145, 110, BOX_BACKGROUND, GFX_BLACK, "000.00");
}

DECL_MODULE(tft_display)
{
	static bool initialized = false;

	if(!initialized)
	{
		initialized = true;

		io_set_output(TFT_LCD_CS);
		io_set_output(TFT_LCD_RS);

		tft_start();

		TFT_INIT();

		TFT_ON();

		GFX_RENDER_SCREEN(main);

		tft_stop();
	}
}

#endif

