/*
	Name: mks_display.c
	Description: Graphic LCD module for µCNC using u8g2 lib.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 08-09-2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "src/cnc.h"
#include "src/modules/system_menu.h"
#include "src/modules/softspi.h"
#include "lvgl/lvgl.h"
#include "lvgl/src/drivers/display/st7796/lv_st7796.h"
#include "ui/ui.h"
#include "src/modules/touch_screen/touch_screen.h"

#if (UCNC_MODULE_VERSION < 10990 || UCNC_MODULE_VERSION > 99999)
#error "This module is not compatible with the current version of µCNC"
#endif

/**
 * Display settings
 * **/
#define TFT_DISPLAY_HW_SPI 0
#define TFT_DISPLAY_HW_SPI2 1
#define TFT_DISPLAY_SW_SPI 2

#ifndef TFT_DISPLAY_SPI_INTERFACE
#define TFT_DISPLAY_SPI_INTERFACE TFT_DISPLAY_HW_SPI
#endif

#ifndef TFT_DISPLAY_SPI_CS
#define TFT_DISPLAY_SPI_CS DOUT6
#endif
#ifndef TFT_DISPLAY_SPI_DC
#define TFT_DISPLAY_SPI_DC DOUT32
#endif
#ifndef TFT_DISPLAY_BKL
#define TFT_DISPLAY_BKL DOUT33
#endif
#ifndef TFT_DISPLAY_RST
#define TFT_DISPLAY_RST DOUT34
#endif
#ifndef TFT_DISPLAY_SPI_FREQ
#define TFT_DISPLAY_SPI_FREQ 24000000UL
#endif
#ifndef TFT_H_RES
#define TFT_H_RES 320
#endif
#ifndef TFT_V_RES
#define TFT_V_RES 480
#endif

/**
 * Touch screen sensor
 * * */
#ifndef TFT_DISPLAY_TOUCH_SPI_FREQ
#define TFT_DISPLAY_TOUCH_SPI_FREQ 1000000UL
#endif
#ifndef TFT_DISPLAY_TOUCH_CS
#define TFT_DISPLAY_TOUCH_CS DOUT35
#endif
#ifndef TFT_DISPLAY_TOUCH_IRQ_PRESS
#define TFT_DISPLAY_TOUCH_IRQ_PRESS DIN35
#endif

// set 0 to disable
// set 8 to perform a byte swap
// other values for custom bit swaping
#ifndef TFT_SWAP_BIT
#define TFT_SWAP_BIT 8
#endif

#if (TFT_DISPLAY_SPI_INTERFACE==TFT_DISPLAY_HW_SPI)
#define TFT_DISPLAY_SPI_PORT mcu_spi_port
#define TFT_DISPLAY_SPI_LOCK LISTENER_HWSPI_LOCK
HARDSPI(tft_spi, TFT_DISPLAY_SPI_FREQ, 3, TFT_DISPLAY_SPI_PORT);
HARDSPI(touch_spi, TFT_DISPLAY_TOUCH_SPI_FREQ, 3, TFT_DISPLAY_SPI_PORT);
#elif (TFT_DISPLAY_SPI_INTERFACE==TFT_DISPLAY_HW_SPI2)
#define TFT_DISPLAY_SPI_PORT mcu_spi2_port
#define TFT_DISPLAY_SPI_LOCK LISTENER_HWSPI2_LOCK
HARDSPI(tft_spi, TFT_DISPLAY_SPI_FREQ, 3, TFT_DISPLAY_SPI_PORT);
HARDSPI(touch_spi, TFT_DISPLAY_TOUCH_SPI_FREQ, 3, TFT_DISPLAY_SPI_PORT);
#elif (TFT_DISPLAY_SPI_INTERFACE==TFT_DISPLAY_SW_SPI)
#error "nor defined"
#endif
// extern void tft_init(void);
// extern void tft_write(uint16_t x, uint16_t y, uint16_t *data, uint16_t w, uint16_t h);
static lv_disp_t *disp;
static lv_indev_t *indev;
static lv_obj_t *cursor_obj;

// buffer
#define SCREENBUFFER_SIZE_PIXELS (TFT_H_RES * TFT_V_RES / 20)
static lv_color_t buf[SCREENBUFFER_SIZE_PIXELS];
static lv_color_t buf2[SCREENBUFFER_SIZE_PIXELS];

/**
 * LVGL callback functions
 *
 */

