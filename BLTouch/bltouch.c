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

#include "../../cnc.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef ENABLE_IO_MODULES

#if (UCNC_MODULE_VERSION != 10800)
#error "This module is not compatible with the current version of µCNC"
#endif

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

bool bltouch_deploy(void *args);
bool bltouch_stow(void *args);

CREATE_EVENT_LISTENER(probe_enable, bltouch_deploy);
CREATE_EVENT_LISTENER(probe_disable, bltouch_stow);

bool bltouch_deploy(void *args)
{
    mcu_set_servo(BLTOUCH_PROBE_SERVO, BLTOUCH_DEPLOY);
    cnc_delay_ms(BLTOUCH_DELAY);
    return EVENT_CONTINUE;
}

bool bltouch_stow(void *args)
{
    mcu_set_servo(BLTOUCH_PROBE_SERVO, BLTOUCH_STOW);
    cnc_delay_ms(BLTOUCH_DELAY);
    return EVENT_CONTINUE;
}

#endif

DECL_MODULE(bltouch)
{
#ifdef ENABLE_IO_MODULES
    ADD_EVENT_LISTENER(probe_enable, bltouch_deploy);
    ADD_EVENT_LISTENER(probe_disable, bltouch_stow);
#else
#warning "IO extensions are not enabled. BLTouch will not work."
#endif
}
