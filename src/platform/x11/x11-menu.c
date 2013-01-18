/*
 * File: platform/x11/x11-menu.c
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
#include "ui-region.h"
#include "ui-menu.h"
#include "tvalsval.h"
#include "button.h"
#include "x11-term.h"
#include "grafmode.h"
#include "x11-tile.h"

extern bool SaveWindow(infowin *win, char*filename);
extern errr Term_xtra_x11_react(void);
extern errr Term_xtra_x11_clear(void);

#ifdef USE_GRAPHICS
extern XTilesheet tiles; /* tiles loaded from disk */
extern XTilesheet viewtiles; /* tiles that are displayed on screen */
extern XTilesheet maptiles; /* tiles same size as font */

int main_menu_x11_graphics_mult(metadpy *mdpy, int mx, int my)
{
	menu_type_a *m;
	rect_region r;
	int selected, tw, th;
	char *labels;
	graphics_mode *mode;

	/* Paranoia */
	if (!mdpy) {
		return 0;
	}
	tw = tile_width_mult;
	th = tile_height_mult;

	m = menu_dynamic_new();
	if (!m) {
		return 0;
	}

	labels = string_make(lower_case);
	m->selections = labels;

	/* list tile multipliers */
#define TILE_MULT_CONTEXT(w,h,i) \
	if ((tile_width_mult == w) && (tile_height_mult == h)) { \
		menu_dynamic_add(m, #w"x"#h, i); \
		menu_dynamic_set_select(m); \
	} else { \
		menu_dynamic_add(m, "$"#w"x"#h, i); \
	}
	TILE_MULT_CONTEXT(1,1,1);
	TILE_MULT_CONTEXT(2,1,2);
	TILE_MULT_CONTEXT(2,2,3);
	TILE_MULT_CONTEXT(3,1,4);
	TILE_MULT_CONTEXT(3,2,5);
	TILE_MULT_CONTEXT(3,3,6);
	TILE_MULT_CONTEXT(4,2,7);
	TILE_MULT_CONTEXT(4,3,8);
	TILE_MULT_CONTEXT(4,4,9);
	TILE_MULT_CONTEXT(5,3,10);
	TILE_MULT_CONTEXT(6,3,11);
	TILE_MULT_CONTEXT(6,4,12);
	TILE_MULT_CONTEXT(6,6,13);
	TILE_MULT_CONTEXT(8,4,14);
	TILE_MULT_CONTEXT(8,6,15);
	TILE_MULT_CONTEXT(8,8,16);
	TILE_MULT_CONTEXT(16,8,17);
	TILE_MULT_CONTEXT(16,12,18);
	TILE_MULT_CONTEXT(16,16,19);
	/*menu_dynamic_add_label(m, "", '\0', 101, labels);*/
	menu_dynamic_add_label(m, "", '\0', 101, labels);
	menu_dynamic_add_label(m, "Back", '*', 100, labels);

	/* work out display region */
	r.width = menu_dynamic_longest_entry(m) + 3 + 2; /* +3 for tag, 2 for pad */
	if (mx > Term->wid - r.width - 1) {
		r.col = Term->wid - r.width - 1;
	} else {
		r.col = mx + 1;
	}
	r.page_rows = m->count;
	if (my > Term->hgt - r.page_rows - 1) {
		if (my - r.page_rows - 1 <= 0) {
			/* menu has too many items, so put in upper right corner */
			r.row = 1;
			r.col = Term->wid - r.width - 1;
		} else {
			r.row = Term->hgt - r.page_rows - 1;
		}
	} else {
		r.row = my + 1;
	}

	/* Hack -- no flush needed */
	msg_flag = FALSE;
	screen_save();

	menu_layout(m, &r);
	rect_region_erase_bordered(&r);

	prtf(0, 0, "($UEnter to select$Y\n$V, $UESC$Y%c$V) Tile Multiplier:", ESCAPE);
	selected = menu_dynamic_select(m);

	menu_dynamic_free(m);
	string_free(labels);

	screen_load();

	switch (selected) {
	case 1:
		{
			tile_width_mult = 1;
			tile_height_mult = 1;
		} break;
	case 2:
		{
			tile_width_mult = 2;
			tile_height_mult = 1;
		} break;
	case 3:
		{
			tile_width_mult = 2;
			tile_height_mult = 2;
		} break;
	case 4:
		{
			tile_width_mult = 3;
			tile_height_mult = 1;
		} break;
	case 5:
		{
			tile_width_mult = 3;
			tile_height_mult = 2;
		} break;
	case 6:
		{
			tile_width_mult = 3;
			tile_height_mult = 3;
		} break;
	case 7:
		{
			tile_width_mult = 4;
			tile_height_mult = 2;
		} break;
	case 8:
		{
			tile_width_mult = 4;
			tile_height_mult = 3;
		} break;
	case 9:
		{
			tile_width_mult = 4;
			tile_height_mult = 4;
		} break;
	case 10:
		{
			tile_width_mult = 5;
			tile_height_mult = 3;
		} break;
	case 11:
		{
			tile_width_mult = 6;
			tile_height_mult = 3;
		} break;
	case 12:
		{
			tile_width_mult = 6;
			tile_height_mult = 4;
		} break;
	case 13:
		{
			tile_width_mult = 6;
			tile_height_mult = 6;
		} break;
	case 14:
		{
			tile_width_mult = 8;
			tile_height_mult = 4;
		} break;
	case 15:
		{
			tile_width_mult = 8;
			tile_height_mult = 6;
		} break;
	case 16:
		{
			tile_width_mult = 8;
			tile_height_mult = 8;
		} break;
	case 17:
		{
			tile_width_mult = 16;
			tile_height_mult = 8;
		} break;
	case 18:
		{
			tile_width_mult = 16;
			tile_height_mult = 12;
		} break;
	case 19:
		{
			tile_width_mult = 16;
			tile_height_mult = 16;
		} break;
	case 100:
		{
			/* go back to the previous menu */
			return 2;
		} break;
	case 101:
		{
			/* a seperator line - do nothing */
			return 3;
		} break;
	}

	if ((tw != tile_width_mult) || (th != tile_height_mult)) {
		int res;
		Display *dpy = mdpy->dpy;
		term_data *td = (term_data*)(Term->data);

		/* resize the tiles used for viewing */
		FreeTiles(&viewtiles);
		viewtiles.CellWidth = td->tile_wid * tile_width_mult;
		viewtiles.CellHeight = td->tile_hgt * tile_height_mult;
		res = ResizeTiles(dpy, &viewtiles, &tiles);

		/* Clear screen */
		Term_xtra_x11_clear();

		/* Hack - redraw everything */
		if (Term->resize_hook)
			Term->resize_hook();
	}
	return 1;
}

int main_menu_x11_graphics(metadpy *mdpy, int mx, int my)
{
	menu_type_a *m;
	rect_region r;
	int selected;
	char *labels;
	graphics_mode *mode;

	m = menu_dynamic_new();
	if (!m) {
		return 0;
	}

	labels = string_make(lower_case);
	m->selections = labels;

	/* show graphics modes */
	if (arg_graphics == GRAPHICS_NONE) {
		menu_dynamic_add(m, "None", GRAPHICS_NONE+1);
		menu_dynamic_set_select(m);
	} else {
		menu_dynamic_add(m, "$None", GRAPHICS_NONE+1);
	}
	menu_dynamic_add_label(m, "", '\0', graphics_mode_high_id+2, labels);
	mode = graphics_modes;
	while (mode) {
		if (mode->grafID != GRAPHICS_NONE) {
			if (arg_graphics == mode->grafID) {
				menu_dynamic_add(m, mode->menuname, mode->grafID+1);
				menu_dynamic_set_select(m);
			} else {
				menu_dynamic_add(m, format("$%s",mode->menuname), mode->grafID+1);
			}
		}
		mode = mode->pNext;
	}
	menu_dynamic_add_label(m, "", '\0', graphics_mode_high_id+2, labels);
	menu_dynamic_add_label(m, "Tile Multiplier", ' ', graphics_mode_high_id+3, labels);
	menu_dynamic_add_label(m, "", '\0', graphics_mode_high_id+2, labels);
	menu_dynamic_add_label(m, "Back", '*', graphics_mode_high_id+4, labels);

	/* work out display region */
	r.width = menu_dynamic_longest_entry(m) + 3 + 2; /* +3 for tag, 2 for pad */
	if (mx > Term->wid - r.width - 1) {
		r.col = Term->wid - r.width - 1;
	} else {
		r.col = mx + 1;
	}
	r.page_rows = m->count;
	if (my > Term->hgt - r.page_rows - 1) {
		if (my - r.page_rows - 1 <= 0) {
			/* menu has too many items, so put in upper right corner */
			r.row = 1;
			r.col = Term->wid - r.width - 1;
		} else {
			r.row = Term->hgt - r.page_rows - 1;
		}
	} else {
		r.row = my + 1;
	}

	/* Hack -- no flush needed */
	msg_flag = FALSE;
	screen_save();

	menu_layout(m, &r);
	rect_region_erase_bordered(&r);

	prtf(0, 0, "($UEnter to select$Y\n$V, $UESC$Y%c$V) Command:", ESCAPE);
	selected = menu_dynamic_select(m);

	menu_dynamic_free(m);
	string_free(labels);

	screen_load();

	if ((selected > 0) && (selected <= graphics_mode_high_id+1)) {
		/* pick the graphics mode */
		if (arg_graphics != (selected-1)) {
			arg_graphics = (selected-1);

			/* React to changes */
			Term_xtra_x11_react();

			/* Hack -- Force redraw */
			do_cmd_redraw();
		}
	} else
	if (selected == graphics_mode_high_id+2) {
		/* a seperator line - do nothing */
		return 3;
	} else
	if (selected == graphics_mode_high_id+3) {
		/* show tile multiplier sub menu */
		int ret;
		while ((ret = main_menu_x11_graphics_mult(mdpy, mx+(r.width>>1), my)) == 3);
		if (ret == 2) return 3;
	} else
	if (selected == graphics_mode_high_id+4) {
		/* go back to the previous menu */
		return 2;
	} else
	{
		/* invalid selection */
	}

	return 1;
}
#endif /* USE_GRAPHICS */

int main_menu_x11_term_tile_size(term_data *td, int i, metadpy *mdpy, int mx, int my)
{
	menu_type_a *m;
	rect_region r;
	int selected, tw, th;
	char *labels;
	graphics_mode *mode;

	/* Paranoia */
	if (!td || !(td->fnt) || !mdpy) {
		return 0;
	}

	tw = td->tile_wid;
	th = td->tile_hgt;

	m = menu_dynamic_new();
	if (!m) {
		return 0;
	}

	labels = string_make(lower_case);
	m->selections = labels;

	/* list tile multipliers */
#define TILE_SIZE_CONTEXT(w,h,i) \
	if ((tw == w) && (th == h)) { \
		menu_dynamic_add(m, #w"x"#h, i); \
		menu_dynamic_set_select(m); \
	} else { \
		menu_dynamic_add(m, "$"#w"x"#h, i); \
	}
	if ((tw == td->fnt->wid) && (th == td->fnt->hgt)) {
		menu_dynamic_add(m, "Font Size", 1);
		menu_dynamic_set_select(m);
	} else {
		menu_dynamic_add(m, "$Font Size", 1);
	}
	TILE_SIZE_CONTEXT(8,8,2);
	TILE_SIZE_CONTEXT(16,16,3);
	TILE_SIZE_CONTEXT(32,32,4);
	TILE_SIZE_CONTEXT(8,16,5);
	TILE_SIZE_CONTEXT(10,15,6);
	TILE_SIZE_CONTEXT(10,20,7);
	TILE_SIZE_CONTEXT(16,32,8);
	TILE_SIZE_CONTEXT(8,13,9);
	TILE_SIZE_CONTEXT(10,17,10);
	TILE_SIZE_CONTEXT(12,13,11);
	TILE_SIZE_CONTEXT(12,20,12);
	TILE_SIZE_CONTEXT(16,25,13);
	/*menu_dynamic_add_label(m, "", '\0', 101, labels);*/
	menu_dynamic_add_label(m, "", '\0', 101, labels);
	menu_dynamic_add_label(m, "Back", '*', 100, labels);

	/* work out display region */
	r.width = menu_dynamic_longest_entry(m) + 3 + 2; /* +3 for tag, 2 for pad */
	if (mx > Term->wid - r.width - 1) {
		r.col = Term->wid - r.width - 1;
	} else {
		r.col = mx + 1;
	}
	r.page_rows = m->count;
	if (my > Term->hgt - r.page_rows - 1) {
		if (my - r.page_rows - 1 <= 0) {
			/* menu has too many items, so put in upper right corner */
			r.row = 1;
			r.col = Term->wid - r.width - 1;
		} else {
			r.row = Term->hgt - r.page_rows - 1;
		}
	} else {
		r.row = my + 1;
	}

	/* Hack -- no flush needed */
	msg_flag = FALSE;
	screen_save();

	menu_layout(m, &r);
	rect_region_erase_bordered(&r);

	prtf(0, 0, "($UEnter to select$Y\n$V, $UESC$Y%c$V) Term %d Tile Size:", ESCAPE, i);
	selected = menu_dynamic_select(m);

	menu_dynamic_free(m);
	string_free(labels);

	screen_load();

	switch (selected) {
	case 1:
		{
			td->tile_wid = td->fnt->wid;
			td->tile_hgt = td->fnt->hgt;
		} break;
	case 2:
		{
			td->tile_wid = 8;
			td->tile_hgt = 8;
		} break;
	case 3:
		{
			td->tile_wid = 16;
			td->tile_hgt = 16;
		} break;
	case 4:
		{
			td->tile_wid = 32;
			td->tile_hgt = 32;
		} break;
	case 5:
		{
			td->tile_wid = 8;
			td->tile_hgt = 16;
		} break;
	case 6:
		{
			td->tile_wid = 10;
			td->tile_hgt = 15;
		} break;
	case 7:
		{
			td->tile_wid = 10;
			td->tile_hgt = 20;
		} break;
	case 8:
		{
			td->tile_wid = 16;
			td->tile_hgt = 32;
		} break;
	case 9:
		{
			td->tile_wid = 8;
			td->tile_hgt = 13;
		} break;
	case 10:
		{
			td->tile_wid = 10;
			td->tile_hgt = 17;
		} break;
	case 11:
		{
			td->tile_wid = 12;
			td->tile_hgt = 13;
		} break;
	case 12:
		{
			td->tile_wid = 12;
			td->tile_hgt = 20;
		} break;
	case 13:
		{
			td->tile_wid = 16;
			td->tile_hgt = 25;
		} break;
	case 100:
		{
			/* go back to the previous menu */
			return 2;
		} break;
	case 101:
		{
			/* a seperator line - do nothing */
			return 3;
		} break;
	}

	if ((i==0) &&((tw != td->tile_wid) || (th != td->tile_hgt))) {
		int res;
		Display *dpy = mdpy->dpy;

		/* resize the tiles used for viewing */
		FreeTiles(&viewtiles);
		viewtiles.CellWidth = td->tile_wid * tile_width_mult;
		viewtiles.CellHeight = td->tile_hgt * tile_height_mult;
		res = ResizeTiles(dpy, &viewtiles, &tiles);

		FreeTiles(&maptiles);
		viewtiles.CellWidth = td->tile_wid;
		viewtiles.CellHeight = td->tile_hgt;
		res = ResizeTiles(dpy, &maptiles, &tiles);

		/* Clear screen */
		Term_xtra_x11_clear();
printf("d\n");
		/* Hack - redraw everything */
		if (Term->resize_hook)
			Term->resize_hook();
printf("e\n");
	}
	return 1;
}

int main_menu_x11_term(term_data *td, int i, metadpy *mdpy, int mx, int my)
{
	menu_type_a *m;
	rect_region r;
	int selected;
	char *labels;

	m = menu_dynamic_new();
	if (!m) {
		return 0;
	}

	labels = string_make(lower_case);
	m->selections = labels;

	/* per term */
	if (td->visible) {
		menu_dynamic_add_label(m, "Visible", ' ', 2, labels);
		menu_dynamic_add_label(m, "", '\0', 1, labels);
		menu_dynamic_add_label(m, "Pick Font", ' ', 3, labels);
		menu_dynamic_add_label(m, "", '\0', 1, labels);
		menu_dynamic_add_label(m, "Font Tile Size", '*', 4, labels);
		menu_dynamic_add_label(m, "", '\0', 1, labels);
		menu_dynamic_add_label(m, "Increase Tile Width", '>', 5, labels);
		menu_dynamic_add_label(m, "Decrease Tile Width", '<', 6, labels);
		menu_dynamic_add_label(m, "", '\0', 1, labels);
		menu_dynamic_add_label(m, "Increase Tile Height", '^', 7, labels);
		menu_dynamic_add_label(m, "Decrease Tile Height", 'v', 8, labels);
	} else {
		menu_dynamic_add_label(m, "$Visible", ' ', 2, labels);
	}
	menu_dynamic_add_label(m, "", '\0', 1, labels);
	menu_dynamic_add_label(m, "Back", '*', 9, labels);

	/* work out display region */
	r.width = menu_dynamic_longest_entry(m) + 3 + 2; /* +3 for tag, 2 for pad */
	if (mx > Term->wid - r.width - 1) {
		r.col = Term->wid - r.width - 1;
	} else {
		r.col = mx + 1;
	}
	r.page_rows = m->count;
	if (my > Term->hgt - r.page_rows - 1) {
		if (my - r.page_rows - 1 <= 0) {
			/* menu has too many items, so put in upper right corner */
			r.row = 1;
			r.col = Term->wid - r.width - 1;
		} else {
			r.row = Term->hgt - r.page_rows - 1;
		}
	} else {
		r.row = my + 1;
	}

	/* Hack -- no flush needed */
	msg_flag = FALSE;
	screen_save();

	menu_layout(m, &r);
	rect_region_erase_bordered(&r);

	prtf(0, 0, "($UEnter to select$Y\n$V, $UESC$Y%c$V) Term %d Command:", ESCAPE, i);
	selected = menu_dynamic_select(m);

	menu_dynamic_free(m);
	string_free(labels);

	screen_load();

	switch(selected) {
	case 1:
		{
			/* a seperator line - do nothing */
			return 3;
		} break;
	case 2:
		{
			/* toggle the terminal window visibility */
			if (i > 0) {
				if (td->visible) {
					td->visible = FALSE;
				} else {
					td->visible = TRUE;
				}
			} else {
				plog("You are not allowed to do that!");
			}
		} break;
	case 3:
		{
			/* pick a font for this terminal window */
			do_cmd_save_and_exit();
		} break;
	case 4:
		{
			/* pick a tile size from a list */
			int ret;
			while ((ret = main_menu_x11_term_tile_size(td, i, mdpy, mx+(r.width>>1), my)) == 3);
			if (ret == 2) return 3;
		} break;
	case 5:
		{
			/* Increase tile width */
			td->tile_wid++;
			/*term_get_size(td);
			term_window_resize(td);*/
		} break;
	case 6:
		{
			/* decrease tile width */
			td->tile_wid--;
			/*term_get_size(td);
			term_window_resize(td);*/
		} break;
	case 7:
		{
			/* Increase tile height */
			td->tile_hgt++;
			/*term_get_size(td);
			term_window_resize(td);*/
		} break;
	case 8:
		{
			/* decrease tile height */
			td->tile_hgt--;
			/*term_get_size(td);
			term_window_resize(td);*/
		} break;
	case 9:
		{
			/* go back to the previous menu */
			return 2;
		} break;
	}

	return 1;
}

int main_menu_x11_pick_term(term_data *td, int n, metadpy *mdpy, int mx, int my)
{
	menu_type_a *m;
	rect_region r;
	int selected, i;
	char *labels;

	if (n ==1) {
		int ret;
		/* if there is only one term, use it automatically */
		ret = main_menu_x11_term(&(td[0]), 1, mdpy, mx, my);
		if (ret == 2) return 3;
		return ret;
	}

	m = menu_dynamic_new();
	if (!m) {
		return 0;
	}

	labels = string_make(lower_case);
	m->selections = labels;

	for (i = 0; i < n; i++) {
		menu_dynamic_add(m, format("Term %d", i), i+1);
	}
	menu_dynamic_add_label(m, "Back", '*', n+1, labels);

	/* work out display region */
	r.width = menu_dynamic_longest_entry(m) + 3 + 2; /* +3 for tag, 2 for pad */
	if (mx > Term->wid - r.width - 1) {
		r.col = Term->wid - r.width - 1;
	} else {
		r.col = mx + 1;
	}
	r.page_rows = m->count;
	if (my > Term->hgt - r.page_rows - 1) {
		if (my - r.page_rows - 1 <= 0) {
			/* menu has too many items, so put in upper right corner */
			r.row = 1;
			r.col = Term->wid - r.width - 1;
		} else {
			r.row = Term->hgt - r.page_rows - 1;
		}
	} else {
		r.row = my + 1;
	}

	/* Hack -- no flush needed */
	msg_flag = FALSE;
	screen_save();

	menu_layout(m, &r);
	rect_region_erase_bordered(&r);

	prtf(0, 0, "($UEnter to select$Y\n$V, $UESC$Y%c$V) Command:", ESCAPE);
	selected = menu_dynamic_select(m);

	menu_dynamic_free(m);
	string_free(labels);

	screen_load();

	if (selected == n+1) {
		return 2;
	} else
	if (selected > 0) {
		int ret;
		while ((ret = main_menu_x11_term(&(td[selected-1]), selected-1, mdpy, mx+(r.width>>1), my)) == 3);
		if (ret == 2) return 3;
	}

	return 1;
}

int main_menu_x11_other(int mx, int my)
{
	menu_type_a *m;
	rect_region r;
	int selected;
	char *labels;

	m = menu_dynamic_new();
	if (!m) {
		return 0;
	}

	labels = string_make(lower_case);
	m->selections = labels;

	menu_dynamic_add_label(m, "Version", ' ', 7, labels);
	menu_dynamic_add_label(m, "", '\0', 1, labels);
	menu_dynamic_add_label(m, "Dump Char", 'f', 2, labels);
	menu_dynamic_add_label(m, "", '\0', 1, labels);
	menu_dynamic_add_label(m, "Save Screenshot", '*', 3, labels);
	menu_dynamic_add_label(m, "", '\0', 1, labels);
	menu_dynamic_add_label(m, "Reset Terminal Layout", ' ', 4, labels);
	menu_dynamic_add_label(m, "", '\0', 1, labels);
	menu_dynamic_add_label(m, "^Redraw", 'R', 8, labels);
	menu_dynamic_add_label(m, "", '\0', 1, labels);
	menu_dynamic_add_label(m, "Dump Screen", '(', 9, labels);
	menu_dynamic_add_label(m, "Load Screen", ')', 10, labels);
	menu_dynamic_add_label(m, "", '\0', 1, labels);
	menu_dynamic_add_label(m, "^Suicide", 'Q', 5, labels);
	menu_dynamic_add_label(m, "", '\0', 1, labels);
	menu_dynamic_add_label(m, "Back", '*', 6, labels);

	/* work out display region */
	r.width = menu_dynamic_longest_entry(m) + 3 + 2; /* +3 for tag, 2 for pad */
	if (mx > Term->wid - r.width - 1) {
		r.col = Term->wid - r.width - 1;
	} else {
		r.col = mx + 1;
	}
	r.page_rows = m->count;
	if (my > Term->hgt - r.page_rows - 1) {
		if (my - r.page_rows - 1 <= 0) {
			/* menu has too many items, so put in upper right corner */
			r.row = 1;
			r.col = Term->wid - r.width - 1;
		} else {
			r.row = Term->hgt - r.page_rows - 1;
		}
	} else {
		r.row = my + 1;
	}

	/* Hack -- no flush needed */
	msg_flag = FALSE;
	screen_save();

	menu_layout(m, &r);
	rect_region_erase_bordered(&r);

	prtf(0, 0, "($UEnter to select$Y\n$V, $UESC$Y%c$V) Command:", ESCAPE);
	selected = menu_dynamic_select(m);

	menu_dynamic_free(m);
	string_free(labels);

	screen_load();

	switch(selected) {
	case 1:
		{
			/* a seperator line - do nothing */
			return 3;
		} break;
	case 2:
		{
			/* dump the character to disk */
			char tmp[160] = "";
			strnfmt(tmp, 160, "%s.txt", player_base);
			if (get_string(tmp, 160, "Filename for char dump:")) {
				/* Dump a character file */
				(void)file_character(tmp, TRUE);
			}
		} break;
	case 3:
		{
#if 0
			/* save a screen shot */
			char filename[1024];
			char path[1024];
			time_t ltime;
			struct tm *today;
			int len;
			term_data *td = (term_data*)(Term->data);

			time( &ltime );
			today = localtime (&ltime );
			strnfmt(filename, sizeof(filename), "%s", player_base);
			len = strlen(filename);
			strftime(filename+len, sizeof(filename)-len, "_%Y%b%d_%H%M%S.png", today);

			/* Get the system-specific path */
			path_build(path, sizeof(path, ANGBAND_DIR_USER, filename);
			td = &data[0];
			if (!SaveWindow(td->win, path)) {
				plog("Screenshot Save Failed.");
			}
#endif
		} break;
	case 4:
		{
			/* reset the layout of the terminal windows */
			/*default_layout_x11(data, MAX_TERM_COUNT, Metadpy);*/
		} break;
	case 5:
		{
			/* have the character commit suicide, and exit to initial screen */
			do_cmd_suicide();
		} break;
	/*case 0:*/
	case 6:
		{
			/* go back to the previous menu */
			return 2;
		} break;
	case 7:
		{
			/* show the version */
			do_cmd_version();
		} break;
	case 8:
		{
			/* redraw the screen */
			do_cmd_redraw();
		} break;
	case 9:
		{
			/* dump the screen to disk */
			do_cmd_save_screen();
		} break;
	case 10:
		{
			/* load the screen from disk */
			do_cmd_load_screen();
		} break;
	}

	return 1;
}

int main_menu_x11(metadpy *mdpy, term_data *data, int data_count, int mx, int my)
{
	menu_type_a *m;
	rect_region r;
	int selected;
	char *labels;

	m = menu_dynamic_new();
	if (!m) {
		return 0;
	}

	labels = string_make(lower_case);
	m->selections = labels;

	menu_dynamic_add_label(m, "Terminal", ' ', 4, labels);
	menu_dynamic_add_label(m, "", '\0', 1, labels);
#ifdef USE_GRAPHICS
	menu_dynamic_add_label(m, "Graphics", ' ', 5, labels);
	menu_dynamic_add_label(m, "", '\0', 1, labels);
#endif /* USE_GRAPHICS */
	menu_dynamic_add_label(m, "Help", '?', 6, labels);
	menu_dynamic_add_label(m, "", '\0', 1, labels);
	menu_dynamic_add_label(m, "Options", '=', 7, labels);
	menu_dynamic_add_label(m, "", '\0', 1, labels);
	menu_dynamic_add_label(m, "Other", ' ', 8, labels);
	menu_dynamic_add_label(m, "", '\0', 1, labels);
	menu_dynamic_add_label(m, "^Save", 'S', 3, labels);
	menu_dynamic_add_label(m, "", '\0', 1, labels);
	menu_dynamic_add_label(m, "^Save and Exit", 'X', 2, labels);

	/* work out display region */
	r.width = menu_dynamic_longest_entry(m) + 3 + 2; /* +3 for tag, 2 for pad */
	if (mx > Term->wid - r.width - 1) {
		r.col = Term->wid - r.width - 1;
	} else {
		r.col = mx + 1;
	}
	r.page_rows = m->count;
	if (my > Term->hgt - r.page_rows - 1) {
		if (my - r.page_rows - 1 <= 0) {
			/* menu has too many items, so put in upper right corner */
			r.row = 1;
			r.col = Term->wid - r.width - 1;
		} else {
			r.row = Term->hgt - r.page_rows - 1;
		}
	} else {
		r.row = my + 1;
	}

	/* Hack -- no flush needed */
	msg_flag = FALSE;

	screen_save();
	button_backup_all(TRUE);

	menu_layout(m, &r);
	rect_region_erase_bordered(&r);

	prtf(0, 0, "($UEnter to select$Y\n$V, $UESC$Y%c$V) Command:", ESCAPE);
	selected = menu_dynamic_select(m);

	menu_dynamic_free(m);
	string_free(labels);

	button_restore();
	screen_load();

	switch(selected) {
	case 1:
		{
			/* a seperator line - do nothing */
			return 3;
		} break;
	case 2:
		{
			/* save and exit */
			do_cmd_save_and_exit();
		} break;
	case 3:
		{
			/* save */
			do_cmd_save_game(FALSE);
		} break;
	case 4:
		{
			/* show font menu */
			int ret;
			while ((ret = main_menu_x11_pick_term(data, data_count, mdpy, mx+(r.width>>1), my)) == 3);
			if (ret == 2) return 3;
		} break;
	case 5:
		{
#ifdef USE_GRAPHICS
			/* show graphics menu */
			int ret;
			while ((ret = main_menu_x11_graphics(mdpy, mx+(r.width>>1), my)) == 3);
			if (ret == 2) return 3;
#endif /* USE_GRAPHICS */
		} break;
	case 6:
		{
			/* show help */
			do_cmd_help();
		} break;
	case 7:
		{
			/* show options */
			do_cmd_options(OPT_FLAG_SERVER | OPT_FLAG_PLAYER);
		} break;
	case 8:
		{
			/* show other commands, like suicide, reset layout, save screenshot, dump char */
			int ret;
			while ((ret = main_menu_x11_other(mx+(r.width>>1), my)) == 3);
			if (ret == 2) return 3;
		} break;
	}

	return 1;
}


