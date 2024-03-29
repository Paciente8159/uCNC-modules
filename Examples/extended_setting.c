/*
	This is and example of how to add a custom $ setting extension to µCNC

	All extended settings minimum address is 256 and maximum is 65535

	There is a new simplified way of creating extensions
	Please check the end of this file

*/

#include "../../cnc.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef ENABLE_SETTINGS_MODULES

#if (UCNC_MODULE_VERSION < 10800 || UCNC_MODULE_VERSION > 99999)
#error "This module is not compatible with the current version of µCNC"
#endif

// dummy setting $1000
#define DUMMY_SETTING_ID 1000
// this will keep the offset address of the stored settings
static uint32_t dummy_settings_address;

// dummy setting
// can be anything float, struct, etc...
static uint32_t dummy_setting;

bool extend_settings_load(void *args)
{
	settings_args_t *set = (settings_args_t *)args;
	// the setting will be saved when trying to save standard setting
	if (set->address == SETTINGS_ADDRESS_OFFSET)
	{
		// load from disk
		// this will do a recursive call

		// at this point you should call a function to read the settings from a different media
		// loading returned error. settings corrupted. stop reading.
		// allow propagate to get settings from eeprom
		return EVENT_CONTINUE;
	}

	// prevent propagation
	// to enable event propagation return EVENT_CONTINUE
	return EVENT_HANDLED;
}

bool extend_settings_save(void *args)
{
	settings_args_t *set = (settings_args_t *)args;
	// if saving main settings load this one too
	if (set->address == SETTINGS_ADDRESS_OFFSET)
	{
		// load from disk
		// this will do an infinite loop

		// save setting to other media
		//  prevent propagation
		return EVENT_HANDLED;
	}

	// allow other settings to be stored in eeprom
	return EVENT_CONTINUE;
}

bool extend_settings_change(void *args)
{
	setting_args_t *set = (setting_args_t *)args;
	if (set->id == DUMMY_SETTING_ID)
	{
		// if matches setting id intercepts the call
		// load from disk
		dummy_setting = (uint32_t)set->value;
		return EVENT_HANDLED;
	}

	return EVENT_CONTINUE;
}

bool extend_settings_erase(void *args)
{
	settings_args_t *set = (settings_args_t *)args;
	// if reseting main settings load this one too
	if (set->address == SETTINGS_ADDRESS_OFFSET)
	{
		// reset
		dummy_setting = 0;
	}

	// prevent propagation
	// to enable event propagation return EVENT_CONTINUE
	return EVENT_HANDLED;
}

uint8_t extend_protocol_send_cnc_settings(void *args)
{
	protocol_send_gcode_setting_line_int(DUMMY_SETTING_ID, dummy_setting);
	return EVENT_CONTINUE;
}

CREATE_EVENT_LISTENER(settings_load, extend_settings_load);
CREATE_EVENT_LISTENER(settings_save, extend_settings_save);
CREATE_EVENT_LISTENER(settings_change, extend_settings_change);
CREATE_EVENT_LISTENER(settings_erase, extend_settings_erase);
CREATE_EVENT_LISTENER(protocol_send_cnc_settings, extend_protocol_send_cnc_settings);

#endif

DECL_MODULE(dummy)
{
#ifdef ENABLE_SETTINGS_MODULES
	dummy_settings_address = settings_register_external_setting(sizeof(dummy_setting));
	ADD_EVENT_LISTENER(settings_load, extend_settings_load);
	ADD_EVENT_LISTENER(settings_save, extend_settings_save);
	ADD_EVENT_LISTENER(settings_change, extend_settings_change);
	ADD_EVENT_LISTENER(settings_erase, extend_settings_erase);
	ADD_EVENT_LISTENER(protocol_send_cnc_settings, extend_protocol_send_cnc_settings);
#else
// just a warning in case you disabled the MAIN_LOOP option on build
#warning "Settings extensions are not enabled. Your module will not work."
#endif
}

/**
 *
 * ALTERNATIVE SIMPLIFIED WAY WITH NEW SETTINGS MACROS
 *
 * The example below does the same thing as the code above
 *
 * **/

// dummy setting $1000
#define DUMMY_SETTING_ID 1000
// this will keep the offset address of the stored settings
static uint32_t dummy_settings_address;

// declarate the setting with
//  args are:
//  ID - setting ID
//  var - pointer to var
//  type - type of var
//  count - var array length (iff single var set to 1)
//  print_cb - protocol setting print callback

DECL_EXTENDED_SETTING(DUMMY_SETTING_ID, &dummy_settings_address, uint32_t, 1, protocol_send_gcode_setting_line_int);

// then just call the initializator from any point in the code
// the initialization will run only once
// ID - setting ID
// var - var as argument (not the pointer)
void some_function_running_in_the_code_loop()
{
	EXTENDED_SETTING_INIT(DUMMY_SETTING_ID, dummy_settings_address);
}
