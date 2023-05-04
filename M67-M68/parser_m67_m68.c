/*
    Name: parser_m67_m68.c
    Description: Implements a parser extension for LinuxCNC M67-M68 for µCNC.

    Copyright: Copyright (c) João Martins
    Author: João Martins
    Date: 31/05/2022

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

#if (UCNC_MODULE_VERSION > 010700)
#error "This module is not compatible with the current version of µCNC"
#endif

// if all conventions changes this must be updated
#define PWM0_ID 24

// this ID must be unique for each code
#define M67 EXTENDED_MCODE(67)
#define M68 EXTENDED_MCODE(68)

bool m67_m68_parse(void *args);
bool m67_m68_exec(void *args);

CREATE_EVENT_LISTENER(gcode_parse, m67_m68_parse);
CREATE_EVENT_LISTENER(gcode_exec, m67_m68_exec);

// this just parses and acceps the code
bool m67_m68_parse(void *args)
{
    gcode_parse_args_t *ptr = (gcode_parse_args_t *)args;

    if (ptr->word == 'M' && (ptr->code == 67 || ptr->code == 68))
    {
        if (ptr->cmd->group_extended != 0)
        {
            // there is a collision of custom gcode commands (only one per line can be processed)
            *(ptr->error) = STATUS_GCODE_MODAL_GROUP_VIOLATION;
            return EVENT_HANDLED;
        }

        // tells the gcode validation and execution functions this is custom code M42 (ID must be unique)
        ptr->cmd->group_extended = M67 + ptr->code - 67;
        *(ptr->error) = STATUS_OK;
        return EVENT_HANDLED;
    }

    if (ptr->cmd->group_extended == 67 || ptr->cmd->group_extended == 68)
    {
        if (ptr->word == 'E')
        {
            if (ptr->value < 0)
            {
                *(ptr->error) = STATUS_NEGATIVE_VALUE;
                return EVENT_HANDLED;
            }

            ptr->words->l = (uint8_t)truncf(ptr->value) + 1;
            if (ptr->words->l > 16)
            {
                *(ptr->error) = STATUS_GCODE_MAX_VALUE_EXCEEDED;
                return EVENT_HANDLED;
            }

            *(ptr->error) = STATUS_OK;
            return EVENT_HANDLED;
        }
    }

    // if this is not catched by this parser, just send back the error so other extenders can process it
    return EVENT_CONTINUE;
}

// this actually performs 2 steps in 1 (validation and execution)
bool m67_m68_exec(void *args)
{
    gcode_exec_args_t *ptr = (gcode_exec_args_t *)args;

    if (ptr->cmd->group_extended == 67 || ptr->cmd->group_extended == 68)
    {
        if (ptr->words->l == 0)
        {
            *(ptr->error) = STATUS_GCODE_VALUE_WORD_MISSING;
            return EVENT_HANDLED;
        }

#ifndef GCODE_ACCEPT_WORD_E
        uint8_t analogoutput = ptr->words->l - 1 + PWM0_ID;
#else
        uint8_t analogoutput = (uint8_t)truncf(ptr->words->xyzabc[AXIS_A]) + PWM0_ID;
#endif

        if (analogoutput > 15)
        {
            *(ptr->error) = STATUS_GCODE_MAX_VALUE_EXCEEDED;
            return EVENT_HANDLED;
        }

        if (ptr->cmd->group_extended == M67)
        {
            itp_sync();
        }

        io_set_pwm(analogoutput, (uint8_t)CLAMP(0, ptr->words->d, 255));

        *(ptr->error) = STATUS_OK;
        return EVENT_HANDLED;
    }

    return EVENT_CONTINUE;
}

#endif

DECL_MODULE(m67_m68)
{
#ifdef ENABLE_PARSER_MODULES
    ADD_EVENT_LISTENER(gcode_parse, m67_m68_parse);
    ADD_EVENT_LISTENER(gcode_exec, m67_m68_exec);
#else
#warning "Parser extensions are not enabled. M67-M68 code extension will not work."
#endif
}
