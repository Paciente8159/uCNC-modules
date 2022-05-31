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

#include "../cnc.h"
#include <stdbool.h>

#ifdef ENABLE_PARSER_MODULES

// if all conventions changes this must be updated
#define PWM0_ID 24

// this ID must be unique for each code
#define M62 EXTENDED_MCODE(67)
#define M63 EXTENDED_MCODE(68)

uint8_t m67_m68_parse(unsigned char c, uint8_t word, uint8_t error, float value, parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd);
uint8_t m67_m68_exec(parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd);

CREATE_LISTENER(gcode_parse_delegate, m67_m68_parse);
CREATE_LISTENER(gcode_exec_delegate, m67_m68_exec);

// this just parses and acceps the code
uint8_t m67_m68_parse(unsigned char word, uint8_t code, uint8_t error, float value, parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd)
{
    if (word == 'M' && (code == 67 || code == 68))
    {
        if (cmd->group_extended != 0)
        {
            // there is a collision of custom gcode commands (only one per line can be processed)
            return STATUS_GCODE_MODAL_GROUP_VIOLATION;
        }

        // tells the gcode validation and execution functions this is custom code M42 (ID must be unique)
        cmd->group_extended = M67 + code - 67;
        return STATUS_OK;
    }

    if (cmd->group_extended == 67 || cmd->group_extended == 68)
    {
        if (word == 'E')
        {
            if (value < 0)
            {
                return STATUS_NEGATIVE_VALUE;
            }

            words->l = (uint8_t)truncf(value) + 1;
            if (words->l > 16)
            {
                return STATUS_GCODE_MAX_VALUE_EXCEEDED;
            }

            return STATUS_OK;
        }
    }

    // if this is not catched by this parser, just send back the error so other extenders can process it
    return error;
}

// this actually performs 2 steps in 1 (validation and execution)
uint8_t m67_m68_exec(parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd)
{
    if (cmd->group_extended == 67 || cmd->group_extended == 68)
    {
        if (words->l == 0)
        {
            return STATUS_GCODE_VALUE_WORD_MISSING;
        }

#ifndef GCODE_ACCEPT_WORD_E
        uint8_t analogoutput = words->l - 1 + PWM0_ID;
#else
        uint8_t analogoutput = (uint8_t)truncf(words->xyzabc[AXIS_A]) + PWM0_ID;
#endif

        if (analogoutput > 15)
        {
            return STATUS_GCODE_MAX_VALUE_EXCEEDED;
        }

        if (cmd->group_extended == M67)
        {
            itp_sync();
        }

        io_set_pwm(analogoutput, (uint8_t)CLAMP(0, words->d, 255));

        return STATUS_OK;
    }

    return STATUS_GCODE_EXTENDED_UNSUPPORTED;
}

#endif

DECL_MODULE(m67_m68)
{
#ifdef ENABLE_PARSER_MODULES
    ADD_LISTENER(gcode_parse_delegate, m67_m68_parse, gcode_parse_event);
    ADD_LISTENER(gcode_exec_delegate, m67_m68_exec, gcode_exec_event);
#else
#warning "Parser extensions are not enabled. M67-M68 code extension will not work."
#endif
}
