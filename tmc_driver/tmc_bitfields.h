/*
    Name: tmc_bitfields.h
    Description: Minimalistic generic driver library for Trinamic stepper drivers.

    Copyright: Copyright (c) João Martins
    Author: João Martins
    Date: 24-01-2024

    µCNC is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License; or
    (at your option) any later version. Please see <http://www.gnu.org/licenses/>

    µCNC is distributed WITHOUT ANY WARRANTY;
    Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the	GNU General Public License for more details.
*/

#ifndef TMC_BITFIELDS_H
#define TMC_BITFIELDS_H

// #pragma pack(push; 1)

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>

    /**
     * Bit fields based on the https://github.com/teemuatlut/TMCStepper.git
     *
     * **/

    typedef struct GCONF_
    {
        union
        {
            uint32_t sr;
            struct
            {
                uint8_t i_scale_analog : 1;
                uint8_t internal_rsense : 1;
                uint8_t en_spreadcycle : 1;
                uint8_t shaft : 1;
                uint8_t index_otpw : 1;
                uint8_t index_step : 1;
                uint8_t pdn_disable : 1;
                uint8_t mstep_reg_select : 1;
                uint8_t multistep_filt : 1;
                uint8_t test_mode : 1;
            };

            struct
            {
                uint8_t : 2; // 2130; 5130
                uint8_t en_pwm_mode : 1;
                uint8_t enc_commutation : 1; // 2130; 5130
                uint8_t shaft : 1;
                uint8_t diag0_error : 1;
                uint8_t diag0_otpw : 1;
                uint8_t diag0_stall : 1;
                uint8_t diag1_stall : 1;
                uint8_t diag1_index : 1;
                uint8_t diag1_onstate : 1;
                uint8_t diag1_steps_skipped : 1;
                uint8_t diag0_int_pushpull : 1;
                uint8_t diag1_pushpull : 1;
                uint8_t small_hysteresis : 1;
                uint8_t stop_enable : 1;
                uint8_t direct_mode : 1;
            } tmc2130;
            struct
            { // TMC5160
                uint8_t recalibrate : 1;
                uint8_t faststandstill : 1;
                uint8_t : 1;
                uint8_t multistep_filt : 1;
                uint8_t : 3;
                uint8_t diag0_step : 1;
                uint8_t diag1_dir : 1;
                uint8_t : 4;
                uint8_t diag1_poscomp_pushpull : 1;
            } tmc5160;
        };
    } GCONF_t;

    struct IHOLD_IRUN_t
    {
        union
        {
            uint32_t sr : 20;
            struct
            {
                uint8_t ihold : 5;
                uint8_t : 3;
                uint8_t irun : 5;
                uint8_t : 3;
                uint8_t iholddelay : 4;
            };
        };
    };

    typedef struct CHOPCONF_
    {
        union
        {
            uint32_t sr;
            struct
            {
                uint32_t toff : 4;
                uint32_t hstrt : 3;
                uint32_t hend : 4;
                uint32_t : 4;
                uint32_t tbl : 2;
                uint32_t vsense : 1;
                uint32_t : 6;
                uint32_t mres : 4;
                uint32_t intpol : 1;
                uint32_t dedge : 1;
                uint32_t diss2g : 1;
                uint32_t diss2vs : 1;
            };

            struct
            {
                uint32_t : 12;
                uint32_t fd3 : 1;
                uint32_t disfdcc : 1;
                uint32_t rndtf : 1;
                uint32_t chm : 1;
                uint32_t : 3;
                uint32_t vhighfs : 1;
                uint32_t vhighchm : 1;
                uint32_t sync : 4; // 2130; 5130
                uint32_t : 8;
            } tmc2130;

            struct
            { // TMC5160
                uint32_t : 20;
                uint32_t tpfd : 4; // 5160
                uint32_t : 8;
            } tmc5160;
        };
    } CHOPCONF_t;

    typedef struct PWMCONF_
    {
        union
        {
            uint32_t sr;
            struct
            {
                uint32_t pwm_ofs : 8;
                uint32_t pwm_grad : 8;
                uint32_t pwm_freq : 2;
                uint32_t pwm_autoscale : 1;
                uint32_t pwm_autograd : 1;
                uint32_t freewheel : 2;
                uint32_t : 2;
                uint32_t pwm_reg : 4;
                uint32_t pwm_lim : 4;
            };
            struct
            {
                uint32_t pwm_ampl : 8;
                uint32_t : 11;
                uint32_t pwm_symmetric : 1;
            } tmc2130;
        };
    } PWMCONF_t;

    typedef struct SGTHRS_
    {
        uint8_t sr : 8;
    } SGTHRS_t;

    typedef struct COOLCONF_
    {
        union
        {
            uint32_t sr;
            struct
            {
                uint32_t semin : 4;
                uint32_t : 1;
                uint32_t seup : 2;
                uint32_t : 1;
                uint32_t semax : 4;
                uint32_t : 1;
                uint32_t sedn : 2;
                uint32_t seimin : 1;
            };
            struct
            {
                uint16_t : 16;
                int8_t sgt : 7;
                uint8_t : 1;
                uint8_t sfilt : 1;
            } tmc2130;
        };
    } COOLCONF_t;

#ifdef __cplusplus
}
#endif

#endif