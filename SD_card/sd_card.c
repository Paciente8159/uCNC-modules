/*
	Name: sd_card.c
	Description: SD card module for µCNC.
	This adds SD card support via SPI

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 08-09-2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../cnc.h"
#include "../softspi.h"
#include "ffconf.h"
#include "mmcsd.h"
#include "ff.h"
#include "diskio.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "../system_menu.h"
#include "sd_messages.h"

#if (UCNC_MODULE_VERSION != 10800)
#error "This module is not compatible with the current version of µCNC"
#endif

#ifndef SD_CARD_DETECT_PIN
#define SD_CARD_DETECT_PIN DIN19
#endif

#ifndef SD_CONTINUE_ON_GCODE_ERROR
#define SD_STOP_ON_GCODE_ERROR
#endif

#ifndef MAX_PATH_LEN
#define MAX_PATH_LEN 128
#endif

enum SD_CARD_STATUS
{
	SD_UNDETECTED = 0,
	SD_DETECTED = 1,
	SD_MOUNTED = 2,
};

static uint8_t sd_card_mounted;
static FATFS fs;
// current opened file
static FIL fp;
// number of runs to executed the file code
static uint32_t file_runs;

void sd_card_mount(void)
{
	if (sd_card_mounted != SD_MOUNTED)
	{
		if ((f_mount(&fs, "", 1) == FR_OK))
		{
			protocol_send_feedback(__romstr__(SD_STR_SD_PREFIX SD_STR_SD_MOUNTED));
			sd_card_mounted = SD_MOUNTED;
			settings_init();
			return;
		}

		protocol_send_feedback(__romstr__(SD_STR_SD_PREFIX SD_STR_SD_ERROR));
	}
}

void sd_card_dir_list(void)
{
	FRESULT res;
	UINT i;
	FILINFO fno;
	DIR dp;

	// current dir
	char curdir[MAX_PATH_LEN];
	if (f_getcwd(curdir, MAX_PATH_LEN) == FR_OK)
	{
		protocol_send_string(__romstr__(SD_STR_DIR_PREFIX));
		serial_print_str(curdir);
		protocol_send_string(MSG_EOL);
		if (f_opendir(&dp, curdir) == FR_OK)
		{

			for (;;)
			{
				res = f_readdir(&dp, &fno); /* Read a directory item */
				if (res != FR_OK || fno.fname[0] == 0)
				{
					break; /* Break on error or end of dir */
				}
				if (fno.fattrib & AM_DIR)
				{ /* It is a directory */
					protocol_send_string(__romstr__(SD_STR_DIR_FORMATER));
				}
				else
				{ /* It is a file. */
					protocol_send_string(__romstr__(SD_STR_FILE_FORMATER));
				}

				i = strlen(fno.fname);
				for (uint8_t j = 0; j < i; j++)
				{
					serial_putc(fno.fname[j]);
				}
				protocol_send_string(MSG_EOL);
			}

			f_closedir(&dp);
		}
	}
}

void sd_card_cd(void)
{
	UINT i = 0;
	TCHAR newdir[RX_BUFFER_CAPACITY]; /* File name */

	while (serial_peek() == ' ')
	{
		serial_getc();
	}

	while (serial_peek() != EOL)
	{
		newdir[i++] = serial_getc();
		newdir[i] = 0;
	}

	if (f_chdir(newdir) == FR_OK)
	{
		char curdir[MAX_PATH_LEN];
		if (f_getcwd(curdir, MAX_PATH_LEN) == FR_OK)
		{
			serial_print_str(curdir);
			serial_print_str(">" STR_EOL);
		}
	}
}

void sd_card_file_print(void)
{
	UINT i = 0;
	TCHAR file[RX_BUFFER_CAPACITY]; /* File name */
	FIL tmp;

	while (serial_peek() == ' ')
	{
		serial_getc();
	}

	while (serial_peek() != EOL)
	{
		file[i++] = serial_getc();
		file[i] = 0;
	}

	if (f_open(&tmp, file, FA_READ) == FR_OK)
	{
		while (!f_eof(&tmp))
		{
			f_read(&tmp, file, 100, &i);
			file[i] = 0;
			serial_print_str(file);
			serial_flush();
		}

		f_close(&tmp);
		return;
	}

	protocol_send_feedback(__romstr__(SD_STR_FILE_PREFIX SD_STR_SD_ERROR));
}

