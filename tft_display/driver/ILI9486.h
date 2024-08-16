/*
	Name: ILI9486.h
	Description: Driver specific code for ILI9486

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

#ifndef TFT_DISPLAY_DRIVER_ILI9486_H
#define TFT_DISPLAY_DRIVER_ILI9486_H

#include <stdint.h>

#ifdef TFT_ALWAYS_16BIT
#define TFT_PGM { 0, 0x0F, 0, 0x1F, 0, 0x1C, 0, 0x0C, 0, 0x0F, 0, 0x08, 0, 0x48, 0, 0x98, 0, 0x37, 0, 0x0A, 0, 0x13, 0, 0x04, 0, 0x11, 0, 0x0D, 0, 0x00 }
#define TFT_NGM { 0, 0x0F, 0, 0x32, 0, 0x2E, 0, 0x0B, 0, 0x0D, 0, 0x05, 0, 0x47, 0, 0x75, 0, 0x37, 0, 0x06, 0, 0x10, 0, 0x03, 0, 0x24, 0, 0x20, 0, 0x00 }
#else
#define TFT_PGM { 0x0F, 0x1F, 0x1C, 0x0C, 0x0F, 0x08, 0x48, 0x98, 0x37, 0x0A, 0x13, 0x04, 0x11, 0x0D, 0x00 }
#define TFT_NGM { 0x0F, 0x32, 0x2E, 0x0B, 0x0D, 0x05, 0x47, 0x75, 0x37, 0x06, 0x10, 0x03, 0x24, 0x20, 0x00 }
#endif

#define TFT_MEMCTL(value) CMD(0x36); DAT(value);

#define TFT_MEMCTL_MIRROR_Y 0x80
#define TFT_MEMCTL_MIRROR_X 0x40
// This bit exchanges row and column addresses (results in a 90 degree clock-wise rotation)
#define TFT_MEMCTL_ROTATE_90 0x20
#define TFT_MEMCTL_BGR 0x08

#define TFT_MEMCTL_VALUE (TFT_MEMCTL_ROTATE_90 | TFT_MEMCTL_MIRROR_Y | TFT_MEMCTL_MIRROR_X | TFT_MEMCTL_BGR)

/*
 * Initialization sequence for ILI9486 driver
 * Reference:
 *	https://github.com/Bodmer/TFT_eSPI/blob/master/TFT_Drivers/ILI9486_Init.h
 *	https://www.waveshare.com/w/upload/7/78/ILI9486_Datasheet.pdf
 */
#define TFT_INIT() { \
	/* Software reset */ \
	cnc_delay_ms(5); \
	CMD(0x01); \
	cnc_delay_ms(120); \
	CMD(0x11); \
	cnc_delay_ms(120); \
	/* Set pixel format to 16bpp */ \
	CMD(0x3A); \
	DAT(0x55); \
	/* Set power control */ \
	CMD(0xC0); \
	DAT(0x0E); \
	DAT(0x0E); \
	CMD(0xC1); \
	DAT(0x41); \
	DAT(0x00); \
	CMD(0xC2); \
	DAT(0x44); \
	/* VCOM control */ \
	CMD(0xC5); \
	DAT(0x00); \
	DAT(0x00); \
	DAT(0x00); \
	DAT(0x00); \
	/* Positive gamma control */ \
	uint8_t pgm[] = TFT_PGM; \
	CMD(0xE0); \
	DAT_BULK(pgm, sizeof(pgm)); \
	/* Negative gamma control */ \
	uint8_t ngm[] = TFT_NGM; \
	CMD(0xE1); \
	DAT_BULK(ngm, sizeof(ngm)); \
	/* Inversion off */ \
	CMD(0x20); \
	/* Set memory access control */ \
	TFT_MEMCTL(TFT_MEMCTL_VALUE); \
}

#define TFT_ON() { \
	CMD(0x29); \
	cnc_delay_ms(150); \
}

#define TFT_OFF() CMD(0x28);

// Column address set command
#define TFT_CAS(start, end) CMD(0x2A); \
	DAT((start) >> 8); \
	DAT(start); \
	DAT((end) >> 8); \
	DAT(end);

// Row address set command
#define TFT_RAS(start, end) CMD(0x2B); \
	DAT((start) >> 8); \
	DAT(start); \
	DAT((end) >> 8); \
	DAT(end);

// Memory write
#define TFT_MEMWR() CMD(0x2C);



/* Definitions required by the graphics library */

#if (TFT_MEMCTL_VALUE & TFT_MEMCTL_ROTATE_90)
#define TFT_DISPLAY_WIDTH 480
#define TFT_DISPLAY_HEIGHT 320
#else
#define TFT_DISPLAY_WIDTH 320
#define TFT_DISPLAY_HEIGHT 480
#endif

#endif

