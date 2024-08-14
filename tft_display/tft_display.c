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

#include "src/cnc.h"
#if (UCNC_MODULE_VERSION < 10990 || UCNC_MODULE_VERSION > 99999)
#error "This module is not compatible with the current version of µCNC"
#endif

#if defined(TFT_SPI_HARDWARE_PORT) || (defined(TFT_SPI_MOSI) && defined(TFT_SPI_CLK))

#include "tft_display.h"
#include "src/modules/softspi.h"
#include "src/modules/lvgl/lvgl_support.h"

#ifdef TFT_CS
// Clear/set IO pin
#define TFT_SELECT() io_clear_output(TFT_CS)
#define TFT_RELEASE() io_set_output(TFT_CS)
#else
// Do nothing
#define TFT_SELECT()
#define TFT_RELEASE()
#endif

#ifndef TFT_RS
#define TFT_RS DOUT1
#endif

#ifndef TFT_SPI_FREQ
#define TFT_SPI_FREQ 1000000
#endif

#ifndef TFT_BUFFER_SIZE
#define TFT_BUFFER_SIZE (32 * 1024)
#endif

#ifndef TFT_SYNC_CS
// Makes sure the driver pulses the chip select line before
// every transfer, this can be used to synchronize the
// receiver in case of lost clocks.
#define TFT_SYNC_CS 1
#endif

#ifndef TFT_CLK_SETTLE_DELAY
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
#define TFT_CLK_SETTLE_DELAY() // mcu_delay_us(1)
#endif

#ifdef TFT_SPI_HARDWARE_PORT
static HARDSPI(tft_spi, TFT_SPI_FREQ, 0, TFT_SPI_HARDWARE_PORT);
#else
static SOFTSPI(tft_spi, TFT_SPI_FREQ, 0, TFT_SPI_MOSI, UNDEF_PIN, TFT_SPI_CLK);
#endif

#define CMD(cmd) tft_command(cmd)
#define DAT(dat) tft_data(dat)
#define DAT_BULK(data, len) tft_bulk_data(data, len)

#include "driver.h"

static void tft_start()
{
	softspi_start(&tft_spi);
#if !TFT_SYNC_CS
	TFT_SELECT();
#endif
}

static void tft_stop()
{
#if !TFT_SYNC_CS
	TFT_RELEASE();
#endif
	softspi_stop(&tft_spi);
}

static void tft_command(uint8_t cmd)
{
#if TFT_SYNC_CS
	TFT_SELECT();
#endif
	io_clear_output(TFT_RS);

#ifdef TFT_ALWAYS_16BIT
	softspi_xmit(&tft_spi, 0);
#endif
	softspi_xmit(&tft_spi, cmd);

	TFT_CLK_SETTLE_DELAY();
	io_set_output(TFT_RS);
#if TFT_SYNC_CS
	TFT_RELEASE();
#endif
}

static void tft_data(uint8_t data)
{
#if TFT_SYNC_CS
	TFT_SELECT();
#endif

#ifdef TFT_ALWAYS_16BIT
	softspi_xmit(&tft_spi, 0);
#endif
	softspi_xmit(&tft_spi, data);

	TFT_CLK_SETTLE_DELAY();
#if TFT_SYNC_CS
	TFT_RELEASE();
#endif
}

static void tft_bulk_data(const uint8_t *data, uint16_t len)
{
#if TFT_SYNC_CS
	TFT_SELECT();
#endif

	softspi_bulk_xmit(&tft_spi, data, 0, len);
	
	TFT_CLK_SETTLE_DELAY();
#if TFT_SYNC_CS
	TFT_RELEASE();
#endif
}

static lv_display_t *lvgl_display = 0;
static bool pending_tx = false;
static lv_area_t tx_area;
static void *tx_pixels;

#ifdef ENABLE_MAIN_LOOP_MODULES
bool tft_update(void *arg)
{
	if(pending_tx)
	{
		tft_start();

		// Set columns
		TFT_CAS(tx_area.x1, tx_area.x2);
		// Set rows
		TFT_RAS(tx_area.y1, tx_area.y2);

		uint32_t w = tx_area.x2 - tx_area.x1 + 1;
		uint32_t h = tx_area.y2 - tx_area.y1 + 1;

		// Send pixels
		TFT_MEMWR();
		// This assumes that the driver sends us data in RGB565 mode with 2 bytes per pixel
		tft_bulk_data(tx_pixels, w * h * 2);

		tft_stop();

		// Notify library of flush completion
		pending_tx = false;
	}
	return EVENT_CONTINUE;
}

CREATE_EVENT_LISTENER_WITHLOCK(cnc_dotasks, tft_update, LISTENER_HWSPI_LOCK);
#endif

static void lvgl_flush_cb(lv_display_t *display, const lv_area_t *area, uint8_t *pixel_map)
{
#if LV_COLOR_16_SWAP
	// Correct the byte order (would be nice if LVGL handled it more efficiently)
	uint32_t w = area->x2 - area->x1 + 1;
	uint32_t h = area->y2 - area->y1 + 1;
	lv_draw_sw_rgb565_swap(pixel_map, w * h);
#endif

	tx_area = *area;
	tx_pixels = pixel_map;
	pending_tx = true;
}

static void lvgl_flush_wait_cb(lv_display_t *display)
{
	while(pending_tx)
		cnc_dotasks();
}

static uint8_t lvgl_display_buffer[TFT_BUFFER_SIZE];

DECL_MODULE(tft_display)
{
	TFT_RELEASE();
	io_set_output(TFT_RS);

	spi_config_t conf = { 0 };
	conf.enable_dma = 1;
	softspi_config(&tft_spi, conf, TFT_SPI_FREQ);

	// Prepare for communication with the display
	tft_start();

	// Initialize
	TFT_INIT();
	TFT_ON();

	// End communication
	tft_stop();

#ifdef ENABLE_MAIN_LOOP_MODULES
	ADD_EVENT_LISTENER(cnc_dotasks, tft_update);
#else
#warning "Main loop extensions are disabled. TFT display module will not work."
#endif

	lvgl_display = lv_display_create(TFT_DISPLAY_WIDTH, TFT_DISPLAY_HEIGHT);
	lv_display_set_flush_cb(lvgl_display , lvgl_flush_cb);
	lv_display_set_flush_wait_cb(lvgl_display, lvgl_flush_wait_cb);
	lv_display_set_buffers(lvgl_display, lvgl_display_buffer, 0, TFT_BUFFER_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);
	lv_display_set_color_format(lvgl_display, LV_COLOR_FORMAT_RGB565);

	// Send the object to LVGL support module
	lvgl_use_display(lvgl_display);
}

#endif

