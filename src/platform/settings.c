/*
 * File: settings.c
 * Purpose: load port specific initialization settings from a text file.
 *
 * Copyright (c) 2012 Brett Reid
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband license":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */

#include <stdio.h>
#include "z-virt.h"
#include "z-file.h"
#include "z-form.h"
#include "settings.h"


ini_settings_value* ini_settings_find_key(ini_settings *ini, const char* section, const char* key, ini_settings_section **retsec)
{
	ini_settings_section *sec;
	ini_settings_value *val;

	sec = ini->pRoot;
	/* see if we have a shortcut */
	if (retsec && *retsec)
		sec = *retsec;
	while (sec) {
		if (!section || strcmp(sec->name, section)==0) {
			val = sec->pRoot;
			while (val) {
				if (strcmp(val->key, key) == 0) {
					if (retsec)
						*retsec = sec;
					return val;
				}
				val = val->pNext;
			}
			break;
		}
		sec = sec->pNext;
	}
	/* store the shortcut if we have it */
	if (retsec && sec)
		*retsec = sec;
	return NULL;
}

ini_settings_value* ini_settings_find_section_key(ini_settings_section* section, const char* key)
{
	ini_settings_value *val;

	if (!section)
		return NULL;

	val = section->pRoot;
	while (val) {
		if (strcmp(val->key, key) == 0) {
			return val;
		}
		val = val->pNext;
	}
	return NULL;
}
ini_settings_section* ini_settings_find_section(ini_settings *ini, const char* section)
{
	ini_settings_section *sec;

	sec = ini->pRoot;
	while (sec) {
		if (!section || strcmp(sec->name, section)==0) {
			return sec;
		}
		sec = sec->pNext;
	}
	return NULL;
}

int ini_settings_load(const char *filename, ini_settings **ret)
{
	int res = 0;
	int line = 0;
	char buf[1024];
	char *sep;
	ang_file *fp;
	ini_settings *ini;
	ini_settings_section *sec, *temps;
	ini_settings_value *val;

	if (!ret) 
		return -1;
	if (*ret) {
		/* the settings structure has already been allocated */
		ini = *ret;
	} else {
		ini = NULL;
		res = ini_settings_new(&ini);
		if (res < 0)
			return res;
		*ret = ini;
	}

	fp = file_open(filename, MODE_READ, FTYPE_TEXT);
	if (!fp) {
		return -4;
	}

	sec = NULL;
	while (0 <= file_getl(fp, buf, 1024)) {
		/* Count lines */
		line++;

		/* Skip "empty" lines */
		if (!buf[0]) continue;

		/* Skip "blank" lines */
		if (isspace(buf[0])) continue;

		/* Skip comments */
		if (buf[0] == '#') continue;

		/* process the line */
		/* See if we are in a new section */
		if (buf[0] == '[') {
			sep = strchr(buf, ']');
			if (sep) *sep = '\0';
			sec = ZNEW(ini_settings_section);
			if (!sec)
				return -3;
			sec->pRoot = NULL;
			sec->pEnd = NULL;
			sec->count = 0;
			sec->name = (char*)string_make(&(buf[1]));
			sec->pNext = NULL;
			/* add the new section to the list */
			if (!(ini->pRoot)) {
				ini->pRoot = sec;
			}
			if (ini->pEnd) {
				ini->pEnd->pNext = sec;
			}
			ini->pEnd = sec;
			ini->count++;
			continue;
		}
    
		/* find the equal sign */
		sep = strchr(buf, '=');
		if (!sep) continue;
		if (!sec) continue;

		*sep = '\0';

		if ((sep > buf) && isspace(*(sep-1))) {
			/* remove trailing whitespace from buf */
			char *end = sep-1;
			while ((end > buf) && isspace(*(end-1)))
				end--;
			*end = '\0';
		}
		if (isspace(*(sep+1))) {
			/* remove leading whitespace from sep */
			while (isspace(*(sep+1)))
				sep++;
		}

		temps = sec;
		val = ini_settings_find_section_key(sec, &(buf[0]));
		if (val) {
			/* set a new value for an existing key */
			if (val->value) {
				string_free(val->value);
			}
			val->value = (char*)string_make((sep+1));
		} else {
			/* create a new key */
			res = ini_settings_new_key(ini, sec, NULL, &(buf[0]), (sep+1));
		}
	}
	file_close(fp);
	return 0;
}

