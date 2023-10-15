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
#include "../system_menu.h"
#include "sd_messages.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define PETIT_FAT_FS 1
#define FAT_FS 2

#ifdef ENABLE_SETTINGS_MODULES
#ifdef SD_FAT_FS
#undef SD_FAT_FS
#endif
#define SD_FAT_FS FAT_FS
#endif

#ifndef SD_FAT_FS
#define SD_FAT_FS PETIT_FAT_FS
#endif

#if (UCNC_MODULE_VERSION != 10800)
#error "This module is not compatible with the current version of µCNC"
#endif

#ifndef SD_CARD_DETECT_PIN
#define SD_CARD_DETECT_PIN DIN19
#endif

#ifndef SD_CONTINUE_ON_GCODE_ERROR
#define SD_STOP_ON_GCODE_ERROR
#endif

#ifndef FS_MAX_PATH_LEN
#define FS_MAX_PATH_LEN 128
#endif

// current opened file
static char cwd[FS_MAX_PATH_LEN];
static char *sd_parentdir(void);
// emulates basic chdir
static uint8_t sd_chfile(const char *newdir, uint8_t mode);

#ifdef DECL_SERIAL_STREAM
// declares a buffer
DECL_BUFFER(uint8_t, sdcard_buffer, RX_BUFFER_SIZE);
// declares the stream function callbacks
uint8_t sd_card_getc(void);
uint8_t sd_card_available(void);
void sd_card_clear(void);
static bool sd_card_stream_running;
#endif

#if (SD_FAT_FS == PETIT_FAT_FS)
#include "petit_fat_fs/pffconf.h"
#include "petit_fat_fs/pff.h"

#define FA_READ 0x01
#define FA_WRITE 0x02
#define FA_OPEN_EXISTING 0x00
#define FA_CREATE_NEW 0x04
#define FA_CREATE_ALWAYS 0x08
#define FA_OPEN_ALWAYS 0x10
#define FA_OPEN_APPEND 0x30

typedef struct
{
	FATFS *fs;
} FIL;

// file system
static FATFS cfs;
// current opend file
static FIL cwf;

FRESULT sd_mount(FATFS *fs)
{
	cwd[0] = 0;
	return pf_mount(fs);
}

#define sd_unmount(fs) (cwd[0] = 0)

FRESULT sd_fopen(const char *name, BYTE mode)
{
	if (pf_open(cwd) == FR_OK)
	{
		cwf.fs = &cfs;
		return FR_OK;
	}

	return FR_NO_FILE;
}

FRESULT sd_fread(char *buff, UINT btr, UINT *br)
{
	if (!cwf.fs)
	{
		return FR_NOT_OPENED;
	}

	return pf_read(buff, btr, br);
}

FRESULT sd_fwrite(const char *buff, UINT btw, UINT *bw)
{
	if (!cwf.fs)
	{
		return FR_NOT_OPENED;
	}
	return pf_write(buff, btw, bw);
}

FRESULT sd_fseek(DWORD ofs)
{
	if (!cwf.fs)
	{
		return FR_NOT_OPENED;
	}
	return pf_lseek(ofs);
}

FRESULT sd_fclose()
{
	if (cwf.fs)
	{
		cwf.fs = NULL;
		return sd_chfile("..", 0);
	}

	return FR_NOT_OPENED;
}
#define sd_fsync()
#define sd_opendir(dj, path) pf_opendir(dj, path)
#define sd_readdir(dj, fno) pf_readdir(dj, fno)
#define sd_closedir(fno)
bool sd_eof()
{
	if (cwf.fs)
	{
		return (cwf.fs->fsize == cwf.fs->fptr);
	}
	return true;
}
#else
#include "fat_fs/ffconf.h"
#include "fat_fs/ff.h"

// file system
static FATFS cfs;
// current opend file
static FIL cwf;

