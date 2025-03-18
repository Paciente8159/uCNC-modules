/*
	Name: spi_flash.c
	Description: Defines the implements non volatile memory in an SPI flash W25Qxx.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 14-03-2025

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "../../cnc.h"
#include "../softspi.h"

#ifndef RAM_ONLY_SETTINGS

#define NVM_STORAGE_SIZE_ALIGNED ((((NVM_STORAGE_SIZE - 1) >> 8) + 1) << 8)
#define FLASH_VALUE(X) (X ^ 0xFF)

#define FLASH_SPI_SW_SPI 1
#define FLASH_SPI_HW_SPI 2
#define FLASH_SPI_HW_SPI2 4

#ifndef FLASH_SPI_FREQ
#define FLASH_SPI_FREQ 10000000UL
#endif

#ifndef FLASH_SPI_INTERFACE
#define FLASH_SPI_INTERFACE FLASH_SPI_SW_SPI
#endif

#if (FLASH_SPI_INTERFACE == FLASH_SPI_SW_SPI)
#ifndef FLASH_SPI_CLK
#define FLASH_SPI_CLK DOUT30
#endif
#ifndef FLASH_SPI_SDO
#define FLASH_SPI_SDO DOUT29
#endif
#ifndef FLASH_SPI_SDI
#define FLASH_SPI_SDI DIN29
#endif
#ifndef FLASH_SPI_CS
#define FLASH_SPI_CS SPI_CS
#endif
SOFTSPI(flash_spi, FLASH_SPI_FREQ, 0, FLASH_SPI_SDO, FLASH_SPI_SDI, FLASH_SPI_CLK);
#elif (FLASH_SPI_INTERFACE == FLASH_SPI_HW_SPI)
#ifndef FLASH_SPI_CS
#define FLASH_SPI_CS SPI_CS
#endif
HARDSPI(flash_spi, FLASH_SPI_FREQ, 0, mcu_spi_port);
#elif (SD_CARD_INTERFACE == FLASH_SPI_HW_SPI2)
#ifndef SD_SPI_CS
#define SD_SPI_CS SPI2_CS
#endif
HARDSPI(mmcsd_spi, FLASH_SPI_FREQ, 0, mcu_spi2_port);
#else
#warning "Flash SPI interface is not defined. Flash NVM will not be available."
#endif

#define SPI_FLASH_SEC_SIZE 0x10000UL // using a 64k sector
#define SPI_FLASH_PAGE_SIZE 0x100UL	 // write page size

// the start offset
// the start offset must be a multiple of the SPI_FLASH_SEC_SIZE
// if there is existing data on the flash this can be use to force the EEPROM to be emulated in the ending part of the flash
#ifndef SPI_FLASH_SEC_OFFSET
#define SPI_FLASH_SEC_OFFSET 0x00000000
#endif

#if (SPI_FLASH_SEC_OFFSET & (SPI_FLASH_SEC_SIZE - 1))
#error "The NVM flash offset is not a multiple of the SPI_FLASH_SEC_SIZE"
#endif

#if (NVM_STORAGE_SIZE_ALIGNED > SPI_FLASH_SEC_SIZE)
#error "NVM cannot exceed Flash sector size"
#else
#define EEPROM_DATA_SIZE NVM_STORAGE_SIZE_ALIGNED // loads a full page into RAM
#endif

static uint32_t flash_capacity;

#define EEPROM_MODIFIED 0x01
#define EEPROM_NEWPAGE_REQUIRED 0x02

static uint8_t eeprom_data[EEPROM_DATA_SIZE]; // loads a full page into RAM
static uint32_t eeprom_current_address;
static bool eeprom_initialized;

#define CMD_FLASH_STATUS 0x05
#define CMD_FLASH_RESET_EN 0x66
#define CMD_FLASH_RESET 0x99
#define CMD_FLASH_READ 0x03
#define CMD_FLASH_WRITE_ENABLE 0x06
#define CMD_FLASH_ERASE_64K 0xD8
#define CMD_FLASH_DEV_ID 0x90
#define CMD_FLASH_PAGE_PROG 0x02

#define __ADDRESS3__(X) ((uint8_t)((X >> 24) & 0xff))
#define __ADDRESS2__(X) ((uint8_t)((X >> 16) & 0xff))
#define __ADDRESS1__(X) ((uint8_t)((X >> 8) & 0xff))
#define __ADDRESS0__(X) ((uint8_t)(X & 0xff))
#define ADDRESS(X, Y) __ADDRESS##Y##__(X)

static inline void norflash_send_cmd(uint8_t cmd)
{
	softspi_start(&flash_spi);
	io_clear_output(FLASH_SPI_CS);
	mcu_delay_us(1);
	softspi_xmit(&flash_spi, cmd);
	mcu_delay_us(1);
	io_set_output(FLASH_SPI_CS);
	softspi_stop(&flash_spi);
}

static inline uint8_t norflash_get_status(void)
{
	softspi_start(&flash_spi);
	io_clear_output(FLASH_SPI_CS);
	mcu_delay_us(1);
	softspi_xmit(&flash_spi, CMD_FLASH_STATUS);
	uint8_t res = softspi_xmit(&flash_spi, 0xFF);
	mcu_delay_us(1);
	io_set_output(FLASH_SPI_CS);
	softspi_stop(&flash_spi);
	return res;
}

static inline void norflash_write_enable(void)
{
	do
	{
		norflash_send_cmd(CMD_FLASH_WRITE_ENABLE);
		if ((norflash_get_status() & 0x03) == 0x02)
		{
			break;
		}
	} while (1);
}

static inline void norflash_send_cmd_address(uint8_t cmd, uint32_t address, uint8_t stop)
{
	softspi_start(&flash_spi);
	uint8_t data[4] = {cmd, ADDRESS(address, 2), ADDRESS(address, 1), ADDRESS(address, 0)};
	io_clear_output(FLASH_SPI_CS);
	mcu_delay_us(1);
	softspi_bulk_xmit(&flash_spi, data, NULL, 4);
	mcu_delay_us(1);
	if (stop)
	{
		io_set_output(FLASH_SPI_CS);
		softspi_stop(&flash_spi);
	}
}

static inline void norflash_send_cmd_data(uint8_t cmd, uint32_t address, uint8_t *data, uint16_t len)
{
	norflash_send_cmd_address(cmd, address, 0);
	softspi_bulk_xmit(&flash_spi, data, NULL, len);
	io_set_output(FLASH_SPI_CS); // terminate command
	softspi_stop(&flash_spi);
}

static inline void norflash_get_cmd_data(uint8_t cmd, uint32_t address, uint8_t *data, uint16_t len)
{
	norflash_send_cmd_address(cmd, address, 0);
	softspi_bulk_xmit(&flash_spi, data, data, len);
	io_set_output(FLASH_SPI_CS); // terminate command
	softspi_stop(&flash_spi);
}

static inline uint32_t norflash_get_size(void)
{
	uint8_t data[6] = {CMD_FLASH_DEV_ID, 0, 0, 0, 0, 0};
	io_clear_output(FLASH_SPI_CS);
	mcu_delay_us(1);
	softspi_bulk_xmit(&flash_spi, data, data, 6);
	mcu_delay_us(1);
	io_set_output(FLASH_SPI_CS);
	switch (data[5])
	{
	case 0x12:
		return (4UL << 17);
	case 0x13:
		return (8UL << 17);
	case 0x14:
		return (16UL << 17);
	case 0x15:
		return (32UL << 17);
	case 0x16:
		return (64UL << 17);
	case 0x17:
		return (128UL << 17);
	case 0x18:
		return (256UL << 17);
	default:
		// unkowned assume one single 64K sector
		return (SPI_FLASH_SEC_OFFSET + (64UL << 10));
	}

	return 0;
}

static inline void norflash_erase_sector(uint32_t address)
{
	address = address & ~(SPI_FLASH_SEC_SIZE - 1);
	norflash_write_enable();
	norflash_send_cmd_address(CMD_FLASH_ERASE_64K, address, 1);
	while ((norflash_get_status() & 0x01))
	{
		// while flash is busy
		mcu_dotasks();
	}
}

static inline uint8_t norflash_read(uint32_t address)
{
	while (norflash_get_status() & 0x1)
		; // flash busy
	norflash_get_cmd_data(CMD_FLASH_READ, address, eeprom_data, EEPROM_DATA_SIZE);
}

static inline uint8_t norflash_write(uint32_t address)
{
	if (address >= flash_capacity)
	{
		address = SPI_FLASH_SEC_OFFSET;
	}
	if (!(address & (SPI_FLASH_SEC_SIZE - 1)))
	{
		// at start of sector erase it
		norflash_erase_sector(SPI_FLASH_SEC_OFFSET);
	}

	eeprom_current_address = address;

	int32_t remaining = EEPROM_DATA_SIZE;
	uint8_t *ptr = eeprom_data;
	while (remaining > 0)
	{
		norflash_write_enable();
		norflash_send_cmd_data(CMD_FLASH_PAGE_PROG, address, ptr, MIN(remaining, SPI_FLASH_PAGE_SIZE));
		while ((norflash_get_status() & 0x01))
		{
			// while flash is busy
			cnc_dotasks();
		}
		remaining -= SPI_FLASH_PAGE_SIZE;
		ptr += SPI_FLASH_PAGE_SIZE;
		address += SPI_FLASH_PAGE_SIZE;
	}
}

bool norflash_reset(void *args)
{
	softspi_start(&flash_spi);
	io_clear_output(FLASH_SPI_CS);
	softspi_xmit(&flash_spi, CMD_FLASH_RESET_EN);
	io_set_output(FLASH_SPI_CS);
	io_clear_output(FLASH_SPI_CS);
	softspi_xmit(&flash_spi, CMD_FLASH_RESET);
	io_set_output(FLASH_SPI_CS);
	cnc_delay_ms(1);
	flash_capacity = norflash_get_size();
	softspi_stop(&flash_spi);
	uint32_t eeprom_start = 0;

	// search the last sector with data
	for (uint32_t i = SPI_FLASH_SEC_OFFSET; i < flash_capacity; i += SPI_FLASH_SEC_SIZE)
	{
		uint8_t c = 0xFF;
		norflash_get_cmd_data(CMD_FLASH_READ, i, &c, 1);
		c = FLASH_VALUE(c);
		if (c == 'V')
		{
			eeprom_start = i;
		}
		else
		{
			break;
		}
	}

	// search the last eeprom page withing the sector
	for (uint32_t i = eeprom_start; i < (eeprom_start + SPI_FLASH_SEC_SIZE); i += EEPROM_DATA_SIZE)
	{
		uint8_t c = 0xFF;
		norflash_get_cmd_data(CMD_FLASH_READ, i, &c, 1);
		c = FLASH_VALUE(c);
		if (c == 'V')
		{
			eeprom_start = i;
			eeprom_initialized = true;
		}
		else
		{
			eeprom_initialized |= (eeprom_start != SPI_FLASH_SEC_OFFSET);
			break;
		}
	}

	eeprom_current_address = eeprom_start;
	if (eeprom_initialized)
	{
		norflash_read(eeprom_start);
		// clear the read error
		g_settings_error &= ~SETTINGS_READ_ERROR;
		// reload all stored settings
		settings_init();
		// reload all non volatile parser parameters
		parser_parameters_load();
		// reinitialize kinematics since some kinematics depend on settings data
		kinematics_init();
	}

	return EVENT_CONTINUE;
}

CREATE_EVENT_LISTENER_WITHLOCK(cnc_reset, norflash_reset, LISTENER_SWSPI_LOCK);

void nvm_start_read(uint16_t address)
{
	if (eeprom_initialized)
	{
		norflash_read(eeprom_current_address);
	}
}
void nvm_start_write(uint16_t address)
{
}
uint8_t nvm_getc(uint16_t address) { return FLASH_VALUE(eeprom_data[address]); }
void nvm_putc(uint16_t address, uint8_t c)
{
	eeprom_data[address] = FLASH_VALUE(c);
}
void nvm_end_read(void) {}
void nvm_end_write(void)
{
	if (eeprom_initialized)
	{
		norflash_write(eeprom_current_address + EEPROM_DATA_SIZE);
	}
	else
	{
		norflash_write(SPI_FLASH_SEC_OFFSET);
	}

	eeprom_initialized = true;
}

DECL_MODULE(spi_flash)
{
	norflash_reset(NULL);
#ifdef ENABLE_MAIN_LOOP_MODULES
	// Makes the event handler 'mycustom_system_cmd' listen to the event 'grbl_cmd'
	ADD_EVENT_LISTENER(cnc_reset, norflash_reset);
#else
// just a warning in case you disabled the PARSER_MODULES option on build
#warning "Main loop modules are not enabled. Flash settings will not be reloaded on reset."
#endif
}
#endif