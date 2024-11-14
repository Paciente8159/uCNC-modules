/*
	Name: tmcdriver.c
	Description: TMC driver support µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 20-03-2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../cnc.h"
#include "../softuart.h"
#include "../softspi.h"
#include "tmc.h"
#include "tmc_driver.h"
#include <stdint.h>
#include <float.h>

#if (UCNC_MODULE_VERSION < 11090 || UCNC_MODULE_VERSION > 99999)
#error "This module is not compatible with the current version of µCNC"
#endif

#ifdef ENABLE_TMC_DRIVER_MODULE

#define TMC_UART_TIMEOUT(X) (MAX(1, ceilf(64000.0f / (float)X)) + 1)

// driver communications read/write
// UART
#define TMC1_STEPPER_RW(CHANNEL)                                                                      \
	static void tmc##CHANNEL##_rw(uint8_t *data, uint8_t wlen, uint8_t rlen)                            \
	{                                                                                                   \
		stepper##CHANNEL##_select();                                                                      \
		__ATOMIC__                                                                                        \
		{                                                                                                 \
			io_config_output(STEPPER##CHANNEL##_UART_TX);                                                   \
			io_set_output(STEPPER##CHANNEL##_UART_TX);                                                      \
			for (uint8_t i = 0; i < wlen; i++)                                                              \
			{                                                                                               \
				softuart_putc(&tmc##CHANNEL##_uart, data[i]);                                                 \
			}                                                                                               \
			io_config_input(STEPPER##CHANNEL##_UART_TX);                                                    \
			for (uint8_t i = 0; i < rlen; i++)                                                              \
			{                                                                                               \
				data[i] = softuart_getc(&tmc##CHANNEL##_uart, TMC_UART_TIMEOUT(STEPPER##CHANNEL##_BAUDRATE)); \
			}                                                                                               \
			io_config_output(STEPPER##CHANNEL##_UART_TX);                                                   \
		}                                                                                                 \
		stepper##CHANNEL##_deselect();                                                                    \
		cnc_delay_ms(TMC_UART_TIMEOUT(STEPPER##CHANNEL##_BAUDRATE));                                      \
	}
// SPI
#define TMCSPI_STEPPER_RW(CHANNEL)                                         \
	static void tmc##CHANNEL##_rw(uint8_t *data, uint8_t wlen, uint8_t rlen) \
	{                                                                        \
		io_clear_output(STEPPER##CHANNEL##_SPI_CS);                            \
		__ATOMIC__                                                             \
		{                                                                      \
			for (uint8_t i = 0; i < wlen; i++)                                   \
			{                                                                    \
				data[i] = softspi_xmit(&tmc##CHANNEL##_spi, data[i]);              \
			}                                                                    \
		}                                                                      \
		io_set_output(STEPPER##CHANNEL##_SPI_CS);                              \
	}

// driver communications read/write
// ONEWIRE
#define TMC3_STEPPER_RW(CHANNEL)                                                                      \
	static void tmc##CHANNEL##_rw(uint8_t *data, uint8_t wlen, uint8_t rlen)                            \
	{                                                                                                   \
		stepper##CHANNEL##_select();                                                                      \
		__ATOMIC__                                                                                        \
		{                                                                                                 \
			io_config_output(STEPPER##CHANNEL##_UART_RX);                                                   \
			io_set_output(STEPPER##CHANNEL##_UART_RX);                                                      \
			for (uint8_t i = 0; i < wlen; i++)                                                              \
			{                                                                                               \
				softuart_putc(&tmc##CHANNEL##_uart, data[i]);                                                 \
			}                                                                                               \
			io_config_input(STEPPER##CHANNEL##_UART_RX);                                                    \
			for (uint8_t i = 0; i < rlen; i++)                                                              \
			{                                                                                               \
				data[i] = softuart_getc(&tmc##CHANNEL##_uart, TMC_UART_TIMEOUT(STEPPER##CHANNEL##_BAUDRATE)); \
			}                                                                                               \
		}                                                                                                 \
		stepper##CHANNEL##_deselect();                                                                    \
		cnc_delay_ms(TMC_UART_TIMEOUT(STEPPER##CHANNEL##_BAUDRATE));                                      \
	}
// UART2 HW
#define TMC4_STEPPER_RW(CHANNEL)                                           \
	static void tmc##CHANNEL##_rw(uint8_t *data, uint8_t wlen, uint8_t rlen) \
	{                                                                        \
		stepper##CHANNEL##_select();                                           \
		__ATOMIC__                                                             \
		{                                                                      \
			for (uint8_t i = 0; i < wlen; i++)                                   \
			{                                                                    \
				softuart_putc(NULL, data[i]);                                      \
			}                                                                    \
			for (uint8_t i = 0; i < rlen; i++)                                   \
			{                                                                    \
				data[i] = softuart_getc(NULL, TMC_UART_TIMEOUT(BAUDRATE2));        \
			}                                                                    \
		}                                                                      \
		stepper##CHANNEL##_deselect();                                         \
		cnc_delay_ms(TMC_UART_TIMEOUT(BAUDRATE2));                             \
	}

// SPI
#define TMC2_STEPPER_RW(CHANNEL) TMCSPI_STEPPER_RW(CHANNEL)
// SPI HW
#define TMC5_STEPPER_RW(CHANNEL) TMCSPI_STEPPER_RW(CHANNEL)
// SPI2 HW
#define TMC6_STEPPER_RW(CHANNEL) TMCSPI_STEPPER_RW(CHANNEL)

#define _TMC_STEPPER_RW(TYPE, CHANNEL) TMC##TYPE##_STEPPER_RW(CHANNEL)

// driver communications declarations
// UART
#define TMC1_STEPPER_DECL(CHANNEL) SOFTUART(tmc##CHANNEL##_uart, STEPPER##CHANNEL##_BAUDRATE, STEPPER##CHANNEL##_UART_TX, STEPPER##CHANNEL##_UART_RX);
// SPI
#define TMC2_STEPPER_DECL(CHANNEL) SOFTSPI(tmc##CHANNEL##_spi, 1000000UL, 0, STEPPER##CHANNEL##_SPI_SDO, STEPPER##CHANNEL##_SPI_SDI, STEPPER##CHANNEL##_SPI_CLK);
// ONEWIRE
#define TMC3_STEPPER_DECL(CHANNEL) ONEWIRE(tmc##CHANNEL##_uart, STEPPER##CHANNEL##_BAUDRATE, STEPPER##CHANNEL##_UART_RX);
// UART2_HW
#define TMC4_STEPPER_DECL(CHANNEL)
// SPI_HW
#define TMC5_STEPPER_DECL(CHANNEL) HARDSPI(tmc##CHANNEL##_spi, 1000000UL, 0, mcu_spi_port);
// SPI2_HW
#define TMC6_STEPPER_DECL(CHANNEL) HARDSPI(tmc##CHANNEL##_spi, 1000000UL, 0, mcu_spi2_port);

#define _TMC_STEPPER_DECL(TYPE, CHANNEL) TMC##TYPE##_STEPPER_DECL(CHANNEL) TMC##TYPE##_STEPPER_RW(CHANNEL)
#define TMC_STEPPER_DECL(TYPE, CHANNEL) _TMC_STEPPER_DECL(TYPE, CHANNEL)

#ifdef STEPPER0_HAS_TMC
TMC_STEPPER_DECL(STEPPER0_TMC_INTERFACE, 0);
tmc_driver_t tmc0_driver;
tmc_driver_setting_t tmc0_settings;
#endif
#ifdef STEPPER1_HAS_TMC
TMC_STEPPER_DECL(STEPPER1_TMC_INTERFACE, 1);
tmc_driver_t tmc1_driver;
tmc_driver_setting_t tmc1_settings;
#endif
#ifdef STEPPER2_HAS_TMC
TMC_STEPPER_DECL(STEPPER2_TMC_INTERFACE, 2);
tmc_driver_t tmc2_driver;
tmc_driver_setting_t tmc2_settings;
#endif
#ifdef STEPPER3_HAS_TMC
TMC_STEPPER_DECL(STEPPER3_TMC_INTERFACE, 3);
tmc_driver_t tmc3_driver;
tmc_driver_setting_t tmc3_settings;
#endif
#ifdef STEPPER4_HAS_TMC
TMC_STEPPER_DECL(STEPPER4_TMC_INTERFACE, 4);
tmc_driver_t tmc4_driver;
tmc_driver_setting_t tmc4_settings;
#endif
#ifdef STEPPER5_HAS_TMC
TMC_STEPPER_DECL(STEPPER5_TMC_INTERFACE, 5);
tmc_driver_t tmc5_driver;
tmc_driver_setting_t tmc5_settings;
#endif
#ifdef STEPPER6_HAS_TMC
TMC_STEPPER_DECL(STEPPER6_TMC_INTERFACE, 6);
tmc_driver_t tmc6_driver;
tmc_driver_setting_t tmc6_settings;
#endif
#ifdef STEPPER7_HAS_TMC
TMC_STEPPER_DECL(STEPPER7_TMC_INTERFACE, 7);
tmc_driver_t tmc7_driver;
tmc_driver_setting_t tmc7_settings;
#endif

#define TMC_STEPPER_PORT(PORT) &tmc##PORT

void tmc_driver_update(tmc_driver_t *driver, tmc_driver_setting_t *settings, tmc_set_param_callback cb)
{
	cb(driver, settings);
}

void tmc_driver_update_all(tmc_set_param_callback cb)
{
#ifdef STEPPER0_HAS_TMC
	tmc_driver_update(&tmc0_driver, &tmc0_settings, cb);
#endif
#ifdef STEPPER1_HAS_TMC
	tmc_driver_update(&tmc1_driver, &tmc1_settings, cb);
#endif
#ifdef STEPPER2_HAS_TMC
	tmc_driver_update(&tmc2_driver, &tmc2_settings, cb);
#endif
#ifdef STEPPER3_HAS_TMC
	tmc_driver_update(&tmc3_driver, &tmc3_settings, cb);
#endif
#ifdef STEPPER4_HAS_TMC
	tmc_driver_update(&tmc4_driver, &tmc4_settings, cb);
#endif
#ifdef STEPPER5_HAS_TMC
	tmc_driver_update(&tmc5_driver, &tmc5_settings, cb);
#endif
#ifdef STEPPER6_HAS_TMC
	tmc_driver_update(&tmc6_driver, &tmc6_settings, cb);
#endif
#ifdef STEPPER7_HAS_TMC
	tmc_driver_update(&tmc7_driver, &tmc7_settings, cb);
#endif
}

bool tmc_driver_config_all(void *args)
{
#ifdef STEPPER0_HAS_TMC
	tmc_init(&tmc0_driver, &tmc0_settings);
#endif
#ifdef STEPPER1_HAS_TMC
	tmc_init(&tmc1_driver, &tmc1_settings);
#endif
#ifdef STEPPER2_HAS_TMC
	tmc_init(&tmc2_driver, &tmc2_settings);
#endif
#ifdef STEPPER3_HAS_TMC
	tmc_init(&tmc3_driver, &tmc3_settings);
#endif
#ifdef STEPPER4_HAS_TMC
	tmc_init(&tmc4_driver, &tmc4_settings);
#endif
#ifdef STEPPER5_HAS_TMC
	tmc_init(&tmc5_driver, &tmc5_settings);
#endif
#ifdef STEPPER6_HAS_TMC
	tmc_init(&tmc6_driver, &tmc6_settings);
#endif
#ifdef STEPPER7_HAS_TMC
	tmc_init(&tmc7_driver, &tmc7_settings);
#endif
	return EVENT_CONTINUE;
}

#ifdef ENABLE_MAIN_LOOP_MODULES
CREATE_EVENT_LISTENER(cnc_reset, tmc_driver_config_all);
#endif

/*custom gcode commands*/
#if defined(ENABLE_PARSER_MODULES)
// this ID must be unique for each code
#define M350 EXTENDED_MCODE(350)
// this ID must be unique for each code
#define M906 EXTENDED_MCODE(906)
// this ID must be unique for each code
#define M913 EXTENDED_MCODE(913)
// this ID must be unique for each code
#define M914 EXTENDED_MCODE(914)
// this ID must be unique for each code
#define M920 EXTENDED_MCODE(920)