int ini_settings_save(const char *filename, ini_settings *ini)
{
	ang_file *fp;
	ini_settings_section *sec;
	ini_settings_value *val;

	if (!ini) 
		return -1;

	fp = file_open(filename, MODE_WRITE, FTYPE_TEXT);
	if (!fp) {
		return -4;
	}

	sec = ini->pRoot;
	while (sec) {
		file_putf(fp, "[%s]\n", sec->name);
		val = sec->pRoot;
		while (val) {
			file_putf(fp, "%s=%s\n", val->key,val->value);
			val = val->pNext;
		}
		file_putf(fp, "\n");
		sec = sec->pNext;
	}
	file_close(fp);
	return 0;
}

int ini_settings_new(ini_settings **ret)
{
	ini_settings *ini;
	if (!ret)
		return -2;
	if (*ret) {
		ini = *ret;
	} else {
		ini = ZNEW(ini_settings);
		if (!ini)
			return -3;
	}
	ini->pRoot = NULL;
	ini->pEnd = NULL;
	ini->count = 0;

	*ret = ini;
	return 0;
}
void ini_settings_close(ini_settings **ini)
{
	ini_settings_section *sec, *nexts;
	ini_settings_value *val, *nextv;

	if (!ini)
		return;
	if (!(*ini))
		return;
	sec = (*ini)->pRoot;
	while (sec) {
		nexts = sec->pNext;
		val = sec->pRoot;
		while (val) {
			nextv = val->pNext;
			string_free(val->key);
			string_free(val->value);
			FREE(val);
			val = nextv;
		}
		string_free(sec->name);
		FREE(sec);
		sec = nexts;
	}
	FREE(*ini);
	*ini = NULL;
}

int ini_settings_new_key(ini_settings *ini, ini_settings_section *sec, const char *section, const char* key, const char *value)
{
	ini_settings_value* val;
	val = ZNEW(ini_settings_value);
	if (!val) {
		return -3;
	}
	val->pNext = NULL;
	val->key = (char*)string_make(key);
	val->value = (char*)string_make(value);

	if (sec) {
		/* add the new pair to the list */
		if (!(sec->pRoot)) {
			sec->pRoot = val;
		}
		if (sec->pEnd) {
			sec->pEnd->pNext = val;
		}
		sec->pEnd = val;
		sec->count++;
	} else
	if (section) {
		/* we need to create a new section */
		sec = ZNEW(ini_settings_section);
		if (!sec) {
			string_free(val->key);
			string_free(val->value);
			FREE(val);
			return -3;
		}
		sec->pRoot = val;
		sec->pEnd = val;
		sec->count = 1;
		sec->name = (char*)string_make(section);
		sec->pNext = NULL;
		/* add the new section to the list */
		if (!(ini->pRoot)) {
			ini->pRoot = sec;
		}
		if (ini->pEnd) {
			ini->pEnd->pNext = sec;
		}
		ini->pEnd = sec;
	} else
	{
		/* no place to put the new pair */
		string_free(val->key);
		string_free(val->value);
		FREE(val);
		return 1;
	}
	return 0;
}


int ini_setting_get_string(ini_settings *ini, const char *section, const char* key, char *buf, int size, const char* def)
{
	ini_settings_section *sec;
	ini_settings_value *val;
	if (!buf) {
		return -2;
	}
	sec = ini->pRoot;
	while (sec) {
		if (strcmp(sec->name, section)==0) {
			val = sec->pRoot;
			while (val) {
				if (strcmp(val->key, key) == 0) {
					strncpy(buf, val->value, size);
					buf[size-1] = '0';
					return 0;
				}
				val = val->pNext;
			}
			break;
		}
		sec = sec->pNext;
	}
	strncpy(buf, def, size);
	return 1;
}

