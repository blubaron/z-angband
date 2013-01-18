/*
 * File: platform/x11/x11-layout.c
 * Purpose: load port specific initialization settings from a text file.
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
#include "x11-term.h"

/*
 * Default window layout function
 *
 * Just a big list of what to do for a specific screen resolution.
 *
 * Note: graphics modes are hardcoded, using info from 
 * angband/lib/xtra/graf/graphics.txt at the time of this writing
 *
 * Return values:    0 - Success
 *                  -1 - Invalid argument
 *                  -3 - Out of memory
 */
int default_layout_x11(term_data *data, int maxterms, metadpy *dpy)
{
	int sx, sy;

	if (!dpy) {
		return -1;
	}
printf("default layout x11\n");
	return 0;
}