bool m350_parse(void *args);
bool m350_exec(void *args);
bool m906_parse(void *args);
bool m906_exec(void *args);
bool m913_parse(void *args);
bool m913_exec(void *args);
bool m914_parse(void *args);
bool m914_exec(void *args);
bool m920_parse(void *args);
bool m920_exec(void *args);

CREATE_EVENT_LISTENER(gcode_parse, m350_parse);
CREATE_EVENT_LISTENER(gcode_exec, m350_exec);
CREATE_EVENT_LISTENER(gcode_parse, m906_parse);
CREATE_EVENT_LISTENER(gcode_exec, m906_exec);
CREATE_EVENT_LISTENER(gcode_parse, m913_parse);
CREATE_EVENT_LISTENER(gcode_exec, m913_exec);
CREATE_EVENT_LISTENER(gcode_parse, m914_parse);
CREATE_EVENT_LISTENER(gcode_exec, m914_exec);
CREATE_EVENT_LISTENER(gcode_parse, m920_parse);
CREATE_EVENT_LISTENER(gcode_exec, m920_exec);

static bool mcodes_parse_words(gcode_parse_args_t *ptr)
{
	switch (ptr->word)
	{
#ifndef AXIS_X
	case 'X':
		ptr->cmd->words |= GCODE_WORD_X;
		ptr->words->xyzabc[0] = ptr->value;
		break;
#endif
#ifndef AXIS_Y
	case 'Y':
		ptr->cmd->words |= GCODE_WORD_Y;
		ptr->words->xyzabc[1] = ptr->value;
		break;
#endif
#ifndef AXIS_Z
	case 'Z':
		ptr->cmd->words |= GCODE_WORD_Z;
		ptr->words->xyzabc[2] = ptr->value;
		break;
#endif
#ifndef AXIS_A
	case 'A':
		ptr->cmd->words |= GCODE_WORD_A;
		ptr->words->xyzabc[3] = ptr->value;
		break;
#endif
#ifndef AXIS_B
	case 'B':
		ptr->cmd->words |= GCODE_WORD_B;
		ptr->words->xyzabc[4] = ptr->value;
		break;
#endif
#ifndef AXIS_C
	case 'C':
		ptr->cmd->words |= GCODE_WORD_C;
		ptr->words->xyzabc[5] = ptr->value;
		break;
#endif
	default:
		// unable to catch the word
		return EVENT_CONTINUE;
	}

	*(ptr->error) = STATUS_OK;
	return EVENT_HANDLED;
}

