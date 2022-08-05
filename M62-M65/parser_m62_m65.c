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

#include "../cnc.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef ENABLE_PARSER_MODULES

#ifndef UCNC_MODULE_VERSION_1_5_0_PLUS
#error "This module is not compatible with the current version of µCNC"
#endif

// if all conventions changes this must be updated
#define PWM0_ID 24
#define DOUT0_ID 46

// this ID must be unique for each code
#define M62 EXTENDED_MCODE(62)
#define M63 EXTENDED_MCODE(63)
#define M64 EXTENDED_MCODE(64)
#define M65 EXTENDED_MCODE(65)

uint8_t m62_m65_parse(void *args, bool *handled);
uint8_t m62_m65_exec(void *args, bool *handled);

CREATE_EVENT_LISTENER(gcode_parse, m62_m65_parse);
CREATE_EVENT_LISTENER(gcode_exec, m62_m65_exec);

// this just parses and acceps the code
uint8_t m62_m65_parse(void *args, bool *handled)
{
	gcode_parse_args_t *ptr = (gcode_parse_args_t *)args;
    if (ptr->word == 'M' && ptr->code >= 62 && ptr->code <= 65)
    {
		// stops event propagation
		*handled = true;

        if (ptr->cmd->group_extended != 0)
        {
            // there is a collision of custom gcode commands (only one per line can be processed)
            return STATUS_GCODE_MODAL_GROUP_VIOLATION;
        }

        // tells the gcode validation and execution functions this is custom code M42 (ID must be unique)
        ptr->cmd->group_extended = EXTENDED_MCODE_BASE + ptr->code;
        return STATUS_OK;
    }

    // if this is not catched by this parser, just send back the error so other extenders can process it
    return ptr->error;
}

// this actually performs 2 steps in 1 (validation and execution)
uint8_t m62_m65_exec(void *args, bool *handled)
{
	gcode_exec_args_t *ptr = (gcode_exec_args_t *)args;

    if (ptr->cmd->group_extended >= 62 && ptr->cmd->group_extended <= 65)
    {
		// stops event propagation
		*handled = true;

        if (!CHECKFLAG(ptr->cmd->words, GCODE_WORD_P))
        {
            return STATUS_GCODE_VALUE_WORD_MISSING;
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

        if (ptr->words->p >= 0 && ptr->words->p < 32)
        {
            io_set_output(ptr->words->p + DOUT0_ID, pinstate);
        }

        return STATUS_OK;
    }

    return STATUS_GCODE_EXTENDED_UNSUPPORTED;
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
