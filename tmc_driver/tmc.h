/*
	Name: tmc.h
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

#ifndef TMC_H
#define TMC_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>

#define TMC_READ_ERROR 0xFFFFFFFFUL
#define TMC_WRITE_ERROR 0xFFFFFFFFUL
#define GCONF 0x00			// RW
#define IFCNT 0x02			// R
#define IHOLD_IRUN 0x10 // W
#define TPWMTHRS 0X13		// W
#define TCOOLTHRS 0x14	// W
#define SGTHRS 0x40			// W
#define SG_RESULT 0x41	// R
#define CHOPCONF 0x6C		// RW
#define COOLCONF 0x6D		// W
#define DRV_STATUS 0x6F // R
#define PWMCONF 0x70		// RW
#define TPOWERDOWN 0x11 // W

#define GET_FIELD(reg, mask, offset) (((reg) & (mask)) >> (offset))
#define SET_FIELD(reg, mask, offset, value) ({(reg) &= ~(mask); (reg) |= ((((uint32_t)value)<<(offset)) & (mask)); })

#define GCONF_PDN_DISABLE_MASK 0x00000040
#define GCONF_PDN_DISABLE_SHIFT 6
#define GCONF_MSTEP_REG_SELECT_MASK 0x00000080
#define GCONF_MSTEP_REG_SELECT_SHIFT 7
#define GCONF_I_SCALE_ANALOG_MASK 0x00000001
#define GCONF_I_SCALE_ANALOG_SHIFT 0
#define GCONF_EN_SPREADCYCLE_MASK 0x00000004
#define GCONF_EN_SPREADCYCLE_SHIFT 2
// TMC2130
#define GCONF_EN_PWM_MODE_MASK 0x00000004
#define GCONF_EN_PWM_MODE_SHIFT 2

#define CHOPCONF_TBL_MASK 0x00018000
#define CHOPCONF_TBL_SHIFT 15
#define CHOPCONF_TOFF_MASK 0x0000000F
#define CHOPCONF_TOFF_SHIFT 0
#define CHOPCONF_HEND_MASK 0x00000780
#define CHOPCONF_HEND_SHIFT 7
#define CHOPCONF_HSTRT_MASK 0x00000070
#define CHOPCONF_HSTRT_SHIFT 4
#define CHOPCONF_VSENSE_MASK 0x00020000
#define CHOPCONF_VSENSE_SHIFT 17
#define CHOPCONF_MRES_MASK 0x0F000000
#define CHOPCONF_MRES_SHIFT 24
#define CHOPCONF_INTPOL_MASK 0x10000000
#define CHOPCONF_INTPOL_SHIFT 28

#define IHOLD_IRUN_IHOLD_MASK 0x0000001F
#define IHOLD_IRUN_IHOLD_SHIFT 0
#define IHOLD_IRUN_IRUN_MASK 0x00001F00
#define IHOLD_IRUN_IRUN_SHIFT 8
#define IHOLD_IRUN_IHOLDDELAY_MASK 0x000F0000
#define IHOLD_IRUN_IHOLDDELAY_SHIFT 16

#define PWMCONF_PWM_OFS_MASK 0x000000FF
#define PWMCONF_PWM_OFS_SHIFT 0
#define PWMCONF_PWM_GRAD_MASK 0x0000FF00
#define PWMCONF_PWM_GRAD_SHIFT 8
#define PWMCONF_PWM_FREQ_MASK 0x00030000
#define PWMCONF_PWM_FREQ_SHIFT 16
#define PWMCONF_PWM_AUTOSCALE_MASK 0x00040000
#define PWMCONF_PWM_AUTOSCALE_SHIFT 18
#define PWMCONF_PWM_AUTOGRAD_MASK 0x00080000
#define PWMCONF_PWM_AUTOGRAD_SHIFT 19
#define PWMCONF_PWM_REG_MASK 0x0F000000
#define PWMCONF_PWM_REG_SHIFT 24
#define PWMCONF_PWM_LIM_MASK 0xF0000000
#define PWMCONF_PWM_LIM_SHIFT 28
#define PWMCONF_PWM_SCALE_SUM_MASK 0x000000FF
#define PWMCONF_PWM_SCALE_SUM_SHIFT 0
#define PWMCONF_PWM_SCALE_AUTO_MASK 0x01FF0000
#define PWMCONF_PWM_SCALE_AUTO_SHIFT 16
#define PWMCONF_PWM_OFS_AUTO_MASK 0x000000FF
#define PWMCONF_PWM_OFS_AUTO_SHIFT 0
#define PWMCONF_PWM_GRAD_AUTO_MASK 0x00FF0000
#define PWMCONF_PWM_GRAD_AUTO_SHIFT 16
// TMC2130
#define PWMCONF_PWM_FREQ_MASK 0x00030000
#define PWMCONF_PWM_FREQ_SHIFT 16
#define PWMCONF_PWM_AUTOSCALE_MASK 0x00040000
#define PWMCONF_PWM_AUTOSCALE_SHIFT 18
#define PWMCONF_PWM_GRAD_MASK 0x0000FF00
#define PWMCONF_PWM_GRAD_SHIFT 8
#define PWMCONF_PWM_AMPL_MASK 0x000000FF
#define PWMCONF_PWM_AMPL_SHIFT 0

// TMC2130
#define COOLCONF_SGT_MASK 0x007F0000
#define COOLCONF_SGT_SHIFT 16

#define GCONF_PDN_DISABLE
#define GCONF_MSTEP_REG_SELECT
#define GCONF_I_SCALE_ANALOG
#define GCONF_EN_SPREADCYCLE
// TMC2130
#define GCONF_EN_PWM_MODE

#define CHOPCONF_TBL
#define CHOPCONF_TOFF
#define CHOPCONF_HEND
#define CHOPCONF_HSTRT
#define CHOPCONF_VSENSE
#define CHOPCONF_MRES
#define CHOPCONF_INTPOL

#define IHOLD_IRUN_IHOLD
#define IHOLD_IRUN_IRUN
#define IHOLD_IRUN_IHOLDDELAY

#define PWMCONF_PWM_OFS
#define PWMCONF_PWM_GRAD
#define PWMCONF_PWM_FREQ
#define PWMCONF_PWM_AUTOSCALE
#define PWMCONF_PWM_AUTOGRAD
#define PWMCONF_PWM_REG
#define PWMCONF_PWM_LIM
#define PWMCONF_PWM_SCALE_SUM
#define PWMCONF_PWM_SCALE_AUTO
#define PWMCONF_PWM_OFS_AUTO
#define PWMCONF_PWM_GRAD_AUTO
// TMC2130
#define PWMCONF_PWM_FREQ
#define PWMCONF_PWM_AUTOSCALE
#define PWMCONF_PWM_GRAD
#define PWMCONF_PWM_AMPL

// TMC2130
#define COOLCONF_SGT

#define TMC_SET_FIELD(reg, tmcreg, val) SET_FIELD(reg, tmcreg##_MASK, tmcreg##_SHIFT, val)
#define TMC_GET_FIELD(reg, tmcreg) GET_FIELD(reg, tmcreg##_MASK, tmcreg##_SHIFT)

	typedef void (*tmc_rw)(uint8_t *, uint8_t, uint8_t);
	typedef void (*tmc_startup)(void);

	typedef struct
	{
		float rms_current;
		float rsense;
		float ihold_mul;
		uint8_t ihold_delay;
		int16_t mstep;
		bool step_interpolation;
		uint32_t stealthchop_threshold;
		int32_t stallguard_threshold;
	} tmc_driver_setting_t;

	typedef struct
	{
		uint8_t ifcnt /*R2*/;
		uint32_t ihold_irun /*R10*/;
		uint8_t tpowerdown /*R11*/;
		uint32_t tpwmthrs /*R13*/;
		uint32_t tcoolthrs /*R14*/;
		uint8_t sgthrs /*R40*/;
		uint32_t coolconf /*R6D*/;
	} tmc_driver_reg_t;

