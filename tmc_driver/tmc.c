/*
	Name: tmc.c
	Description: Minimalistic generic driver library for Trinamic stepper drivers.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 21-03-2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../cnc.h"
#include "tmc.h"

#ifndef TMC_MAX_WRITE_RETRIES
#define TMC_MAX_WRITE_RETRIES 3
#endif

// uncomment this to allow write to any register
// #define TMC_UNSAFE_MODE

uint8_t tmc_crc8(uint8_t *data, uint8_t len)
{
	int i, j;
	uint8_t crc = 0; // CRC located in last byte of message
	uint8_t currentByte;
	for (i = 0; i < len; i++)
	{												 // Execute for all bytes of a message
		currentByte = data[i]; // Retrieve a byte to be sent from Array
		for (j = 0; j < 8; j++)
		{
			if ((crc >> 7) ^ (currentByte & 0x01)) // update CRC based result of XOR operation
			{
				crc = (crc << 1) ^ 0x07;
			}
			else
			{
				crc = (crc << 1);
			}
			currentByte = currentByte >> 1;
		} // for CRC bit
	} // for message byte

	return crc;
}

uint32_t tmc_read_register(tmc_driver_t *driver, uint8_t address)
{
	if (!(driver->rw))
	{
		return TMC_READ_ERROR;
	}

	// write only registers
	// return shadow register
	switch (address)
	{
	case IHOLD_IRUN:
		return driver->reg.ihold_irun;
	case TPWMTHRS:
		return driver->reg.tpwmthrs;
	case TCOOLTHRS:
		return driver->reg.tcoolthrs;
	case SGTHRS:
		switch (driver->type)
		{
		case 2209:
		case 2226:
			return driver->reg.sgthrs;
		default:
			return 0;
		}
	case COOLCONF:
		switch (driver->type)
		{
		case 2130:
			return driver->reg.coolconf;
		default:
			return 0;
		}
		break;
	case TPOWERDOWN:
		return driver->reg.tpowerdown;
	}

	uint8_t data[8];
	uint8_t crc = 0;
	uint32_t result = TMC_READ_ERROR;
	switch (driver->type)
	{
	case 2202:
	case 2208:
	case 2225:
		driver->slave = 0;
	case 2209:
	case 2226:
		/* code */
		data[0] = 0x05;
		data[1] = driver->slave;
		data[2] = address & 0x7F;
		data[3] = tmc_crc8(data, 3);
		driver->rw(data, 4, 8);
		crc = tmc_crc8(data, 7);
		if (data[0] != 0x05)
		{
			return TMC_READ_ERROR;
		}
		if (data[1] != 0xFF)
		{
			return TMC_READ_ERROR;
		}
		if (data[2] != address)
		{
			return TMC_READ_ERROR;
		}
		if (crc != data[7])
		{
			return TMC_READ_ERROR;
		}
		result = ((uint32_t)data[3] << 24) | ((uint32_t)data[4] << 16) | (data[5] << 8) | data[6];
		break;
	case 2130:
		data[0] = address & 0x7F;
		data[1] = 0;
		data[2] = 0;
		data[3] = 0;
		data[4] = 0;
		driver->rw(data, 5, 5);
		result = ((uint32_t)data[1] << 24) | ((uint32_t)data[2] << 16) | (data[3] << 8) | data[4];
	}

	return result;
}

