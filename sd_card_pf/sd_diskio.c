/*
	Name: pf_mmpc.c
	Description: Modified avr_mmpc.c MMC example to work under µCNC.
	This adds SD card support via SPI

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 09-05-2023

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

/*-------------------------------------------------------------------------*/
/* PFF - Low level disk control module for AVR            (C)ChaN, 2014    */
/*-------------------------------------------------------------------------*/

#include "petit_fat_fs/diskio.h"

/*-------------------------------------------------------------------------*/
/* Platform dependent macros and functions needed to be modified           */
/*-------------------------------------------------------------------------*/

#include "../../cnc.h" /* Device specific include files */

#ifndef SD_CARD_USE_SW_SPI
#define SD_CARD_USE_HW_SPI
#endif

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
SOFTSPI(mmcsd_spi, 100000UL, 0, SD_SPI_SDO, SD_SPI_SDI, SD_SPI_CLK);
#define spi_xmit(c) softspi_xmit(&mmcsd_spi, c)
#define spi_speed(s) softspi_config(&mmcsd_spi, 0, s)
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
#define spi_xmit(c) mcu_spi_xmit(c)
#define spi_speed(s) mcu_spi_config(0, s)
#endif

#define CS_LOW() mcu_clear_output(SD_SPI_CS) /* Set CS low */
#define CS_HIGH() mcu_set_output(SD_SPI_CS)	 /* Set CS high */
#define IS_CS_LOW mcu_get_output(SD_SPI_CS)	 /* Test if CS is low */
#define FORWARD(d)							 /* Data streaming function (console out) */

#define dly_100us() mcu_delay_us(100)
#define init_spi(X) spi_speed(X) /* usi.S: Initialize MMC control ports */
#define xmit_spi(X) spi_xmit(X)			 /* usi.S: Send a byte to the MMC */
#define rcv_spi() spi_xmit(0xFF)		 /* usi.S: Send a 0xFF to the MMC and get the received byte */

/*--------------------------------------------------------------------------

   Module Private Functions

---------------------------------------------------------------------------*/

/* Definitions for MMC/SDC command */
#define CMD0 (0x40 + 0)	   /* GO_IDLE_STATE */
#define CMD1 (0x40 + 1)	   /* SEND_OP_COND (MMC) */
#define ACMD41 (0xC0 + 41) /* SEND_OP_COND (SDC) */
#define CMD8 (0x40 + 8)	   /* SEND_IF_COND */
#define CMD16 (0x40 + 16)  /* SET_BLOCKLEN */
#define CMD17 (0x40 + 17)  /* READ_SINGLE_BLOCK */
#define CMD24 (0x40 + 24)  /* WRITE_BLOCK */
#define CMD55 (0x40 + 55)  /* APP_CMD */
#define CMD58 (0x40 + 58)  /* READ_OCR */

/* Card type flags (CardType) */
#define CT_MMC 0x01	  /* MMC version 3 */
#define CT_SD1 0x02	  /* SD version 1 */
#define CT_SD2 0x04	  /* SD version 2+ */
#define CT_BLOCK 0x08 /* Block addressing */

static BYTE CardType;

/*-----------------------------------------------------------------------*/
/* Send a command packet to MMC                                          */
/*-----------------------------------------------------------------------*/

static BYTE send_cmd(
	BYTE cmd, /* 1st byte (Start + Index) */
	DWORD arg /* Argument (32 bits) */
)
{
	BYTE n, res;

	if (cmd & 0x80)
	{ /* ACMD<n> is the command sequense of CMD55-CMD<n> */
		cmd &= 0x7F;
		res = send_cmd(CMD55, 0);
		if (res > 1)
			return res;
	}

	/* Select the card */
	CS_HIGH();
	rcv_spi();
	CS_LOW();
	rcv_spi();

	/* Send a command packet */
	xmit_spi(cmd);				 /* Start + Command index */
	xmit_spi((BYTE)(arg >> 24)); /* Argument[31..24] */
	xmit_spi((BYTE)(arg >> 16)); /* Argument[23..16] */
	xmit_spi((BYTE)(arg >> 8));	 /* Argument[15..8] */
	xmit_spi((BYTE)arg);		 /* Argument[7..0] */
	n = 0x01;					 /* Dummy CRC + Stop */
	if (cmd == CMD0)
		n = 0x95; /* Valid CRC for CMD0(0) */
	if (cmd == CMD8)
		n = 0x87; /* Valid CRC for CMD8(0x1AA) */
	xmit_spi(n);

	/* Receive a command response */
	n = 10; /* Wait for a valid response in timeout of 10 attempts */
	do
	{
		res = rcv_spi();
	} while ((res & 0x80) && --n);

	return res; /* Return with the response value */
}

