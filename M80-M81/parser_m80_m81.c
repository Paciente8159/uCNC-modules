/*
    Name: parser_m80_m81.c
    Description: Implements a parser extension for Marlin M80 (PSU ON) and M81 (PSU OFF) for µCNC.

    Copyright: Copyright (c) João Martins
    Author: João Martins
    Date: 22/05/2022

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

// setup PSU pins and value to enable the PSU
#ifndef PSU_PIN
#define PSU_PIN DOUT31
#endif
#ifndef PSU_ON
#define PSU_ON true
#endif

// this ID must be unique for each code
#define M80 1080
#define M81 1081

uint8_t m80_m81_parse(unsigned char c, uint8_t word, uint8_t error, float value, parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd);
uint8_t m80_m81_exec(parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd);

CREATE_LISTENER(gcode_parse_delegate, m80_m81_parse);
CREATE_LISTENER(gcode_exec_delegate, m80_m81_exec);

// this just parses and acceps the code
uint8_t m80_m81_parse(unsigned char word, uint8_t code, uint8_t error, float value, parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd)
{
    if (word == 'M')
    {
        if (cmd->group_extended != 0)
        {
            // there is a collision of custom gcode commands (only one per line can be processed)
            return STATUS_GCODE_MODAL_GROUP_VIOLATION;
        }

        switch (code)
        {
        case 80:
        case 81:
            cmd->group_extended = 1000 + code;
            return STATUS_OK;
        }
    }

    // if this is not catched by this parser, just send back the error so other extenders can process it
    return error;
}

// this actually performs 2 steps in 1 (validation and execution)
uint8_t m80_m81_exec(parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd)
{
    switch (cmd->group_extended)
    {
    case M80:
        io_set_output(PSU_PIN, PSU_ON);
        return STATUS_OK;
    case M81:
        io_set_output(PSU_PIN, !PSU_ON);
        return STATUS_OK;
    }

    return STATUS_GCODE_EXTENDED_UNSUPPORTED;
}

#endif

DECL_MODULE(m80_m81)
{
#ifdef ENABLE_PARSER_MODULES
    ADD_LISTENER(gcode_parse_delegate, m80_m81_parse, gcode_parse_event);
    ADD_LISTENER(gcode_exec_delegate, m80_m81_exec, gcode_exec_event);
#else
#warning "Parser extensions are not enabled. M80 and M81 code extension will not work."
#endif
}
