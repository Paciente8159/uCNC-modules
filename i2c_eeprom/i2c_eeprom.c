/*
	Name: i2c_eeprom.c
	Description: Defines the i2c_eeprom interface.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 13-08-2024

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
#include "../softi2c.h"

#if (UCNC_MODULE_VERSION < 11100 || UCNC_MODULE_VERSION > 99999)
#error "This module is not compatible with the current version of µCNC"
#endif

#define SW_I2C 1
#define HW_I2C 2

#ifndef I2C_EEPROM_INTERFACE
#define I2C_EEPROM_INTERFACE HW_I2C
#endif

#ifndef I2C_EEPROM_ADDRESS
#define I2C_EEPROM_ADDRESS 0x50
#endif

#ifndef I2C_EEPROM_ACCESS_DELAY
#define I2C_EEPROM_ACCESS_DELAY 10
#endif

#ifndef I2C_EEPROM_RW_TIMEOUT
#define I2C_EEPROM_RW_TIMEOUT 200
#endif

#ifndef I2C_EEPROM_SIZE
#define I2C_EEPROM_SIZE 0x8000
#endif

#ifndef I2C_EEPROM_OFFSET
#define I2C_EEPROM_OFFSET 0
#endif

#ifndef I2C_EEPROM_PAGE_SIZE
#define I2C_EEPROM_PAGE_SIZE 32
#endif

// the maximum usable size
// this depends on the size of the setting you need
// depending on the setting the EEPROM can do ware rotation to extend EEPROM life time
#ifndef I2C_EEPROM_SETTINGS_SIZE
#define I2C_EEPROM_SETTINGS_SIZE 0x1000
#endif

#if (I2C_EEPROM_INTERFACE == SW_I2C)
#ifndef I2C_EEPROM_I2C_CLOCK
#define I2C_EEPROM_I2C_CLOCK DIN30
#endif
#ifndef I2C_EEPROM_I2C_DATA
#define I2C_EEPROM_I2C_DATA DIN31
#endif
SOFTI2C(eeprom_bus, 400000, I2C_EEPROM_I2C_CLOCK, I2C_EEPROM_I2C_DATA);
#define EEPROM_BUS &eeprom_bus
#elif (I2C_EEPROM_INTERFACE == HW_I2C)
#define EEPROM_BUS NULL
#endif

#ifdef ENABLE_SETTINGS_MODULES

static uint8_t i2c_eeprom_read_byte(uint16_t address, uint8_t *data)
{
	uint8_t address_bytes[2] = {(uint8_t)((address >> 8) & 0xFF),
															(address & 0xFF)};

	cnc_delay_ms(I2C_EEPROM_ACCESS_DELAY);

	if (softi2c_send(EEPROM_BUS, I2C_EEPROM_ADDRESS, address_bytes, 2, true, I2C_EEPROM_RW_TIMEOUT) == I2C_OK)
	{
		return softi2c_receive(EEPROM_BUS, I2C_EEPROM_ADDRESS, data, 1, I2C_EEPROM_RW_TIMEOUT);
	}

	return I2C_NOTOK;
}

static uint8_t i2c_eeprom_write_byte(uint16_t address, uint8_t data)
{
	uint8_t address_bytes[3] = {(uint8_t)((address >> 8) & 0xFF),
															(address & 0xFF), data};
	cnc_delay_ms(I2C_EEPROM_ACCESS_DELAY);
	return softi2c_send(EEPROM_BUS, I2C_EEPROM_ADDRESS, address_bytes, 3, true, I2C_EEPROM_RW_TIMEOUT);
}

static uint8_t i2c_eeprom_read(uint16_t address, uint8_t *data, uint16_t len)
{
	while (len--)
	{
		if (i2c_eeprom_read_byte(address++, data++) == I2C_NOTOK)
		{
			return I2C_NOTOK;
		}
	}

	return I2C_OK;
}

static uint8_t i2c_eeprom_write(uint16_t address, uint8_t *data, uint16_t len)
{
	while (len--)
	{
		uint8_t c = 0;
		i2c_eeprom_read_byte(address, &c);
		if (c != *data)
		{
			if (i2c_eeprom_write_byte(address, *data) == I2C_NOTOK)
			{
				return I2C_NOTOK;
			}
		}
		address++;
		data++;
	}

	return I2C_OK;
}

void nvm_start_read(uint16_t address)
{
	softi2c_config(EEPROM_BUS, 400000);
}

void nvm_start_write(uint16_t address)
{
	softi2c_config(EEPROM_BUS, 400000);
}

uint8_t nvm_getc(uint16_t address)
{
	uint8_t c = 255;
	if (i2c_eeprom_read_byte(address, &c) != I2C_OK)
	{
		g_settings_error |= SETTINGS_READ_ERROR;
	}

	return c;
}

void nvm_putc(uint16_t address, uint8_t c)
{
	uint8_t old_c;
	if (i2c_eeprom_read_byte(address, &old_c) != I2C_OK)
	{
		g_settings_error |= SETTINGS_WRITE_ERROR;
		return;
	}

	if (old_c != c)
	{
		if (i2c_eeprom_write_byte(address, c) != I2C_OK)
		{
			g_settings_error |= SETTINGS_WRITE_ERROR;
		}
	}
}

void nvm_end_read(void)
{
}

void nvm_end_write(void)
{
}
#endif

#ifdef ENABLE_MAIN_LOOP_MODULES
bool i2c_eeprom_write_failsafe(void *args)
{
	static uint32_t retry = 0;
	// the previous write failed
	if (g_settings_error & SETTINGS_WRITE_ERROR)
	{
		if (retry < mcu_millis())
		{
			retry = mcu_millis() + 5000; // retry every 5 seconds
			// try again
			g_settings_error &= ~SETTINGS_WRITE_ERROR;
			// save all settings and parameters again
			settings_save(SETTINGS_ADDRESS_OFFSET, (uint8_t *)&g_settings, (uint8_t)sizeof(settings_t));
			parser_parameters_save();
		}
	}

	return EVENT_CONTINUE;
}

CREATE_EVENT_LISTENER(cnc_dotasks, i2c_eeprom_write_failsafe);
#endif

DECL_MODULE(i2c_eeprom)
{
#ifdef ENABLE_MAIN_LOOP_MODULES
	ADD_EVENT_LISTENER(cnc_dotasks, i2c_eeprom_write_failsafe);
#else
#warning "Main loop extensions are not enabled. I2C EEPROM write failsafe will be disabled."
#endif
}