void sd_card_file_run(void)
{
	UINT i = 0;
	TCHAR args[RX_BUFFER_CAPACITY]; /* get parameters */
	TCHAR *file;

	while (serial_peek() == ' ')
	{
		serial_getc();
	}

	while (serial_peek() != EOL)
	{
		args[i++] = serial_getc();
		args[i] = 0;
	}

	uint32_t runs = (uint32_t)strtol(args, &file, 10);

	while (*file == ' ')
	{
		file++;
	}

	if (f_open(&fp, file, FA_READ) == FR_OK)
	{
		file_runs = (runs != 0) ? runs : 1;
		protocol_send_string(MSG_START);
		protocol_send_string(__romstr__(SD_STR_FILE_PREFIX SD_STR_SD_RUNNING " - "));
		serial_print_int(file_runs);
		protocol_send_string(MSG_END);
		return;
	}
	else
	{
		f_close(&fp);
	}

	protocol_send_feedback(__romstr__(SD_STR_FILE_PREFIX SD_STR_SD_ERROR));
}

#ifdef SD_STOP_ON_GCODE_ERROR
// uint8_t sd_card_stop_onerror(void *args)
OVERRIDE_EVENT_HANDLER(cnc_exec_cmd_error)
{
	file_runs = 0;
	f_close(&fp);
	serial_rx_clear();
	// *handled = true;
	return EVENT_CONTINUE;
}

// CREATE_EVENT_LISTENER(cnc_exec_cmd_error, sd_card_stop_onerror);
#endif

#ifdef ENABLE_MAIN_LOOP_MODULES
/**
 * Handles SD card in the main loop
 * */
bool sd_card_loop(void *args)
{
#if (!(SD_CARD_DETECT_PIN < 0))
	if (mcu_get_input(SD_CARD_DETECT_PIN) && sd_card_mounted)
	{
		protocol_send_feedback(__romstr__(SD_STR_SD_PREFIX SD_STR_SD_NOT_FOUND));
		if (sd_card_mounted == SD_MOUNTED)
		{
			f_unmount("");
		}
		sd_card_mounted = SD_UNDETECTED;
		g_system_menu.flags |= SYSTEM_MENU_MODE_REDRAW;
	}
	else if (!mcu_get_input(SD_CARD_DETECT_PIN) && !sd_card_mounted)
	{
		sd_card_mounted = SD_DETECTED;
		cnc_delay_ms(2000);
		sd_card_mount();
		g_system_menu.flags |= SYSTEM_MENU_MODE_REDRAW;
	}
#endif

	uint32_t runs = file_runs;
	while (runs)
	{
		TCHAR buff[32];
		UINT i = 0;
		while (!f_eof(&fp))
		{
			if (serial_get_rx_freebytes() < 32)
			{
				// leaves the loop to enable code to run
				return EVENT_CONTINUE;
			}
			f_read(&fp, buff, 32, &i);
			uint8_t j = 0;
			do
			{
				mcu_com_rx_cb(buff[j++]);
			} while (--i);
		}

		if (--runs)
		{
			protocol_send_string(MSG_START);
			protocol_send_string(__romstr__(SD_STR_FILE_PREFIX SD_STR_SD_RUNNING " - "));
			serial_print_int(file_runs);
			protocol_send_string(MSG_END);
			f_lseek(&fp, 0);
		}
		else
		{
			protocol_send_feedback(__romstr__(SD_STR_FILE_PREFIX SD_STR_SD_FINISHED));
			f_close(&fp);
		}

		file_runs = runs;
	}

	return EVENT_CONTINUE;
}

CREATE_EVENT_LISTENER(cnc_dotasks, sd_card_loop);
#endif