// based on https://docs.lvgl.io/master/integration/driver/display/lcd_stm32_guide.html#lcd-stm32-guide
/* Platform-specific implementation of the LCD send command function. In general this should use polling transfer. */
static void tft_send_cmd(lv_display_t *display, const uint8_t *cmd, size_t cmd_size, const uint8_t *param, size_t param_size)
{
	LV_UNUSED(disp);
	tft_spi.spiconfig.enable_dma = 0;
	softspi_start(&tft_spi);
	/* DCX low (command) */
	io_clear_output(TFT_DISPLAY_SPI_DC);
	/* CS low */
	io_clear_output(TFT_DISPLAY_SPI_CS);
	/* send command */
	softspi_bulk_xmit(&tft_spi, cmd, NULL, (uint16_t)cmd_size);

	/* DCX high (data) */
	io_set_output(TFT_DISPLAY_SPI_DC);
	/* for short data blocks we use polling transfer */
	softspi_bulk_xmit(&tft_spi, param, NULL, (uint16_t)param_size);
	/* CS high */
	io_set_output(TFT_DISPLAY_SPI_CS);
	softspi_stop(&tft_spi);
	lv_display_flush_ready(display);
}

static FORCEINLINE void swap_colors(uint16_t *buff, size_t pixel_count)
{
	while (pixel_count--)
	{
		uint16_t pixel = *buff;
		*buff++ = (((pixel >> (TFT_SWAP_BIT & 0x0F)) | (pixel << (16 - (TFT_SWAP_BIT & 0x0F)))) & 0xFFFF);
	}
}

/* Platform-specific implementation of the LCD send color function. For better performance this should use DMA transfer.
 * In case of a DMA transfer a callback must be installed to notify LVGL about the end of the transfer.
 */
static void tft_send_color(lv_display_t *display, const uint8_t *cmd, size_t cmd_size, uint8_t *param, size_t param_size)
{
	LV_UNUSED(disp);
	tft_spi.spiconfig.enable_dma = 0;
	softspi_start(&tft_spi);
	/* DCX low (command) */
	io_clear_output(TFT_DISPLAY_SPI_DC);
	/* CS low */
	io_clear_output(TFT_DISPLAY_SPI_CS);
	/* send command */
	softspi_bulk_xmit(&tft_spi, cmd, NULL, (uint16_t)cmd_size);
	softspi_stop(&tft_spi);
	tft_spi.spiconfig.enable_dma = 1;
	/* DCX high (data) */
	io_set_output(TFT_DISPLAY_SPI_DC);
	softspi_start(&tft_spi);
/* for short data blocks we use polling transfer */
// fix color swap
#if TFT_SWAP_BIT == 8
	lv_draw_sw_rgb565_swap(param, param_size >> 1);
#elif TFT_SWAP_BIT != 0
	swap_colors((uint16_t *)param, param_size >> 1);
#endif
			softspi_bulk_xmit(&tft_spi, param, NULL, (uint16_t)param_size);
	/* CS high */
	io_set_output(TFT_DISPLAY_SPI_CS);
	softspi_stop(&tft_spi);
	lv_display_flush_ready(display);
}

// void tft_flush_cb(lv_display_t *display, const lv_area_t *area, uint8_t *px_map)
// {
// 	tft_write(area->x1, area->y1, (uint16_t *)px_map, area->x2 - area->x1, area->y2 - area->y1);
// 	/* IMPORTANT!!!
// 	 * Inform LVGL that you are ready with the flushing and buf is not used anymore*/
// 	lv_display_flush_ready(display);
// }

void tft_log(lv_log_level_t level, const char *buf)
{
	serial_print_str(buf);
}

#if ASSERT_PIN(TFT_DISPLAY_TOUCH_CS)
void tft_touch_read(lv_indev_t *indev, lv_indev_data_t *data)
{
	data->state = LV_INDEV_STATE_RELEASED;
	data->point.x = -1;
	data->point.y = -1;
	if (touch_screen_is_touching())
	{
		uint16_t x, y;
		touch_screen_get_position(&x, &y);
		if (x > 0 && x < TFT_H_RES && y > 0 && x < TFT_V_RES)
		{
			data->point.x = x;
			data->point.y = y;
			data->state = LV_INDEV_STATE_PRESSED;
			if (lv_screen_active() != ui_idle)
			{
				system_menu_action(255); // uncatched code just to prevent going idle
			}
		}
	}
}
#endif

/**
 *
 * Module event listener callbacks
 *
 */

#ifdef ENABLE_MAIN_LOOP_MODULES
static uint32_t next_update = 0;
bool mks_display_update(void *args)
{
	if (next_update < mcu_millis())
	{
		system_menu_action(SYSTEM_MENU_ACTION_NONE);
		system_menu_render();
		next_update = lv_timer_handler() + mcu_millis();
	}

	return EVENT_CONTINUE;
}
CREATE_EVENT_LISTENER_WITHLOCK(cnc_io_dotasks, mks_display_update, TFT_DISPLAY_SPI_LOCK);