// this just parses and acceps the code
bool m350_parse(void *args)
{
	gcode_parse_args_t *ptr = (gcode_parse_args_t *)args;

	if (ptr->word == 'M' && ptr->value == 350)
	{
		if (ptr->cmd->group_extended != 0)
		{
			// there is a collision of custom gcode commands (only one per line can be processed)
			*(ptr->error) = STATUS_GCODE_MODAL_GROUP_VIOLATION;
			return EVENT_HANDLED;
		}
		// tells the gcode validation and execution functions this is custom code M42 (ID must be unique)
		ptr->cmd->group_extended = M350;
		*(ptr->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	// parse undefined axis words directly if the regular parser does not handle them
	if (ptr->cmd->group_extended == M350)
	{
		return mcodes_parse_words(ptr);
	}

	// if this is not catched by this parser, just send back the error so other extenders can process it
	return EVENT_CONTINUE;
}

// this actually performs 2 steps in 1 (validation and execution)
bool m350_exec(void *args)
{
	gcode_exec_args_t *ptr = (gcode_exec_args_t *)args;

	if (ptr->cmd->group_extended == M350)
	{
		itp_sync();
		if (!ptr->cmd->words)
		{
			int32_t val = 0;
			// if no additional args then print the
			proto_print("[MICROSTEPS:");
			val = 0;
			proto_putc('X');
#ifdef STEPPER0_HAS_TMC
			val = tmc_get_microstep(&tmc0_driver);
#endif
			proto_itoa(val);
			proto_putc(',');
			val = 0;
			proto_putc('Y');
#ifdef STEPPER1_HAS_TMC
			val = tmc_get_microstep(&tmc1_driver);
#endif
			proto_itoa(val);
			proto_putc(',');
			val = 0;
			proto_putc('Z');
#ifdef STEPPER2_HAS_TMC
			val = tmc_get_microstep(&tmc2_driver);
#endif
			proto_itoa(val);
			proto_putc(',');
			val = 0;
			proto_putc('A');
#ifdef STEPPER3_HAS_TMC
			val = tmc_get_microstep(&tmc3_driver);
#endif
			proto_itoa(val);
			proto_putc(',');
			val = 0;
			proto_putc('B');
#ifdef STEPPER4_HAS_TMC
			val = tmc_get_microstep(&tmc4_driver);
#endif
			proto_itoa(val);
			proto_putc(',');
			val = 0;
			proto_putc('C');
#ifdef STEPPER5_HAS_TMC
			val = tmc_get_microstep(&tmc5_driver);
#endif
			proto_itoa(val);
			proto_putc(',');
			val = 0;
			proto_putc('I');
#ifdef STEPPER6_HAS_TMC
			val = tmc_get_microstep(&tmc6_driver);
#endif
			proto_itoa(val);
			proto_putc(',');
			val = 0;
			proto_putc('J');
#ifdef STEPPER7_HAS_TMC
			val = tmc_get_microstep(&tmc7_driver);
#endif
			proto_itoa(val);
			proto_putc(']');
			proto_putc('\n');
			proto_putc('\r');
		}
		else
		{
			if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_X))
			{
#ifdef STEPPER0_HAS_TMC
				tmc0_settings.mstep = (uint8_t)ptr->words->xyzabc[0];
#endif
			}
			if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_Y))
			{
#ifdef STEPPER1_HAS_TMC
				tmc1_settings.mstep = (uint8_t)ptr->words->xyzabc[1];
#endif
			}
			if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_Z))
			{
#ifdef STEPPER2_HAS_TMC
				tmc2_settings.mstep = (uint8_t)ptr->words->xyzabc[2];
#endif
			}
			if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_A))
			{
#ifdef STEPPER3_HAS_TMC
				tmc3_settings.mstep = (uint8_t)ptr->words->xyzabc[3];
#endif
			}
			if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_B))
			{
#ifdef STEPPER4_HAS_TMC
				tmc4_settings.mstep = (uint8_t)ptr->words->xyzabc[4];
#endif
			}
			if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_C))
			{
#ifdef STEPPER5_HAS_TMC
				tmc5_settings.mstep = (uint8_t)ptr->words->xyzabc[5];
#endif
			}
			if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_I))
			{
#ifdef STEPPER6_HAS_TMC
				tmc6_settings.mstep = (uint8_t)ptr->words->ijk[0];
#endif
			}
			if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_J))
			{
#ifdef STEPPER7_HAS_TMC
				tmc7_settings.mstep = (uint8_t)ptr->words->ijk[1];
#endif
			}

			tmc_driver_update_all(&tmc_set_microstep);
		}

		*(ptr->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	return EVENT_CONTINUE;
}

