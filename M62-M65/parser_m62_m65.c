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
#include <stdbool.h>

#ifdef ENABLE_PARSER_MODULES

// if all conventions changes this must be updated
#define PWM0_ID 24
#define DOUT0_ID 46

// this ID must be unique for each code
#define M62 EXTENDED_MCODE(62)
#define M63 EXTENDED_MCODE(63)
#define M64 EXTENDED_MCODE(64)
#define M65 EXTENDED_MCODE(65)

uint8_t m62_m65_parse(unsigned char c, uint8_t word, uint8_t error, float value, parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd);
uint8_t m62_m65_exec(parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd);

CREATE_LISTENER(gcode_parse_delegate, m62_m65_parse);
CREATE_LISTENER(gcode_exec_delegate, m62_m65_exec);

// this just parses and acceps the code
uint8_t m62_m65_parse(unsigned char word, uint8_t code, uint8_t error, float value, parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd)
{
    if (word == 'M' && code >= 62 && code <= 65)
    {
        if (cmd->group_extended != 0)
        {
            // there is a collision of custom gcode commands (only one per line can be processed)
            return STATUS_GCODE_MODAL_GROUP_VIOLATION;
        }

        // tells the gcode validation and execution functions this is custom code M42 (ID must be unique)
        cmd->group_extended = M62 + code - 62;
        return STATUS_OK;
    }

    // if this is not catched by this parser, just send back the error so other extenders can process it
    return error;
}

// this actually performs 2 steps in 1 (validation and execution)
uint8_t m62_m65_exec(parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd)
{
    if (cmd->group_extended >= 62 && cmd->group_extended <= 65)
    {
        if (!CHECKFLAG(cmd->words, GCODE_WORD_P))
        {
            return STATUS_GCODE_VALUE_WORD_MISSING;
        }

        bool pinstate = false;

        switch (cmd->group_extended)
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

        if (words->p >= 0 && words->p < 32)
        {
            io_set_output(words->p + DOUT0_ID, pinstate);
        }

        return STATUS_OK;
    }

    return STATUS_GCODE_EXTENDED_UNSUPPORTED;
}

#endif

DECL_MODULE(m62_m65)
{
#ifdef ENABLE_PARSER_MODULES
    ADD_LISTENER(gcode_parse_delegate, m62_m65_parse, gcode_parse_event);
    ADD_LISTENER(gcode_exec_delegate, m62_m65_exec, gcode_exec_event);
#else
#warning "Parser extensions are not enabled. M62-M65 code extension will not work."
#endif
}
