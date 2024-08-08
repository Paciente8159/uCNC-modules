/*
	Name: lvgl.c
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

#include "lvgl_support.h"
#include "style/style.h"
#include "src/modules/system_menu.h"

static lv_display_t *display = 0;
static indev_list_t *indev_list = 0;

void lvgl_use_display(lv_display_t *disp)
{
	display = disp;
}

void lvgl_add_indev(indev_list_t *entry)
{
	if(indev_list == 0)
	{
		indev_list = entry;
		return;
	}

	indev_list_t *last = indev_list;
	while(last->next != 0) last = last->next;
	last->next = entry;
}

void lvgl_set_indev_group(lv_group_t *group)
{
	for(indev_list_t *entry = indev_list; entry != 0; entry = entry->next)
	{
		lv_indev_set_group(entry->dev, group);
	}
}

/*** -------======= Event handlers =======------- ***/
#ifdef ENABLE_MAIN_LOOP_MODULES
bool lvgl_startup(void *args)
{
	system_menu_render_startup();
	return EVENT_CONTINUE;
}

bool lvgl_update(void *args)
{
	system_menu_action(SYSTEM_MENU_ACTION_NONE);
	system_menu_render();

	static uint32_t next_run_time = 0;
	if(mcu_millis() >= next_run_time)
	{
		uint32_t until_next = lv_timer_handler();
		next_run_time = mcu_millis() + until_next;

		cnc_dotasks();
	}

	return EVENT_CONTINUE;
}

CREATE_EVENT_LISTENER(cnc_reset, lvgl_startup);
CREATE_EVENT_LISTENER(cnc_dotasks, lvgl_update);
CREATE_EVENT_LISTENER(cnc_alarm, lvgl_update);

#endif

static bool initialized = false;

DECL_MODULE(lvgl_support)
{
	// Initialize LVGL
	lv_init();

	// Set callbacks to system functions
	lv_tick_set_cb(mcu_millis);
	lv_delay_set_cb(cnc_delay_ms);

	initialized = true;
}

// This is called after display/input modules were initialized
void lvgl_support_end_init()
{
	if(!initialized)
		return;

#ifdef ENABLE_MAIN_LOOP_MODULES
	ADD_EVENT_LISTENER(cnc_reset, lvgl_startup);
	ADD_EVENT_LISTENER(cnc_dotasks, lvgl_update);
	ADD_EVENT_LISTENER(cnc_alarm, lvgl_update);
#else
#warning "Main loop extensions not enabled. LVGL module will not function properly."
#endif

	// Init system menu module
	system_menu_init();

	// The style initialization happens after system menu init
	// to allow for overrides of renderers and menus.
	style_init(display);
}

/*** -------======= System menu module bindings =======------- ***/
void system_menu_render_startup(void)
{
	style_startup();
}

void system_menu_render_idle(void)
{
	style_idle();
}

void system_menu_render_modal_popup(const char *__s)
{
	// style_popup(__s);
}

void system_menu_render_alarm()
{
	style_alarm();
	lv_refr_now(display);
}

