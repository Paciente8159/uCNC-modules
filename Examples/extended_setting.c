/*
	This is and example of how to add a custom $ setting extension to µCNC

	All extended settings minimum address is 256 and maximum is 65535

*/

#include "../cnc.h"
#include <stdint.h>
#include <stdbool.h>

// dummy setting $1000
#define DUMMY_SETTING_ID 1000
// this will keep the offset address of the stored settings
static uint32_t dummy_settings_address;

// dummy setting
// can be anything float, struct, etc...
static uint32_t dummy_setting;

uint8_t extend_settings_load(void *args, bool *handled)
{
	settings_args_t *set = (settings_args_t *)args;
	// the setting will be saved when trying to save standard setting
	if (set->address == SETTINGS_ADDRESS_OFFSET)
	{
		// load from disk
		// this will do a recursive call 
		if(settings_load(dummy_settings_address, &dummy_setting, sizeof(dummy_setting))){
			// loading returned error. settings corrupted. stop reading.
			*handled = true;
			return STATUS_SETTING_READ_FAIL;
		}
	}

	return STATUS_OK;
}

uint8_t extend_settings_save(void *args, bool *handled)
{
	settings_args_t *set = (settings_args_t *)args;
	//if saving main settings load this one too
	if (set->address == SETTINGS_ADDRESS_OFFSET)
	{
		//load from disk
		return settings_save(dummy_settings_address, &dummy_setting, sizeof(dummy_setting));
	}

	return STATUS_OK;
}

uint8_t extend_settings_change(void *args, bool *handled)
{
	setting_args_t *set = (setting_args_t *)args;
	if (set->id == DUMMY_SETTING_ID)
	{
		//load from disk
		*handled = true;
		dummy_setting = (uint32_t)set->value;
		return STATUS_OK;
	}
	
	return STATUS_INVALID_STATEMENT;
}

uint8_t extend_settings_erase(void *args, bool *handled)
{
	settings_args_t *set = (settings_args_t *)args;
	//if reseting main settings load this one too
	if (set->address == SETTINGS_ADDRESS_OFFSET)
	{
		//reset
		dummy_setting = 0;
	}

	return STATUS_OK;
}

uint8_t extend_protocol_send_cnc_settings(void *args, bool *handled)
{
	protocol_send_gcode_setting_line_int(DUMMY_SETTING_ID, dummy_setting);
	return STATUS_OK;
}

CREATE_EVENT_LISTENER(settings_load, extend_settings_load);
CREATE_EVENT_LISTENER(settings_save, extend_settings_save);
CREATE_EVENT_LISTENER(settings_change, extend_settings_change);
CREATE_EVENT_LISTENER(settings_erase, extend_settings_erase);
CREATE_EVENT_LISTENER(protocol_send_cnc_settings, extend_protocol_send_cnc_settings);

DECL_MODULE(dummy)
{
	dummy_settings_address = settings_register_external_setting(sizeof(dummy_setting));
	ADD_EVENT_LISTENER(settings_load, extend_settings_load);
	ADD_EVENT_LISTENER(settings_save, extend_settings_save);
	ADD_EVENT_LISTENER(settings_change, extend_settings_change);
	ADD_EVENT_LISTENER(settings_erase, extend_settings_erase);
	ADD_EVENT_LISTENER(protocol_send_cnc_settings, extend_protocol_send_cnc_settings);
}