uint32_t tmc_write_register(tmc_driver_t *driver, uint8_t address, uint32_t val)
{
	if (!(driver->rw))
	{
		return TMC_WRITE_ERROR;
	}

	switch (address)
	{
	case IHOLD_IRUN:
		if (driver->reg.ihold_irun == val)
		{
			return val;
		}
		break;
	case TPWMTHRS:
		if (driver->reg.tpwmthrs == val)
		{
			return val;
		}
		break;
	case TCOOLTHRS:
		if (driver->reg.tcoolthrs == val)
		{
			return val;
		}
		break;
	case SGTHRS:
		switch (driver->type)
		{
		case 2209:
		case 2226:
			if (driver->reg.sgthrs == val)
			{
				return val;
			}
			break;
		default:
			return TMC_WRITE_ERROR;
		}
		break;
	case COOLCONF:
		switch (driver->type)
		{
		case 2130:
			if (driver->reg.coolconf == val)
			{
				return val;
			}
			break;
		default:
			return TMC_WRITE_ERROR;
		}
		break;
	case TPOWERDOWN:
		if (driver->reg.tpowerdown == val)
		{
			return val;
		}
		break;
		// restricts write commands build it on this driver (prevents write commands to other addresses)
#ifndef TMC_UNSAFE_MODE
	case GCONF:
	case CHOPCONF:
	case PWMCONF:
		break;
	default:
		return TMC_WRITE_ERROR;
#endif
	}

	int8_t retries = TMC_MAX_WRITE_RETRIES;

	do
	{
		uint8_t data[8];
		switch (driver->type)
		{
		case 2202:
		case 2208:
		case 2225:
			driver->slave = 0;
		case 2209:
		case 2226:
			/* code */
			data[0] = 0x05;
			data[1] = driver->slave;
			data[2] = address | 0x80;
			data[3] = (val >> 24) & 0xFF;
			data[4] = (val >> 16) & 0xFF;
			data[5] = (val >> 8) & 0xFF;
			data[6] = (val) & 0xFF;
			data[7] = tmc_crc8(data, 7);
			driver->rw(data, 8, 0);
			break;
		case 2130:
			data[0] = address | 0x80;
			data[4] = (uint8_t)(val & 0xFF);
			val >>= 8;
			data[3] = (uint8_t)(val & 0xFF);
			val >>= 8;
			data[2] = (uint8_t)(val & 0xFF);
			val >>= 8;
			data[1] = (uint8_t)(val & 0xFF);
			driver->rw(data, 5, 5);
			break;
		default:
			return TMC_WRITE_ERROR;
		}

		// checks if write was executed
		uint8_t cnt = tmc_read_register(driver, IFCNT);
		if (driver->reg.ifcnt != cnt)
		{
			driver->reg.ifcnt = cnt;
			switch (address)
			{
			case IHOLD_IRUN:
				driver->reg.ihold_irun = val;
				break;
			case TPWMTHRS:
				driver->reg.tpwmthrs = val;
				break;
			case TCOOLTHRS:
				driver->reg.tcoolthrs = val;
				break;
			case SGTHRS:
				driver->reg.sgthrs = val;
				break;
			case COOLCONF:
				driver->reg.coolconf = val;
				break;
			case TPOWERDOWN:
				driver->reg.tpowerdown = val;
				break;
			}

			return val;
		}
	} while (--retries > 0);

	return TMC_WRITE_ERROR;
}

// specific initializations
// based on Marlin
static void tmc22xx_init(tmc_driver_t *driver)
{
	uint32_t gconf = 0;
	gconf = tmc_read_register(driver, GCONF);

	SET_FIELD(gconf, GCONF_PDN_DISABLE_MASK, GCONF_PDN_DISABLE_SHIFT, 1);						// Use UART
	SET_FIELD(gconf, GCONF_MSTEP_REG_SELECT_MASK, GCONF_MSTEP_REG_SELECT_SHIFT, 1); // Select microsteps with UART
	SET_FIELD(gconf, GCONF_I_SCALE_ANALOG_MASK, GCONF_I_SCALE_ANALOG_SHIFT, 0);			// disable I_scale_analog

	if (driver->reg.tpwmthrs == 0)
	{
		SET_FIELD(gconf, GCONF_EN_SPREADCYCLE_MASK, GCONF_EN_SPREADCYCLE_SHIFT, 1); // set spreadcycle
	}
	else
	{
		SET_FIELD(gconf, GCONF_EN_SPREADCYCLE_MASK, GCONF_EN_SPREADCYCLE_SHIFT, 0); // set cyclechop
	}
	tmc_write_register(driver, GCONF, gconf);

	uint32_t chopconf = 0;
	chopconf = tmc_read_register(driver, CHOPCONF);
	SET_FIELD(chopconf, CHOPCONF_TBL_MASK, CHOPCONF_TBL_SHIFT, 1); // tbl = 0b01 blank_time = 24
	/**
	 * {toff, hend, hstrt}
	 * #define CHOPPER_DEFAULT_12V  { 3, -1, 1 }
	 * #define CHOPPER_DEFAULT_19V  { 4,  1, 1 }
	 * #define CHOPPER_DEFAULT_24V  { 4,  2, 1 }
	 * #define CHOPPER_DEFAULT_36V  { 5,  2, 4 }
	 * #define CHOPPER_PRUSAMK3_24V { 3, -2, 6 }
	 * #define CHOPPER_MARLIN_119   { 5,  2, 3 }
	 * #define CHOPPER_09STEP_24V   { 3, -1, 5 }
	 * **/
	// using 24V by default
	SET_FIELD(chopconf, CHOPCONF_TOFF_MASK, CHOPCONF_TOFF_SHIFT, 4);			 // toff
	SET_FIELD(chopconf, CHOPCONF_HEND_MASK, CHOPCONF_HEND_SHIFT, (2 + 3)); // hend
	SET_FIELD(chopconf, CHOPCONF_HSTRT_MASK, CHOPCONF_HSTRT_SHIFT, (1 - 1));
	tmc_write_register(driver, CHOPCONF, chopconf);
}

