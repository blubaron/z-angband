/*
 * File: pickfile.c
 * Purpose: pick a file from a directory, using the virtual terminal.
 *
 * Copyright (c) 2013 Brett Reid
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

#include "angband.h"
#include "pickfile.h"

/* Pick a file from the given directory, with one of the given extensions.
 * If ext1 is NULL, allow any file in the directory.
 * If ext1 is NULL, and ext4 is not, only list files that have ext4 as a
 * prefix, and possibly ext2 or ext3 as an extension.
 *
 * Return value is 0 in success, -1 if the selection was canceled, otherwise
 * < 0 if there was an error, > 0 if the buffer was to small.
 *
 * The filenames, including path, can have up to 1024 characters.
 */
errr file_pick(char*out, int outlen, cptr title, cptr dir, cptr ext1, cptr ext2, cptr ext3, cptr ext4)
{
	ang_dir *pdir;
	char buf[1024];
	menu_type *menu;
	rect_region r;
	ui_event selected;
	char *ext;
	char **filelist;
	int count = 0;
	int i = 0;
	int prefixlen = 0;

	/* get the number of matching files in the directory */
	pdir = dir_open(dir);
	if (!pdir) {
		return -4;
	}

	if (ext1) {
		while (dir_read(pdir, buf, 1024)) {
			ext = strrchr(buf, '.');
			if (ext) {
				if (streq(ext+1,ext1)) {
					count++;
				} else
				if (ext2 && streq(ext+1,ext2)) {
					count++;
				} else
				if (ext3 && streq(ext+1,ext3)) {
					count++;
				} else
				if (ext4 && streq(ext+1,ext4)) {
					count++;
				}
			}
		}
	} else
	if (ext4) {
		/* only display items with ext4 as prefix */
		while (dir_read(pdir, buf, 1024)) {
			if (prefix(buf, ext4)) {
				if (ext2 || ext3) {
					ext = strrchr(buf, '.');
					if (ext) {
						if (ext2 && streq(ext+1,ext2)) {
							count++;
						} else
						if (ext3 && streq(ext+1,ext3)) {
							count++;
						}
					}
				} else {
					count++;
				}
			}
		}
	} else
	{
		while (dir_read(pdir, buf, 1024)) {
			count++;
		}
	}

	dir_close(pdir);

	if (count) {
		/* allocate the list of files */
		filelist = C_RNEW(count, char*);
		if (!filelist) {
			return -3;
		}

		/* build the list of files */
		pdir = dir_open(dir);
		if (!pdir) {
			return -4;
		}

		i = 0;
		if (ext1) {
			while (dir_read(pdir, buf, 1024)) {
				ext = strrchr(buf, '.');
				if (ext) {
					if (streq(ext+1,ext1)) {
						filelist[i++] = (char*) string_make(buf);
					} else
					if (ext2 && streq(ext+1,ext2)) {
						filelist[i++] = (char*) string_make(buf);
					} else
					if (ext3 && streq(ext+1,ext3)) {
						filelist[i++] = (char*) string_make(buf);
					} else
					if (ext4 && streq(ext+1,ext4)) {
						filelist[i++] = (char*) string_make(buf);
					}
				}
			}
		} else
		if (ext4) {
			/* only display items with ext4 as prefix */
			prefixlen = strlen(ext4);
			while (dir_read(pdir, buf, 1024)) {
				if (prefix(buf, ext4)) {
					if (ext2 || ext3) {
						ext = strrchr(buf, '.');
						if (ext) {
							if (ext2 && streq(ext+1,ext2)) {
								filelist[i++] = (char*) string_make(buf+prefixlen);
							} else
							if (ext3 && streq(ext+1,ext3)) {
								filelist[i++] = (char*) string_make(buf+prefixlen);
							}
						}
					} else {
						filelist[i++] = (char*) string_make(buf+prefixlen);
					}
				}
			}
		} else
		{
			while (dir_read(pdir, buf, 1024)) {
				filelist[i++] = (char*) string_make(buf);
			}
		}

		dir_close(pdir);
	} else {
		/* no files were found, display an empty list */
		filelist = NULL;
	}

	/* Sort the strings */
	ang_sort_comp = ang_sort_comp_hook_string;
	ang_sort_swap = ang_sort_swap_hook_string;

	/* Sort the (unique) slopes */
	ang_sort((void *)filelist, NULL, count);

	menu = menu_new(MN_SKIN_COLUMNS, menu_find_iter(MN_ITER_STRINGS));
	if (!menu) {
		if (filelist) {
			for (i = 0; i < count; i++) {
				string_free(filelist[i]);
			}
			FREE(filelist);
		}
		return -3;
	}
	menu->flags = MN_NO_ACTION | MN_DBL_TAP | MN_NO_TAGS;
	menu->title = (char*)title;
	/*menu->prompt = "Pick a file or $Upress [ESC] to cancel$Y" ESCAPE_STR "$V";*/
	menu->prompt = "Pick a file or press ESC to cancel";
	menu_setpriv(menu, count, filelist);
	screen_save();
	button_backup_all(TRUE);

	r.col = 1;
	r.row = 1;
	r.width = -1;
	r.page_rows = -1;
	menu_layout(menu, &r);
	rect_region_erase_bordered(&r);
	selected = menu_select(menu, 0, TRUE);

	button_restore();
	screen_load();

	if (selected == EVT_SELECT) {
		int cursor = menu->cursor;
		if (prefixlen) {
			strnfmt(out, outlen, "%s%s", ext4, filelist[cursor]);
		} else {
  			strncpy(out, filelist[cursor], outlen);
		}
		out[outlen-1] = 0;
	}

	if (filelist) {
		for (i = 0; i < count; i++) {
			string_free(filelist[i]);
		}
		FREE(filelist);
	}
	FREE(menu);

	if (selected != EVT_SELECT) {
		return -1;
	}
	return 0;
}