int ini_setting_ret_sint32(ini_settings *ini, const char *section, const char* key, int *ret, int def)
{
	ini_settings_section *sec;
	ini_settings_value *val;
	if (!ret) {
		return -2;
	}
	sec = ini->pRoot;
	while (sec) {
		if (strcmp(sec->name, section)==0) {
			val = sec->pRoot;
			while (val) {
				if (strcmp(val->key, key) == 0) {
					*ret = atoi(val->value);
					return 0;
				}
				val = val->pNext;
			}
			break;
		}
		sec = sec->pNext;
	}
	*ret = def;
	return 1;
}

int ini_setting_ret_uint32(ini_settings *ini, const char *section, const char* key, u32b *ret, u32b def)
{
	ini_settings_section *sec;
	ini_settings_value *val;
	if (!ret) {
		return -2;
	}
	sec = ini->pRoot;
	while (sec) {
		if (strcmp(sec->name, section)==0) {
			val = sec->pRoot;
			while (val) {
				if (strcmp(val->key, key) == 0) {
					*ret = (u32b)atoi(val->value);
					return 0;
				}
				val = val->pNext;
			}
			break;
		}
		sec = sec->pNext;
	}
	*ret = def;
	return 1;
}

int ini_setting_get_sint32(ini_settings *ini, const char *section, const char* key, int def)
{
	ini_settings_section *sec;
	ini_settings_value *val;

	sec = ini->pRoot;
	while (sec) {
		if (strcmp(sec->name, section)==0) {
			val = sec->pRoot;
			while (val) {
				if (strcmp(val->key, key) == 0) {
					return atoi(val->value);
				}
				val = val->pNext;
			}
			break;
		}
		sec = sec->pNext;
	}
	return def;
}

u32b ini_setting_get_uint32(ini_settings *ini, const char *section, const char* key, u32b def)
{
	ini_settings_section *sec;
	ini_settings_value *val;

	sec = ini->pRoot;
	while (sec) {
		if (strcmp(sec->name, section)==0) {
			val = sec->pRoot;
			while (val) {
				if (strcmp(val->key, key) == 0) {
					return (u32b)atoi(val->value);
				}
				val = val->pNext;
			}
			break;
		}
		sec = sec->pNext;
	}
	return def;
}


int ini_setting_set_string(ini_settings *ini, const char *section, const char* key, const char *value, int size)
{
	ini_settings_section *sec = NULL;
	ini_settings_value *val;

	/* prevent a compiler warning */
	(void)size;

	val = ini_settings_find_key(ini, section, key,  &sec);
	if (val) {
		/* we already have this key for this section, so just change its value */
		if (val->value) {
			string_free(val->value);
		}
		val->value = (char*)string_make(value);
		return 1;
	}
	/* we need to make a new key */
	return ini_settings_new_key(ini, sec, section, key, value);
}

int ini_setting_set_sint32(ini_settings *ini, const char *section, const char* key, int value)
{
	ini_settings_section *sec = NULL;
	ini_settings_value *val;

	val = ini_settings_find_key(ini, section, key,  &sec);
	if (val) {
		/* we already have this key for this section, so just change its value */
		if (val->value) {
			string_free(val->value);
		}
		val->value = (char*)string_make(format("%d",value));
		return 1;
	}
	/* we need to make a new key */
	return ini_settings_new_key(ini, sec, section, key, format("%d",value));
}

int ini_setting_set_uint32(ini_settings *ini, const char *section, const char* key, u32b value)
{
	ini_settings_section *sec = NULL;
	ini_settings_value *val;

	val = ini_settings_find_key(ini, section, key,  &sec);
	if (val) {
		/* we already have this key for this section, so just change its value */
		if (val->value) {
			string_free(val->value);
		}
		val->value = (char*)string_make(format("%u",value));
		return 1;
	}
	/* we need to make a new key */
	return ini_settings_new_key(ini, sec, section, key, format("%u",value));
}