void tmc_init(tmc_driver_t *driver, tmc_driver_setting_t *settings)
{
	driver->reg.ifcnt = tmc_read_register(driver, IFCNT);
	switch (driver->type)
	{
	case 2202:
	case 2208:
	case 2225:
	case 2209:
	case 2226:
		tmc22xx_init(driver);
		break;
	}

	tmc_set_current(driver, settings);
	tmc_set_microstep(driver, settings);
	tmc_write_register(driver, TPOWERDOWN, 128);
	tmc_set_stealthchop(driver, settings);
	tmc_set_stepinterpol(driver, settings);
	switch (driver->type)
	{
	case 2209:
	case 2130:
		tmc_set_stallguard(driver, settings);
		break;
	}

	if (driver->init)
	{
		driver->init();
	}
}

float tmc_get_current(tmc_driver_t *driver, tmc_driver_setting_t *settings)
{
	uint32_t chopconf = 0;
	chopconf = tmc_read_register(driver, CHOPCONF);
	if (chopconf == TMC_READ_ERROR)
	{
		return -1;
	}

	uint8_t irun = (uint8_t)(GET_FIELD(driver->reg.ihold_irun, IHOLD_IRUN_IRUN_MASK, IHOLD_IRUN_IRUN_SHIFT));
	return (float)(irun + 1) / 32.0 * ((GET_FIELD(chopconf, CHOPCONF_VSENSE_MASK, CHOPCONF_VSENSE_SHIFT)) ? 0.180 : 0.325) / (settings->rsense + 0.02) / 1.41421 * 1000;
}

void tmc_set_current(tmc_driver_t *driver, tmc_driver_setting_t *settings)
{
	uint8_t currentsense = (uint8_t)roundf(32.0f * 1.41421f * settings->rms_current / 1000.0f * (settings->rsense + 0.02f) / 0.325f) - 1;
	// If Current Scale is too low, turn on high sensitivity R_sense and calculate again
	uint32_t chopconf = 0;
	chopconf = tmc_read_register(driver, CHOPCONF);

	if (chopconf == TMC_READ_ERROR)
	{
		return;
	}

	if (currentsense < 16)
	{
		// enable vsense
		SET_FIELD(chopconf, CHOPCONF_VSENSE_MASK, CHOPCONF_VSENSE_SHIFT, 1);
		currentsense = (uint8_t)roundf(32.0f * 1.41421f * settings->rms_current / 1000.0f * (settings->rsense + 0.02f) / 0.180f) - 1;
	}
	else
	{
		// If CS >= 16, turn off high_sense_r if it's currently ON
		// disable vsense
		//  enable vsense
		SET_FIELD(chopconf, CHOPCONF_VSENSE_MASK, CHOPCONF_VSENSE_SHIFT, 0);
	}

	tmc_write_register(driver, CHOPCONF, chopconf);

	// rms current
	uint32_t ihold_irun = driver->reg.ihold_irun;
	SET_FIELD(ihold_irun, IHOLD_IRUN_IRUN_MASK, IHOLD_IRUN_IRUN_SHIFT, currentsense);
	SET_FIELD(ihold_irun, IHOLD_IRUN_IHOLD_MASK, IHOLD_IRUN_IHOLD_SHIFT, (uint8_t)(currentsense * settings->ihold_mul));
	SET_FIELD(ihold_irun, IHOLD_IRUN_IHOLDDELAY_MASK, IHOLD_IRUN_IHOLDDELAY_SHIFT, (uint8_t)(settings->ihold_mul));
	tmc_write_register(driver, IHOLD_IRUN, ihold_irun);
}

