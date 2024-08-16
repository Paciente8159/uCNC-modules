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

#if (UCNC_MODULE_VERSION < 10990 || UCNC_MODULE_VERSION > 99999)
#error "This module is not compatible with the current version of µCNC"
#endif

#include "style/style.h"
#include "src/modules/system_menu.h"
#include "src/utils.h"

static lv_display_t *display = 0;
static indev_list_t *indev_list = 0;
static lv_group_t *current_group = 0;
static lv_group_t *action_translator = 0;

DECL_BUFFER(uint8_t, lvgl_action_buffer, 16);

void lvgl_add_indev(indev_list_t *entry)
{
	if(indev_list == 0)
	{
		indev_list = entry;
		return;
	}

	lv_indev_set_display(entry->device, display);

	indev_list_t *last = indev_list;
	while(last->next != 0) last = last->next;
	last->next = entry;
}

static void timer_set_group(lv_timer_t *timer)
{
	lv_group_t *group = (lv_group_t*)lv_timer_get_user_data(timer);
	if(group == NULL)
		group = action_translator;
	for(indev_list_t *entry = indev_list; entry != 0; entry = entry->next)
	{
		lv_indev_set_group(entry->device, group);
	}
	current_group = group;
}

void lvgl_set_indev_group(lv_group_t *group)
{
	// Delay group change to prevent encoder from pressing buttons in the new group.
	lv_timer_t *timer = lv_timer_create(timer_set_group, 250, group);
	lv_timer_set_repeat_count(timer, 1);
}

static void action_translator_cb(lv_event_t *event)
{
	char key = lv_event_get_key(event);
	uint8_t action = SYSTEM_MENU_ACTION_NONE;

	switch(key)
	{
		case LV_KEY_ENTER:
			action = SYSTEM_MENU_ACTION_SELECT;
			break;
		case LV_KEY_LEFT:
			action = SYSTEM_MENU_ACTION_PREV;
			break;
		case LV_KEY_RIGHT:
			action = SYSTEM_MENU_ACTION_NEXT;
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			action = SYSTEM_MENU_ACTION_CHAR_INPUT(key);
			break;
		default:
			return;
	}

	BUFFER_ENQUEUE(lvgl_action_buffer, &action);
}

static void action_translator_edge_cb(lv_group_t *group, bool next)
{
	uint8_t action = next ? SYSTEM_MENU_ACTION_NEXT : SYSTEM_MENU_ACTION_PREV;
	BUFFER_ENQUEUE(lvgl_action_buffer, &action);
}

#ifdef DECL_SERIAL_STREAM
DECL_BUFFER(uint8_t, lvgl_stream_buffer, 64);
static uint8_t lvgl_ss_getc(void)
{
	uint8_t c = 0;
	BUFFER_DEQUEUE(lvgl_stream_buffer, &c);
	return c;
}

uint8_t lvgl_ss_available(void)
{
	return BUFFER_READ_AVAILABLE(lvgl_stream_buffer);
}

void lvgl_ss_clear(void)
{
	BUFFER_CLEAR(lvgl_stream_buffer);
}

DECL_SERIAL_STREAM(lvgl_stream, lvgl_ss_getc, lvgl_ss_available, lvgl_ss_clear, NULL, NULL);

uint8_t system_menu_send_cmd(const char *__s)
{
	// if machine is running rejects the command
	if (cnc_get_exec_state(EXEC_RUN | EXEC_JOG) == EXEC_RUN)
	{
		return STATUS_SYSTEM_GC_LOCK;
	}

	uint8_t len = strlen(__s);
	uint8_t w;

	if (BUFFER_WRITE_AVAILABLE(lvgl_stream_buffer) < len)
	{
		return STATUS_STREAM_FAILED;
	}

	BUFFER_WRITE(lvgl_stream_buffer, (void *)__s, len, w);

	return STATUS_OK;
}

#endif

/*** -------======= Event handlers =======------- ***/
#ifdef ENABLE_MAIN_LOOP_MODULES
bool lvgl_startup(void *args)
{
	system_menu_render_startup();
	return EVENT_CONTINUE;
}