#define TMC22XX_DEFAULTS(x) ({(x).ihold_irun = 0x00071703; (x).tpowerdown = 0x00000014;})
#define TMC2130_DEFAULTS(x) ({(x).ihold_irun = 0x00071703;})

	typedef struct
	{
		// identifies the type of driver (2208, 2209, 2225, etc...)
		uint16_t type;
		// if the driver has a slave ID this should be set here
		uint8_t slave;
		// Callback for the UART/SPI interface initialization if needed
		tmc_startup init;
		// Callback for RW
		tmc_rw rw;
		// internal driver write only registers
		tmc_driver_reg_t reg;
	} tmc_driver_t;

	typedef void (*tmc_set_param_callback)(tmc_driver_t *, tmc_driver_setting_t *);

	void tmc_init(tmc_driver_t *driver, tmc_driver_setting_t *settings);
	float tmc_get_current(tmc_driver_t *driver, tmc_driver_setting_t *settings);
	void tmc_set_current(tmc_driver_t *driver, tmc_driver_setting_t *settings);
	int32_t tmc_get_microstep(tmc_driver_t *driver);
	void tmc_set_microstep(tmc_driver_t *driver, tmc_driver_setting_t *settings);
	uint8_t tmc_get_stepinterpol(tmc_driver_t *driver);
	void tmc_set_stepinterpol(tmc_driver_t *driver, tmc_driver_setting_t *settings);
	int32_t tmc_get_stealthchop(tmc_driver_t *driver);
	void tmc_set_stealthchop(tmc_driver_t *driver, tmc_driver_setting_t *settings);
	int32_t tmc_get_stallguard(tmc_driver_t *driver);
	void tmc_set_stallguard(tmc_driver_t *driver, tmc_driver_setting_t *settings);
	uint32_t tmc_get_status(tmc_driver_t *driver);
	uint32_t tmc_read_register(tmc_driver_t *driver, uint8_t address);
	uint32_t tmc_write_register(tmc_driver_t *driver, uint8_t address, uint32_t val);

#ifdef __cplusplus
}
#endif

#endif