#ifdef ENABLE_SETTINGS_MODULES
bool sd_settings_load(void *args)
// OVERRIDE_EVENT_HANDLER(settings_load)
{
	if ((sd_card_mounted != SD_MOUNTED))
	{
		return EVENT_CONTINUE;
	}

	UINT i = 0;
	bool result = false;
	FIL tmp;
	settings_args_t *p = args;

	if (f_open(&tmp, "/uCNCsettings.raw", FA_READ | FA_OPEN_EXISTING) == FR_OK)
	{
		protocol_send_feedback(__romstr__(SD_STR_SETTINGS_FOUND));
		f_lseek(&tmp, p->address);
		uint8_t error = f_read(&tmp, p->data, p->size, &i);
		if (p->size == i && !error)
		{
			protocol_send_feedback(__romstr__(SD_STR_SETTINGS_LOADED));
			result = true;
		}
	}
	else
	{
		protocol_send_feedback(__romstr__(SD_STR_SETTINGS_NOT_FOUND));
		f_close(&tmp);
	}

	return result;
}

CREATE_EVENT_LISTENER(settings_load, sd_settings_load);

bool sd_settings_save(void *args)
// OVERRIDE_EVENT_HANDLER(settings_save)
{
	if ((sd_card_mounted != SD_MOUNTED))
	{
		return EVENT_CONTINUE;
	}

	UINT i = 0;
	FIL tmp;
	bool result = false;
	settings_args_t *p = args;

	if (f_open(&tmp, "/uCNCsettings.raw", FA_OPEN_APPEND | FA_WRITE | FA_READ) == FR_OK)
	{
		f_lseek(&tmp, p->address);
		uint8_t error = f_write(&tmp, p->data, p->size, &i);
		f_sync(&tmp);

		if (p->size == i && !error)
		{
			protocol_send_feedback(__romstr__(SD_STR_SETTINGS_SAVED));
			result = true;
		}
	}

	f_close(&tmp);

	return result;
}

CREATE_EVENT_LISTENER(settings_save, sd_settings_save);

bool sd_settings_erase(void *args)
// OVERRIDE_EVENT_HANDLER(settings_erase)
{
	FIL tmp;
	bool result = false;

	if ((sd_card_mounted != SD_MOUNTED))
	{
		return EVENT_CONTINUE;
	}

	if (f_open(&tmp, "/uCNCsettings.raw", FA_CREATE_ALWAYS | FA_WRITE) == FR_OK)
	{
		protocol_send_feedback(__romstr__(SD_STR_SETTINGS_ERASED));
		result = true;
	}

	f_close(&tmp);

	return result;
}

CREATE_EVENT_LISTENER(settings_erase, sd_settings_erase);
#endif

#ifdef ENABLE_PARSER_MODULES
/**
 * Handles grbl commands for the SD card
 * */
