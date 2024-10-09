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

	// uint8_t address_bytes[2] = {(uint8_t)((address >> 8) & 0xFF),
	// 														(address & 0xFF)};

	// if (softi2c_send(EEPROM_BUS, I2C_EEPROM_ADDRESS, address_bytes, 2, true, I2C_EEPROM_RW_TIMEOUT) == I2C_OK)
	// {
	// 	return softi2c_receive(EEPROM_BUS, I2C_EEPROM_ADDRESS, data, len, I2C_EEPROM_RW_TIMEOUT);
	// }

	// return I2C_NOTOK;
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

	// // address byte aligned to the eeprom page size
	// uint16_t write_address = (address & ~(I2C_EEPROM_PAGE_SIZE - 1));
	// while (len)
	// {
	// 	uint8_t buffer[I2C_EEPROM_PAGE_SIZE];
	// 	uint8_t write_offset = (address - write_address);
	// 	uint16_t write_len = MIN(I2C_EEPROM_PAGE_SIZE - write_offset, len);

	// 	// loads a page of eeprom to memory and compares it to the data to check if it's dirty or not
	// 	i2c_eeprom_read(write_address, buffer, write_len);
	// 	len -= write_len;
	// 	write_address += write_offset;
	// 	uint8_t *ptr = &buffer[write_offset];
	// 	while (write_len)
	// 	{
	// 		if (*ptr != *data)
	// 		{
	// 			uint8_t eepromdata[3] = {(uint8_t)((write_address >> 8) & 0xFF),
	// 															 (write_address & 0xFF), *data};
	// 			if (softi2c_send(EEPROM_BUS, I2C_EEPROM_ADDRESS, eepromdata, 3, true, I2C_EEPROM_RW_TIMEOUT) != I2C_OK)
	// 			{
	// 				return I2C_NOTOK;
	// 			}
	// 		}
	// 		ptr++;
	// 		data++;
	// 		write_len--;
	// 		write_address++;
	// 	}
	// }

	// return I2C_NOTOK;
}

bool i2c_eeprom_settings_load(void *args)
{
	settings_args_t *p = args;
	p->error = 1;

	if (i2c_eeprom_read(p->address, (uint8_t *)p->data, p->size) != I2C_OK)
	{
		protocol_send_feedback(__romstr__("External EEPROM read error"));
	}
	else
	{
		p->error = 0;
	}

	// read eeprom
	return EVENT_HANDLED;
}

CREATE_EVENT_LISTENER(settings_load, i2c_eeprom_settings_load);

bool i2c_eeprom_settings_save(void *args)
{
	settings_args_t *p = args;
	p->error = 1;

	if (i2c_eeprom_write(p->address, (uint8_t *)p->data, p->size) != I2C_OK)
	{
		protocol_send_feedback(__romstr__("External EEPROM write error"));
	}
	else
	{
		p->error = 0;
	}

	return EVENT_HANDLED;
}

CREATE_EVENT_LISTENER(settings_save, i2c_eeprom_settings_save);
#endif

DECL_MODULE(i2c_eeprom)
{
	softi2c_config(EEPROM_BUS, 400000);

	// test eeprom
	uint8_t data;
	if (i2c_eeprom_read(I2C_EEPROM_OFFSET, &data, 1) != I2C_OK)
	{
		// error prevents initialization
		protocol_send_feedback(__romstr__("External EEPROM initialization error"));
		return;
	}

#ifdef ENABLE_SETTINGS_MODULES
	ADD_EVENT_LISTENER(settings_load, i2c_eeprom_settings_load);
	ADD_EVENT_LISTENER(settings_save, i2c_eeprom_settings_save);
#else
#warning "Settings extension not enabled. I2C EEPROM stored settings will not work."
#endif
}