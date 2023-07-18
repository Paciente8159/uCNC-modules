/*
	Name: parser_m66.c
	Description: Implements a parser extension for Marlin M66 for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 18/07/2023

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

#ifdef ENABLE_PARSER_MODULES

#if (UCNC_MODULE_VERSION > 10800)
#error "This module is not compatible with the current version of µCNC"
#endif

// this ID must be unique for each code
#define M66 EXTENDED_MCODE(66)

bool m66_parse(void *args);
bool m66_exec(void *args);

CREATE_EVENT_LISTENER(gcode_parse, m66_parse);
CREATE_EVENT_LISTENER(gcode_exec, m66_exec);

// this just parses and acceps the code
bool m66_parse(void *args)
{
	gcode_parse_args_t *ptr = (gcode_parse_args_t *)args;
	if (ptr->word == 'M' && ptr->code == 66)
	{
		if (ptr->cmd->group_extended != 0)
		{
			// there is a collision of custom gcode commands (only one per line can be processed)
			*(ptr->error) = STATUS_GCODE_MODAL_GROUP_VIOLATION;
			return EVENT_HANDLED;
		}
		// tells the gcode validation and execution functions this is custom code M66 (ID must be unique)
		ptr->cmd->group_extended = M66;
		*(ptr->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	// catches E if GCODE_ACCEPT_WORD_E disabled and place it in A
	if (ptr->word == 'E')
	{
		ptr->cmd->words |= GCODE_WORD_A;
		ptr->words->xyzabc[3] = ptr->value;
		*(ptr->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	// if this is not catched by this parser, just send back the error so other extenders can process it
	return EVENT_CONTINUE;
}

// this actually performs 2 steps in 1 (validation and execution)
bool m66_exec(void *args)
{
	gcode_exec_args_t *ptr = (gcode_exec_args_t *)args;
	if (ptr->cmd->group_extended == M66)
	{
		// it's an error if no pin defined (E or P)
		if (!CHECKFLAG(ptr->cmd->words, (GCODE_WORD_A | GCODE_WORD_P)))
		{
			*(ptr->error) = STATUS_INVALID_STATEMENT;
			return EVENT_HANDLED;
		}

		// it's an error if both a digital and analog pin are selected
		if (CHECKFLAG(ptr->cmd->words, (GCODE_WORD_A | GCODE_WORD_P) == (GCODE_WORD_A | GCODE_WORD_P)))
		{
			*(ptr->error) = STATUS_INVALID_STATEMENT;
			return EVENT_HANDLED;
		}

		// L must be present and must be between 0 and 4
		if (!CHECKFLAG(ptr->cmd->words, GCODE_WORD_L) || ptr->words->l >= 5)
		{
			*(ptr->error) = STATUS_INVALID_STATEMENT;
			return EVENT_HANDLED;
		}

		// A Q value of zero is an error if the L-word is non-zero
		if (ptr->words->l && CHECKFLAG(ptr->cmd->words, GCODE_WORD_Q) && !ptr->words->d)
		{
			*(ptr->error) = STATUS_INVALID_STATEMENT;
			return EVENT_HANDLED;
		}

		// L0 is the only one permitted for an analog input
		if (!ptr->words->l && CHECKFLAG(ptr->cmd->words, GCODE_WORD_P))
		{
			*(ptr->error) = STATUS_INVALID_STATEMENT;
			return EVENT_HANDLED;
		}

		uint8_t pin = 0;

		// digital pin
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_P))
		{
			pin = DIN_PINS_OFFSET + (uint8_t)ptr->words->p;
		}

		// analog pin
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_A))
		{
			pin = ANALOG_PINS_OFFSET + (uint8_t)ptr->words->xyzabc[3];
		}

		// initial state
		int16_t value = io_get_pinvalue(pin);
		bool pin_state = (CHECKFLAG(ptr->cmd->words, GCODE_WORD_A) ? (value >= 512) : (value != 0));
		uint32_t timeout = 0xffffffff;

		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_Q))
		{
			timeout = mcu_millis() + (uint32_t)(ptr->words->d * 1000);
		}

		if (!ptr->words->l)
		{
			timeout = 0;
		}

		*(ptr->error) = STATUS_OK;

		while (timeout > mcu_millis())
		{
			value = io_get_pinvalue(pin);
			bool new_pin_state = (CHECKFLAG(ptr->cmd->words, GCODE_WORD_A) ? (value >= 512) : (value != 0));

			switch (ptr->words->l)
			{
			case 1:
				if (!pin_state && (pin_state != new_pin_state))
				{
					return EVENT_HANDLED;
				}
				pin_state = new_pin_state;
				break;
			case 2:
				if (pin_state && (pin_state != new_pin_state))
				{
					return EVENT_HANDLED;
				}
				pin_state = new_pin_state;
				break;
			case 3:
				if (new_pin_state)
				{
					return EVENT_HANDLED;
				}
				break;
			case 4:
				if (!new_pin_state)
				{
					return EVENT_HANDLED;
				}
				break;
			default:
				return EVENT_HANDLED;
			}

			cnc_dotasks();
		}

		return EVENT_HANDLED;
	}

	return EVENT_CONTINUE;
}

#endif

DECL_MODULE(m66)
{
#ifdef ENABLE_PARSER_MODULES
	ADD_EVENT_LISTENER(gcode_parse, m66_parse);
	ADD_EVENT_LISTENER(gcode_exec, m66_exec);
#else
#warning "Parser extensions are not enabled. M66 code extension will not work."
#endif
}