bool sd_card_cmd_parser(void *args)
{
	grbl_cmd_args_t *cmd = args;

	strupr((char *)cmd->cmd);

	if (!strcmp("MNT", (char *)(cmd->cmd)))
	{
		sd_card_mount();
		*(cmd->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	if (!strcmp("UNMNT", (char *)(cmd->cmd)))
	{
		if (sd_card_mounted == SD_MOUNTED)
		{
			f_unmount("");
			sd_card_mounted = SD_DETECTED;
		}
		*(cmd->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	if (!strcmp("LS", (char *)(cmd->cmd)))
	{
		sd_card_dir_list();
		*(cmd->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	if (!strcmp("CD", (char *)(cmd->cmd)))
	{
		sd_card_cd();
		*(cmd->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	if (!strcmp("LPR", (char *)(cmd->cmd)))
	{
		sd_card_file_print();
		*(cmd->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	if (!strcmp("RUN", (char *)(cmd->cmd)))
	{
		sd_card_file_run();
		*(cmd->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	return GRBL_SYSTEM_CMD_EXTENDED_UNSUPPORTED;
}

CREATE_EVENT_LISTENER(grbl_cmd, sd_card_cmd_parser);
#endif

static FILINFO current_file;
static uint8_t dir_level;
static void system_menu_render_sd_card_item(uint8_t render_flags, system_menu_item_t *item)
{
	char buffer[SYSTEM_MENU_MAX_STR_LEN];
	if (mcu_get_input(SD_CARD_DETECT_PIN))
	{
		rom_strcpy(buffer, __romstr__(SD_STR_SD_NOT_FOUND));
	}
	else if (sd_card_mounted != SD_MOUNTED)
	{
		rom_strcpy(buffer, __romstr__(SD_STR_SD_UNMOUNTED));
	}
	else if (file_runs)
	{
		rom_strcpy(buffer, __romstr__(SD_STR_SD_RUNNING));
	}
	else
	{
		rom_strcpy(buffer, __romstr__(SD_STR_SD_MOUNTED));
	}

	system_menu_item_render_arg(render_flags, buffer);
}

static bool system_menu_action_sd_card_item(uint8_t action, system_menu_item_t *item)
{
	if (action == SYSTEM_MENU_ACTION_SELECT)
	{
		if (mcu_get_input(SD_CARD_DETECT_PIN))
		{
			char buffer[SYSTEM_MENU_MAX_STR_LEN];
			rom_strcpy(buffer, __romstr__(SD_STR_SD_PREFIX SD_STR_SD_NOT_FOUND "!"));
			system_menu_show_modal_popup(SYSTEM_MENU_MODAL_POPUP_MS, buffer);
		}
		else if (sd_card_mounted != SD_MOUNTED)
		{
			// mount the card
			sd_card_mount();
		}
		else if (file_runs)
		{
			// currently running job
			// do nothing
		}
		else
		{
			// go back to root dir
			f_chdir(("/"));
			dir_level = 0;
			// goto sd card menu
			g_system_menu.current_menu = 10;
			g_system_menu.current_index = 0;
			g_system_menu.current_multiplier = 0;
			g_system_menu.flags &= ~(SYSTEM_MENU_MODE_EDIT | SYSTEM_MENU_MODE_MODIFY);
		}

		return true;
	}

	return false;
}

// dynamic rendering of the sd card menu
// lists all dirs and files
static void system_menu_sd_card_render(uint8_t render_flags)
{
	uint8_t cur_index = g_system_menu.current_index;

	if (render_flags & SYSTEM_MENU_MODE_EDIT)
	{
		system_menu_render_header(current_file.fname);
		char buffer[SYSTEM_MENU_MAX_STR_LEN];
		rom_strcpy(buffer, __romstr__(SD_STR_FILE_PREFIX SD_STR_SD_CONFIRM));
		system_menu_item_render_label(render_flags, buffer);
		system_menu_item_render_arg(render_flags, current_file.fname);
	}
	else
	{
		FRESULT res;
		FILINFO fno;
		DIR dp;

		// current dir
		char curdir[MAX_PATH_LEN];
		if (f_getcwd(curdir, MAX_PATH_LEN) == FR_OK)
		{
			char *last_slash = strrchr(curdir, '/');
			system_menu_render_header((!last_slash) ? last_slash : "/\0");
			uint8_t index = 0;
			if (f_opendir(&dp, curdir) == FR_OK)
			{
				for (;;)
				{
					res = f_readdir(&dp, &fno); /* Read a directory item */
					if (res != FR_OK || fno.fname[0] == 0)
					{
						break; /* Break on error or end of dir */
					}

					if (system_menu_render_menu_item_filter(index))
					{
						char buffer[SYSTEM_MENU_MAX_STR_LEN];
						buffer[0] = (fno.fattrib & AM_DIR) ? '/' : ' ';
						memcpy(&buffer[1], fno.fname, MIN(SYSTEM_MENU_MAX_STR_LEN - 1, strlen(fno.fname)));
						system_menu_item_render_label(render_flags | ((cur_index == index) ? SYSTEM_MENU_MODE_SELECT : 0), buffer);
						// stores the current file info
						if ((cur_index == index))
						{
							memcpy(&current_file, &fno, sizeof(FILINFO));
						}
					}
					index++;
				}
				g_system_menu.total_items = index;
				f_closedir(&dp);
			}
		}
	}

	system_menu_render_nav_back((g_system_menu.current_index < 0 || g_system_menu.current_multiplier < 0));
	system_menu_render_footer();
}

bool system_menu_sd_card_action(uint8_t action)
{
	uint8_t render_flags = g_system_menu.flags;
	bool go_back = (g_system_menu.current_index < 0 || g_system_menu.current_multiplier < 0);

	// selects a file or a dir
	if (action == SYSTEM_MENU_ACTION_SELECT)
	{
		char buffer[SYSTEM_MENU_MAX_STR_LEN];

		if (render_flags & SYSTEM_MENU_MODE_EDIT)
		{
			// file print or quit
			// if it's over the nav back element
			if (go_back)
			{
				// don't run file and return to render sd content
				g_system_menu.flags &= ~SYSTEM_MENU_MODE_EDIT;
			}
			else
			{
				// run file
				if (f_open(&fp, current_file.fname, FA_READ) == FR_OK)
				{
					file_runs = 1;
					protocol_send_string(MSG_START);
					protocol_send_string(__romstr__(SD_STR_FILE_PREFIX SD_STR_SD_RUNNING " - "));
					serial_print_int(file_runs);
					protocol_send_string(MSG_END);
					system_menu_go_idle();
					rom_strcpy(buffer, __romstr__(SD_STR_FILE_PREFIX SD_STR_SD_RUNNING));
					system_menu_show_modal_popup(SYSTEM_MENU_MODAL_POPUP_MS, buffer);
				}
				else
				{
					rom_strcpy(buffer, __romstr__(SD_STR_FILE_PREFIX SD_STR_SD_FAILED));
					system_menu_show_modal_popup(SYSTEM_MENU_MODAL_POPUP_MS, buffer);
					f_close(&fp);
				}
			}
		}
		else
		{
			if (go_back)
			{
				if (dir_level)
				{
					// up one dir
					f_chdir((".."));
					g_system_menu.current_index = 0;
					g_system_menu.current_multiplier = 0;
					g_system_menu.total_items = 0;
					dir_level--;
					return true;
				}
				else
				{
					// return back let system menu handle it
					return false;
				}
			}

			if (current_file.fname[0] != 0)
			{
				if ((current_file.fattrib & AM_DIR))
				{
					if (f_chdir(current_file.fname) == FR_OK)
					{
						g_system_menu.current_index = 0;
						g_system_menu.current_multiplier = 0;
						g_system_menu.total_items = 0;
						dir_level++;
					}
					else
					{
						rom_strcpy(buffer, __romstr__(SD_STR_SD_PREFIX SD_STR_SD_ERROR));
						system_menu_show_modal_popup(SYSTEM_MENU_MODAL_POPUP_MS, buffer);
					}
				}
				else
				{
					// go to file run or quit menu
					g_system_menu.current_multiplier = 0;
					g_system_menu.flags |= SYSTEM_MENU_MODE_EDIT;
				}
			}
			else
			{
				rom_strcpy(buffer, __romstr__(SD_STR_SD_PREFIX SD_STR_SD_ERROR));
				system_menu_show_modal_popup(SYSTEM_MENU_MODAL_POPUP_MS, buffer);
			}
		}

		return true;
	}

	return false;
}

DECL_MODULE(sd_card)
{
	file_runs = 0;
	// STARTS SYSTEM MENU MODULE
	LOAD_MODULE(system_menu);
	// adds the sd card item to main menu
	DECL_MENU_ENTRY(1, sd_menu, "SD Card", NULL, system_menu_render_sd_card_item, NULL, system_menu_action_sd_card_item, NULL);

	// sd card file system rendering menu
	DECL_DYNAMIC_MENU(10, 1, system_menu_sd_card_render, system_menu_sd_card_action);

#ifdef ENABLE_MAIN_LOOP_MODULES
	ADD_EVENT_LISTENER(cnc_dotasks, sd_card_loop);
#else
#warning "Main loop extensions are not enabled. SD card will not work."
#endif

#ifdef ENABLE_PARSER_MODULES
	ADD_EVENT_LISTENER(grbl_cmd, sd_card_cmd_parser);
#ifdef SD_STOP_ON_GCODE_ERROR
// ADD_EVENT_LISTENER(cnc_exec_cmd_error, sd_card_stop_onerror);
#endif
#else
#warning "Parser extensions are not enabled. SD card commands will not work."
#endif
#ifdef ENABLE_SETTINGS_MODULES
	ADD_EVENT_LISTENER(settings_load, sd_settings_load);
	ADD_EVENT_LISTENER(settings_save, sd_settings_save);
	ADD_EVENT_LISTENER(settings_erase, sd_settings_erase);
#else
#warning "Settings extension not enabled. SD card stored settings will not work."
#endif
}