/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(BYTE pdrv)
{
	BYTE n, cmd, ty, ocr[4];
	UINT tmr;

#if PF_USE_WRITE
	if (CardType != 0 && IS_CS_LOW)
		disk_writep(0, 0); /* Finalize write process if it is in progress */
#endif
	init_spi(100000UL); /* Initialize ports to control MMC */
	CS_HIGH();
	for (n = 10; n; n--)
		rcv_spi(); /* 80 dummy clocks with CS=H */

	ty = 0;
	if (send_cmd(CMD0, 0) == 1)
	{ /* GO_IDLE_STATE */
		if (send_cmd(CMD8, 0x1AA) == 1)
		{ /* SDv2 */
			for (n = 0; n < 4; n++)
				ocr[n] = rcv_spi(); /* Get trailing return value of R7 resp */
			if (ocr[2] == 0x01 && ocr[3] == 0xAA)
			{ /* The card can work at vdd range of 2.7-3.6V */
				for (tmr = 10000; tmr && send_cmd(ACMD41, 1UL << 30); tmr--)
					dly_100us(); /* Wait for leaving idle state (ACMD41 with HCS bit) */
				if (tmr && send_cmd(CMD58, 0) == 0)
				{ /* Check CCS bit in the OCR */
					for (n = 0; n < 4; n++)
						ocr[n] = rcv_spi();
					ty = (ocr[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2; /* SDv2 (HC or SC) */
				}
			}
		}
		else
		{ /* SDv1 or MMCv3 */
			if (send_cmd(ACMD41, 0) <= 1)
			{
				ty = CT_SD1;
				cmd = ACMD41; /* SDv1 */
			}
			else
			{
				ty = CT_MMC;
				cmd = CMD1; /* MMCv3 */
			}
			for (tmr = 10000; tmr && send_cmd(cmd, 0); tmr--)
				dly_100us(); /* Wait for leaving idle state */
			if (!tmr || send_cmd(CMD16, 512) != 0)
			{ /* Set R/W block length to 512 */
				ty = 0;
			}
		}
	}
	CardType = ty;
	CS_HIGH();
	rcv_spi();
	//spi high speed
	init_spi(10000000UL);
	return ty ? 0 : STA_NOINIT;
}

/*-----------------------------------------------------------------------*/
/* Read partial sector                                                   */
/*-----------------------------------------------------------------------*/

DRESULT disk_readp(
	BYTE *buff,	  /* Pointer to the read buffer (NULL:Forward to the stream) */
	DWORD sector, /* Sector number (LBA) */
	UINT offset,  /* Byte offset to read from (0..511) */
	UINT count	  /* Number of bytes to read (ofs + cnt mus be <= 512) */
)
{
	DRESULT res;
	BYTE rc;
	UINT bc;

	if (!(CardType & CT_BLOCK))
		sector *= 512; /* Convert to byte address if needed */

	res = RES_ERROR;
	if (send_cmd(CMD17, sector) == 0)
	{ /* READ_SINGLE_BLOCK */

		bc = 40000; /* Time counter */
		do
		{ /* Wait for data block */
			rc = rcv_spi();
		} while (rc == 0xFF && --bc);

		if (rc == 0xFE)
		{ /* A data block arrived */

			bc = 512 + 2 - offset - count; /* Number of trailing bytes to skip */

			/* Skip leading bytes in the sector */
			while (offset--)
				rcv_spi();

			/* Receive a part of the sector */
			if (buff)
			{ /* Store data to the memory */
				do
				{
					*buff++ = rcv_spi();
				} while (--count);
			}
			else
			{ /* Forward data to the outgoing stream */
				do
				{
					FORWARD(rcv_spi());
				} while (--count);
			}

			/* Skip trailing bytes in the sector and block CRC */
			do
				rcv_spi();
			while (--bc);

			res = RES_OK;
		}
	}

	CS_HIGH();
	rcv_spi();

	return res;
}

/*-----------------------------------------------------------------------*/
/* Write partial sector                                                  */
/*-----------------------------------------------------------------------*/

#if PF_USE_WRITE
DRESULT disk_writep(
	const BYTE *buff, /* Pointer to the bytes to be written (NULL:Initiate/Finalize sector write) */
	DWORD sc		  /* Number of bytes to send, Sector number (LBA) or zero */
)
{
	DRESULT res;
	UINT bc;
	static UINT wc; /* Sector write counter */

	res = RES_ERROR;

	if (buff)
	{ /* Send data bytes */
		bc = sc;
		while (bc && wc)
		{ /* Send data bytes to the card */
			xmit_spi(*buff++);
			wc--;
			bc--;
		}
		res = RES_OK;
	}
	else
	{
		if (sc)
		{ /* Initiate sector write process */
			if (!(CardType & CT_BLOCK))
				sc *= 512; /* Convert to byte address if needed */
			if (send_cmd(CMD24, sc) == 0)
			{ /* WRITE_SINGLE_BLOCK */
				xmit_spi(0xFF);
				xmit_spi(0xFE); /* Data block header */
				wc = 512;		/* Set byte counter */
				res = RES_OK;
			}
		}
		else
		{ /* Finalize sector write process */
			bc = wc + 2;
			while (bc--)
				xmit_spi(0); /* Fill left bytes and CRC with zeros */
			if ((rcv_spi() & 0x1F) == 0x05)
			{ /* Receive data resp and wait for end of write process in timeout of 500ms */
				for (bc = 5000; rcv_spi() != 0xFF && bc; bc--)
				{ /* Wait for ready */
					dly_100us();
				}
				if (bc)
					res = RES_OK;
			}
			CS_HIGH();
			rcv_spi();
		}
	}

	return res;
}
#endif
