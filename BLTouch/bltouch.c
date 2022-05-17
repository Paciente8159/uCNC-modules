/*
    Name: bltouch.c
    Description: BLTouch probe module for µCNC.

    Copyright: Copyright (c) João Martins
    Author: João Martins
    Date: 23-02-2022

    µCNC is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version. Please see <http://www.gnu.org/licenses/>

    µCNC is distributed WITHOUT ANY WARRANTY;
    Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the	GNU General Public License for more details.
*/

#include "../cnc.h"

#if ENABLE_IO_MODULES

#ifndef BLTOUCH_PROBE_SERVO
#define BLTOUCH_PROBE_SERVO SERVO0
#endif

#define BLTOUCH_DELAY 500
// tunned values with scope
#define BLTOUCH_DEPLOY (23)             // 10º
#define BLTOUCH_ALARM_REL_TOUCH_SW (85) // 60º
#define BLTOUCH_STOW (125)              // 90º
#define BLTOUCH_SELF_TEST (165)         // 120º
#define BLTOUCH_ALARM_REL_PUSH_UP (216) // 160º

void bltouch_deploy(void);
void bltouch_stow(void);

CREATE_LISTENER(probe_enable_delegate, bltouch_deploy);
CREATE_LISTENER(probe_disable_delegate, bltouch_stow);

void bltouch_deploy(void)
{
    mcu_set_servo(BLTOUCH_PROBE_SERVO, BLTOUCH_DEPLOY);
    cnc_delay_ms(BLTOUCH_DELAY);
}

void bltouch_stow(void)
{
    mcu_set_servo(BLTOUCH_PROBE_SERVO, BLTOUCH_STOW);
    cnc_delay_ms(BLTOUCH_DELAY);
}

DECL_MODULE(bltouch)
{
#ifdef ENABLE_IO_MODULES
    ADD_LISTENER(probe_enable_delegate, bltouch_deploy, probe_enable_event);
    ADD_LISTENER(probe_disable_delegate, bltouch_stow, probe_disable_event);
#endif
}

#endif