// this just parses and acceps the code
bool m906_parse(void *args)
{
	gcode_parse_args_t *ptr = (gcode_parse_args_t *)args;

	if (ptr->word == 'M' && ptr->value == 906.0f)
	{
		if (ptr->cmd->group_extended != 0)
		{
			// there is a collision of custom gcode commands (only one per line can be processed)
			*(ptr->error) = STATUS_GCODE_MODAL_GROUP_VIOLATION;
			return EVENT_HANDLED;
		}
		// tells the gcode validation and execution functions this is custom code M42 (ID must be unique)
		ptr->cmd->group_extended = M906;
		*(ptr->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	// parse undefined axis words directly if the regular parser does not handle them
	if (ptr->cmd->group_extended == M906)
	{
		return mcodes_parse_words(ptr);
	}

	// if this is not catched by this parser, just send back the error so other extenders can process it
	return EVENT_CONTINUE;
}

// this actually performs 2 steps in 1 (validation and execution)
bool m906_exec(void *args)
{
	gcode_exec_args_t *ptr = (gcode_exec_args_t *)args;

	if (ptr->cmd->group_extended == M906)
	{
		itp_sync();
		if (!ptr->cmd->words)
		{
			float val;
			// if no additional args then print the
			proto_print("[STEPPER CURRENT:");
			val = -1;
			proto_putc('X');
#ifdef STEPPER0_HAS_TMC
			val = tmc_get_current(&tmc0_driver, &tmc0_settings);
#endif
			proto_ftoa(val);
			proto_putc(',');
			val = -1;
			proto_putc('Y');
#ifdef STEPPER1_HAS_TMC
			val = tmc_get_current(&tmc1_driver, &tmc1_settings);
#endif
			proto_ftoa(val);
			proto_putc(',');
			val = -1;
			proto_putc('Z');
#ifdef STEPPER2_HAS_TMC
			val = tmc_get_current(&tmc2_driver, &tmc2_settings);
#endif
			proto_ftoa(val);
			proto_putc(',');
			val = -1;
			proto_putc('A');
#ifdef STEPPER3_HAS_TMC
			val = tmc_get_current(&tmc3_driver, &tmc3_settings);
#endif
			proto_ftoa(val);
			proto_putc(',');
			val = -1;
			proto_putc('B');
#ifdef STEPPER4_HAS_TMC
			val = tmc_get_current(&tmc4_driver, &tmc4_settings);
#endif
			proto_ftoa(val);
			proto_putc(',');
			val = -1;
			proto_putc('C');
#ifdef STEPPER5_HAS_TMC
			val = tmc_get_current(&tmc5_driver, &tmc5_settings);
#endif
			proto_ftoa(val);
			proto_putc(',');
			val = -1;
			proto_putc('I');
#ifdef STEPPER6_HAS_TMC
			val = tmc_get_current(&tmc6_driver, &tmc6_settings);
#endif
			proto_ftoa(val);
			proto_putc(',');
			val = -1;
			proto_putc('J');
#ifdef STEPPER7_HAS_TMC
			val = tmc_get_current(&tmc7_driver, &tmc7_settings);
#endif
			proto_ftoa(val);
			proto_putc(']');
			proto_putc('\n');
			proto_putc('\r');
		}
		else
		{
			if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_X))
			{
#ifdef STEPPER0_HAS_TMC
				tmc0_settings.rms_current = ptr->words->xyzabc[0];
#endif
			}
			if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_Y))
			{
#ifdef STEPPER1_HAS_TMC
				tmc1_settings.rms_current = ptr->words->xyzabc[1];
#endif
			}
			if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_Z))
			{
#ifdef STEPPER2_HAS_TMC
				tmc2_settings.rms_current = ptr->words->xyzabc[2];
#endif
			}
			if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_A))
			{
#ifdef STEPPER3_HAS_TMC
				tmc3_settings.rms_current = ptr->words->xyzabc[3];
#endif
			}
			if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_B))
			{
#ifdef STEPPER4_HAS_TMC
				tmc4_settings.rms_current = ptr->words->xyzabc[4];
#endif
			}
			if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_C))
			{
#ifdef STEPPER5_HAS_TMC
				tmc5_settings.rms_current = ptr->words->xyzabc[5];
#endif
			}
			if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_I))
			{
#ifdef STEPPER6_HAS_TMC
				tmc6_settings.rms_current = ptr->words->ijk[0];
#endif
			}
			if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_J))
			{
#ifdef STEPPER7_HAS_TMC
				tmc7_settings.rms_current = ptr->words->ijk[1];
#endif
			}

			tmc_driver_update_all(&tmc_set_current);
		}
		*(ptr->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	return EVENT_CONTINUE;
}

// this just parses and acceps the code
bool m913_parse(void *args)
{
	gcode_parse_args_t *ptr = (gcode_parse_args_t *)args;

	if (ptr->word == 'M' && ptr->value == 913.0f)
	{
		if (ptr->cmd->group_extended != 0)
		{
			// there is a collision of custom gcode commands (only one per line can be processed)
			*(ptr->error) = STATUS_GCODE_MODAL_GROUP_VIOLATION;
			return EVENT_HANDLED;
		}
		// tells the gcode validation and execution functions this is custom code M42 (ID must be unique)
		ptr->cmd->group_extended = M913;
		*(ptr->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	// parse undefined axis words directly if the regular parser does not handle them
	if (ptr->cmd->group_extended == M913)
	{
		return mcodes_parse_words(ptr);
	}

	// if this is not catched by this parser, just send back the error so other extenders can process it
	return EVENT_CONTINUE;
}

// this actually performs 2 steps in 1 (validation and execution)
bool m913_exec(void *args)
{
	gcode_exec_args_t *ptr = (gcode_exec_args_t *)args;

	if (ptr->cmd->group_extended == M913)
	{
		itp_sync();
		if (!ptr->cmd->words)
		{
			int32_t val;
			// if no additional args then print the
			proto_print("[STEPPER HYBRID THRESHOLD:");
			val = -1;
			proto_putc('X');
#ifdef STEPPER0_HAS_TMC
			val = tmc_get_stealthchop(&tmc0_driver);
#endif
			proto_itoa(val);
			proto_putc(',');
			val = -1;
			proto_putc('Y');
#ifdef STEPPER1_HAS_TMC
			val = tmc_get_stealthchop(&tmc1_driver);
#endif
			proto_itoa(val);
			proto_putc(',');
			val = -1;
			proto_putc('Z');
#ifdef STEPPER2_HAS_TMC
			val = tmc_get_stealthchop(&tmc2_driver);
#endif
			proto_itoa(val);
			proto_putc(',');
			val = -1;
			proto_putc('A');
#ifdef STEPPER3_HAS_TMC
			val = tmc_get_stealthchop(&tmc3_driver);
#endif
			proto_itoa(val);
			proto_putc(',');
			val = -1;
			proto_putc('B');
#ifdef STEPPER4_HAS_TMC
			val = tmc_get_stealthchop(&tmc4_driver);
#endif
			proto_itoa(val);
			proto_putc(',');
			val = -1;
			proto_putc('C');
#ifdef STEPPER5_HAS_TMC
			val = tmc_get_stealthchop(&tmc5_driver);
#endif
			proto_itoa(val);
			proto_putc(',');
			val = -1;
			proto_putc('I');
#ifdef STEPPER6_HAS_TMC
			val = tmc_get_stealthchop(&tmc6_driver);
#endif
			proto_itoa(val);
			proto_putc(',');
			val = -1;
			proto_putc('J');
#ifdef STEPPER7_HAS_TMC
			val = tmc_get_stealthchop(&tmc7_driver);
#endif
			proto_itoa(val);
			proto_putc(']');
			proto_putc('\n');
			proto_putc('\r');
		}
		else
		{
			if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_X))
			{
#ifdef STEPPER0_HAS_TMC
				tmc0_settings.stealthchop_threshold = (uint32_t)ptr->words->xyzabc[0];
#endif
			}
			if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_Y))
			{
#ifdef STEPPER1_HAS_TMC
				tmc1_settings.stealthchop_threshold = (uint32_t)ptr->words->xyzabc[1];
#endif
			}
			if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_Z))
			{
#ifdef STEPPER2_HAS_TMC
				tmc2_settings.stealthchop_threshold = (uint32_t)ptr->words->xyzabc[2];
#endif
			}
			if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_A))
			{
#ifdef STEPPER3_HAS_TMC
				tmc3_settings.stealthchop_threshold = (uint32_t)ptr->words->xyzabc[3];
#endif
			}
			if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_B))
			{
#ifdef STEPPER4_HAS_TMC
				tmc4_settings.stealthchop_threshold = (uint32_t)ptr->words->xyzabc[4];
#endif
			}
			if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_C))
			{
#ifdef STEPPER5_HAS_TMC
				tmc5_settings.stealthchop_threshold = (uint32_t)ptr->words->xyzabc[5];
#endif
			}
			if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_I))
			{
#ifdef STEPPER6_HAS_TMC
				tmc6_settings.stealthchop_threshold = (uint32_t)ptr->words->ijk[0];
#endif
			}
			if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_J))
			{
#ifdef STEPPER7_HAS_TMC
				tmc7_settings.stealthchop_threshold = (uint32_t)ptr->words->ijk[1];
#endif
			}

			tmc_driver_update_all(&tmc_set_stealthchop);
		}
		*(ptr->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	return EVENT_CONTINUE;
}