#define sd_mount(fs) f_mount(fs, "/", 1)
#define sd_unmount(fs) f_unmount(fs)
#define sd_fopen(path, mode) f_open(&cwf, path, mode)
#define sd_fread(buff, btr, br) f_read(&cwf, buff, btr, br)
#define sd_fwrite(buff, btw, bw) f_write(&cwf, buff, btw, bw)
#define sd_fseek(ofs) f_lseek(&cwf, ofs)
#define sd_fclose()         \
	if (cwf.obj.fs)         \
	{                       \
		f_close(&cwf);      \
		sd_chfile("..", 0); \
	}
#define sd_fsync() f_sync(&cwf)
#define sd_opendir(dj, path) f_opendir(dj, path)
#define sd_readdir(dj, fno) f_readdir(dj, fno)
#define sd_closedir(fno) f_closedir(fno);
#define sd_eof() f_eof(&cwf)
#endif

#include "diskio.h"

enum SD_CARD_STATUS
{
	SD_UNDETECTED = 0,
	SD_DETECTED = 1,
	SD_MOUNTED = 2,
};

static uint8_t sd_card_mounted;
// number of runs to executed the file code
static uint32_t file_runs;

static char *sd_parentdir(void)
{
	char *tail = strrchr(cwd, '/');
	if (!tail)
	{
		// is in root
		tail = cwd;
	}

	*tail = 0;

	return tail;
}

// emulates basic chdir and opens the dir or file if it exists
static uint8_t sd_chfile(const char *newdir, uint8_t mode)
{
	int16_t state = 0;
	uint16_t len = strlen(cwd);
	char *tail = &cwd[len];
	DIR dp;

	if (*newdir == '/')
	{
		// root dir
		tail = cwd;
		len = 0;
		*tail = 0;
		newdir++;
	}
	else if (len)
	{
		*tail = '/';
		tail++;
	}

	for (;;)
	{
		*tail = *newdir;
		switch (*newdir)
		{
		case '/':
		case 0:
			switch (state)
			{
			case 2:
				*tail = 0;
				tail = sd_parentdir();
				// continue
			case 1:
				// deletes dot or slash
				*tail = 0;
				tail = sd_parentdir();
				*tail = *newdir;
				len = strlen(cwd);
				break;
			default:
				if (state && state < FS_MAX_PATH_LEN)
				{
					// path with only dots not allowed
					*tail = 0;
					tail = sd_parentdir();
					return FR_NO_FILE;
				}
			}

			*tail = 0;
			// checks if is valid dir
			if (sd_opendir(&dp, cwd) != FR_OK)
			{
				// not a valid dir
				// if the end of the path checks if it's a valid file
				if (!*newdir && mode)
				{
					if (sd_fopen(cwd, mode) == FR_OK)
					{
						return FR_OK;
					}
				}

				// rewind
				tail = sd_parentdir();
				return FR_NO_FILE;
			}

			*tail = *newdir;

			if (!*newdir)
			{
				if (len)
				{
					tail--;
					if (*tail == '/')
					{
						*tail = 0;
					}
				}
				// terminated
				return FR_OK;
			}

			state = 0;
			break;
		default:
			state++;
			state = (*newdir == '.') ? state : FS_MAX_PATH_LEN;
			break;
		}

		newdir++;
		len++;
		tail++;
		*tail = 0;

		if (len >= FS_MAX_PATH_LEN)
		{
			// clamp
			tail = strrchr(cwd, '/');
			*tail = '/';
			tail++;
			*tail = 0;
			return FR_NO_FILE;
		}
	}

	// never reaches
	return FR_NO_FILE;
}

