/*
 * File: pickfile.h
 * Purpose: pick a file from a directory, using the virtual terminal.
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
#ifndef INCLUDED_PICKFILE_H
#define INCLUDED_PICKFILE_H

#include "h-basic.h"

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
errr file_pick(char*out, int outlen, cptr title, cptr dir,
                cptr ext1, cptr ext2, cptr ext3, cptr ext4);

#endif /* INCLUDED_PICKFILE_H */