// this just parses and acceps the code
bool m914_parse(void *args)
{
	gcode_parse_args_t *ptr = (gcode_parse_args_t *)args;

	if (ptr->word == 'M' && ptr->value == 914.0f)
	{
		if (ptr->cmd->group_extended != 0)
		{
			// there is a collision of custom gcode commands (only one per line can be processed)
			*(ptr->error) = STATUS_GCODE_MODAL_GROUP_VIOLATION;
			return EVENT_HANDLED;
		}
		// tells the gcode validation and execution functions this is custom code M42 (ID must be unique)
		ptr->cmd->group_extended = M914;
		*(ptr->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	// parse undefined axis words directly if the regular parser does not handle them
	if (ptr->cmd->group_extended == M914)
	{
		return mcodes_parse_words(ptr);
	}

	// if this is not catched by this parser, just send back the error so other extenders can process it
	return EVENT_CONTINUE;
}

// this actually performs 2 steps in 1 (validation and execution)
bool m914_exec(void *args)
{
	gcode_exec_args_t *ptr = (gcode_exec_args_t *)args;

	if (ptr->cmd->group_extended == M914)
	{
		itp_sync();
		if (!ptr->cmd->words)
		{
			int32_t val;
			// if no additional args then print the
			proto_print("[STEPPER STALL SENSITIVITY:");
			val = -255;
			proto_putc('X');
#ifdef STEPPER0_HAS_TMC
			val = tmc_get_stallguard(&tmc0_driver);
#endif
			proto_itoa(val);
			proto_putc(',');
			val = -255;
			proto_putc('Y');
#ifdef STEPPER1_HAS_TMC
			val = tmc_get_stallguard(&tmc1_driver);
#endif
			proto_itoa(val);
			proto_putc(',');
			val = -255;
			proto_putc('Z');
#ifdef STEPPER2_HAS_TMC
			val = tmc_get_stallguard(&tmc2_driver);
#endif
			proto_itoa(val);
			proto_putc(',');
			val = -255;
			proto_putc('A');
#ifdef STEPPER3_HAS_TMC
			val = tmc_get_stallguard(&tmc3_driver);
#endif
			proto_itoa(val);
			proto_putc(',');
			val = -255;
			proto_putc('B');
#ifdef STEPPER4_HAS_TMC
			val = tmc_get_stallguard(&tmc4_driver);
#endif
			proto_itoa(val);
			proto_putc(',');
			val = -255;
			proto_putc('C');
#ifdef STEPPER5_HAS_TMC
			val = tmc_get_stallguard(&tmc5_driver);
#endif
			proto_itoa(val);
			proto_putc(',');
			val = -255;
			proto_putc('I');
#ifdef STEPPER6_HAS_TMC
			val = tmc_get_stallguard(&tmc6_driver);
#endif
			proto_itoa(val);
			proto_putc(',');
			val = -255;
			proto_putc('J');
#ifdef STEPPER7_HAS_TMC
			val = tmc_get_stallguard(&tmc7_driver);
#endif
			proto_itoa(val);
			proto_putc(']');
			proto_putc('\n');
			proto_putc('\r');
		}
		else
		{
			if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_X))
			{
#ifdef STEPPER0_HAS_TMC
				tmc0_settings.stallguard_threshold = (int32_t)ptr->words->xyzabc[0];
#endif
			}
			if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_Y))
			{
#ifdef STEPPER1_HAS_TMC
				tmc1_settings.stallguard_threshold = (int16_t)ptr->words->xyzabc[1];
#endif
			}
			if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_Z))
			{
#ifdef STEPPER2_HAS_TMC
				tmc2_settings.stallguard_threshold = (int16_t)ptr->words->xyzabc[2];
#endif
			}
			if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_A))
			{
#ifdef STEPPER3_HAS_TMC
				tmc3_settings.stallguard_threshold = (int16_t)ptr->words->xyzabc[3];
#endif
			}
			if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_B))
			{
#ifdef STEPPER4_HAS_TMC
				tmc4_settings.stallguard_threshold = (int16_t)ptr->words->xyzabc[4];
#endif
			}
			if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_C))
			{
#ifdef STEPPER5_HAS_TMC
				tmc5_settings.stallguard_threshold = (int16_t)ptr->words->xyzabc[5];
#endif
			}
			if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_I))
			{
#ifdef STEPPER6_HAS_TMC
				tmc6_settings.stallguard_threshold = (int16_t)ptr->words->ijk[0];
#endif
			}
			if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_J))
			{
#ifdef STEPPER7_HAS_TMC
				tmc7_settings.stallguard_threshold = (int16_t)ptr->words->ijk[1];
#endif
			}

			tmc_driver_update_all(&tmc_set_stallguard);
		}
		*(ptr->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	return EVENT_CONTINUE;
}