void sd_card_mount(void)
{
	if (sd_card_mounted != SD_MOUNTED)
	{
		if ((sd_mount(&cfs) == FR_OK))
		{
			cwd[0] = 0;
			protocol_send_feedback(__romstr__(SD_STR_SD_PREFIX SD_STR_SD_MOUNTED));
			sd_card_mounted = SD_MOUNTED;
#ifdef ENABLE_SETTINGS_MODULES
			settings_init();
#endif
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
	protocol_send_string(__romstr__(SD_STR_DIR_PREFIX));
	serial_print_str(cwd);
	protocol_send_string(MSG_EOL);
	if (sd_opendir(&dp, cwd) == FR_OK)
	{
		for (;;)
		{
			res = sd_readdir(&dp, &fno); /* Read a directory item */
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

		sd_closedir(&dp);
	}
}

void sd_card_cd(void)
{
	UINT i = 0;
	char newdir[RX_BUFFER_CAPACITY]; /* File name */

	while (serial_peek() == ' ')
	{
		serial_getc();
	}

	while (serial_peek() != EOL)
	{
		newdir[i++] = serial_getc();
	}

	newdir[i] = 0;

	if (sd_chfile(newdir, 0) == FR_OK)
	{
		if (strlen(cwd))
		{
			serial_print_str(cwd);
		}
		else
		{
			serial_putc('/');
		}
		serial_print_str(">" STR_EOL);
	}
	else
	{
		protocol_send_feedback(__romstr__(SD_STR_DIR_PREFIX SD_STR_SD_NOT_FOUND));
	}
}

void sd_card_file_print(void)
{
	UINT i = 0;
	char file[RX_BUFFER_CAPACITY]; /* File name */

	while (serial_peek() == ' ')
	{
		serial_getc();
	}

	while (serial_peek() != EOL)
	{
		file[i++] = serial_getc();
	}

	file[i] = 0;

	if (sd_chfile(file, FA_READ) == FR_OK)
	{
		while (!sd_eof())
		{
			memset(file, 0, RX_BUFFER_CAPACITY);
			if (sd_fread(file, RX_BUFFER_CAPACITY - 1, &i) != FR_OK)
			{
				sd_fclose();
				protocol_send_feedback(__romstr__(SD_STR_FILE_PREFIX SD_STR_SD_ERROR));
				return;
			}
			file[i] = 0;
			serial_print_str(file);
		}

		sd_fclose();
		protocol_send_feedback(__romstr__(SD_STR_FILE_PREFIX SD_STR_SD_FINISHED));
		return;
	}
	else
	{
		protocol_send_feedback(__romstr__(SD_STR_FILE_PREFIX SD_STR_SD_NOT_FOUND));
	}

	sd_fclose();
}

void sd_card_file_run(void)
{
	UINT i = 0;
	char args[RX_BUFFER_CAPACITY]; /* get parameters */
	char *file;

	while (serial_peek() == ' ')
	{
		serial_getc();
	}

	while (serial_peek() != EOL)
	{
		args[i++] = serial_getc();
	}

	args[i] = 0;

	uint32_t runs = (uint32_t)strtol(args, &file, 10);

	while (*file == ' ')
	{
		file++;
	}

	if (sd_chfile(file, FA_READ) == FR_OK)
	{
		file_runs = (runs != 0) ? runs : 1;
		protocol_send_string(MSG_START);
		protocol_send_string(__romstr__(SD_STR_FILE_PREFIX SD_STR_SD_RUNNING " - "));
		serial_print_int(file_runs);
		protocol_send_string(MSG_END);
#ifdef DECL_SERIAL_STREAM
		// open a readonly stream
		// the output is sent to the current holding interface
		sd_card_stream_running = true;
		serial_stream_readonly(&sd_card_getc, &sd_card_available, &sd_card_clear);
#endif
		return;
	}
	else
	{
		sd_fclose();
	}

	protocol_send_feedback(__romstr__(SD_STR_FILE_PREFIX SD_STR_SD_ERROR));
}

#ifdef SD_STOP_ON_GCODE_ERROR
// uint8_t sd_card_stop_onerror(void *args)
OVERRIDE_EVENT_HANDLER(cnc_exec_cmd_error)
{
	file_runs = 0;
	sd_fclose();
	serial_clear();
	// *handled = true;
	return EVENT_CONTINUE;
}

// CREATE_EVENT_LISTENER(cnc_exec_cmd_error, sd_card_stop_onerror);
#endif

#ifdef ENABLE_MAIN_LOOP_MODULES
/**
 * Handles SD card in the main loop
 * */
bool sd_card_dotasks(void *args)
{
#if (ASSERT_PIN(SD_CARD_DETECT_PIN))
	if (mcu_get_input(SD_CARD_DETECT_PIN) && sd_card_mounted)
	{
		protocol_send_feedback(__romstr__(SD_STR_SD_PREFIX SD_STR_SD_NOT_FOUND));
		if (sd_card_mounted == SD_MOUNTED)
		{
			sd_unmount("");
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
	if (runs)
	{
		UINT i = 0;

		if (!sd_eof())
		{
			if (BUFFER_WRITE_AVAILABLE(sdcard_buffer) <= 32)
			{
				return EVENT_CONTINUE;
			}
			uint8_t buff[32];
			uint8_t j;
			sd_fread(buff, 32, &i);
			BUFFER_WRITE(sdcard_buffer, buff, i, j);
		}
		else
		{
			if (--runs)
			{
				protocol_send_string(MSG_START);
				protocol_send_string(__romstr__(SD_STR_FILE_PREFIX SD_STR_SD_RUNNING " - "));
				serial_print_int(file_runs);
				protocol_send_string(MSG_END);
				sd_fseek(0);
			}
			else
			{
				protocol_send_feedback(__romstr__(SD_STR_FILE_PREFIX SD_STR_SD_FINISHED));
				sd_fclose();
			}

			file_runs = runs;
		}
	}
	else if (sd_card_stream_running)
	{
		if (BUFFER_EMPTY(sdcard_buffer))
		{
#ifdef DECL_SERIAL_STREAM
			// frees the stream
			serial_stream_change(NULL);
#endif
			sd_card_stream_running = false;
		}
	}

	return EVENT_CONTINUE;
}

CREATE_EVENT_LISTENER(cnc_dotasks, sd_card_dotasks);

#ifdef DECL_SERIAL_STREAM
// declares the stream function callbacks
uint8_t sd_card_getc(void)
{
	uint8_t c = 0;
	if (sd_card_available() > 0)
	{
		BUFFER_DEQUEUE(sdcard_buffer, &c);
	}
	return c;
}

uint8_t sd_card_available(void)
{
	if (BUFFER_READ_AVAILABLE(sdcard_buffer) < 32)
	{
		// if empty runs the sdcard tasks to see if it's filled
		sd_card_dotasks(NULL);
	}

	return BUFFER_READ_AVAILABLE(sdcard_buffer);
}

void sd_card_clear(void)
{
	BUFFER_CLEAR(sdcard_buffer);
}
#endif

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
	settings_args_t *p = args;

	if (sd_chfile("/uCNC.cfg", FA_READ | FA_OPEN_EXISTING) == FR_OK)
	{
		protocol_send_feedback(__romstr__(SD_STR_SETTINGS_FOUND));
		sd_fseek(p->address);
		uint8_t error = sd_fread(p->data, p->size, &i);
		if (p->size == i && !error)
		{
			protocol_send_feedback(__romstr__(SD_STR_SETTINGS_LOADED));
			result = true;
		}
	}
	else
	{
		protocol_send_feedback(__romstr__(SD_STR_SETTINGS_NOT_FOUND));
	}

	sd_fclose();

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
	bool result = false;
	settings_args_t *p = args;

	if (sd_chfile("/uCNC.cfg", FA_OPEN_APPEND | FA_WRITE | FA_READ) == FR_OK)
	{
		sd_fseek(p->address);
		uint8_t error = sd_fwrite(p->data, p->size, &i);
		sd_fsync();

		if (p->size == i && !error)
		{
			protocol_send_feedback(__romstr__(SD_STR_SETTINGS_SAVED));
			result = true;
		}
	}

	sd_fclose();

	return result;
}

CREATE_EVENT_LISTENER(settings_save, sd_settings_save);

bool sd_settings_erase(void *args)
// OVERRIDE_EVENT_HANDLER(settings_erase)
{
	bool result = false;

	if ((sd_card_mounted != SD_MOUNTED))
	{
		return EVENT_CONTINUE;
	}

	if (sd_chfile("/uCNC.cfg", FA_CREATE_ALWAYS | FA_WRITE) == FR_OK)
	{
		protocol_send_feedback(__romstr__(SD_STR_SETTINGS_ERASED));
		result = true;
	}

	sd_fclose();

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

	if (!strcmp("MNT", (char *)(cmd->cmd)) && !file_runs)
	{
		sd_card_mount();
		*(cmd->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	if (!strcmp("UNMNT", (char *)(cmd->cmd)) && !file_runs)
	{
		if (sd_card_mounted == SD_MOUNTED)
		{
			sd_unmount("");
			sd_card_mounted = SD_DETECTED;
		}
		*(cmd->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	if (!strcmp("LS", (char *)(cmd->cmd)) && !file_runs)
	{
		sd_card_dir_list();
		*(cmd->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	if (!strcmp("CD", (char *)(cmd->cmd)) && !file_runs)
	{
		sd_card_cd();
		*(cmd->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	if (!strcmp("LPR", (char *)(cmd->cmd)) && !file_runs)
	{
		sd_card_file_print();
		*(cmd->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	if (!strcmp("RUN", (char *)(cmd->cmd)) && !file_runs)
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
#if (ASSERT_PIN(SD_CARD_DETECT_PIN))
	if (mcu_get_input(SD_CARD_DETECT_PIN))
	{
		rom_strcpy(buffer, __romstr__(SD_STR_SD_NOT_FOUND));
	}
#else
	if (false)
	{
	}
#endif
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
#if (ASSERT_PIN(SD_CARD_DETECT_PIN))
		if (mcu_get_input(SD_CARD_DETECT_PIN))
		{
			char buffer[SYSTEM_MENU_MAX_STR_LEN];
			rom_strcpy(buffer, __romstr__(SD_STR_SD_PREFIX SD_STR_SD_NOT_FOUND "!"));
			system_menu_show_modal_popup(SYSTEM_MENU_MODAL_POPUP_MS, buffer);
		}
#else
		if (false)
		{
		}
#endif
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
			sd_chfile("/", 0);
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
		memset(buffer, 0, SYSTEM_MENU_MAX_STR_LEN);
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
		if (!strlen(cwd))
		{
			system_menu_render_header("/\0");
		}
		else
		{
			char *last_slash = strrchr(cwd, '/');
			if (last_slash == NULL)
			{
				last_slash = cwd;
			}
			else
			{
				last_slash++;
			}
			system_menu_render_header(last_slash);
		}
		uint8_t index = 0;
		if (sd_opendir(&dp, cwd) == FR_OK)
		{
			for (;;)
			{
				res = sd_readdir(&dp, &fno); /* Read a directory item */
				if (res != FR_OK || fno.fname[0] == 0)
				{
					break; /* Break on error or end of dir */
				}

				if (system_menu_render_menu_item_filter(index))
				{
					char buffer[SYSTEM_MENU_MAX_STR_LEN];
					memset(buffer, 0, SYSTEM_MENU_MAX_STR_LEN);
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
			sd_closedir(&dp);
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
				if (sd_chfile(current_file.fname, FA_READ) == FR_OK)
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
					sd_fclose();
				}
			}
		}
		else
		{
			if (go_back)
			{
				if (dir_level)
				{
					// up one dirs
					sd_chfile("..", 0);
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
					if (sd_chfile(current_file.fname, 0) == FR_OK)
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

DECL_MODULE(sd_card_pf)
{
	file_runs = 0;
	// STARTS SYSTEM MENU MODULE
	LOAD_MODULE(system_menu);
	// adds the sd card item to main menu
	DECL_MENU_ENTRY(1, sd_menu, "SD Card", NULL, system_menu_render_sd_card_item, NULL, system_menu_action_sd_card_item, NULL);

	// sd card file system rendering menu
	DECL_DYNAMIC_MENU(10, 1, system_menu_sd_card_render, system_menu_sd_card_action);

#ifdef ENABLE_MAIN_LOOP_MODULES
	ADD_EVENT_LISTENER(cnc_dotasks, sd_card_dotasks);
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
