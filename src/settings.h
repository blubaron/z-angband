/*
 * File: settings.h
 * Purpose: load/save port specific initialization settings from/to a text file.
 *
 * Copyright (c) 2012 Brett Reid
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */
#ifndef INCLUDED_SETTINGS_H
#define INCLUDED_SETTINGS_H

#include "h-basic.h"
/* Specifications for settings initialization file.
 * 
 * Section header should be in brackets. IE "[Angband]" or "[Term-1]"
 * values should be key value pairs, that are all on one line seperated by an
 * equal sign IE "Graphics=3". Everything before the = is the key, everything
 * after is the value.
 *
 * This is the same format as the windows ini file.
 *
 * Intended usage: when loading, load the file all at once, then use all of
 * the loaded settings (or a default value), then free all of the settings.
 * when saving, create a new settings structure, then set all of the settings
 * (including defaults used earlier), then write the whole file at once, then
 * free then the settings structure.
 */
typedef struct _ini_settings_value {
	struct _ini_settings_value *pNext;
	struct _ini_settings_section *pSection;
	char* key;
	char* value;
} ini_settings_value;

typedef struct _ini_settings_section {
	struct _ini_settings_section *pNext;
	char* name;
	ini_settings_value *pRoot;
	ini_settings_value *pEnd;
	int count;
} ini_settings_section;

typedef struct _ini_settings {
	ini_settings_section *pRoot;
	ini_settings_section *pEnd;
	int count;
} ini_settings;

int ini_settings_load(const char *filename, ini_settings **ret);
int ini_settings_save(const char *filename, ini_settings *ini);
int ini_settings_new(ini_settings **ret);
void ini_settings_close(ini_settings **ini);

ini_settings_value* ini_settings_find_key(ini_settings *ini, const char* section,
		 const char* key, ini_settings_section **retsec);

ini_settings_section* ini_settings_find_section(ini_settings *ini, const char* section);
ini_settings_value* ini_settings_find_section_key(ini_settings_section* section, const char* key);

int ini_settings_new_key(ini_settings *ini, ini_settings_section *sec,
		 const char *section, const char* key, char *value);

int ini_setting_get_string(ini_settings *ini, const char *section, const char* key, char *buf, int size, const char* def);
int ini_setting_get_sint32(ini_settings *ini, const char *section, const char* key, int def);
u32b ini_setting_get_uint32(ini_settings *ini, const char *section, const char* key, u32b def);

int ini_setting_ret_sint32(ini_settings *ini, const char *section, const char* key, int *ret, int def);
int ini_setting_ret_uint32(ini_settings *ini, const char *section, const char* key, u32b *ret, u32b def);

int ini_setting_set_string(ini_settings *ini, const char *section, const char* key, const char *value, int size);
int ini_setting_set_sint32(ini_settings *ini, const char *section, const char* key, int value);
int ini_setting_set_uint32(ini_settings *ini, const char *section, const char* key, u32b value);

int ini_setting_set_string(ini_settings *ini, const char *section, const char* key, const char *value, int size, const char* def);
int ini_setting_set_sint32(ini_settings *ini, const char *section, const char* key, int value, int def);
int ini_setting_set_uint32(ini_settings *ini, const char *section, const char* key, u32b value, u32b def);

#endif /* INCLUDED_SETTING_H */
