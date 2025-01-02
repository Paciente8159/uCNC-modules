/*
	Name: parser_m62_m65.c
	Description: Implements a parser extension for LinuxCNC M62-M65 for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 30/05/2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../cnc.h"
#include <stdint.h>
#include <stdbool.h>

#ifndef AUX_IC74HC595_COUNT
#define AUX_IC74HC595_COUNT 0
#endif

#ifndef AUX_IC74HC595_DATA
#define AUX_IC74HC595_DATA DOUT40
#endif

#ifndef AUX_IC74HC595_CLK
#define AUX_IC74HC595_CLK DOUT41
#endif

#ifndef AUX_IC74HC595_LATCH
#define AUX_IC74HC595_LATCH DOUT42
#endif

#ifndef AUX_IC74HC595_DELAY_CYCLES
#define AUX_IC74HC595_DELAY_CYCLES 0
#endif

#define aux_ic74hc595_delay() mcu_delay_cycles(AUX_IC74HC595_DELAY_CYCLES)
MCU_CALLBACK void aux_ic74hc595_update_pins(uint8_t pinoffset, bool value)
{
	static uint8_t pin_values;
	if (((pin_values & (1 << (pinoffset & 0x7))) != 0) == value)
	{
		// pin did not change. exit
		return;
	}
	__ATOMIC__
	{
		mcu_clear_output(AUX_IC74HC595_LATCH);
		uint8_t pinbyte = pin_values;
		for (uint8_t j = 0x80; j != 0; j >>= 1)
		{
#if (IC74HC595_DELAY_CYCLES)
			mcu_delay_cycles(AUX_IC74HC595_DELAY_CYCLES);
#endif
			mcu_clear_output(AUX_IC74HC595_CLK);
			if (pinbyte & j)
			{
				mcu_set_output(AUX_IC74HC595_DATA);
			}
			else
			{
				mcu_clear_output(AUX_IC74HC595_DATA);
			}
			mcu_set_output(AUX_IC74HC595_CLK);
		}
#if (IC74HC595_DELAY_CYCLES)
		mcu_delay_cycles(AUX_IC74HC595_DELAY_CYCLES);
#endif
		mcu_set_output(AUX_IC74HC595_LATCH);
	}
}

#ifdef ENABLE_PARSER_MODULES

#if (UCNC_MODULE_VERSION < 10800 || UCNC_MODULE_VERSION > 99999)
#error "This module is not compatible with the current version of µCNC"
#endif

// this ID must be unique for each code
#define M62 EXTENDED_MCODE(62)
#define M63 EXTENDED_MCODE(63)
#define M64 EXTENDED_MCODE(64)
#define M65 EXTENDED_MCODE(65)

bool m62_m65_parse(void *args);
bool m62_m65_exec(void *args);

CREATE_EVENT_LISTENER(gcode_parse, m62_m65_parse);
CREATE_EVENT_LISTENER(gcode_exec, m62_m65_exec);

// this just parses and acceps the code
bool m62_m65_parse(void *args)
{
	gcode_parse_args_t *ptr = (gcode_parse_args_t *)args;
	if (ptr->word == 'M' && ptr->code >= 62 && ptr->code <= 65)
	{
		if (ptr->cmd->group_extended != 0)
		{
			// there is a collision of custom gcode commands (only one per line can be processed)
			*(ptr->error) = STATUS_GCODE_MODAL_GROUP_VIOLATION;
			return EVENT_HANDLED;
		}

		// tells the gcode validation and execution functions this is custom code M42 (ID must be unique)
		ptr->cmd->group_extended = EXTENDED_MCODE(ptr->code);
		*(ptr->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	// if this is not catched by this parser, just send back the error so other extenders can process it
	return EVENT_CONTINUE;
}

// this actually performs 2 steps in 1 (validation and execution)
bool m62_m65_exec(void *args)
{
	gcode_exec_args_t *ptr = (gcode_exec_args_t *)args;

	if (ptr->cmd->group_extended >= M62 && ptr->cmd->group_extended <= M65)
	{
		if (!CHECKFLAG(ptr->cmd->words, GCODE_WORD_P))
		{
			*(ptr->error) = STATUS_GCODE_VALUE_WORD_MISSING;
			return EVENT_HANDLED;
		}

		if (ptr->words->p < 0)
		{
			*(ptr->error) = STATUS_NEGATIVE_VALUE;
			return EVENT_HANDLED;
		}

		bool pinstate = false;

		switch (ptr->cmd->group_extended)
		{
		case M62:
			pinstate = true;
		case M63:
			itp_sync();
			break;
		case M64:
			pinstate = true;
			break;
		}

		*(ptr->error) = STATUS_INVALID_STATEMENT;

		// pins 64 to 71 will use the extended 74HC595
		if (ptr->words->p >= 64 && ptr->words->p <= 71)
			{
				aux_ic74hc595_update_pins(ptr->words->p - 64, pinstate);
				*(ptr->error) = STATUS_OK;
			}
		else
		{
			// allow propagation
			return EVENT_CONTINUE;
		}

		return EVENT_HANDLED;
	}

	return EVENT_CONTINUE;
}

#endif

DECL_MODULE(m62_m65)
{
#ifdef ENABLE_PARSER_MODULES
	ADD_EVENT_LISTENER(gcode_parse, m62_m65_parse);
	ADD_EVENT_LISTENER(gcode_exec, m62_m65_exec);
#else
#warning "Parser extensions are not enabled. M62-M65 code extension will not work."
#endif
}