// this just parses and acceps the code
bool m920_parse(void *args)
{
	gcode_parse_args_t *ptr = (gcode_parse_args_t *)args;

	if (ptr->word == 'M' && ptr->value == 920.0f)
	{
		if (ptr->cmd->group_extended != 0)
		{
			// there is a collision of custom gcode commands (only one per line can be processed)
			*(ptr->error) = STATUS_GCODE_MODAL_GROUP_VIOLATION;
			return EVENT_HANDLED;
		}
		// tells the gcode validation and execution functions this is custom code M42 (ID must be unique)
		ptr->cmd->group_extended = M920;
		*(ptr->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	// parse undefined axis words directly if the regular parser does not handle them
	if (ptr->cmd->group_extended == M920)
	{
		return mcodes_parse_words(ptr);
	}

	// if this is not catched by this parser, just send back the error so other extenders can process it
	return EVENT_CONTINUE;
}

// this actually performs 2 steps in 1 (validation and execution)
bool m920_exec(void *args)
{
	gcode_exec_args_t *ptr = (gcode_exec_args_t *)args;

	if (ptr->cmd->group_extended == M920)
	{
		if (!CHECKFLAG(ptr->cmd->words, GCODE_ALL_AXIS | GCODE_IJK_AXIS))
		{
			*(ptr->error) = STATUS_TMC_CMD_MISSING_ARGS;
			return EVENT_HANDLED;
		}

		int8_t wordreg = -1;
		uint16_t wordval = 0;
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_L))
		{
			wordreg = (int8_t)ptr->words->l;
			if (wordreg > 1 || wordreg < 0)
			{
				*(ptr->error) = STATUS_INVALID_STATEMENT;
				return EVENT_HANDLED;
			}
			wordval = ptr->words->s;
		}

		uint32_t reg;

		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_X))
		{
			proto_print("[TMCREG X:");
			reg = (uint32_t)ptr->words->xyzabc[0];
			proto_itoa(reg);
			proto_putc(',');
#ifdef STEPPER0_HAS_TMC
			if (wordreg >= 0)
			{
				reg = tmc_read_register(&tmc0_driver, (uint8_t)ptr->words->xyzabc[0]);
				switch (wordreg)
				{
				case 0:
					reg &= 0xFFFF0000;
					reg |= (((uint32_t)wordval));
					break;
				case 1:
					reg &= 0x0000FFFF;
					reg |= (((uint32_t)wordval) << 16);
					break;
				}
				tmc_write_register(&tmc0_driver, (uint8_t)ptr->words->xyzabc[0], reg);
			}
			reg = tmc_read_register(&tmc0_driver, (uint8_t)ptr->words->xyzabc[0]);
#else
			reg = 0xFFFFFFFFUL;
#endif
			proto_itoa(reg);
			proto_putc(']');
			proto_putc('\n');
			proto_putc('\r');
		}

		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_Y))
		{
			proto_print("[TMCREG Y:");
			reg = (uint32_t)ptr->words->xyzabc[1];
			proto_itoa(reg);
			proto_putc(',');
#ifdef STEPPER1_HAS_TMC
			if (wordreg >= 0)
			{
				reg = tmc_read_register(&tmc1_driver, (uint8_t)ptr->words->xyzabc[1]);
				switch (wordreg)
				{
				case 0:
					reg &= 0xFFFF0000;
					reg |= (((uint32_t)wordval));
					break;
				case 1:
					reg &= 0x0000FFFF;
					reg |= (((uint32_t)wordval) << 16);
					break;
				}
				tmc_write_register(&tmc1_driver, (uint8_t)ptr->words->xyzabc[1], reg);
			}
			reg = tmc_read_register(&tmc1_driver, (uint8_t)ptr->words->xyzabc[1]);
#else
			reg = 0xFFFFFFFFUL;
#endif
			proto_itoa(reg);
			proto_putc(']');
			proto_putc('\n');
			proto_putc('\r');
		}

		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_Z))
		{
			proto_print("[TMCREG Z:");
			reg = (uint32_t)ptr->words->xyzabc[2];
			proto_itoa(reg);
			proto_putc(',');
#ifdef STEPPER2_HAS_TMC
			if (wordreg >= 0)
			{
				reg = tmc_read_register(&tmc2_driver, (uint8_t)ptr->words->xyzabc[2]);
				switch (wordreg)
				{
				case 0:
					reg &= 0xFFFF0000;
					reg |= (((uint32_t)wordval));
					break;
				case 1:
					reg &= 0x0000FFFF;
					reg |= (((uint32_t)wordval) << 16);
					break;
				}
				tmc_write_register(&tmc2_driver, (uint8_t)ptr->words->xyzabc[2], reg);
			}
			reg = tmc_read_register(&tmc2_driver, (uint8_t)ptr->words->xyzabc[2]);
#else
			reg = 0xFFFFFFFFUL;
#endif
			proto_itoa(reg);
			proto_putc(']');
			proto_putc('\n');
			proto_putc('\r');
		}

		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_A))
		{
			proto_print("[TMCREG A:");
			reg = (uint32_t)ptr->words->xyzabc[3];
			proto_itoa(reg);
			proto_putc(',');
#ifdef STEPPER3_HAS_TMC
			if (wordreg >= 0)
			{
				reg = tmc_read_register(&tmc3_driver, (uint8_t)ptr->words->xyzabc[3]);
				switch (wordreg)
				{
				case 0:
					reg &= 0xFFFF0000;
					reg |= (((uint32_t)wordval));
					break;
				case 1:
					reg &= 0x0000FFFF;
					reg |= (((uint32_t)wordval) << 16);
					break;
				}
				tmc_write_register(&tmc3_driver, (uint8_t)ptr->words->xyzabc[3], reg);
			}
			reg = tmc_read_register(&tmc3_driver, (uint8_t)ptr->words->xyzabc[3]);
#else
			reg = 0xFFFFFFFFUL;
#endif
			proto_itoa(reg);
			proto_putc(']');
			proto_putc('\n');
			proto_putc('\r');
		}

		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_B))
		{
			proto_print("[TMCREG B:");
			reg = (uint32_t)ptr->words->xyzabc[4];
			proto_itoa(reg);
			proto_putc(',');
#ifdef STEPPER4_HAS_TMC
			if (wordreg >= 0)
			{
				reg = tmc_read_register(&tmc4_driver, (uint8_t)ptr->words->xyzabc[4]);
				switch (wordreg)
				{
				case 0:
					reg &= 0xFFFF0000;
					reg |= (((uint32_t)wordval));
					break;
				case 1:
					reg &= 0x0000FFFF;
					reg |= (((uint32_t)wordval) << 16);
					break;
				}
				tmc_write_register(&tmc4_driver, (uint8_t)ptr->words->xyzabc[4], reg);
			}
			reg = tmc_read_register(&tmc4_driver, (uint8_t)ptr->words->xyzabc[4]);
#else
			reg = 0xFFFFFFFFUL;
#endif
			proto_itoa(reg);
			proto_putc(']');
			proto_putc('\n');
			proto_putc('\r');
		}

		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_C))
		{
			proto_print("[TMCREG C:");
			reg = (uint32_t)ptr->words->xyzabc[5];
			proto_itoa(reg);
			proto_putc(',');
#ifdef STEPPER5_HAS_TMC
			if (wordreg >= 0)
			{
				reg = tmc_read_register(&tmc5_driver, (uint8_t)ptr->words->xyzabc[5]);
				switch (wordreg)
				{
				case 0:
					reg &= 0xFFFF0000;
					reg |= (((uint32_t)wordval));
					break;
				case 1:
					reg &= 0x0000FFFF;
					reg |= (((uint32_t)wordval) << 16);
					break;
				}
				tmc_write_register(&tmc5_driver, (uint8_t)ptr->words->xyzabc[5], reg);
			}
			reg = tmc_read_register(&tmc5_driver, (uint8_t)ptr->words->xyzabc[5]);
#else
			reg = 0xFFFFFFFFUL;
#endif
			proto_itoa(reg);
			proto_putc(']');
			proto_putc('\n');
			proto_putc('\r');
		}

		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_I))
		{
			proto_print("[TMCREG I:");
			reg = (uint32_t)ptr->words->ijk[0];
			proto_itoa(reg);
			proto_putc(',');
#ifdef STEPPER6_HAS_TMC
			if (wordreg >= 0)
			{
				reg = tmc_read_register(&tmc6_driver, (uint8_t)ptr->words->ijk[0]);
				switch (wordreg)
				{
				case 0:
					reg &= 0xFFFF0000;
					reg |= (((uint32_t)wordval));
					break;
				case 1:
					reg &= 0x0000FFFF;
					reg |= (((uint32_t)wordval) << 16);
					break;
				}
				tmc_write_register(&tmc6_driver, (uint8_t)ptr->words->ijk[0], reg);
			}
			reg = tmc_read_register(&tmc6_driver, (uint8_t)ptr->words->ijk[0]);
#else
			reg = 0xFFFFFFFFUL;
#endif
			proto_itoa(reg);
			proto_putc(']');
			proto_putc('\n');
			proto_putc('\r');
		}

		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_J))
		{
			proto_print("[TMCREG J:");
			reg = (uint32_t)ptr->words->ijk[1];
			proto_itoa(reg);
			proto_putc(',');
#ifdef STEPPER7_HAS_TMC
			if (wordreg >= 0)
			{
				reg = tmc_read_register(&tmc7_driver, (uint8_t)ptr->words->ijk[1]);
				switch (wordreg)
				{
				case 0:
					reg &= 0xFFFF0000;
					reg |= (((uint32_t)wordval));
					break;
				case 1:
					reg &= 0x0000FFFF;
					reg |= (((uint32_t)wordval) << 16);
					break;
				}
				tmc_write_register(&tmc7_driver, (uint8_t)ptr->words->ijk[1], reg);
			}
			reg = tmc_read_register(&tmc7_driver, (uint8_t)ptr->words->ijk[1]);
#else
			reg = 0xFFFFFFFFUL;
#endif
			proto_itoa(reg);
			proto_putc(']');
			proto_putc('\n');
			proto_putc('\r');
		}

		*(ptr->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	return EVENT_CONTINUE;
}

