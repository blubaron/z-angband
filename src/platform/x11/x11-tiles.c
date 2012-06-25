/*
 * File: x11-tile.c
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
 * b) the "Angband license":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */

#include "x11-tile.h"
#include "maid-x11.h"

#ifdef USE_GRAPHICS

int LoadPNG(char *filename, XImage **ret_color, XImage **ret_mask,
	int *ret_wid, int *ret_hgt, bool premultiply);
int SavePNG(char *filename, XImage *color, XImage *mask,
	int wid, int hgt, bool premultiply);

int ReadTiles(char* filename, XTilesheet *tiles)
{
	Display *dpy = Metadpy->dpy;
	XImage *tiles_raw = NULL;
	XImage *mask_raw = NULL;
	char *ext;
	int filelen;
	int wid,hgt;

	filelen = strlen(filename);
	ext = strrchr(filename, '.');
	if (!ext) {
		return -1;
	}
	if (strncmp(ext+1, "bmp", 3) == 0) {
		/* Load the graphical tiles */
		tiles_raw = ReadBMP(dpy, filename, &wid, &hgt);
		if (filelen < 1019) {
			FILE *exists;
			my_strcpy(ext, "msk.bmp",1024-filelen-4);
			exists = my_fopen(filename, "r");
			if (exists) {
				my_fclose(exists);
				mask_raw = ReadBMP(dpy, filename, NULL, NULL);
				if (!mask_raw) {
					if (tiles_raw)
						XFREE(tiles_raw);
					plog_fmt("Cannot read mask file '%s'.", filename);
					return -4;
				}
			}
		}
		if (!tiles_raw) {
			plog_fmt("Cannot read file '%s'.", filename);
			return -4;
		}
	} else
	if (strncmp(ext+1, "png", 3) == 0) {
		if (tiles->bFlag & 1) {
			/* load for alphablending */
			/* png files are not supposed to have their color premultiplied
			 * with alpha, so see if the given file is already premultiplied */
			if (strstr(filename, "_pre")) {
				/* if so, just load it */
				if (LoadPNG(filename, &tiles_raw, NULL, &wid, &hgt, FALSE) < 0) {
					plog_fmt("Cannot read file '%s'.", filename);
					return -4;
				}
			} else {
				/* if not, see if there is already a premultiplied tileset */
				/* if there is load it */
				bool have_space = FALSE;
				char modname[1024];
				char *ext2 = NULL;

				my_strcpy(modname, filename, 1024);
				if (filelen < 1018) {
					FILE *exists;
					ext2 = strrchr(modname, '.');
					my_strcpy(ext2, "_pre.png", 1024-filelen-4);					
					exists = my_fopen(modname, "r");
					if (exists) {
						my_fclose(exists);
						if (filenewer(filename, modname)) {
							/* base file is newer so we want to recreate 
							 * premultiplied file */
							ext2 = NULL;
						}
					} else {
						/* premultiplied file does not exist, so create it */
						ext2 = NULL;
					}
				}
				if (ext2 && (filelen < 1018)) {
					/* at this point we know the file exists, so load it */
					if (LoadPNG(modname, &tiles_raw, NULL, &wid, &hgt, FALSE) < 0) {
						plog_fmt("Cannot read premultiplied version of file '%s'.", filename);
						return -4;
					}
				} else
				/* if not, load the base file and premultiply it */
				{
					if (LoadPNG(filename, &tiles_raw, NULL, &wid, &hgt, TRUE) < 0) {
						plog_fmt("Cannot read premultiplied version of file '%s'.", filename);
						return -4;
					}
					/* save the premultiplied file */
					/*if (SavePNG(modname, &tiles_raw, NULL, &wid, &hgt, FALSE) < 0) {
						plog_fmt("Cannot write premultiplied version of file '%s'.", filename);
					}*/
				}
			}
		} else {
			if (LoadPNG(filename, &tiles_raw, &mask_raw, &wid, &hgt, FALSE) < 0) {
				plog_fmt("Cannot read file '%s'.", filename);
				return -4;
			}
		}
	} else
	{
		plog_fmt("Unknown file type: %s.", filename);
		return -1;
	}

	/* return the new values */
	if (tiles->color)
		XDestroyImage(tiles->color);
	if (tiles->mask)
		XDestroyImage(tiles->mask);
	tiles->color = tiles_raw;
	tiles->mask = mask_raw;
	tiles->ImageWidth = wid;
	tiles->ImageHeight = hgt;
	return 0;
}

int FreeTiles(XTilesheet *tiles)
{
	if (!tiles) {
		return 0;
	}
	if (tiles->color) {
		XDestroyImage(tiles->color);
		tiles->color = NULL;
	}
	if (tiles->mask) {
		XDestroyImage(tiles->mask);
		tiles->mask = NULL;
	}
	ImageWidth = ImageHeight = 0;
	CellWidth = CellHeight = 0;
	bFlags = bReserved = 0;
	CellCenterX = CellCenterY = 0;
	CellShiftX = CellShiftY = 0;
	return 0;
}

int ResizeTiles(XTilesheet *dest, XTilesheet *src)
{
	if (!(src->color) && !(src->mask)) {
		return-1;
	}
	if ((dest->CellWidth == src->CellWidth)
		&& (dest->CellHeight == src->CellHeight))
	{
		return 0;
	}
	if ((dest->CellWidth == 0) || (dest->CellHeight == 0)
		|| (src->CellWidth == 0) || (src->CellHeight == 0))
	{
		return -2;
	}
	/* looks at cell sizes for scaling info */
	if (dest->color) {
		XDestroyImage(dest->color);
		dest->color = NULL;
	}
	if (dest->mask) {
		XDestroyImage(dest->mask);
		dest->mask = NULL;
	}
	if (src->color) {
		dest->color = ResizeImage(dpy, src->color, src->CellWidth,
			src->CellHeight, dest->CellWidth, dest->CellHeight);
	}
	if (src->mask) {
		dest->mask = ResizeImage(dpy, src->mask, src->CellWidth,
			src->CellHeight, dest->CellWidth, dest->CellHeight);
	}
	if (dest->color) {
		dest->ImageWidth = dest->color->width;
		dest->ImageHeight = dest->color->height;
	} else {
		dest->ImageWidth = dest->mask->width;
		dest->ImageHeight = dest->mask->height;
	}

	dest->bFlags = src->bFlags;
	dest->bReserved = src->bReserved;
	if (src->CellCenterX)
		dest->CellCenterX = dest->CellWidth * src->CellCenterX / src->CellWidth;
	else
		dest->CellCenterX = 0;
	if (src->CellCentery)
		dest->CellCenterY = dest->CellHeight * src->CellCenterY / src->CellHeight;
	else
		dest->CellCenterY = 0;
	if (src->CellShiftX)
		dest->CellShiftX = dest->CellWidth * src->CellShiftX / src->CellWidth;
	else
		dest->CellShiftX = 0;
	if (src->CellShiftY)
		dest->CellShiftY = dest->CellHeight * src->CellShiftY / src->CellHeight;
	else
		dest->CellShiftY = 0;

	return 0;
}

int SaveTiles(char* filename, XTilesheet *tiles)
{
	ext = strrchr(filename, '.');
	if (!ext) {
		return -1;
	}
	if (strncmp(ext+1, "png", 3) == 0) {
		return SavePNG(filename, tiles->color, tiles->mask, tiles->ImageWidth, tiles->ImageHeight, FALSE);
	}
	plog_fmt("Unknown file type trying to save tiles: %s", fielname);
	return -1;
}
#endif /* USE_GRAPHICS */


