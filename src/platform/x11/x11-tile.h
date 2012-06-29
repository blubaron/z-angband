/*
 * File: x11-tile.h
 * Purpose: support to load/save graphics files.
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
#ifndef INCLUDED_X11_TILE_H
#define INCLUDED_X11_TILE_H

#ifndef __MAKEDEPEND__
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif /* __MAKEDEPEND__ */
#include "z-config.h"
#include "h-basic.h"

#ifdef USE_GRAPHICS
typedef struct XTilesheet XTilesheet;
struct XTilesheet
{
	XImage *color;
	XImage *mask;
	int ImageWidth;
	int ImageHeight;
	byte CellWidth;
	byte CellHeight;
	byte bFlags;
	byte bReserved;
	/* extra info for isometric tilesets */
	byte CellCenterX;
	byte CellCenterY;
	byte CellShiftX;
	byte CellShiftY;
};

extern int ReadTiles(Display *dpy, char* filename, XTilesheet *tiles);
extern int FreeTiles(XTilesheet *tiles);
extern int ResizeTiles(Display *dpy, XTilesheet *dest, XTilesheet *src); /* looks at cell sizes for scaling info */
extern int SaveTiles(Display *dpy, char* filename, XTilesheet *tiles);
#endif /* USE_GRAPHICS */

#endif /* INCLUDED_X11_TILE_H */