int ini_setting_set_string_def(ini_settings *ini, const char *section, const char* key, const char *value, int size, const char *def)
{
	ini_settings_section *sec = NULL;
	ini_settings_value *val;

	/* prevent a compiler warning */
	(void)size;

	val = ini_settings_find_key(ini, section, key,  &sec);
	if (val) {
		if (def && (strcmp(value, def) == 0)) {
			/* we already have this key for this section, so remove it */
			/* since a value the same as the default will not be stored */
			ini_settings_value *prev,*test;
			test = sec->pRoot;
			prev = NULL;
			while (test) {
				if (test == val) {
					break;
				}
				prev = test;
				test = test->pNext;
			}
			if (test) {
				if (prev) {
					prev->pNext = test->pNext;
				}
				if (test == sec->pRoot) {
					sec->pRoot = test->pNext;
				}
				if (test == sec->pEnd) {
					sec->pEnd = prev;
				}
			}
			if (val->key) {
				string_free(val->key);
			}
			if (val->value) {
				string_free(val->value);
			}
			FREE(val);
		} else {
			/* we already have this key for this section, so just change its value */
			if (val->value) {
				string_free(val->value);
			}
			val->value = (char*)string_make(value);
		}
		return 1;
	}
	if (def && (strcmp(value, def) == 0)) {
		/* a value the same as the default will not be stored */
		return 0;
	}
	/* we need to make a new key */
	return ini_settings_new_key(ini, sec, section, key, value);
}

int ini_setting_set_sint32_def(ini_settings *ini, const char *section, const char* key, int value, int def)
{
	ini_settings_section *sec = NULL;
	ini_settings_value *val;

	val = ini_settings_find_key(ini, section, key,  &sec);
	if (val) {
		if (value == def) {
			/* we already have this key for this section, so remove it */
			/* since a value the same as the default will not be stored */
			ini_settings_value *prev,*test;
			test = sec->pRoot;
			prev = NULL;
			while (test) {
				if (test == val) {
					break;
				}
				prev = test;
				test = test->pNext;
			}
			if (test) {
				if (prev) {
					prev->pNext = test->pNext;
				}
				if (test == sec->pRoot) {
					sec->pRoot = test->pNext;
				}
				if (test == sec->pEnd) {
					sec->pEnd = prev;
				}
			}
			if (val->key) {
				string_free(val->key);
			}
			if (val->value) {
				string_free(val->value);
			}
			FREE(val);
		} else {
			/* we already have this key for this section, so just change its value */
			if (val->value) {
				string_free(val->value);
			}
			val->value = (char*)string_make(format("%d",value));
		}
		return 1;
	}
	if (value == def) {
		/* a value the same as the default will not be stored */
		return 0;
	}
	/* we need to make a new key */
	return ini_settings_new_key(ini, sec, section, key, format("%d",value));
}

int ini_setting_set_uint32_def(ini_settings *ini, const char *section, const char* key, u32b value, u32b def)
{
	ini_settings_section *sec = NULL;
	ini_settings_value *val;

	val = ini_settings_find_key(ini, section, key,  &sec);
	if (val) {
		if (value == def) {
			/* we already have this key for this section, so remove it */
			/* since a value the same as the default will not be stored */
			ini_settings_value *prev,*test;
			test = sec->pRoot;
			prev = NULL;
			while (test) {
				if (test == val) {
					break;
				}
				prev = test;
				test = test->pNext;
			}
			if (test) {
				if (prev) {
					prev->pNext = test->pNext;
				}
				if (test == sec->pRoot) {
					sec->pRoot = test->pNext;
				}
				if (test == sec->pEnd) {
					sec->pEnd = prev;
				}
			}
			if (val->key) {
				string_free(val->key);
			}
			if (val->value) {
				string_free(val->value);
			}
			FREE(val);
		} else {
			/* we already have this key for this section, so just change its value */
			if (val->value) {
				string_free(val->value);
			}
			val->value = (char*)string_make(format("%u",value));
		}
		return 1;
	}
	if (value == def) {
		/* a value the same as the default will not be stored */
		return 0;
	}
	/* we need to make a new key */
	return ini_settings_new_key(ini, sec, section, key, format("%u",value));
}

