#ifndef SD_MESSAGES_H
#define SD_MESSAGES_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../system_languages.h"

#if (SYSTEM_LANGUAGE == LANGUAGE_EN)
#define SD_STR_SD_PREFIX "SD card "
#define SD_STR_FILE_PREFIX "File "
#define SD_STR_SD_MOUNTED "mounted"
#define SD_STR_SD_UNMOUNTED "unmounted"
#define SD_STR_SD_NOT_FOUND "not found"
#define SD_STR_SD_RUNNING "running"
#define SD_STR_SD_FAILED "failed!"
#define SD_STR_SD_FINISHED "finished!"
#define SD_STR_SD_CONFIRM "run?"
#define SD_STR_SD_ERROR "error!"
#define SD_STR_DIR_PREFIX "Directory of "
#define SD_STR_DIR_FORMATER "<dir>\t"
#define SD_STR_FILE_FORMATER "     \t"
#define SD_STR_SETTINGS_FOUND SD_STR_SD_PREFIX "settings found"
#define SD_STR_SETTINGS_LOADED SD_STR_SD_PREFIX "settings loaded"
#define SD_STR_SETTINGS_NOT_FOUND SD_STR_SD_PREFIX "settings not found"
#define SD_STR_SETTINGS_SAVED SD_STR_SD_PREFIX "settings saved"
#define SD_STR_SETTINGS_ERASED SD_STR_SD_PREFIX "settings erased"
#else
//if language not present defaults to english
#define SD_STR_SD_PREFIX "SD card "
#define SD_STR_FILE_PREFIX "File "
#define SD_STR_SD_MOUNTED "mounted"
#define SD_STR_SD_UNMOUNTED "unmounted"
#define SD_STR_SD_NOT_FOUND "not found"
#define SD_STR_SD_RUNNING "running"
#define SD_STR_SD_FAILED "failed!"
#define SD_STR_SD_FINISHED "finished!"
#define SD_STR_SD_CONFIRM "run?"
#define SD_STR_SD_ERROR "error!"
#define SD_STR_DIR_PREFIX "Directory of "
#define SD_STR_DIR_FORMATER "<dir>\t"
#define SD_STR_FILE_FORMATER "     \t"
#define SD_STR_SETTINGS_FOUND SD_STR_SD_PREFIX "settings found"
#define SD_STR_SETTINGS_LOADED SD_STR_SD_PREFIX "settings loaded"
#define SD_STR_SETTINGS_NOT_FOUND SD_STR_SD_PREFIX "settings not found"
#define SD_STR_SETTINGS_SAVED SD_STR_SD_PREFIX "settings saved"
#define SD_STR_SETTINGS_ERASED SD_STR_SD_PREFIX "settings erased"
#endif

#ifdef __cplusplus
}
#endif

#endif