bool mks_display_start(void *args)
{
	if (!disp)
	{
		io_clear_output(TFT_DISPLAY_BKL);
		cnc_delay_ms(50);
		io_set_output(TFT_DISPLAY_BKL);
#if ASSERT_PIN(TFT_DISPLAY_RST)
		io_set_output(TFT_DISPLAY_RST);
		cnc_delay_ms(100);
		io_clear_output(TFT_DISPLAY_RST);
		cnc_delay_ms(100);
		io_set_output(TFT_DISPLAY_RST);
#endif
		io_set_output(TFT_DISPLAY_SPI_DC);
		io_set_output(TFT_DISPLAY_SPI_CS);
		cnc_delay_ms(100);
		// tft_init();

		// disp = lv_display_create(TFT_H_RES, TFT_V_RES);
		disp = lv_st7796_create(TFT_H_RES, TFT_V_RES, LV_LCD_FLAG_BGR, tft_send_cmd, tft_send_color);
		lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_90);
		// lv_display_set_flush_cb(disp, tft_flush_cb);
		// lv_log_register_print_cb(tft_log);

		lv_display_set_buffers(disp, buf, buf2, SCREENBUFFER_SIZE_PIXELS * sizeof(lv_color_t), LV_DISPLAY_RENDER_MODE_PARTIAL);

#if ASSERT_PIN(TFT_DISPLAY_TOUCH_CS)
		spi_config_t config = {0};
		softspi_config(&touch_spi, config, TFT_DISPLAY_TOUCH_SPI_FREQ);
		touch_screen_init(&touch_spi, TFT_H_RES, TFT_V_RES, 0, TFT_DISPLAY_TOUCH_CS, TFT_DISPLAY_TOUCH_IRQ_PRESS);
		indev = lv_indev_create();
		lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
		lv_indev_set_read_cb(indev, tft_touch_read);
#else
#warning "No input device selected. You will not be able to interact with the menus."
#endif
		// cursor_obj = lv_image_create(lv_screen_active());		 /*Create an image object for the cursor */
		// lv_image_set_src(cursor_obj, &ui_img_checkmark_png); /*Set the image source*/
		// lv_indev_set_cursor(indev, cursor_obj);

		ui_init();

		// register the other listeners
		ADD_EVENT_LISTENER(cnc_io_dotasks, mks_display_update);
	}
	return EVENT_CONTINUE;
}

CREATE_EVENT_LISTENER_WITHLOCK(cnc_reset, mks_display_start, TFT_DISPLAY_SPI_LOCK);

#endif

/**
 * Screen serial stream
 */

#ifdef DECL_SERIAL_STREAM
DECL_BUFFER(uint8_t, mks_display_stream_buffer, 32);
static uint8_t mks_display_getc(void)
{
	uint8_t c = 0;
	BUFFER_DEQUEUE(mks_display_stream_buffer, &c);
	return c;
}

uint8_t mks_display_available(void)
{
	return BUFFER_READ_AVAILABLE(mks_display_stream_buffer);
}

void mks_display_clear(void)
{
	BUFFER_CLEAR(mks_display_stream_buffer);
}

DECL_SERIAL_STREAM(mks_display_stream, mks_display_getc, mks_display_available, mks_display_clear, NULL, NULL);

uint8_t system_menu_send_cmd(const char *__s)
{
	// if machine is running rejects the command
	if (cnc_get_exec_state(EXEC_RUN | EXEC_JOG) == EXEC_RUN)
	{
		return STATUS_SYSTEM_GC_LOCK;
	}

	uint8_t len = strlen(__s);
	uint8_t w;

	if (BUFFER_WRITE_AVAILABLE(mks_display_stream_buffer) < len)
	{
		return STATUS_STREAM_FAILED;
	}

	BUFFER_WRITE(mks_display_stream_buffer, (void *)__s, len, w);

	return STATUS_OK;
}

#endif

// custom render jog screen
extern void system_menu_render_jog(uint8_t flags);

DECL_MODULE(mks_display)
{
#ifdef DECL_SERIAL_STREAM
	serial_stream_register(&mks_display_stream);
#endif
	// STARTS SYSTEM MENU MODULE
	system_menu_init();
	system_menu_set_render_callback(SYSTEM_MENU_ID_JOG, system_menu_render_jog);
	lv_init();
	lv_tick_set_cb(mcu_millis);
	lv_delay_set_cb(cnc_delay_ms);
#ifdef ENABLE_MAIN_LOOP_MODULES
	ADD_EVENT_LISTENER(cnc_reset, mks_display_start);
#else
#warning "Main loop extensions are not enabled. Graphic display card will not work."
#endif
}