#endif

DECL_MODULE(tmc_driver)
{
#if ASSERT_PIN(STEPPER0_SPI_CS)
	io_set_output(STEPPER0_SPI_CS);
#endif
#if ASSERT_PIN(STEPPER1_SPI_CS)
	io_set_output(STEPPER1_SPI_CS);
#endif
#if ASSERT_PIN(STEPPER2_SPI_CS)
	io_set_output(STEPPER2_SPI_CS);
#endif
#if ASSERT_PIN(STEPPER3_SPI_CS)
	io_set_output(STEPPER3_SPI_CS);
#endif
#if ASSERT_PIN(STEPPER4_SPI_CS)
	io_set_output(STEPPER4_SPI_CS);
#endif
#if ASSERT_PIN(STEPPER5_SPI_CS)
	io_set_output(STEPPER5_SPI_CS);
#endif
#if ASSERT_PIN(STEPPER6_SPI_CS)
	io_set_output(STEPPER6_SPI_CS);
#endif
#if ASSERT_PIN(STEPPER7_SPI_CS)
	io_set_output(STEPPER7_SPI_CS);
#endif

#ifdef ENABLE_MAIN_LOOP_MODULES
	ADD_EVENT_LISTENER(cnc_reset, tmc_driver_config_all);
#else
#error "Main loop extensions are not enabled. TMC configurations will not work."
#endif
#ifdef ENABLE_PARSER_MODULES
	ADD_EVENT_LISTENER(gcode_parse, m350_parse);
	ADD_EVENT_LISTENER(gcode_exec, m350_exec);
	ADD_EVENT_LISTENER(gcode_parse, m906_parse);
	ADD_EVENT_LISTENER(gcode_exec, m906_exec);
	ADD_EVENT_LISTENER(gcode_parse, m913_parse);
	ADD_EVENT_LISTENER(gcode_exec, m913_exec);
	ADD_EVENT_LISTENER(gcode_parse, m914_parse);
	ADD_EVENT_LISTENER(gcode_exec, m914_exec);
	ADD_EVENT_LISTENER(gcode_parse, m920_parse);
	ADD_EVENT_LISTENER(gcode_exec, m920_exec);
#else
#warning "Parser extensions are not enabled. M350, M906, M913, M914 and M920 code extensions will not work."
#endif

#ifdef STEPPER0_HAS_TMC
	tmc0_driver.type = STEPPER0_DRIVER_TYPE;
	tmc0_driver.slave = STEPPER0_UART_ADDRESS;
	tmc0_driver.init = NULL;
	tmc0_driver.rw = &tmc0_rw;
	tmc0_settings.rms_current = STEPPER0_CURRENT_MA;
	tmc0_settings.rsense = STEPPER0_RSENSE;
	tmc0_settings.ihold_mul = STEPPER0_HOLD_MULT;
	tmc0_settings.ihold_delay = 5;
	tmc0_settings.mstep = STEPPER0_MICROSTEP;
	tmc0_settings.stealthchop_threshold = STEPPER0_STEALTHCHOP_THERSHOLD;
	tmc0_settings.step_interpolation = STEPPER0_ENABLE_INTERPLATION;
	switch (STEPPER0_DRIVER_TYPE)
	{
	case 2209:
		tmc0_settings.stallguard_threshold = 0;
		break;
	case 2130:
		tmc0_settings.stallguard_threshold = 63;
		break;
	}
#endif
#ifdef STEPPER1_HAS_TMC
	tmc1_driver.type = STEPPER1_DRIVER_TYPE;
	tmc1_driver.slave = STEPPER1_UART_ADDRESS;
	tmc1_driver.init = NULL;
	tmc1_driver.rw = &tmc1_rw;
	tmc1_settings.rms_current = STEPPER1_CURRENT_MA;
	tmc1_settings.rsense = STEPPER1_RSENSE;
	tmc1_settings.ihold_mul = STEPPER1_HOLD_MULT;
	tmc1_settings.ihold_delay = 5;
	tmc1_settings.mstep = STEPPER1_MICROSTEP;
	tmc1_settings.stealthchop_threshold = STEPPER1_STEALTHCHOP_THERSHOLD;
	tmc1_settings.step_interpolation = STEPPER1_ENABLE_INTERPLATION;
	switch (STEPPER1_DRIVER_TYPE)
	{
	case 2209:
		tmc1_settings.stallguard_threshold = 0;
		break;
	case 2130:
		tmc1_settings.stallguard_threshold = 63;
		break;
	}
#endif
#ifdef STEPPER2_HAS_TMC
	tmc2_driver.type = STEPPER2_DRIVER_TYPE;
	tmc2_driver.slave = STEPPER2_UART_ADDRESS;
	tmc2_driver.init = NULL;
	tmc2_driver.rw = &tmc2_rw;
	tmc2_settings.rms_current = STEPPER2_CURRENT_MA;
	tmc2_settings.rsense = STEPPER2_RSENSE;
	tmc2_settings.ihold_mul = STEPPER2_HOLD_MULT;
	tmc2_settings.ihold_delay = 5;
	tmc2_settings.mstep = STEPPER2_MICROSTEP;
	tmc2_settings.stealthchop_threshold = STEPPER2_STEALTHCHOP_THERSHOLD;
	tmc2_settings.step_interpolation = STEPPER2_ENABLE_INTERPLATION;
	switch (STEPPER2_DRIVER_TYPE)
	{
	case 2209:
		tmc2_settings.stallguard_threshold = 0;
		break;
	case 2130:
		tmc2_settings.stallguard_threshold = 63;
		break;
	}
#endif
#ifdef STEPPER3_HAS_TMC
	tmc3_driver.type = STEPPER3_DRIVER_TYPE;
	tmc3_driver.slave = STEPPER3_UART_ADDRESS;
	tmc3_driver.init = NULL;
	tmc3_driver.rw = &tmc3_rw;
	tmc3_settings.rms_current = STEPPER3_CURRENT_MA;
	tmc3_settings.rsense = STEPPER3_RSENSE;
	tmc3_settings.ihold_mul = STEPPER3_HOLD_MULT;
	tmc3_settings.ihold_delay = 5;
	tmc3_settings.mstep = STEPPER3_MICROSTEP;
	tmc3_settings.stealthchop_threshold = STEPPER3_STEALTHCHOP_THERSHOLD;
	tmc3_settings.step_interpolation = STEPPER3_ENABLE_INTERPLATION;
	switch (STEPPER3_DRIVER_TYPE)
	{
	case 2209:
		tmc3_settings.stallguard_threshold = 0;
		break;
	case 2130:
		tmc3_settings.stallguard_threshold = 63;
		break;
	}
#endif
#ifdef STEPPER4_HAS_TMC
	tmc4_driver.type = STEPPER4_DRIVER_TYPE;
	tmc4_driver.slave = STEPPER4_UART_ADDRESS;
	tmc4_driver.init = NULL;
	tmc4_driver.rw = &tmc4_rw;
	tmc4_settings.rms_current = STEPPER4_CURRENT_MA;
	tmc4_settings.rsense = STEPPER4_RSENSE;
	tmc4_settings.ihold_mul = STEPPER4_HOLD_MULT;
	tmc4_settings.ihold_delay = 5;
	tmc4_settings.mstep = STEPPER4_MICROSTEP;
	tmc4_settings.stealthchop_threshold = STEPPER4_STEALTHCHOP_THERSHOLD;
	tmc4_settings.step_interpolation = STEPPER4_ENABLE_INTERPLATION;
	switch (STEPPER4_DRIVER_TYPE)
	{
	case 2209:
		tmc4_settings.stallguard_threshold = 0;
		break;
	case 2130:
		tmc4_settings.stallguard_threshold = 63;
		break;
	}
#endif
#ifdef STEPPER5_HAS_TMC
	tmc5_driver.type = STEPPER5_DRIVER_TYPE;
	tmc5_driver.slave = STEPPER5_UART_ADDRESS;
	tmc5_driver.init = NULL;
	tmc5_driver.rw = &tmc5_rw;
	tmc5_settings.rms_current = STEPPER5_CURRENT_MA;
	tmc5_settings.rsense = STEPPER5_RSENSE;
	tmc5_settings.ihold_mul = STEPPER5_HOLD_MULT;
	tmc5_settings.ihold_delay = 5;
	tmc5_settings.mstep = STEPPER5_MICROSTEP;
	tmc5_settings.stealthchop_threshold = STEPPER5_STEALTHCHOP_THERSHOLD;
	tmc5_settings.step_interpolation = STEPPER5_ENABLE_INTERPLATION;
	switch (STEPPER5_DRIVER_TYPE)
	{
	case 2209:
		tmc5_settings.stallguard_threshold = 0;
		break;
	case 2130:
		tmc5_settings.stallguard_threshold = 63;
		break;
	}
#endif
#ifdef STEPPER6_HAS_TMC
	tmc6_driver.type = STEPPER6_DRIVER_TYPE;
	tmc6_driver.slave = STEPPER6_UART_ADDRESS;
	tmc6_driver.init = NULL;
	tmc6_driver.rw = &tmc6_rw;
	tmc6_settings.rms_current = STEPPER6_CURRENT_MA;
	tmc6_settings.rsense = STEPPER6_RSENSE;
	tmc6_settings.ihold_mul = STEPPER6_HOLD_MULT;
	tmc6_settings.ihold_delay = 6;
	tmc6_settings.mstep = STEPPER6_MICROSTEP;
	tmc6_settings.stealthchop_threshold = STEPPER6_STEALTHCHOP_THERSHOLD;
	tmc6_settings.step_interpolation = STEPPER6_ENABLE_INTERPLATION;
	switch (STEPPER6_DRIVER_TYPE)
	{
	case 2209:
		tmc6_settings.stallguard_threshold = 0;
		break;
	case 2130:
		tmc6_settings.stallguard_threshold = 63;
		break;
	}
#endif
#ifdef STEPPER7_HAS_TMC
	tmc7_driver.type = STEPPER7_DRIVER_TYPE;
	tmc7_driver.slave = STEPPER7_UART_ADDRESS;
	tmc7_driver.init = NULL;
	tmc7_driver.rw = &tmc7_rw;
	tmc7_settings.rms_current = STEPPER7_CURRENT_MA;
	tmc7_settings.rsense = STEPPER7_RSENSE;
	tmc7_settings.ihold_mul = STEPPER7_HOLD_MULT;
	tmc7_settings.ihold_delay = 7;
	tmc7_settings.mstep = STEPPER7_MICROSTEP;
	tmc7_settings.stealthchop_threshold = STEPPER7_STEALTHCHOP_THERSHOLD;
	tmc7_settings.step_interpolation = STEPPER7_ENABLE_INTERPLATION;
	switch (STEPPER7_DRIVER_TYPE)
	{
	case 2209:
		tmc7_settings.stallguard_threshold = 0;
		break;
	case 2130:
		tmc7_settings.stallguard_threshold = 63;
		break;
	}
#endif
}

#endif
