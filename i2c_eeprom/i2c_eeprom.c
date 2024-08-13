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

#ifndef I2C_EEPROM_RW_TIMEOUT
#define I2C_EEPROM_RW_TIMEOUT 200
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
bool i2c_eeprom_settings_load(void *args)
{
	settings_args_t *p = args;

	if (softi2c_send(EEPROM_BUS, I2C_EEPROM_ADDRESS, (uint8_t *)p->address, 2, false, I2C_EEPROM_RW_TIMEOUT) != I2C_OK)
	{
		return EVENT_CONTINUE;
	}
	if (softi2c_receive(EEPROM_BUS, I2C_EEPROM_ADDRESS, (uint8_t *)p->data, p->size, I2C_EEPROM_RW_TIMEOUT) != I2C_OK)
	{
		return EVENT_CONTINUE;
	}
	// read eeprom
	return EVENT_HANDLED;
}

CREATE_EVENT_LISTENER(settings_load, i2c_eeprom_settings_load);

bool i2c_eeprom_settings_save(void *args)
{
	settings_args_t *p = args;


	// write eeprom
	return EVENT_HANDLED;
}

CREATE_EVENT_LISTENER(settings_save, i2c_eeprom_settings_save);

bool i2c_eeprom_settings_erase(void *args)
{
	// erase eeprom
	return EVENT_HANDLED;
}

CREATE_EVENT_LISTENER(settings_erase, i2c_eeprom_settings_erase);
#endif

DECL_MODULE(i2c_eeprom)
{
	softi2c_config(EEPROM_BUS, 400000);
#ifdef ENABLE_SETTINGS_MODULES
	ADD_EVENT_LISTENER(settings_load, i2c_eeprom_settings_load);
	ADD_EVENT_LISTENER(settings_save, i2c_eeprom_settings_save);
	ADD_EVENT_LISTENER(settings_erase, i2c_eeprom_settings_erase);
#else
#warning "Settings extension not enabled. I2C EEPROM stored settings will not work."
#endif
}