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
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "h-basic.h"
/*#include "png.h"*/

int LoadPNG(char *filename, XImage **ret_color, XImage **ret_mask,
	int *ret_wid, int *ret_hgt, bool premultiply)
{
	return 0;
}
int SavePNG(char *filename, XImage *color, XImage *mask,
	int wid, int hgt, bool premultiply)
{
	return 0;
}

