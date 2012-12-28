/*
 * File: win-tile.h
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
#ifndef INCLUDED_WIN_TILE_H
#define INCLUDED_WIN_TILE_H

#ifndef __MAKEDEPEND__
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif /* __MAKEDEPEND__ */
#include "z-config.h"
#include "h-basic.h"

#ifdef USE_GRAPHICS
typedef struct win32Tilesheet win32Tilesheet;
struct win32Tilesheet
{
	HBITMAP  hColorBitmap;
	HPALETTE hColorPalette;
	HBITMAP  hMaskBitmap;
	HPALETTE hMaskPalette;
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

extern int ReadTiles(HWND hWnd, char* filename, win32Tilesheet *tiles);
extern int FreeTiles(win32Tilesheet *tiles);
extern int ResizeTiles(HWND hWnd, win32Tilesheet *dest, win32Tilesheet *src); /* looks at cell sizes for scaling info */
extern int SaveTiles(HWND hWnd, char* filename, win32Tilesheet *tiles);

/*
 * return the image and rectangle of a tile
 * can be used to aggregate multiple image files?
 */
extern int GetTile(winTilesheet *tiles, byte y, byte x, byte flags,
                   HBITMAP *img, HBITMAP *msk, int *sy, int *sx, int *ey, int *ex);
#endif /* USE_GRAPHICS */

#endif /* INCLUDED_WIN_TILE_H */