int32_t tmc_get_microstep(tmc_driver_t *driver)
{
	uint32_t chopconf = 0;
	chopconf = tmc_read_register(driver, CHOPCONF);

	if (chopconf == TMC_READ_ERROR)
	{
		return -1;
	}

	switch ((uint8_t)GET_FIELD(chopconf, CHOPCONF_MRES_MASK, CHOPCONF_MRES_SHIFT))
	{
	case 0:
		return 256;
	case 1:
		return 128;
	case 2:
		return 64;
	case 3:
		return 32;
	case 4:
		return 16;
	case 5:
		return 8;
	case 6:
		return 4;
	case 7:
		return 2;
	case 8:
		return 1;
	}
	return 0;
}

void tmc_set_microstep(tmc_driver_t *driver, tmc_driver_setting_t *settings)
{
	uint32_t gconf = 0;
	gconf = tmc_read_register(driver, GCONF);
	uint32_t chopconf = 0;
	chopconf = tmc_read_register(driver, CHOPCONF);

	if (gconf == TMC_READ_ERROR)
	{
		return;
	}

	if (chopconf == TMC_READ_ERROR)
	{
		return;
	}

	uint8_t ms;
	switch (settings->mstep)
	{
	case 256:
		ms = 0;
		break;
	case 128:
		ms = 1;
		break;
	case 64:
		ms = 2;
		break;
	case 32:
		ms = 3;
		break;
	case 16:
		ms = 4;
		break;
	case 8:
		ms = 5;
		break;
	case 4:
		ms = 6;
		break;
	case 2:
		ms = 7;
		break;
	case 1:
		ms = 8;
		break;
	default:
		if (settings->mstep < 0)
		{
			if (driver->type != 2130)
			{
				SET_FIELD(gconf, GCONF_MSTEP_REG_SELECT_MASK, GCONF_MSTEP_REG_SELECT_SHIFT, 0);
				tmc_write_register(driver, GCONF, gconf);
			}
			return;
		}
		break;
	}

	if (driver->type != 2130)
	{
		SET_FIELD(gconf, GCONF_MSTEP_REG_SELECT_MASK, GCONF_MSTEP_REG_SELECT_SHIFT, 1);
		tmc_write_register(driver, GCONF, gconf);
	}

	SET_FIELD(chopconf, CHOPCONF_MRES_MASK, CHOPCONF_MRES_SHIFT, ms);
	tmc_write_register(driver, CHOPCONF, chopconf);
}

uint8_t tmc_get_stepinterpol(tmc_driver_t *driver)
{
	uint32_t chopconf = 0;
	chopconf = tmc_read_register(driver, CHOPCONF);
	if (chopconf == TMC_READ_ERROR)
	{
		return 0;
	}
	return (GET_FIELD(chopconf, CHOPCONF_INTPOL_MASK, CHOPCONF_INTPOL_SHIFT) != 0) ? 1 : 0;
}

void tmc_set_stepinterpol(tmc_driver_t *driver, tmc_driver_setting_t *settings)
{
	uint32_t chopconf = 0;
	chopconf = tmc_read_register(driver, CHOPCONF);

	if (chopconf == TMC_READ_ERROR)
	{
		return;
	}

	if (settings->step_interpolation)
	{
		SET_FIELD(chopconf, CHOPCONF_INTPOL_MASK, CHOPCONF_INTPOL_SHIFT, 1);
	}
	else
	{
		SET_FIELD(chopconf, CHOPCONF_INTPOL_MASK, CHOPCONF_INTPOL_SHIFT, 0);
	}

	tmc_write_register(driver, CHOPCONF, chopconf);
}

int32_t tmc_get_stealthchop(tmc_driver_t *driver)
{
	return driver->reg.tpwmthrs;
}