bool lvgl_update(void *args)
{
	bool has_popup = g_system_menu.flags & SYSTEM_MENU_MODE_MODAL_POPUP;

	if(current_group == action_translator)
	{
		if(BUFFER_EMPTY(lvgl_action_buffer))
		{
			system_menu_action(SYSTEM_MENU_ACTION_NONE);
		}
		else
		{
			while(!BUFFER_EMPTY(lvgl_action_buffer))
			{
				uint8_t action;
				BUFFER_DEQUEUE(lvgl_action_buffer, &action);
				system_menu_action(action);
			}
		}
	}
	else if(current_group == NULL)
	{
		system_menu_action(SYSTEM_MENU_ACTION_NONE);
	}
	else
	{
		// Make sure to update the menu every once in a while
		if(g_system_menu.action_timeout < mcu_millis())
		{
			g_system_menu.flags = SYSTEM_MENU_MODE_REDRAW;
			system_menu_action_timeout(SYSTEM_MENU_REDRAW_IDLE_MS);
		}
	}

	if(has_popup && !(g_system_menu.flags & SYSTEM_MENU_MODE_MODAL_POPUP))
	{
		// Popup disappeared
		style_remove_popup();
	}

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

__attribute__((weak)) lv_display_t *lvgl_create_display()
{
	return 0;
}

DECL_MODULE(lvgl_support)
{
	// Initialize LVGL
	lv_init();

#ifdef DECL_SERIAL_STREAM
	serial_stream_register(&lvgl_stream);
#endif

	// Set callbacks to system functions
	lv_tick_set_cb(mcu_millis);
	lv_delay_set_cb(cnc_delay_ms);

	// Create a display
	display = lvgl_create_display();
	lv_display_set_default(display);

#ifdef ENABLE_MAIN_LOOP_MODULES
	ADD_EVENT_LISTENER(cnc_reset, lvgl_startup);
	ADD_EVENT_LISTENER(cnc_dotasks, lvgl_update);
	ADD_EVENT_LISTENER(cnc_alarm, lvgl_update);
#else
#warning "Main loop extensions not enabled. LVGL module will not function properly."
#endif

	BUFFER_INIT(uint8_t, lvgl_action_buffer, 16);

	action_translator = lv_group_create();
	lv_obj_t *full = lv_obj_create(NULL);
	lv_group_add_obj(action_translator, full);
	lv_group_set_edge_cb(action_translator, action_translator_edge_cb);
	lv_obj_add_event_cb(full, action_translator_cb, LV_EVENT_KEY, NULL);

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
	style_popup(__s);
}

void system_menu_render_alarm()
{
	style_alarm();
	lv_refr_now(display);
}

/* TODO: [14/08/2024]
 * Currently, list menus are rebuilt on menu id changes (required) and flag changes (might not be required every time).
 * It might be beneficial to have a style function signal when a complete menu rebuild is required on flag changes to
 * improve performance and reuse the already built menus.
 */
static lv_obj_t *list_menu = NULL;
static uint8_t list_menu_id = 0;
static uint8_t list_menu_flags = 0;
static bool list_menu_rebuild = false;

static int16_t list_menu_item_index;
static list_menu_item_arg_t list_menu_arg;

void system_menu_render_header(const char *__s)
{
	uint8_t flags = g_system_menu.flags & (SYSTEM_MENU_MODE_SELECT | SYSTEM_MENU_MODE_EDIT | SYSTEM_MENU_MODE_MODIFY | SYSTEM_MENU_MODE_SIMPLE_EDIT);
	if(g_system_menu.current_menu != list_menu_id || list_menu_flags != flags)
	{
		// A different menu was loaded, we need to rebuild the screen.
		if(list_menu != NULL)
		{
			// Delete old menu screen
			lv_obj_delete_async(list_menu);
		}

		list_menu = lv_obj_create(NULL);
		list_menu_id = g_system_menu.current_menu;
		list_menu_flags = flags;
		list_menu_rebuild = true;
		style_list_menu_header(list_menu, __s);
	}
	else
	{
		list_menu_rebuild = false;
	}
	list_menu_item_index = 0;
	list_menu_arg.must_rebuild = list_menu_rebuild;
}

void system_menu_render_nav_back(bool is_hover)
{
	style_list_menu_nav_back(list_menu, is_hover, list_menu_rebuild);
}

void system_menu_item_render_label(uint8_t render_flags, const char *label)
{
	list_menu_arg.text = label;
	list_menu_arg.render_flags = render_flags;
	list_menu_arg.item_index = list_menu_item_index++;
	style_list_menu_item_label(list_menu, &list_menu_arg);
}

void system_menu_item_render_arg(uint8_t render_flags, const char *label)
{
	if(label == NULL)
		return;
	list_menu_arg.text = label;
	list_menu_arg.render_flags = render_flags;
	style_list_menu_item_value(list_menu, &list_menu_arg);
}

void system_menu_render_footer(void)
{
	if(list_menu_rebuild)
	{
		style_list_menu_footer(list_menu);
	}

	// Make the style load the screen to make sure it knows of it.
	style_list_menu(list_menu);
}

void lvgl_callback_goto(lv_event_t *event)
{
	cb_goto_arg_t *arg = (cb_goto_arg_t*) lv_event_get_user_data(event);
	system_menu_goto(arg->menu_id);
}

void lvgl_callback_back(lv_event_t *event)
{
	MENU_LOOP(g_system_menu.menu_entry, menu)
	{
		if(menu->menu_id == g_system_menu.current_menu)
		{
			system_menu_goto(menu->parent_id);
			return;
		}
	}

	// If all else fails, go to idle screen
	system_menu_goto(0);
}

/*** -------======= Weak function bindings =======------- ***/

__attribute__((weak)) void style_init(lv_display_t *display) { }
__attribute__((weak)) void style_startup() { }
__attribute__((weak)) void style_idle() { }
__attribute__((weak)) void style_popup(const char* text) { }
__attribute__((weak)) void style_remove_popup() { }
__attribute__((weak)) void style_alarm() { }
__attribute__((weak)) void style_list_menu_header(lv_obj_t *screen, const char *header) { }
__attribute__((weak)) void style_list_menu_item_label(lv_obj_t *screen, list_menu_item_arg_t *arg) { }
__attribute__((weak)) void style_list_menu_item_value(lv_obj_t *screen, list_menu_item_arg_t *arg) { }
__attribute__((weak)) void style_list_menu_nav_back(lv_obj_t *screen, bool is_hover, bool rebuild) { }
__attribute__((weak)) void style_list_menu_footer(lv_obj_t *screen) { }
__attribute__((weak)) void style_list_menu(lv_obj_t *screen) { }

