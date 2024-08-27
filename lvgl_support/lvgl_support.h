/*
	Name: lvgl.h
	Description: Support module for LVGL

	Copyright: Copyright (c) Patryk Mierzyński
	Author: Patryk Mierzyński
	Date: 07/08/2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the GNU General Public License for more details.
*/

#ifndef LVGL_SUPPORT_H
#define LVGL_SUPPORT_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "src/module.h"
#include "lvgl.h"

DECL_MODULE(lvgl_support);

typedef struct _indev_list {
	lv_indev_t *device;
	struct _indev_list *next;
} indev_list_t;

extern void lvgl_add_indev(indev_list_t *entry);
#define LVGL_ADD_INDEV(dev) \
	static indev_list_t dev##_list_entry = { 0, 0 }; \
	dev##_list_entry.device = dev; \
	lvgl_add_indev(&dev##_list_entry)

extern void lvgl_set_indev_group(lv_group_t *group);

extern void lvgl_serial_putc(char c);
extern uint8_t lvgl_serial_write_available();

typedef struct _cb_goto
{
	uint8_t menu_id;
} cb_goto_arg_t;
extern void lvgl_callback_goto(lv_event_t *event);

extern void lvgl_callback_back(lv_event_t *event);

#ifdef __cplusplus
}
#endif

#endif