void tmc_set_stealthchop(tmc_driver_t *driver, tmc_driver_setting_t *settings)
{
	uint32_t gconf = 0;
	gconf = tmc_read_register(driver, GCONF);
	uint32_t pwmconf = {0};

	gconf = tmc_read_register(driver, GCONF);

	if (gconf == TMC_READ_ERROR)
	{
		return;
	}

	switch (driver->type)
	{
	case 2202:
	case 2208:
	case 2225:
	case 2209:
	case 2226:
		SET_FIELD(gconf, GCONF_EN_SPREADCYCLE_MASK, GCONF_EN_SPREADCYCLE_SHIFT, ((!settings->stealthchop_threshold) ? 1 : 0));
		SET_FIELD(pwmconf, PWMCONF_PWM_LIM_MASK, PWMCONF_PWM_LIM_SHIFT, 12);
		SET_FIELD(pwmconf, PWMCONF_PWM_REG_MASK, PWMCONF_PWM_REG_SHIFT, 8);
		SET_FIELD(pwmconf, PWMCONF_PWM_AUTOGRAD_MASK, PWMCONF_PWM_AUTOGRAD_SHIFT, 1);
		SET_FIELD(pwmconf, PWMCONF_PWM_AUTOSCALE_MASK, PWMCONF_PWM_AUTOSCALE_SHIFT, 1);
		SET_FIELD(pwmconf, PWMCONF_PWM_FREQ_MASK, PWMCONF_PWM_FREQ_SHIFT, 1);
		SET_FIELD(pwmconf, PWMCONF_PWM_GRAD_MASK, PWMCONF_PWM_GRAD_SHIFT, 14);
		SET_FIELD(pwmconf, PWMCONF_PWM_OFS_MASK, PWMCONF_PWM_OFS_SHIFT, 36);
		break;
	case 2130:
		SET_FIELD(pwmconf, PWMCONF_PWM_FREQ_MASK, PWMCONF_PWM_FREQ_SHIFT, 1); // f_pwm = 2/683 f_clk
		SET_FIELD(pwmconf, PWMCONF_PWM_AUTOSCALE_MASK, PWMCONF_PWM_AUTOSCALE_SHIFT, 1);
		SET_FIELD(pwmconf, PWMCONF_PWM_GRAD_MASK, PWMCONF_PWM_GRAD_SHIFT, 5);
		SET_FIELD(pwmconf, PWMCONF_PWM_AMPL_MASK, PWMCONF_PWM_AMPL_SHIFT, 180);
		SET_FIELD(gconf, GCONF_EN_PWM_MODE_MASK, GCONF_EN_PWM_MODE_SHIFT, ((!settings->stealthchop_threshold) ? 0 : 1));
		break;
	}

	tmc_write_register(driver, GCONF, gconf);
	tmc_write_register(driver, PWMCONF, pwmconf);
	tmc_write_register(driver, TPWMTHRS, settings->stealthchop_threshold);
}

uint32_t tmc_get_status(tmc_driver_t *driver)
{
	return tmc_read_register(driver, DRV_STATUS);
}

int32_t tmc_get_stallguard(tmc_driver_t *driver)
{
	uint32_t coolconf = 0;
	switch (driver->type)
	{
	case 2209:
	case 2226:
		return tmc_read_register(driver, SGTHRS);
	case 2130:
		coolconf = tmc_read_register(driver, COOLCONF);
		return GET_FIELD(coolconf, COOLCONF_SGT_MASK, COOLCONF_SGT_SHIFT);
	}

	// return an invalid value
	return -255;
}

void tmc_set_stallguard(tmc_driver_t *driver, tmc_driver_setting_t *settings)
{
	uint32_t coolconf = 0;

	switch (driver->type)
	{
	case 2209:
	case 2226:
		tmc_write_register(driver, SGTHRS, (uint32_t)settings->stallguard_threshold);
		break;
	case 2130:
		coolconf = tmc_read_register(driver, COOLCONF);
		if (coolconf == TMC_READ_ERROR)
		{
			return;
		}
		SET_FIELD(coolconf, COOLCONF_SGT_MASK, COOLCONF_SGT_SHIFT, settings->stallguard_threshold);
		tmc_write_register(driver, COOLCONF, coolconf);
		break;
	}
}
