/*
	Name: mmcsd_card.c
	Description: SD card module for µCNC.
	This adds SD card support via SPI

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

#include "../../cnc.h"
#include "../softspi.h"
#include "ffconf.h"
#include "mmcsd.h"
#include "ff.h"
#include "diskio.h"
#include <stdint.h>
#include <stdbool.h>

#ifndef MMCSD_MAX_NCR
#define MMCSD_MAX_NCR 250
#endif
#ifndef MMCSD_MAX_TIMEOUT
#define MMCSD_MAX_TIMEOUT 255
#endif
#ifndef MMCSD_MAX_BUFFER_SIZE
#define MMCSD_MAX_BUFFER_SIZE 512
#endif

#define SD_CARD_USE_HW_SPI

#if (!defined(SD_CARD_USE_HW_SPI) || !defined(MCU_HAS_SPI))
#ifndef SD_SPI_CLK
#define SD_SPI_CLK DOUT30
#endif
#ifndef SD_SPI_SDO
#define SD_SPI_SDO DOUT29
#endif
#ifndef SD_SPI_SDI
#define SD_SPI_SDI DIN29
#endif
#ifndef SD_SPI_CS
#define SD_SPI_CS SPI_CS
#endif
SOFTSPI(mmcsd_spi_fast, 1000000UL, 0, SD_SPI_SDO, SD_SPI_SDI, SD_SPI_CLK);
#else
#ifndef SD_SPI_CLK
#define SD_SPI_CLK SPI_CLK
#endif
#ifndef SD_SPI_SDO
#define SD_SPI_SDO SPI_SDO
#endif
#ifndef SD_SPI_SDI
#define SD_SPI_SDI SPI_SDI
#endif
#ifndef SD_SPI_CS
#define SD_SPI_CS SPI_CS
#endif
#endif

// this is a slow speed SPI mode
SOFTSPI(mmcsd_spi_slow, 100000UL, 0, SD_SPI_SDO, SD_SPI_SDI, SD_SPI_CLK);
static softspi_port_t *mmcsd_spi;

#define spi_xmit(c) softspi_xmit(mmcsd_spi, c)

static mmcsd_card_t mmcsd_card;

FORCEINLINE static void mmcsd_spi_speed(bool highspeed)
{
	if (highspeed)
	{
#if (!defined(SD_CARD_USE_HW_SPI) || !defined(MCU_HAS_SPI))
		mmcsd_spi = &mmcsd_spi_fast;
#else
		mmcsd_spi = NULL;
#endif
	}
	else
	{
		mmcsd_spi = &mmcsd_spi_slow;
	}
}

void mmcsd_response(uint8_t *result, uint8_t len)
{
	mcu_clear_output(SD_SPI_CS);
	while (len--)
	{
		*result++ = spi_xmit(0xFF);
	}

	spi_xmit(0xFF);
	spi_xmit(0xFF);
	mcu_set_output(SD_SPI_CS);
}

FORCEINLINE static uint8_t mmcsd_command(uint8_t cmd, uint32_t arg, int8_t crc)
{
	uint8_t packet[6];
	uint8_t response, t = MMCSD_MAX_NCR;
	uint8_t *bytes = (uint8_t *)&arg;

	packet[0] = cmd | 0x40;
	packet[1] = bytes[3];
	packet[2] = bytes[2];
	packet[3] = bytes[1];
	packet[4] = bytes[0];

#ifdef MMCSD_CRC_CHECK
	packet[5] = crc7(packet);
#else
	packet[5] = crc;
#endif

	// flushes command flow
	mcu_set_output(SD_SPI_CS);
	spi_xmit(0xFF);
	spi_xmit(0xFF);
	mcu_clear_output(SD_SPI_CS);
	spi_xmit(0xFF);

	// sends command, arg and CRC
	spi_xmit(packet[0]);
	spi_xmit(packet[1]);
	spi_xmit(packet[2]);
	spi_xmit(packet[3]);
	spi_xmit(packet[4]);
	spi_xmit(packet[5]);

	// returns response (R1 format)
	while (t--)
	{
		response = spi_xmit(0xFF);
		if (!CHECKBIT(response, 7))
		{
			break;
		}
	}

	mcu_set_output(SD_SPI_CS);

	return response;
}

DSTATUS disk_status(BYTE pdrv)
{
	if (!mmcsd_card.detected)
	{
		return (STA_NOINIT | STA_NODISK);
	}

	if (!mmcsd_card.initialized)
	{
		return (STA_NOINIT);
	}

	if (mmcsd_card.writeprotected)
	{
		return (STA_PROTECT);
	}

	return 0;
}

DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count)
{
	uint8_t timeout;

	if (disk_status(0) & (STA_NOINIT | STA_NODISK))
	{
		return RES_NOTRDY;
	}

	// address must be a multiple of the block size
	uint32_t address = sector * MMCSD_MAX_BUFFER_SIZE;

	for (uint32_t offset = 0; offset <= count; offset++)
	{
		// send read command
		if (mmcsd_command(17, address, 0xFF))
		{
			return RES_ERROR;
		}

		mcu_clear_output(SD_SPI_CS);
		// waits token
		timeout = MMCSD_MAX_TIMEOUT;
		while (spi_xmit(0xFF) != 0xFE && --timeout)
			;

		// checks of a timeout occured
		if (!timeout)
		{
			return RES_ERROR;
		}

		for (uint16_t i = 0; i < MMCSD_MAX_BUFFER_SIZE; i++)
		{
			if (buff)
			{
				*buff = spi_xmit(0xFF);
				buff++;
			}
		}

		spi_xmit(0xFF);
		spi_xmit(0xFF);

		mcu_set_output(SD_SPI_CS);

		address += MMCSD_MAX_BUFFER_SIZE;
	}
	return RES_OK;
}

#if FF_FS_READONLY == 0

DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count)
{
#ifdef MMCSD_WRITE_VERIFY
	int r2;
	int r1;
#endif

	for (uint32_t offset = 0; offset <= count; offset++)
	{
		// address must be a multiple of the block size
		uint32_t address = sector + offset;
		// write cmd
		if (mmcsd_command(24, address, 0xFF))
		{
			return RES_WRPRT;
		}

		mcu_clear_output(SD_SPI_CS);

		// sends token
		spi_xmit(0xFF);
		spi_xmit(0xFE);

		// sends data to buffer
		for (uint16_t i = 0; i < MMCSD_MAX_BUFFER_SIZE; i++)
		{
			spi_xmit(*buff++);
		}

		// CRC dummy
		spi_xmit(0xFF);
		spi_xmit(0xFF);

		// checks for transmision error
		if ((spi_xmit(0xFF) & 0x0F) != 0x05)
		{
			return RES_ERROR;
		}

		// waits for write to complete
		while (!spi_xmit(0xFF))
			;

#ifdef MMCSD_WRITE_VERIFY
		// checks write status
		r1 = mmcsd_command(13, 0x00, 0xFF);
		mmcsd_response(&r2, 1);

		if (r1 || r2)
		{
			return RES_ERROR;
		}
#endif

		mcu_set_output(SD_SPI_CS);
	}
	return RES_OK;
}

#endif

DSTATUS disk_initialize(BYTE pdrv)
{
	uint8_t i, resp[4], timeout, crc41;
	uint32_t high_arg;

	// if (!mmcsd_card.detected)
	// {
	// 	return (STA_NODISK | STA_NOINIT);
	// }

	// if (mmcsd_card.initialized)
	// {
	// 	return disk_status(0);
	// }

	memset(&mmcsd_card, 0, sizeof(mmcsd_card_t));

	// mmcsd_spi_speed(false);

	// initializes card
	mcu_set_output(SD_SPI_CS);
	// flushes transmision
	for (i = 15; i != 0; i--)
	{
		spi_xmit(0xFF);
	}

	mcu_clear_output(SD_SPI_CS);
	if (mmcsd_command(0, 0x00, 0x95) != 0x01)
	{
		protocol_send_feedback(__romstr__("int failed"));
		return STA_NOINIT;
	}

	// tests if card is SD/MMC v2 (if CMD8 is legal-interface conditions)
	if (!CHECKBIT(mmcsd_command(8, 0x1AA, 0x87), ILLEGAL_CMD))
	{
		mmcsd_response(resp, 4);
		// card is not v2 or not usable
		if (!CHECKBIT(resp[2], 0) || resp[3] != 0xAA)
		{
			protocol_send_feedback(__romstr__("not v2"));
			return STA_NOINIT;
		}
		// card is v2 and usable
		else
		{
			mmcsd_card.card_type = SDv2;
		}
	}

	// CMD55 is a specific SD card command
	if (!CHECKBIT(mmcsd_command(55, 0x00, 0x65), ILLEGAL_CMD))
	{
		// sends ACMD41 with bit HCS set to 1 if it's v2 (high density cards) else sends HCS set to 0
		if (mmcsd_card.card_type == SDv2)
		{
			crc41 = 0xD3;
			high_arg = 0x80000000;
		}
		else
		{
			crc41 = 0xE5;
			high_arg = 0;
		}

		timeout = MMCSD_MAX_TIMEOUT;
		while (timeout && mmcsd_command(41, high_arg, crc41))
		{
			mmcsd_command(55, 0x00, 0x65);
			timeout--;
		}

		mmcsd_card.card_type = (!timeout) ? MMCv3 : SDv1;
	}

	// check if is MMC card
	if (mmcsd_card.card_type == MMCv3)
	{
		timeout = MMCSD_MAX_TIMEOUT;
		while (mmcsd_command(1, 0x00, 0xF9) && timeout--)
			;

		if (!timeout)
		{
			protocol_send_feedback(__romstr__("timeout"));
			return STA_NOINIT;
		}
	}

	// checks if it's high density card (if returns error retries with arg = 0)
	mmcsd_command(58, 0x40000000, 0x6F);
	mmcsd_response(resp, 4);
	if (CHECKBIT(resp[2], 6))
	{
		mmcsd_command(58, 0x00, 0xFD);
		mmcsd_response(resp, 4);
	}

	if (CHECKBIT(resp[0], 6))
	{
		mmcsd_card.is_highdensity = 1;
	}

	#ifdef MMCSD_CRC_CHECK
		if (mmcsd_command(59, 0x01, 0x91))
	#else
		if (mmcsd_command(59, 0x00, 0x25))
	#endif
		{
			protocol_send_feedback(__romstr__("no crc"));
			return STA_NOINIT;
		}

	if (mmcsd_command(16, MMCSD_MAX_BUFFER_SIZE, 0xFF))
	{
		protocol_send_feedback(__romstr__("block size"));
		return STA_NOINIT;
	}

	uint8_t csd[16];
	disk_ioctl(0, MMC_GET_CSD, csd);

	uint32_t c_size = (0x3F & csd[7]);
	c_size <<= 8;
	c_size += csd[8];
	c_size <<= 8;
	c_size += csd[9];
	mmcsd_card.sectors = (c_size + 1) << 10;
	mmcsd_card.size = mmcsd_card.sectors << 9;

	mcu_set_output(SD_SPI_CS);
	mmcsd_spi_speed(true);
	return 0;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
	uint8_t timeout = MMCSD_MAX_TIMEOUT;

	switch (cmd)
	{
	case CTRL_SYNC:
		// do nothing
		break;
	case GET_SECTOR_COUNT:
		*((UINT *)buff) = mmcsd_card.sectors;
		break;
	case GET_SECTOR_SIZE:
		*((UINT *)buff) = MMCSD_MAX_BUFFER_SIZE;
		break;
	case GET_BLOCK_SIZE:
		*((UINT *)buff) = MMCSD_MAX_BUFFER_SIZE;
		break;
	case CTRL_TRIM:
		/* code */
		break;
	case MMC_GET_TYPE:
		*((uint8_t *)buff) = mmcsd_card.card_type;
		break;
	case MMC_GET_CSD:
		if (mmcsd_command(9, 0x00, 0xFF))
		{
			return RES_ERROR;
		}

		mcu_clear_output(SD_SPI_CS);
		// waits for token
		while (spi_xmit(0xFF) != 0xFE && timeout)
			timeout--;

		// check for timeout
		if (!timeout)
		{
			return RES_NOTRDY;
		}

		mmcsd_response(buff, 16);
		break;
	case MMC_GET_CID:
		if (mmcsd_command(10, 0x00, 0xFF))
		{
			return RES_ERROR;
		}

		mcu_clear_output(SD_SPI_CS);
		// waits for token
		while (spi_xmit(0xFF) != 0xFE && timeout)
			timeout--;

		// check for timeout
		if (!timeout)
		{
			return RES_NOTRDY;
		}

		mmcsd_response(buff, 16);
		break;
	case MMC_GET_OCR:
		mmcsd_command(58, 0x40000000, 0x6F);
		mmcsd_response(buff, 4);
		if (CHECKBIT(((uint8_t *)buff)[2], 6))
		{
			mmcsd_command(58, 0x00, 0xFD);
			mmcsd_response(buff, 4);
		}
		break;
	case MMC_GET_SDSTAT:
		/* code */
		break;
	case ISDIO_READ:
		/* code */
		break;
	case ISDIO_WRITE:
		/* code */
		break;
	case ISDIO_MRITE:
		/* code */
		break;
	default:
		break;
	}

	return 0;
}
/*
int32 mmcsd_get_cardsize()
{
	uint8_t i, timeout = MMCSD_MAX_TIMEOUT;
	uint8_t block[16];
	int32 CARD_size;

	CARD_error.func = 0;
	CARD_error.error = 0;
	CARD_error.detail1 = 0;
	CARD_error.detail2 = 0;

	// envia o comando CID
	i = mmcsd_command(9, 0x00, 0xFF);
	if (i)
	{
		CARD_error.func = 4;
		CARD_error.error = 1;
		CARD_error.detail1 = i;
		CARD_error.detail2 = 0;
		return 0;
	}

	mcu_clear_output(SD_SPI_CS);
	// aguarda testemunho
	while (spi_xmit(0xFF) != 0xFE && timeout)
		timeout--;

	// verifica se ocorreu timeout
	if (!timeout)
	{
		CARD_error.func = 4;
		CARD_error.error = 2;
		CARD_error.detail1 = MMCSD_TIMEOUT;
		CARD_error.detail2 = 0;
		return 0;
	}

	for (i = 0; i < 16; i++)
		block[i] = spi_xmit(0xFF);

	spi_xmit(0xFF);
	spi_xmit(0xFF);

	// extrai informa��o
	CARD_size = (0x03 & block[6]) << 8;
	CARD_size |= block[7];
	CARD_size <<= 2;
	CARD_size |= (block[8] >> 6);
	i = (block[9] & 0x03) << 1;
	i |= CHECKBIT(block[10], 7);
	i += 2;
	i += (block[5] & 0x0F);
	CARD_size <<= i;

	mcu_set_output(SD_SPI_CS);
	return CARD_size;
}
*/
