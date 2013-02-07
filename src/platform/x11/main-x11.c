/* File: main-x11.c */

/*
 * Copyright (c) 1997 Ben Harrison, and others
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 */


/*
 * This file helps Angband work with UNIX/X11 computers.
 *
 * To use this file, compile with "USE_X11" defined, and link against all
 * the various "X11" libraries which may be needed.
 *
 * See also "main-xaw.c".
 *
 * Part of this file provides a user interface package composed of several
 * pseudo-objects, including "metadpy" (a display), "infowin" (a window),
 * "infoclr" (a color), and "infofnt" (a font).  Actually, the package was
 * originally much more interesting, but it was bastardized to keep this
 * file simple.
 *
 * The rest of this file is an implementation of "main-xxx.c" for X11.
 *
 * Most of this file is by Ben Harrison (benh@phial.com).
 */

#include "angband.h"
#include "button.h"
#include "settings.h"
#include "grafmode.h"
#include "x11-term.h"
#include "x11-tile.h"


#ifdef USE_X11

cptr help_x11[] =
{
	"To use X11",
	"-d    Set display name",
#ifdef USE_GRAPHICS
	"-s    Turn off smoothscaling graphics",
#endif /* USE_GRAPHICS */
	"-x<file> Set settings file name",
	"-n#   Number of terms to use",
	NULL
};


#ifndef __MAKEDEPEND__
#include <X11/keysym.h>
#include <X11/keysymdef.h>
#include <X11/Xatom.h>
#endif /* __MAKEDEPEND__ */


/*
 * Include some helpful X11 code.
 */
#include "maid-x11.h"

/*
 * Hack -- avoid some compiler warnings
 */
#define IGNORE_UNUSED_FUNCTIONS


/*
 * Notes on Colors:
 *
 *   1) On a monochrome (or "fake-monochrome") display, all colors
 *   will be "cast" to "fg," except for the bg color, which is,
 *   obviously, cast to "bg".  Thus, one can ignore this setting.
 *
 *   2) Because of the inner functioning of the color allocation
 *   routines, colors may be specified as (a) a typical color name,
 *   (b) a hexidecimal color specification (preceded by a pound sign),
 *   or (c) by strings such as "fg", "bg", "zg".
 *
 *   3) Due to the workings of the init routines, many colors
 *   may also be dealt with by their actual pixel values.  Note that
 *   the pixel with all bits set is "zg = (1<<metadpy->depth)-1", which
 *   is not necessarily either black or white.
 */




/**** Generic Globals ****/


/*
 * The "default" values
 */
static metadpy metadpy_default;


/*
 * The "current" variables
 */
static metadpy *Metadpy = &metadpy_default;
static infowin *Infowin = (infowin*)(NULL);
static infoclr *Infoclr = (infoclr*)(NULL);
static infofnt *Infofnt = (infofnt*)(NULL);

/*
 * The number of term data structures
 */
#define MAX_TERM_DATA 8

/*
 * The array of term data structures
 */
static term_data data[MAX_TERM_DATA];

/*
 * Path to the X11 settings file
 */
static const char *x11_settings = "x11-settings.txt";
static char *ini_file = NULL;
int default_layout_x11(term_data *data, int maxterms, metadpy *dpy);

#ifdef USE_GRAPHICS
XTilesheet tiles; /* tiles loaded from disk */
XTilesheet viewtiles; /* tiles that are displayed on screen */
XTilesheet maptiles; /* tiles same size as font */
#endif /* USE_GRAPHICS */

#ifdef SUPPORT_GAMMA
static int gamma_correction;
#endif /*SUPPORT_GAMMA */

/*
 * Hack -- cursor color
 */
static infoclr *xor;

/*
 * Actual color table
 */
static infoclr *clr[256];

/*
 * Color info (unused, red, green, blue).
 */
static byte color_table[256][4];


/**** Generic code ****/


/*
 * Init the current metadpy, with various initialization stuff.
 *
 * Inputs:
 *	dpy:  The Display* to use (if NULL, create it)
 *	name: The name of the Display (if NULL, the current)
 *
 * Notes:
 *	If 'name' is NULL, but 'dpy' is set, extract name from dpy
 *	If 'dpy' is NULL, then Create the named Display
 *	If 'name' is NULL, and so is 'dpy', use current Display
 *
 * Return -1 if no Display given, and none can be opened.
 */
static errr Metadpy_init_2(Display *dpy, cptr name)
{
	metadpy *m = Metadpy;

	/*** Open the display if needed ***/

	/* If no Display given, attempt to Create one */
	if (!dpy)
	{
		/* Attempt to open the display */
		dpy = XOpenDisplay(name);

		/* Failure */
		if (!dpy) return (-1);

		/* We will have to nuke it when done */
		m->nuke = 1;
	}

	/* Since the Display was given, use it */
	else
	{
		/* We will not have to nuke it when done */
		m->nuke = 0;
	}


	/*** Save some information ***/

	/* Save the Display itself */
	m->dpy = dpy;

	/* Get the Screen and Virtual Root Window */
	m->screen = DefaultScreenOfDisplay(dpy);
	m->root = RootWindowOfScreen(m->screen);

	/* Get the default colormap */
	m->cmap = DefaultColormapOfScreen(m->screen);

	/* Extract the true name of the display */
	m->name = DisplayString(dpy);

	/* Extract the fd */
	m->fd = ConnectionNumber(Metadpy->dpy);

	/* Save the Size and Depth of the screen */
	m->width = WidthOfScreen(m->screen);
	m->height = HeightOfScreen(m->screen);
	m->depth = DefaultDepthOfScreen(m->screen);

	/* Save the Standard Colors */
	m->black = BlackPixelOfScreen(m->screen);
	m->white = WhitePixelOfScreen(m->screen);

	/*** Make some clever Guesses ***/

	/* Guess at the desired 'fg' and 'bg' Pixell's */
	m->bg = m->black;
	m->fg = m->white;

	/* Calculate the Maximum allowed Pixel value.  */
	m->zg = ((Pixell)1 << m->depth) - 1;

	/* Save various default Flag Settings */
	m->color = ((m->depth > 1) ? 1 : 0);
	m->mono = ((m->color) ? 0 : 1);

	/* register our interest in close window requests */
	m->wmDeleteWindow = XInternAtom(m->dpy, "WM_DELETE_WINDOW",FALSE);

	/* Return "success" */
	return (0);
}


/*#ifndef IGNORE_UNUSED_FUNCTIONS*/

/*
 * Nuke the current metadpy
 */
static errr Metadpy_nuke(void)
{
	metadpy *m = Metadpy;


	/* If required, Free the Display */
	if (m->nuke)
	{
		/* Close the Display */
		XCloseDisplay(m->dpy);

		/* Forget the Display */
		m->dpy = (Display*)(NULL);

		/* Do not nuke it again */
		m->nuke = 0;
	}

	/* Return Success */
	return (0);
}

/*#endif *//* IGNORE_UNUSED_FUNCTIONS */


/*
 * General Flush/ Sync/ Discard routine
 */
static errr Metadpy_update(int flush, int sync, int discard)
{
	/* Flush if desired */
	if (flush) XFlush(Metadpy->dpy);

	/* Sync if desired, using 'discard' */
	if (sync) XSync(Metadpy->dpy, discard);

	/* Success */
	return (0);
}


/*
 * Make a simple beep
 */
static errr Metadpy_do_beep(void)
{
	/* Make a simple beep */
	XBell(Metadpy->dpy, 100);

	return (0);
}



/*
 * Set the name (in the title bar) of Infowin
 */
static errr Infowin_set_name(cptr name)
{
	Status st;
	XTextProperty tp;
	char buf[128];
	char *bp = buf;
	strnfmt(buf, sizeof(buf), "%s", name);
	st = XStringListToTextProperty(&bp, 1, &tp);
	if (st) XSetWMName(Metadpy->dpy, Infowin->win, &tp);
	return (0);
}


#ifndef IGNORE_UNUSED_FUNCTIONS

/*
 * Set the icon name of Infowin
 */
static errr Infowin_set_icon_name(cptr name)
{
	Status st;
	XTextProperty tp;
	char buf[128];
	char *bp = buf;
	strnfmt(buf, sizeof(buf), "%s", name);
	st = XStringListToTextProperty(&bp, 1, &tp);
	if (st) XSetWMIconName(Metadpy->dpy, Infowin->win, &tp);
	return (0);
}

#endif /* IGNORE_UNUSED_FUNCTIONS */

/*
 * Nuke Infowin
 */
static errr Infowin_nuke(void)
{
	infowin *iwin = Infowin;

	/* Nuke if requested */
	if (iwin->nuke)
	{
		/* Destory the old window */
		XDestroyWindow(Metadpy->dpy, iwin->win);
	}

	/* Success */
	return (0);
}

/*#endif *//* IGNORE_UNUSED_FUNCTIONS */


/*
 * Prepare a new 'infowin'.
 */
static errr Infowin_prepare(Window xid)
{
	infowin *iwin = Infowin;

	Window tmp_win;
	XWindowAttributes xwa;
	int x, y;
	unsigned int w, h, b, d;

	/* Assign stuff */
	iwin->win = xid;

	/* Check For Error XXX Extract some ACTUAL data from 'xid' */
	XGetGeometry(Metadpy->dpy, xid, &tmp_win, &x, &y, &w, &h, &b, &d);

	/* Apply the above info */
	iwin->x = x;
	iwin->y = y;
	iwin->w = w;
	iwin->h = h;
	iwin->b = b;

	/* Check Error XXX Extract some more ACTUAL data */
	XGetWindowAttributes(Metadpy->dpy, xid, &xwa);

	/* Apply the above info */
	iwin->mask = xwa.your_event_mask;
	iwin->mapped = ((xwa.map_state == IsUnmapped) ? 0 : 1);

	/* Success */
	return (0);
}


#ifndef IGNORE_UNUSED_FUNCTIONS

/*
 * Initialize a new 'infowin'.
 */
static errr Infowin_init_real(Window xid)
{
	/* Wipe it clean */
	(void)WIPE(Infowin, infowin);

	/* Start out non-nukable */
	Infowin->nuke = 0;

	/* Attempt to Prepare ourself */
	return (Infowin_prepare(xid));
}

#endif /* IGNORE_UNUSED_FUNCTIONS */


/*
 * Init an infowin by giving some data.
 *
 * Inputs:
 *	dad: The Window that should own this Window (if any)
 *	x,y: The position of this Window
 *	w,h: The size of this Window
 *	b,d: The border width and pixel depth
 *
 * Notes:
 *	If 'dad == None' assume 'dad == root'
 */
static errr Infowin_init_data(Window dad, int x, int y, int w, int h,
                              int b, Pixell fg, Pixell bg)
{
	Window xid;

	/* Wipe it clean */
	(void)WIPE(Infowin, infowin);


	/*** Error Check XXX ***/


	/*** Create the Window 'xid' from data ***/

	/* What happened here?  XXX XXX XXX */

	/* If no parent given, depend on root */
	if (dad == None)

/* #ifdef USE_GRAPHICS

		xid = XCreateWindow(Metadpy->dpy, Metadpy->root, x, y, w, h, b, 8, InputOutput, CopyFromParent, 0, 0);

	else
*/

/* #else */

		dad = Metadpy->root;

/* #endif */

	/* Create the Window XXX Error Check */
	xid = XCreateSimpleWindow(Metadpy->dpy, dad, x, y, w, h, b, fg, bg);

	/* Start out selecting No events */
	XSelectInput(Metadpy->dpy, xid, 0L);

	/* register our interest in close window requests */
	XSetWMProtocols(Metadpy->dpy, xid, &(Metadpy->wmDeleteWindow), 1);

	/*** Prepare the new infowin ***/

	/* Mark it as nukable */
	Infowin->nuke = 1;

	/* Attempt to Initialize the infowin */
	return (Infowin_prepare(xid));
}



/*
 * Modify the event mask of an Infowin
 */
static errr Infowin_set_mask(long mask)
{
	/* Save the new setting */
	Infowin->mask = mask;

	/* Execute the Mapping */
	XSelectInput(Metadpy->dpy, Infowin->win, Infowin->mask);

	/* Success */
	return (0);
}


/*
 * Request that Infowin be mapped
 */
static errr Infowin_map(void)
{
	/* Execute the Mapping */
	XMapWindow(Metadpy->dpy, Infowin->win);

	/* Success */
	return (0);
}


#ifndef IGNORE_UNUSED_FUNCTIONS

/*
 * Request that Infowin be unmapped
 */
static errr Infowin_unmap(void)
{
	/* Execute the Un-Mapping */
	XUnmapWindow(Metadpy->dpy, Infowin->win);

	/* Success */
	return (0);
}

#endif /* IGNORE_UNUSED_FUNCTIONS */


/*
 * Request that Infowin be raised
 */
static errr Infowin_raise(void)
{
	/* Raise towards visibility */
	XRaiseWindow(Metadpy->dpy, Infowin->win);

	/* Success */
	return (0);
}


#ifndef IGNORE_UNUSED_FUNCTIONS

/*
 * Request that Infowin be lowered
 */
static errr Infowin_lower(void)
{
	/* Lower towards invisibility */
	XLowerWindow(Metadpy->dpy, Infowin->win);

	/* Success */
	return (0);
}

#endif /* IGNORE_UNUSED_FUNCTIONS */


/*
 * Request that Infowin be moved to a new location
 */
static errr Infowin_impell(int x, int y)
{
	/* Execute the request */
	XMoveWindow(Metadpy->dpy, Infowin->win, x, y);

	/* Success */
	return (0);
}


/*
 * Resize an infowin
 */
static errr Infowin_resize(int w, int h)
{
	/* Execute the request */
	XResizeWindow(Metadpy->dpy, Infowin->win, w, h);

	/* Success */
	return (0);
}


#ifndef IGNORE_UNUSED_FUNCTIONS

/*
 * Move and Resize an infowin
 */
static errr Infowin_locate(int x, int y, int w, int h)
{
	/* Execute the request */
	XMoveResizeWindow(Metadpy->dpy, Infowin->win, x, y, w, h);

	/* Success */
	return (0);
}


/*
 * Visually clear Infowin
 */
static errr Infowin_wipe(void)
{
	/* Execute the request */
	XClearWindow(Metadpy->dpy, Infowin->win);

	/* Success */
	return (0);
}


/*
 * Visually Paint Infowin with the current color
 */
static errr Infowin_fill(void)
{
	/* Execute the request */
	XFillRectangle(Metadpy->dpy, Infowin->win, Infoclr->gc,
	               0, 0, Infowin->w, Infowin->h);

	/* Success */
	return (0);
}

#endif /* IGNORE_UNUSED_FUNCTIONS */


/*
 * A NULL terminated pair list of legal "operation names"
 *
 * Pairs of values, first is texttual name, second is the string
 * holding the decimal value that the operation corresponds to.
 */
static cptr opcode_pairs[] =
{
	"cpy", "3",
	"xor", "6",
	"and", "1",
	"ior", "7",
	"nor", "8",
	"inv", "10",
	"clr", "0",
	"set", "15",

	"src", "3",
	"dst", "5",

	"+andReverse", "2",
	"+andInverted", "4",
	"+noop", "5",
	"+equiv", "9",
	"+orReverse", "11",
	"+copyInverted", "12",
	"+orInverted", "13",
	"+nand", "14",
	NULL
};


/*
 * Parse a word into an operation "code"
 *
 * Inputs:
 *	str: A string, hopefully representing an Operation
 *
 * Output:
 *	0-15: if 'str' is a valid Operation
 *	-1:   if 'str' could not be parsed
 */
static int Infoclr_Opcode(cptr str)
{
	register int i;

	/* Scan through all legal operation names */
	for (i = 0; opcode_pairs[i*2]; ++i)
	{
		/* Is this the right oprname? */
		if (streq(opcode_pairs[i*2], str))
		{
			/* Convert the second element in the pair into a Code */
			return (atoi(opcode_pairs[i*2+1]));
		}
	}

	/* The code was not found, return -1 */
	return (-1);
}


#ifndef IGNORE_UNUSED_FUNCTIONS

/*
 * Request a Pixell by name.  Note: uses 'Metadpy'.
 *
 * Inputs:
 *      name: The name of the color to try to load (see below)
 *
 * Output:
 *	The Pixell value that metched the given name
 *	'Metadpy->fg' if the name was unparseable
 *
 * Valid forms for 'name':
 *	'fg', 'bg', 'zg', '<name>' and '#<code>'
 */
static Pixell Infoclr_Pixell(cptr name)
{
	XColor scrn;

	/* Attempt to Parse the name */
	if (name && name[0])
	{
		/* The 'bg' color is available */
		if (streq(name, "bg")) return (Metadpy->bg);

		/* The 'fg' color is available */
		if (streq(name, "fg")) return (Metadpy->fg);

		/* The 'zg' color is available */
		if (streq(name, "zg")) return (Metadpy->zg);

		/* The 'white' color is available */
		if (streq(name, "white")) return (Metadpy->white);

		/* The 'black' color is available */
		if (streq(name, "black")) return (Metadpy->black);

		/* Attempt to parse 'name' into 'scrn' */
		if (!(XParseColor(Metadpy->dpy, Metadpy->cmap, name, &scrn)))
		{
			plog_fmt("Warning: Couldn't parse color '%s'\n", name);
		}

		/* Attempt to Allocate the Parsed color */
		if (!(XAllocColor(Metadpy->dpy, Metadpy->cmap, &scrn)))
		{
			plog_fmt("Warning: Couldn't allocate color '%s'\n", name);
		}

		/* The Pixel was Allocated correctly */
		else return (scrn.pixel);
	}

	/* Warn about the Default being Used */
	plog_fmt("Warning: Using 'fg' for unknown color '%s'\n", name);

	/* Default to the 'Foreground' color */
	return (Metadpy->fg);
}


/*
 * Initialize a new 'infoclr' with a real GC.
 */
static errr Infoclr_init_1(GC gc)
{
	infoclr *iclr = Infoclr;

	/* Wipe the iclr clean */
	(void)WIPE(iclr, infoclr);

	/* Assign the GC */
	iclr->gc = gc;

	/* Success */
	return (0);
}
#endif /* IGNORE_UNUSED_FUNCTIONS */


/*
 * Nuke an old 'infoclr'.
 */
static errr Infoclr_nuke(void)
{
	infoclr *iclr = Infoclr;

	/* Deal with 'GC' */
	if (iclr->nuke)
	{
		/* Free the GC */
		XFreeGC(Metadpy->dpy, iclr->gc);
	}

	/* Forget the current */
	Infoclr = (infoclr*)(NULL);

	/* Success */
	return (0);
}

/*#endif *//* IGNORE_UNUSED_FUNCTIONS */


/*
 * Initialize an infoclr with some data
 *
 * Inputs:
 *	fg:   The Pixell for the requested Foreground (see above)
 *	bg:   The Pixell for the requested Background (see above)
 *	op:   The Opcode for the requested Operation (see above)
 *	stip: The stipple mode
 */
static errr Infoclr_init_data(Pixell fg, Pixell bg, int op, int stip)
{
	infoclr *iclr = Infoclr;

	GC gc;
	XGCValues gcv;
	unsigned long gc_mask;



	/*** Simple error checking of opr and clr ***/

	/* Check the 'Pixells' for realism */
	if (bg > Metadpy->zg) return (-1);
	if (fg > Metadpy->zg) return (-1);

	/* Check the data for trueness */
	if ((op < 0) || (op > 15)) return (-1);


	/*** Create the requested 'GC' ***/

	/* Assign the proper GC function */
	gcv.function = op;

	/* Assign the proper GC background */
	gcv.background = bg;

	/* Assign the proper GC foreground */
	gcv.foreground = fg;

	/* Hack -- Handle XOR (xor is code 6) by hacking bg and fg */
	if (op == 6) gcv.background = 0;
	if (op == 6) gcv.foreground = (bg ^ fg);

	/* Assign the proper GC Fill Style */
	gcv.fill_style = (stip ? FillStippled : FillSolid);

	/* Turn off 'Give exposure events for pixmap copying' */
	gcv.graphics_exposures = False;

	/* Set up the GC mask */
	gc_mask = (GCFunction | GCBackground | GCForeground |
	           GCFillStyle | GCGraphicsExposures);

	/* Create the GC detailed above */
	gc = XCreateGC(Metadpy->dpy, Metadpy->root, gc_mask, &gcv);


	/*** Initialize ***/

	/* Wipe the iclr clean */
	(void)WIPE(iclr, infoclr);

	/* Assign the GC */
	iclr->gc = gc;

	/* Nuke it when done */
	iclr->nuke = 1;

	/* Assign the parms */
	iclr->fg = fg;
	iclr->bg = bg;
	iclr->code = op;
	iclr->stip = stip ? 1 : 0;

	/* Success */
	return (0);
}



/*
 * Change the 'fg' for an infoclr
 *
 * Inputs:
 *	fg:   The Pixell for the requested Foreground (see above)
 */
static errr Infoclr_change_fg(Pixell fg)
{
	infoclr *iclr = Infoclr;


	/*** Simple error checking of opr and clr ***/

	/* Check the 'Pixells' for realism */
	if (fg > Metadpy->zg) return (-1);


	/*** Change ***/

	/* Change */
	XSetForeground(Metadpy->dpy, iclr->gc, fg);

	/* Success */
	return (0);
}



/*#ifndef IGNORE_UNUSED_FUNCTIONS*/

/*
 * Nuke an old 'infofnt'.
 */
static errr Infofnt_nuke(void)
{
	infofnt *ifnt = Infofnt;

	/* Deal with 'name' */
	if (ifnt->name)
	{
		/* Free the name */
		string_free(ifnt->name);
	}

	/* Nuke info if needed */
	if (ifnt->nuke)
	{
		/* Free the font */
		XFreeFont(Metadpy->dpy, ifnt->info);
	}

	/* Success */
	return (0);
}

/*#endif *//* IGNORE_UNUSED_FUNCTIONS */


/*
 * Prepare a new 'infofnt'
 */
static errr Infofnt_prepare(XFontStruct *info)
{
	infofnt *ifnt = Infofnt;

	XCharStruct *cs;

	/* Assign the struct */
	ifnt->info = info;

	/* Jump into the max bounds thing */
	cs = &(info->max_bounds);

	/* Extract default sizing info */
	ifnt->asc = info->ascent;
	ifnt->hgt = info->ascent + info->descent;
	ifnt->wid = cs->width;

#ifdef OBSOLETE_SIZING_METHOD
	/* Extract default sizing info */
	ifnt->asc = cs->ascent;
	ifnt->hgt = (cs->ascent + cs->descent);
	ifnt->wid = cs->width;
#endif

	if (use_bigtile)
		ifnt->twid = 2 * cs->width;
	else
		ifnt->twid = cs->width;

	/* Success */
	return (0);
}


#ifndef IGNORE_UNUSED_FUNCTIONS

/*
 * Initialize a new 'infofnt'.
 */
static errr Infofnt_init_real(XFontStruct *info)
{
	/* Wipe the thing */
	(void)WIPE(Infofnt, infofnt);

	/* No nuking */
	Infofnt->nuke = 0;

	/* Attempt to prepare it */
	return (Infofnt_prepare(info));
}

#endif /* IGNORE_UNUSED_FUNCTIONS */


/*
 * Init an infofnt by its Name
 *
 * Inputs:
 *	name: The name of the requested Font
 */
static errr Infofnt_init_data(cptr name)
{
	XFontStruct *info;


	/*** Load the info Fresh, using the name ***/

	/* If the name is not given, report an error */
	if (!name)
	{
		plog_fmt("Missing font! %s", name);
		
		return (-1);
	}
	
	/* Attempt to load the font */
	info = XLoadQueryFont(Metadpy->dpy, name);

	/* The load failed */
	if (!info)
	{
		plog_fmt("Failed to find font:\"%s\"", name);

		return (-1);
	}

	/*** Init the font ***/

	/* Wipe the thing */
	(void)WIPE(Infofnt, infofnt);

	/* Attempt to prepare it */
	if (Infofnt_prepare(info))
	{
		/* Free the font */
		XFreeFont(Metadpy->dpy, info);

		/* Fail */
		plog_fmt("Failed to prepare font:\"%s\"", name);
		
		return (-1);
	}

	/* Save a copy of the font name */
	Infofnt->name = string_make(name);

	/* Mark it as nukable */
	Infofnt->nuke = 1;
	
	/* Success */
	return (0);
}


/*
 * Find the square a particular pixel is part of.
 */
static void pixel_to_square(int *x, int *y, int ox, int oy)
{
	(*x) = (ox - Infowin->ox) / Infofnt->wid;
	(*y) = (oy - Infowin->oy) / Infofnt->hgt;
#if 0	
	if (((*y) >= Term->scr->big_y1)
		&& ((*y) <= Term->scr->big_y2)
		&& ((*x) >= Term->scr->big_x1))
	{
		if (tile_width_mult > 1) {
			(*x) -= ((*x) - Term->scr->big_x1 + 1) / tile_width_mult;
		}
		if (tile_height_mult > 1) {
			(*y) -= ((*y) - Term->scr->big_y1 + 1) / tile_height_mult;
		}
	}
#endif
}

/*
 * Find the pixel at the top-left corner of a square.
 */
static void square_to_pixel(int *x, int *y, int ox, int oy)
{
	term_data *td = (term_data*)(Term->data);
	
	if (is_bigtiled(ox, oy)) {
		(*x) = ox * td->tile_wid * tile_width_mult + Infowin->ox;
		(*y) = oy * td->tile_hgt * tile_height_mult + Infowin->oy;
	} else 
	{
		(*x) = ox * td->tile_wid + Infowin->ox;
		(*y) = oy * td->tile_hgt + Infowin->oy;
	}
	/*if ((tile_width_mult > 1)
			&& (oy >= Term->scr->big_y1)
			&& (oy <= Term->scr->big_y2)
			&& (ox > Term->scr->big_x1))
	{
		(*x) = ox * td->tile_wid * tile_width_mult + Infowin->ox -
				Term->scr->big_x1 * td->tile_wid * (tile_width_mult-1);
	} else {
		(*x) = ox * td->tile_wid + Infowin->ox;
	}
	if ((tile_height_mult > 1)
			&& (oy > Term->scr->big_y1)
			&& (oy <= Term->scr->big_y2)
			&& (ox >= Term->scr->big_x1))
	{
		(*y) = oy * td->tile_hgt * tile_height_mult + Infowin->oy -
				Term->scr->big_y1 * td->tile_hgt * (tile_height_mult-1);
	} else {
		(*y) = oy * td->tile_hgt + Infowin->oy;
	}*/
}


#if 0
/*
 * Painting where text would be
 */
static errr Infofnt_text_non(int x, int y, int len)
{
	int x1, y1, x2, y2;

	/*** Find the dimensions ***/
	/*square_to_pixel(&x1, &y1, x, y);
	square_to_pixel(&x2, &y2, x + len, y);*/
	x1 = x * td->tile_wid + Infowin->ox;
	y1 = y * td->tile_hgt + Infowin->oy;
	x2 = (x+n) * td->tile_wid + Infowin->ox;
	y2 = (y+1) * td->tile_hgt + Infowin->oy;
	
	/*** Actually 'paint' the area ***/
	
	/* Just do a Fill Rectangle */
	XFillRectangle(Metadpy->dpy, Infowin->win, Infoclr->gc,
					x1, y1,
					x2 - x1, y2 - y1);
					/*x2 - x1, Infofnt->hgt);*/

	/* Success */
	return (0);
}

#endif
/*
 * Standard Text
 */
static errr Infofnt_text_std(int cx, int cy, cptr str, int len)
{
	int i;
	
	int x, y;

	term_data *td = (term_data*)(Term->data);

	/*** Do a brief info analysis ***/

	/* Do nothing if the string is null */
	if (!str || !*str) return (-1);

	/* Get the length of the string */
	if (len < 0) len = strlen(str);

	/*** Decide where to place the string, vertically ***/

	/*square_to_pixel(&x, &y, cx, cy);*/
	x = cx * td->tile_wid + Infowin->ox;
	y = cy * td->tile_hgt + Infowin->oy;
	
	/* Ignore Vertical Justifications */
	y += Infofnt->asc;
	

	/*** Actually draw 'str' onto the infowin ***/

	/* Be sure the correct font is ready */
	XSetFont(Metadpy->dpy, Infoclr->gc, Infofnt->info->fid);


	/*** Handle the fake mono we can enforce on fonts ***/
	
	/* Monotize the font if required */
	if (Infofnt->mono || is_bigtiled(cx + len - 1, cy))
	{
		/* Horizontal Justification */
		x += Infofnt->off;
		
		/* Do each character */
		for (i = 0; i < len; ++i)
		{
			if (is_bigtiled(cx + i, cy))
			{
				/* Note that the Infoclr is set up to contain the Infofnt */
				XDrawImageString(Metadpy->dpy, Infowin->win, Infoclr->gc,
								x, y, str + i, 1);
				x += td->tile_wid * tile_width_mult;
			}
			else
			{
				/* Note that the Infoclr is set up to contain the Infofnt */
				XDrawImageString(Metadpy->dpy, Infowin->win, Infoclr->gc,
								x, y, str + i, 1);
				x += td->tile_wid;
			}
		}
	}

	/* Assume monoospaced font */
	else
	{
		/* Note that the Infoclr is set up to contain the Infofnt */
		XDrawImageString(Metadpy->dpy, Infowin->win, Infoclr->gc,
		                 x, y, str, len);
	}


	/* Success */
	return (0);
}


extern bool SaveWindow(infowin *win, char*filename);
extern int main_menu_x11(metadpy *mdpy, term_data *data, int data_count, int mx, int my);

extern bool use_main_menu; /* whether a port is using the textui menu bar */
extern int (*main_menu_bar_fn) (keycode_t); /* the button function for the textui menu bar */
int menu_bar_x11(keycode_t buttonid)
{
	if (buttonid == ESCAPE) {
		/* show the main context menu */
		int ret;
		bool ikf = p_ptr->cmd.inkey_flag;
		while ((ret = main_menu_x11(Metadpy, data, MAX_TERM_DATA, (COL_MAP>>1), 1)) == 3);
		p_ptr->cmd.inkey_flag = ikf;
		if (ret <= 1)
			Term_redraw();
	}
	return 1;
}


/*
 * Write the "prefs" for a single term
 */
static void save_prefs_aux(term_data *td, ini_settings *ini, cptr sec_name)
{
	char buf[256];

	/* Paranoia */
	if (!td->win) {
		/* Visible */
		ini_setting_set_string(ini, sec_name, "Visible", "0", 2);
		return;
	}

	/* Visible */
	strncpy(buf, td->visible ? "1" : "0", 256);
	ini_setting_set_string(ini, sec_name, "Visible", buf, 256);

	/* Font */
	if(td->fnt && td->fnt->name) {
		strncpy(buf, td->fnt->name, 256);
	} else {
		strncpy(buf, "8X13.FON", 256);
	}
	ini_setting_set_string(ini, sec_name, "Font", buf, 256);

	/* Tile size (x) */
	strnfmt(buf, 256, "%d", td->tile_wid);
	ini_setting_set_string(ini, sec_name, "TileWid", buf, 256);

	/* Tile size (y) */
	strnfmt(buf, 256, "%d", td->tile_hgt);
	ini_setting_set_string(ini, sec_name, "TileHgt", buf, 256);

	/* Window size (x) */
	strnfmt(buf, 256, "%d", td->t.wid);
	ini_setting_set_string(ini, sec_name, "NumCols", buf, 256);

	/* Window size (y) */
	strnfmt(buf, 256, "%d", td->t.hgt);
	ini_setting_set_string(ini, sec_name, "NumRows", buf, 256);

	/* Window position (x) */
	strnfmt(buf, 256, "%d", td->win->x - Metadpy->wmbdr_x);
	ini_setting_set_string(ini, sec_name, "PositionX", buf, 256);

	/* Window position (y) */
	if (td->win->y == 2*Metadpy->wmbdr_y) {
		/* HACK - special case what usually happens when the initial
		 * y pos is 0 ( or <Metadpy->wmbdr_y) */
		buf[0] = '0';
		buf[1] = '\0';
	} else {
		strnfmt(buf, 256, "%d", td->win->y - Metadpy->wmbdr_y);
	}
	ini_setting_set_string(ini, sec_name, "PositionY", buf, 256);

	/* Border position (x) */
	strnfmt(buf, 256, "%d", td->win->ox);
	ini_setting_set_string(ini, sec_name, "BorderX", buf, 256);

	/* Window position (y) */
	strnfmt(buf, 256, "%d", td->win->oy);
	ini_setting_set_string(ini, sec_name, "BorderY", buf, 256);

}


/*
 * Write the "prefs"
 *
 * We assume that the windows have all been initialized
 */
static bool save_prefs(cptr ini_file)
{
	int i;
	ini_settings *ini = NULL;
	char buf[128];

	if (ini_settings_new(&ini) < 0)
		return;

	/* Save the "arg_graphics" flag */
	strnfmt(buf, 128, "%d", arg_graphics);
	ini_setting_set_string(ini, "Angband", "Graphics", buf, 128);

	/* Save the tile width */
	strnfmt(buf, 128, "%d", smoothRescaling ? 1 : 0);
	ini_setting_set_string(ini, "Angband", "SmoothRescaling", buf, 128);

	/* Save the tile width */
	strnfmt(buf, 128, "%d", tile_width_mult);
	ini_setting_set_string(ini, "Angband", "TileWidth", buf, 128);

	/* Save the tile height */
	strnfmt(buf, 128, "%d", tile_height_mult);
	ini_setting_set_string(ini, "Angband", "TileHeight", buf, 128);

	/* Save the "use_bigtile" flag */
	strnfmt(buf, 128, "%d", use_bigtile ? 1 : 0);
	ini_setting_set_string(ini, "Angband", "Bigtile", buf, 128);

	/* Save the "arg_sound" flag */
	strnfmt(buf, 128, "%d", arg_sound ? 1 : 0);
	ini_setting_set_string(ini, "Angband", "Sound", buf, 128);

	/* some optional flags */
	strnfmt(buf, 128, "%d", arg_fiddle ? 1 : 0);
	ini_setting_set_string_def(ini, "Angband", "Fiddle", buf, 128, "0");

	strnfmt(buf, 128, "%d", arg_wizard ? 1 : 0);
	ini_setting_set_string_def(ini, "Angband", "Wizard", buf, 128, "0");

	strnfmt(buf, 128, "%d", arg_force_roguelike ? 1 : 0);
	ini_setting_set_string_def(ini, "Angband", "force_roguelike", buf, 128, "0");

	strnfmt(buf, 128, "%d", arg_force_original ? 1 : 0);
	ini_setting_set_string_def(ini, "Angband", "force_original", buf, 128, "0");

#ifdef SUPPORT_GAMMA
	if (gamma_correction > 0) {
		strnfmt(buf, 128, "%d", gamma_correction);
		ini_setting_set_string(ini, "Angband", "Gamma", buf, 128);
	}
#endif /* SUPPORT_GAMMA */

	/* Save window prefs */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		term_data *td = &data[i];

		strnfmt(buf, 128, "Term-%d", i);

		save_prefs_aux(td, ini, buf);
	}

	/* actually write the file */
	ini_settings_save(ini_file, ini);

	/* cleanup the ini memory */
	ini_settings_close(&ini);

	/* Success */
	return TRUE;
}


/*
 * Load the "prefs" for a single term
 */
static void load_prefs_aux(term_data *td, ini_settings *ini, cptr sec_name)
{
	char tmp[256];

	int wid, hgt;

	/* Visible */
	td->visible = (ini_setting_get_uint32(ini, sec_name, "Visible", td->visible) != 0);

	/* Desired font, with default */
	ini_setting_get_string(ini, sec_name, "Font", tmp, 255, td->font_want);

	/* Analyze font, save desired font name */
	if (td->font_want) string_free(td->font_want);
	td->font_want = string_make(analyze_file(tmp, &wid, &hgt));

	/* Tile size */
	td->tile_wid = ini_setting_get_uint32(ini, sec_name, "TileWid", wid);
	td->tile_hgt = ini_setting_get_uint32(ini, sec_name, "TileHgt", hgt);

	/* Window size */
	td->cols = ini_setting_get_uint32(ini, sec_name, "NumCols", td->cols);
	td->rows = ini_setting_get_uint32(ini, sec_name, "NumRows", td->rows);

	/* Window position */
	td->pos_x = ini_setting_get_uint32(ini, sec_name, "PositionX", td->pos_x);
	td->pos_y = ini_setting_get_uint32(ini, sec_name, "PositionY", td->pos_y);

	/* Border size */
	td->bdr_x = ini_setting_get_uint32(ini, sec_name, "BorderX", td->bdr_x);
	td->bdr_y = ini_setting_get_uint32(ini, sec_name, "BorderY", td->bdr_y);
}


/*
 * Load the "prefs"
 */
static bool load_prefs(cptr ini_file)
{
	int i;

	char buf[256];
	bool first_start;
	ini_settings *ini = NULL;

	i = ini_settings_load(ini_file, &ini);
	first_start = FALSE;
	if (i<0) {
		if (i == -4) {
			first_start = TRUE;
		} else {
			return FALSE;
		}
	}

	/* Extract the "arg_graphics" flag */
	arg_graphics = ini_setting_get_uint32(ini, "Angband", "Graphics", arg_graphics);
	
	/* Extract the tile width */
	tile_width_mult = ini_setting_get_uint32(ini, "Angband", "TileWidth", 1);

	/* Extract the tile height */
	tile_height_mult = ini_setting_get_uint32(ini, "Angband", "TileHeight", 1);

	/* Extract the "use_bigtile" flag */
	use_bigtile = ini_setting_get_uint32(ini, "Angband", "Bigtile", arg_bigtile);

	/* Extract the "arg_sound" flag */
	arg_sound = (ini_setting_get_uint32(ini, "Angband", "Sound", arg_sound) != 0);

	/* Extract the "arg_fiddle" flag */
	arg_fiddle = (ini_setting_get_uint32(ini, "Angband", "Fiddle", arg_fiddle) != 0);

	/* Extract the "arg_wizard" flag */
	arg_wizard = (ini_setting_get_uint32(ini, "Angband", "Wizard", arg_wizard) != 0);

	/* Extract the "arg_roguelike" flag */
	arg_force_roguelike = (ini_setting_get_uint32(ini, "Angband", "force_roguelike", arg_force_roguelike) != 0);

	/* Extract the "arg_original" flag */
	arg_force_original = (ini_setting_get_uint32(ini, "Angband", "force_original", arg_force_original) != 0);

#ifdef SUPPORT_GAMMA

	/* Extract the gamma correction */
	gamma_correction = ini_setting_get_uint32(ini, "Angband", "Gamma", 0);

#endif /* SUPPORT_GAMMA */

	/* Load window prefs */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		term_data *td = &data[i];

		strnfmt(buf, 256, "Term-%d", i);

		load_prefs_aux(td, ini, buf);
	}

	/* cleanup the ini memory */
	ini_settings_close(&ini);

	if (first_start) {
		default_layout_x11(data, MAX_TERM_DATA, Metadpy);
	}

	/* Paranoia */
	if (data[0].cols < 80) data[0].cols = 80;
	if (data[0].rows < 24) data[0].rows = 24;
	data[0].visible = 1;

	/* Success */
	if (first_start) {
		return FALSE;
	} else {
		return TRUE;
	}
}


/*
 * Process a keypress event
 *
 * Also appears in "main-xaw.c".
 */
static void react_keypress(XKeyEvent *ev)
{
	int i, n, mc, ms, mo, mx;

	uint ks1;

	KeySym ks;

	char buf[128];
	char msg[128];


	/* Check for "normal" keypresses */
	n = XLookupString(ev, buf, 125, &ks, NULL);

	/* Terminate */
	buf[n] = '\0';


	/* Hack -- Ignore "modifier keys" */
	if (IsModifierKey(ks)) return;


	/* Hack -- convert into an unsigned int */
	ks1 = (uint)(ks);

	/* Extract four "modifier flags" */
	mc = (ev->state & ControlMask) ? TRUE : FALSE;
	ms = (ev->state & ShiftMask) ? TRUE : FALSE;
	mo = (ev->state & Mod1Mask) ? TRUE : FALSE;
	mx = (ev->state & Mod2Mask) ? TRUE : FALSE;


	/* Normal keys with no modifiers */
	if (n && !mo && !mx && !IsSpecialKey(ks))
	{
		/* Enqueue the normal key(s) */
		for (i = 0; buf[i]; i++) Term_keypress(buf[i]);

		/* All done */
		return;
	}


	/* Handle a few standard keys (bypass modifiers) XXX XXX XXX */
	switch (ks1)
	{
		case XK_Escape:
		{
			Term_keypress(ESCAPE);
			return;
		}

		case XK_Return:
		{
			Term_keypress('\r');
			return;
		}

		case XK_Tab:
		{
			Term_keypress('\t');
			return;
		}

		case XK_Delete:
		case XK_BackSpace:
		{
			Term_keypress('\010');
			return;
		}
	}


	/* Hack -- Use the KeySym */
	if (ks)
	{
		strnfmt(msg, sizeof(msg), "%c%s%s%s%s_%lX%c", 31,
		        mc ? "N" : "", ms ? "S" : "",
		        mo ? "O" : "", mx ? "M" : "",
		        (unsigned long)(ks), 13);
	}

	/* Hack -- Use the Keycode */
	else
	{
		strnfmt(msg, sizeof(msg), "%c%s%s%s%sK_%X%c", 31,
		        mc ? "N" : "", ms ? "S" : "",
		        mo ? "O" : "", mx ? "M" : "",
		        ev->keycode, 13);
	}

	/* Enqueue the "macro trigger" string */
	for (i = 0; msg[i]; i++) Term_keypress(msg[i]);


	/* Hack -- auto-define macros as needed */
	if (n && (macro_find_exact(msg) < 0))
	{
		/* Create a macro */
		macro_add(msg, buf);
	}
}

/*
 * Process a keypress event
 *
 * Also appears in "main-xaw.c".
 */
static void react_mousepress(XButtonEvent *ev)
{
	int mods, x, y;

	/* Extract four "modifier flags" */
	mods = 0;
	if (ev->state & ControlMask) mods |= 2;
	if (ev->state & ShiftMask) mods |= 1;
	if (ev->state & Mod1Mask) mods |= 4;
	//if (ev->state & Mod2Mask) mods |= 8;

	/* The co-ordinates are only used in Angband format. */
	pixel_to_square(&x, &y, ev->x, ev->y);
	
	/* switch buttons 2 and 3 to match other platforms (for 3 button mice) */
	if (ev->button == 3) ev->button = 2;
	else if (ev->button == 2) ev->button = 3;

	Term_mousepress(ev->button, mods, x, y);
}


errr Term_pict_x11(int ox, int oy, int n, const byte *ap, const char *cp, const byte *tap, const char *tcp);
int init_graphics_x11()
{
	int res, i;
	graphics_mode *mode;
	char filename[1024];
	Display *dpy = Metadpy->dpy;
	term_data *td = &data[0];

	mode = get_graphics_mode(arg_graphics);
	if (mode && mode->file) {
		if (mode->alphablend) {
			/* set or clear the flags needed in the ReadTiles function */
			tiles.bFlags |= 1;
		} else {
			tiles.bFlags &= ~1;
		}

		/* Try the file */
		path_build(filename, 1024, ANGBAND_DIR_XTRA, format("graf/%s",mode->file));

		res = ReadTiles(dpy, filename, &tiles);
		if (res >= 0) {
			/* set the cell size used during resize */
			tiles.CellWidth = mode->cell_width;
			tiles.CellHeight = mode->cell_height;

			/* copy the tiles to the other tile sheets */
			viewtiles.CellWidth = td->tile_wid * tile_width_mult;
			viewtiles.CellHeight = td->tile_hgt * tile_height_mult;
			res = ResizeTiles(dpy, &viewtiles, &tiles);

			maptiles.CellWidth = td->tile_wid;
			maptiles.CellHeight = td->tile_hgt;
			res = ResizeTiles(dpy, &maptiles, &tiles);

			current_graphics_mode = mode;
		}
		if (res < 0) {
			return res;
		}
	} else {
		plog_fmt("Desired graphics mode (%d) not found.", arg_graphics);
		return -2;
	}

	/* Initialize the windows */
	for (i = 0; i < MAX_TERM_DATA; i++) {
		char *TmpData;
		int j;
		int ii, jj;
		int depth = DefaultDepth(dpy, DefaultScreen(dpy));
		Visual *visual = DefaultVisual(dpy, DefaultScreen(dpy));
		int total;

		term_data *td = &data[i];
		term_data *o_td = NULL;

		term *t = &td->t;

		/* Graphics hook */
		/*if (current_graphics_mode->alphablend) {
			t->pict_hook = Term_pict_alpha_x11;
		} else {*/
			t->pict_hook = Term_pict_x11;
		/*}*/

		/* Use graphics sometimes */
		t->higher_pict = TRUE;

		if (!(td->visible)) continue;

		/* Initialize the transparency masks */
		/* Determine total bytes needed for image */
		ii = 1;
		jj = (depth - 1) >> 2;
		while (jj >>= 1) ii <<= 1;

		/* Pad the scanline to a multiple of 4 bytes */
		total = td->tile_wid * ii * tile_width_mult;
		total = (total + 3) & ~3;
		total *= td->tile_hgt;

		TmpData = (char *)malloc(total);

		td->TmpImage = XCreateImage(dpy,visual,depth,
			ZPixmap, 0, TmpData, td->tile_wid*tile_width_mult,
			td->tile_hgt * tile_height_mult, 32, 0);
	}

	return 0;
}

void close_graphics_x11()
{
	int i;

	FreeTiles(&tiles);
	FreeTiles(&viewtiles);
	FreeTiles(&maptiles);
	current_graphics_mode = NULL;

	for (i = 0; i < MAX_TERM_DATA; i++) {
		/* Graphics hook */
		data[i].t.pict_hook = NULL;

		/* Use graphics sometimes */
		data[i].t.higher_pict = FALSE;
	}
}


/*
 * Delay resizing/redrawing windows so that don't waste cpu
 */
static bool event_pending = FALSE;

static void scan_pending_windows(void)
{
	term_data *old_td = (term_data*)(Term->data);
	term_data *td;
	
	int i;
	
	/* Unset flag */
	event_pending = FALSE;
	
	/* Scan the windows */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		td = &data[i];
		
		/* Skip nonexistant terms */
		if (!td->win) continue;
	
		/* Hack -- activate the Term */
		Term_activate(&td->t);
		
		/* Need to resize? */
		if (td->win->resize == TRUE)
		{
			int cols, rows, wid, hgt;

			int ox = Infowin->ox;
			int oy = Infowin->oy;
		
			/* Unset flags */
			td->win->resize = FALSE;
			td->win->redraw = FALSE;
			
			/* Hack -- activate the window */
			Infowin_set(td->win);
			
			/* Determine "proper" number of rows/cols */
			cols = ((Infowin->w - (ox + ox)) / td->tile_wid);
			rows = ((Infowin->h - (oy + oy)) / td->tile_hgt);
			
			/* Hack -- minimal size */
			if (cols < 1) cols = 1;
			if (rows < 1) rows = 1;

			/* Hack the main window must be at least 80x24 */
			if (i == 0)
			{
				if (cols < 80) cols = 80;
				if (rows < 24) rows = 24;
			}
			
			/* Desired size of window */
			wid = cols * td->tile_wid + (ox + ox);
			hgt = rows * td->tile_hgt + (oy + oy);
			
			/* Resize the windows if any "change" is needed */
			if ((Infowin->w != wid) || (Infowin->h != hgt))
			{
				/* Resize window */
				Infowin_set(td->win);
				Infowin_resize(wid, hgt);
			}

			/* Resize the Term (if needed) */
			(void)Term_resize(cols, rows);

		}
		else if (td->win->redraw == TRUE)
		{
			/* Unset flag */
			td->win->redraw = FALSE;
			
			Term_redraw();
		}
	}
	
	/* Hack -- Activate the old term */
	Term_activate(&old_td->t);

	/* Hack -- Activate the proper window */
	Infowin_set(old_td->win);
}

/*
 * Process events
 */
static errr CheckEvent(bool wait)
{
	term_data *old_td = (term_data*)(Term->data);

	XEvent xev_body, *xev = &xev_body;

	term_data *td = NULL;
	infowin *iwin = NULL;

	int i;
	int window = 0;

	/* No pending events? */
	if (!XPending(Metadpy->dpy))
	{
		/* Need to resize/redraw? */
		if (event_pending) scan_pending_windows();
	
		/* Do not wait unless requested */
		if (!wait) return (1);
	}
	
	/* Load the Event */
	XNextEvent(Metadpy->dpy, xev);


	/* Notice new keymaps */
	if (xev->type == MappingNotify)
	{
		XRefreshKeyboardMapping(&xev->xmapping);
		return 0;
	}


	/* Scan the windows */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		if (xev->xany.window == data[i].win->win)
		{
			td = &data[i];
			iwin = td->win;
			window = i;
			break;
		}
	}

	/* Unknown window */
	if (!td || !iwin) return (0);


	/* Hack -- activate the Term */
	Term_activate(&td->t);

	/* Hack -- activate the window */
	Infowin_set(iwin);


	/* Switch on the Type */
	switch (xev->type)
	{
		case ButtonRelease:
		{
			/* Nothing */
			break;
		}
		case ButtonPress:
		{
			if (xev->type == ButtonPress) {
				/* Hack -- use "old" term */
				Term_activate(&old_td->t);

				/* Process the key */
				if (window == 0) {
					/* only send the press if it was in the main window */
					react_mousepress(&(xev->xbutton));
				}
			}
			break;
		}
		case MotionNotify:
		{
			/* Nothing */
			break;
		}


		case KeyRelease:
		{
			/* Nothing */
			break;
		}

		case KeyPress:
		{
			/* Hack -- use "old" term */
			Term_activate(&old_td->t);

			/* Process the key */
			react_keypress(&(xev->xkey));

			break;
		}

		case Expose:
		{
			int x1, x2, y1, y2;
			
			/* Hack - if we have a pending resize, ignore */
			if (!event_pending)
			{
				pixel_to_square(&x1, &y1, xev->xexpose.x, xev->xexpose.y);
				pixel_to_square(&x2, &y2, xev->xexpose.x + xev->xexpose.width,
										xev->xexpose.y + xev->xexpose.height);

				Term_redraw_section(x1, y1, x2, y2);
			}
			else
			{
				/* Make sure to redraw later */
				event_pending = TRUE;
				Infowin->redraw = TRUE;
			}

			break;
		}

		case MapNotify:
		{
			Infowin->mapped = 1;
			Term->mapped_flag = TRUE;
			break;
		}

		case UnmapNotify:
		{
			Infowin->mapped = 0;
			Term->mapped_flag = FALSE;
			break;
		}

		/* Move and/or Resize */
		case ConfigureNotify:
		{
			int cols, rows, wid, hgt;

			int ox = Infowin->ox;
			int oy = Infowin->oy;
			
			/* Save the new Window Parms */
			Infowin->x = xev->xconfigure.x;
			Infowin->y = xev->xconfigure.y;

			/* Hack - store the values of the first configure which
			 * should be the window border width and window title bar
			 * height  */
			if ((Metadpy->wmbdr_x == 0) && xev->xconfigure.x) {
				Metadpy->wmbdr_x = xev->xconfigure.x;
				Metadpy->wmbdr_y = xev->xconfigure.y;
			}

			if ((Infowin->w != xev->xconfigure.width) ||
				(Infowin->h != xev->xconfigure.height))
			{
				Infowin->w = xev->xconfigure.width;
				Infowin->h = xev->xconfigure.height;
							
				/* We need to notice the resize of this window (later) */
				Infowin->resize = TRUE;
				event_pending = TRUE;
			}
			
			/* Determine "proper" number of rows/cols */
			cols = ((Infowin->w - (ox + ox)) / td->tile_wid);
			rows = ((Infowin->h - (oy + oy)) / td->tile_hgt);
			
			/* Hack -- minimal size */
			if (cols < 1) cols = 1;
			if (rows < 1) rows = 1;

			/* Hack the main window must be at least 80x24 */
			if (i == 0)
			{
				if (cols < 80) cols = 80;
				if (rows < 24) rows = 24;
			}
			
			/* Desired size of window */
			wid = cols * td->tile_wid + (ox + ox);
			hgt = rows * td->tile_hgt + (oy + oy);
			
			/* Resize the windows if any "change" is needed */
			if ((Infowin->w != wid) || (Infowin->h != hgt))
			{
				/* Resize window */
				Infowin_set(td->win);
				Infowin_resize(wid, hgt);
			}
			
			break;
		}

		/* a requested message type */
		case ClientMessage:
		{
			if ((Atom)xev->xclient.data.l[0] == Metadpy->wmDeleteWindow) {
				if (character_generated) {
					if (!p_ptr->cmd.inkey_flag) {
						plog("You may not do that right now.");
						break;
					}
					/* forget messages */
					msg_flag = FALSE;

					/* save the game */
#ifdef ZANGBAND
					do_cmd_save_game(FALSE);
#else /* ZANGBAND */
					do_cmd_save_game();
#endif /* ZANGBAND */
				}

				/* Free resources */
				cleanup_angband();

				/* Quit */
				quit(NULL);
			}
			break;
		}
	}


	/* Hack -- Activate the old term */
	Term_activate(&old_td->t);

	/* Hack -- Activate the proper window */
	Infowin_set(old_td->win);


	/* Success */
	return (0);
}


/*
 * Handle "activation" of a term
 */
static errr Term_xtra_x11_level(int v)
{
	term_data *td = (term_data*)(Term->data);

	/* Handle "activate" */
	if (v)
	{
		/* Activate the window */
		Infowin_set(td->win);

		/* Activate the font */
		Infofnt_set(td->fnt);
	}

	/* Success */
	return (0);
}

errr Term_xtra_x11_clear(void)
{
	term_data *td = (term_data*)(Term->data);

	/* Erase (use black) */
	Infoclr_set(clr[TERM_DARK]);

	/* Just do a Fill Rectangle */
	XFillRectangle(Metadpy->dpy, Infowin->win, Infoclr->gc,
					0, 0,
					Infowin->w, Infowin->h);

	/* Success */
	return (0);
}

/*
 * React to changes
 */
errr Term_xtra_x11_react(void)
{
	int i;

	if (Metadpy->color)
	{
		/* Check the colors */
		for (i = 0; i < 256; i++)
		{
			if ((color_table[i][0] != angband_color_table[i][0]) ||
			    (color_table[i][1] != angband_color_table[i][1]) ||
			    (color_table[i][2] != angband_color_table[i][2]) ||
			    (color_table[i][3] != angband_color_table[i][3]))
			{
				Pixell pixel;

				/* Save new values */
				color_table[i][0] = angband_color_table[i][0];
				color_table[i][1] = angband_color_table[i][1];
				color_table[i][2] = angband_color_table[i][2];
				color_table[i][3] = angband_color_table[i][3];

				/* Create pixel */
				pixel = create_pixel(Metadpy->dpy,
				                     color_table[i][1],
				                     color_table[i][2],
				                     color_table[i][3]);

				/* Change the foreground */
				Infoclr_set(clr[i]);
				Infoclr_change_fg(pixel);
			}
		}
	}

#ifdef USE_GRAPHICS

	/* Handle "arg_graphics" */
	if (use_graphics != arg_graphics)
	{
		/* Switch off transparency */
		use_transparency = FALSE;
		
		/* Free any existing images */
		FreeTiles(&tiles);
		FreeTiles(&viewtiles);
		FreeTiles(&maptiles);

		/* Initialize (if needed) */
		if (arg_graphics && (init_graphics_x11() < 0))
		{
			/* Warning */
			plog("Cannot initialize graphics!");

			/* Cannot enable */
			arg_graphics = GRAPHICS_NONE;
			tile_width_mult = 1;
			tile_height_mult = 1;
			close_graphics_x11();
		}
    
		/* Change setting */
		use_graphics = arg_graphics;

		/* Reset visuals */
		#ifdef ANGBAND_2_8_1
			reset_visuals();
		#else /* ANGBAND_2_8_1 */
			reset_visuals(TRUE);
		#endif /* ANGBAND_2_8_1 */

	}

#endif /* USE_GRAPHICS */

#if 0
	/* Clean up windows */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		term *old = Term;

		term_data *td = &data[i];

		/* Update resized windows */
		if ((td->cols != td->t.wid) || (td->rows != td->t.hgt))
		{
			/* Activate */
			Term_activate(&td->t);

			/* Hack -- Resize the term */
			Term_resize(td->cols, td->rows);

			/* Redraw the contents */
			Term_redraw();

			/* Restore */
			Term_activate(old);
		}
	}
#endif
	/* Success */
	return (0);
}


/*
 * Handle a "special request"
 */
static errr Term_xtra_x11(int n, int v)
{
	/* Handle a subset of the legal requests */
	switch (n)
	{
		/* Make a noise */
		case TERM_XTRA_NOISE: Metadpy_do_beep(); return (0);

		/* Flush the output XXX XXX */
		case TERM_XTRA_FRESH: Metadpy_update(1, 0, 0); return (0);

		/* Process random events XXX */
		case TERM_XTRA_BORED: return (CheckEvent(0));

		/* Process Events XXX */
		case TERM_XTRA_EVENT: return (CheckEvent(v));

		/* Flush the events XXX */
		case TERM_XTRA_FLUSH: while (!CheckEvent(FALSE)); return (0);

		/* Handle change in the "level" */
		case TERM_XTRA_LEVEL: return (Term_xtra_x11_level(v));
		
		/* Delay for some milliseconds */
		case TERM_XTRA_DELAY:
			if (v > 0) usleep(1000 * v);
			return (0);

		/* React to changes */
		case TERM_XTRA_REACT: return (Term_xtra_x11_react());
	}

	/* Unknown */
	return (1);
}

/*
 * Draw the cursor as a rectangular outline
 */
static errr Term_curs_x11(int x, int y)
{
	term_data *td = (term_data*)(Term->data);

	int x1, y1;

	/*square_to_pixel(&x1, &y1, x, y);*/
	x1 = x * td->tile_wid + Infowin->ox;
	y1 = y * td->tile_hgt + Infowin->oy;
	
	if (is_bigtiled(x, y))
	{
		XDrawRectangle(Metadpy->dpy, Infowin->win, xor->gc,
			 x1, y1,
			 td->tile_wid * tile_width_mult - 1, td->tile_hgt * tile_height_mult - 1);
	}
	else
	{
		XDrawRectangle(Metadpy->dpy, Infowin->win, xor->gc,
			 x1, y1,
			 td->tile_wid - 1, td->tile_hgt - 1);
	}
	
	/* Success */
	return (0);
}


/*
 * Erase some characters.
 */
static errr Term_wipe_x11(int x, int y, int n)
{
	int x1, y1, x2, y2;
	term_data *td = (term_data*)(Term->data);

	/* Erase (use black) */
	Infoclr_set(clr[TERM_DARK]);

	/* Erase some space */

	/*** Find the dimensions ***/
	x1 = x * td->tile_wid + Infowin->ox;
	y1 = y * td->tile_hgt + Infowin->oy;
	x2 = (x+n) * td->tile_wid + Infowin->ox;
	y2 = (y+1) * td->tile_hgt + Infowin->oy;
	
	/*** Actually 'paint' the area ***/
	
	/* Just do a Fill Rectangle */
	XFillRectangle(Metadpy->dpy, Infowin->win, Infoclr->gc,
					x1, y1,
					x2 - x1, y2 - y1);
					/*x2 - x1, Infofnt->hgt);*/
	
	/* Success */
	return (0);
}


/*
 * Draw some textual characters.
 */
static errr Term_text_x11(int x, int y, int n, byte a, cptr s)
{
	/* Erase area first */
	Term_wipe_x11(x, y, n);

	/* Draw the text */
	Infoclr_set(clr[a]);

	/* Draw the text */
	Infofnt_text_std(x, y, s, n);
	
	/* Success */
	return (0);
}


#ifdef USE_GRAPHICS




/*
 * Draw some graphical characters.
 */
errr Term_pict_x11(int ox, int oy, int n, const byte *ap, const char *cp, const byte *tap, const char *tcp)
{
	int i;
	int x1 = 0, y1 = 0;

	byte a;
	char c;

	byte ta;
	char tc;
	
	int x, y;
	int x2, y2;
	int k,l;

	unsigned long pixel, blank;

	term_data *td = (term_data*)(Term->data);
	
	int wid, hgt;
	XImage *tiles;
	XImage *mask;

	/* Starting point */
	/*square_to_pixel(&x, &y, ox, oy);*/
	x = ox * td->tile_wid + Infowin->ox;
	y = oy * td->tile_hgt + Infowin->oy;
	
	for (i = 0; i < n; ++i)
	{
		a = *ap++;
		c = *cp++;

		ta = *tap++;
		tc = *tcp++;

		/* What are we drawing? */
		if (is_bigtiled(ox + i, oy))
		{
			tiles = viewtiles.color;
			mask = viewtiles.mask;
			wid = viewtiles.CellWidth;
			hgt = viewtiles.CellHeight;
		}
		else
		{
			tiles = maptiles.color;
			mask = maptiles.mask;
			wid = maptiles.CellWidth;
			hgt = maptiles.CellHeight;
		}

		/* For extra speed - cache these values */
		x1 = (c & 0x7F) * wid;
		y1 = (a & 0x7F) * hgt;

		/* For extra speed - cache these values */
		x2 = (tc & 0x7F) * wid;
		y2 = (ta & 0x7F) * hgt;

		if (mask) {
			/* Draw terrain */
			XPutImage(Metadpy->dpy, td->win->win,
						clr[0]->gc,
						tiles,
						x2, y2,
						x, y,
						wid, hgt);
			/* Draw object */
			if (((x1 != x2) || (y1 != y2))) {
				XSetFunction(Metadpy->dpy, clr[0]->gc, GXand);
				XPutImage(Metadpy->dpy, td->win->win,
							clr[0]->gc,
							mask,
							x1, y1,
							x, y,
							wid, hgt);
				XSetFunction(Metadpy->dpy, clr[0]->gc, GXor);
				XPutImage(Metadpy->dpy, td->win->win,
							clr[0]->gc,
							tiles,
							x1, y1,
							x, y,
							wid, hgt);
				XSetFunction(Metadpy->dpy, clr[0]->gc, GXcopy);
			}
		} else {
			/* Optimise the common case */
			if (((x1 == x2) && (y1 == y2)) ||
			    !(((byte)ta & 0x80) && ((byte)tc & 0x80)))
			{
				/* Draw object / terrain */
				XPutImage(Metadpy->dpy, td->win->win,
							clr[0]->gc,
							tiles,
							x1, y1,
							x, y,
							wid, hgt);
			} else {
				/* Mega Hack^2 - assume the top left corner is "blank" */
				blank = XGetPixel(tiles, 0, 0);
	
				for (k = 0; k < wid; k++)
				{
					for (l = 0; l < hgt; l++)
					{
						/* If mask set... */
						if ((pixel = XGetPixel(tiles, x1 + k, y1 + l)) == blank)
						{
							/* Output from the terrain */
							pixel = XGetPixel(tiles, x2 + k, y2 + l);

							if (pixel == blank) pixel = 0L;
						}

						/* Store into the temp storage. */
						XPutPixel(td->TmpImage, k, l, pixel);
					}
				}


				/* Draw to screen */
				XPutImage(Metadpy->dpy, td->win->win,
							clr[0]->gc,
							td->TmpImage,
							0, 0, x, y,
							wid, hgt);
			}
		}
			
		x += wid;
	}

	/* Success */
	return (0);
}

#endif /* USE_GRAPHICS */



/*
 * Initialize a term_data
 */
static errr term_data_init(term_data *td, int i)
{
	term *t = &td->t;

	cptr name = angband_term_name[i];

	cptr font;

	int x = 0;
	int y = 0;

	int cols = 80;
	int rows = 24;

	int ox = 1;
	int oy = 1;

	int wid, hgt, num;

	char buf[80];

	cptr str;

	int val;

	XClassHint *ch;

	char res_name[20];
	char res_class[20];

	XSizeHints *sh;

	/* don't do anything if not visible */
	if (!(td->visible)) {
		return (0);
	}


	/* Get default font for this term */
	font = td->font_want;

	x = td->pos_x;
	y = td->pos_y;
	ox = td->bdr_x;
	oy = td->bdr_y;

	rows = td->rows;
	cols = td->cols;

	/* Hack the main window must be at least 80x24 */
	if (!i)
	{
		if (cols < 80) cols = 80;
		if (rows < 24) rows = 24;
	}


	/* Prepare the standard font */
	MAKE(td->fnt, infofnt);
	Infofnt_set(td->fnt);
	if (Infofnt_init_data(font))
	{
		quit("Stopping");
	}

	/* Hack -- key buffer size */
	num = ((i == 0) ? 1024 : 16);

	/* Assume full size windows */
	wid = cols * td->tile_wid + (ox + ox);
	hgt = rows * td->tile_hgt + (oy + oy);

	/* Create a top-window */
	MAKE(td->win, infowin);
	Infowin_set(td->win);
	Infowin_init_top(x, y, wid, hgt, 0,
	                 Metadpy->fg, Metadpy->bg);
	if ((Infowin->x != x) && (Infowin->x - x > Metadpy->wmbdr_x)) {
		Metadpy->wmbdr_x = Infowin->x - x;
	}
	if ((Infowin->y != y) && (Infowin->y - y > Metadpy->wmbdr_y)) {
		Metadpy->wmbdr_y = Infowin->y - y;
	}

	/* Ask for certain events */
	Infowin_set_mask(ExposureMask | StructureNotifyMask | KeyPressMask |
	                 PointerMotionMask | ButtonPressMask | ButtonReleaseMask);

	/* Set the window name */
	Infowin_set_name(name);

	/* Save the inner border */
	Infowin->ox = ox;
	Infowin->oy = oy;

	/* Make Class Hints */
	ch = XAllocClassHint();

	if (ch == NULL) quit("XAllocClassHint failed");

	strnfmt(res_name, sizeof(res_name), "%s", name);
	res_name[0] = FORCELOWER(res_name[0]);
	ch->res_name = res_name;

	strnfmt(res_class, sizeof(res_class), "Angband");
	ch->res_class = res_class;

	XSetClassHint(Metadpy->dpy, Infowin->win, ch);

	/* Make Size Hints */
	sh = XAllocSizeHints();

	/* Oops */
	if (sh == NULL) quit("XAllocSizeHints failed");

	/* Main window has a differing minimum size */
	if (i == 0)
	{
		/* Main window min size is 80x24 */
		sh->flags = (PMinSize | PMaxSize);
		sh->min_width = 80 * td->tile_wid + (ox + ox);
		sh->min_height = 24 * td->tile_hgt + (oy + oy);
		sh->max_width = 255 * td->tile_wid + (ox + ox);
		sh->max_height = 255 * td->tile_hgt + (oy + oy);
	}

	/* Other windows can be shrunk to 1x1 */
	else
	{
		/* Other windows */
		sh->flags = (PMinSize | PMaxSize);
		sh->min_width = td->tile_wid + (ox + ox);
		sh->min_height = td->tile_hgt + (oy + oy);
		sh->max_width = 255 * td->tile_wid + (ox + ox);
		sh->max_height = 255 * td->tile_hgt + (oy + oy);
	}

	/* Resize increment */
	sh->flags |= PResizeInc;
	sh->width_inc = td->tile_wid;
	sh->height_inc = td->tile_hgt;

	/* Base window size */
	sh->flags |= PBaseSize;
	sh->base_width = (ox + ox);
	sh->base_height = (oy + oy);

	/* Use the size hints */
	XSetWMNormalHints(Metadpy->dpy, Infowin->win, sh);

	/* Map the window */
	Infowin_map();

	/* Set pointers to allocated data */
	td->sizeh = sh;
	td->classh = ch;

	/* Move the window to requested location */
	if ((x >= 0) && (y >= 0)) Infowin_impell(x, y);


	/* Initialize the term */
	term_init(t, cols, rows, num);

	/* Use a "soft" cursor */
	t->soft_cursor = TRUE;

	/* Erase with "black space" */
	t->attr_blank = TERM_DARK;
	t->char_blank = ' ';

	/* Hooks */
	t->xtra_hook = Term_xtra_x11;
	t->curs_hook = Term_curs_x11;
	t->wipe_hook = Term_wipe_x11;
	t->text_hook = Term_text_x11;

	/* Save the data */
	t->data = td;

	/* Activate (important) */
	Term_activate(t);

	/* Success */
	return (0);
}


/*
 * Initialization function for an "X11" module to Angband
 */
errr init_x11(int argc, char *argv[])
{
	int i;
	
	cptr dpy_name = "";

	int num_term = 3;

	/* Parse args */
	for (i = 1; i < argc; i++)
	{
		if (prefix(argv[i], "-d"))
		{
			dpy_name = &argv[i][2];
			continue;
		}

#ifdef USE_GRAPHICS
		if (prefix(argv[i], "-s"))
		{
			smoothRescaling = FALSE;
			continue;
		}	
#endif /* USE_GRAPHICS */

		if (prefix(argv[i], "-n"))
		{
			num_term = atoi(&argv[i][2]);
			if (num_term > MAX_TERM_DATA) num_term = MAX_TERM_DATA;
			else if (num_term < 1) num_term = 1;
			continue;
		}

		if (prefix(argv[i], "-x"))
		{
			x11_settings = &argv[i][2];
			continue;
		}

		plog_fmt("Ignoring option: %s", argv[i]);
	}

	/* Init the Metadpy if possible */
	if (Metadpy_init_name(dpy_name)) return (-1);
	
#ifdef USE_GRAPHICS
	/* We support bigtile mode */
	if (arg_bigtile && arg_graphics) use_bigtile = TRUE;
#endif /* USE_GRAPHICS */

	/* Initialize default term data */
	for (i = 0; i < MAX_TERM_DATA; i++) {
		int wid, hgt;
		term_data *td = &data[i];

		td->cols = 80;
		td->rows = 24;
		td->visible = 0;
		td->pos_x = i*10;
		td->pos_y = i*10;
		td->bdr_x = 0;
		td->bdr_y = 0;
		/*if (i)
			td->font_want = string_make("8X13");
		else
			td->font_want = string_make("10X20");*/
		td->font_want = string_make(get_default_font(i));
		(void)analyze_file(td->font_want, &wid, &hgt);
		td->tile_wid = wid;
		td->tile_hgt = hgt;
		td->TmpImage = NULL;
		if (i < num_term) {
			td->visible = 1;
		}
	}
	/* load settings */
	ini_file = string_make(format("%s%s%s", ANGBAND_DIR_USER, PATH_SEP, x11_settings));
	if (ini_file)
		load_prefs(ini_file);

#ifdef SUPPORT_GAMMA
	if (gamma_correction > 0) {
		/* Build the gamma corrected color table */
		build_gamma_table(gamma_correction);
	}
#endif /* SUPPORT_GAMMA */

	/* Prepare cursor color */
	MAKE(xor, infoclr);
	Infoclr_set(xor);
	Infoclr_init_ppn(Metadpy->fg, Metadpy->bg, "xor", 0);


	/* Prepare normal colors */
	for (i = 0; i < 256; ++i)
	{
		Pixell pixel;

		MAKE(clr[i], infoclr);

		Infoclr_set(clr[i]);

		/* Acquire Angband colors */
		color_table[i][0] = angband_color_table[i][0];
		color_table[i][1] = angband_color_table[i][1];
		color_table[i][2] = angband_color_table[i][2];
		color_table[i][3] = angband_color_table[i][3];

#ifdef SUPPORT_GAMMA
		if (gamma_correction > 0) {
			color_table[i][0] = gamma_table[color_table[i][0]];
			color_table[i][1] = gamma_table[color_table[i][1]];
			color_table[i][2] = gamma_table[color_table[i][2]];
			color_table[i][3] = gamma_table[color_table[i][3]];
		}
#endif /* SUPPORT_GAMMA */

		/* Default to monochrome */
		pixel = ((i == 0) ? Metadpy->bg : Metadpy->fg);

		/* Handle color */
		if (Metadpy->color)
		{
			/* Create pixel */
			pixel = create_pixel(Metadpy->dpy,
			                     color_table[i][1],
			                     color_table[i][2],
			                     color_table[i][3]);
		}

		/* Initialize the color */
		Infoclr_init_ppn(pixel, Metadpy->bg, "cpy", 0);
	}

	/* Initialize the windows */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		term_data *td = &data[i];

		if (td->visible) {
			/* Initialize the term_data */
			term_data_init(td, i);

			/* Save global entry */
			angband_term[i] = Term;
		}
	}
	
	/* Activate the "Angband" window screen */
	Term_activate(&data[0].t);

#ifdef USE_GRAPHICS
	/* Try graphics */
	if (arg_graphics)
	{
		int res;
		res = init_graphics_x11();
		if (res < 0) {
			use_graphics = 0;
			tile_width_mult = 1;
			tile_height_mult = 1;
			close_graphics_x11();
		} else {
			use_graphics = arg_graphics;
		}
	}

#endif /* USE_GRAPHICS */

	/* Raise the "Angband" window */
	Infowin_set(data[0].win);
	Infowin_raise();

	/* Activate the "Angband" window screen */
	Term_activate(&data[0].t);

	use_main_menu = TRUE;
	main_menu_bar_fn = menu_bar_x11;
	/* Success */
	return (0);
}

void close_x11(void)
{

	term_data *td;
	term *t;
	int i;

	if (ini_file) {
		save_prefs(ini_file);
		string_free(ini_file);
		ini_file = NULL;
	}

	close_graphics_x11();

	for (i = 0; i < MAX_TERM_DATA; i++) {
		td = &data[i];
		t = &td->t;

		if (td->font_want) string_free(td->font_want);

		if (td->TmpImage) {
			XDestroyImage(td->TmpImage);
		}

		/* Free size hints */
		if (td->sizeh) XFree(td->sizeh);

		/* Free class hints */
		if (td->classh) XFree(td->classh);

		/* Free fonts */
		if (td->fnt) {
			Infofnt_set(td->fnt);
			(void)Infofnt_nuke();
			FREE(td->fnt);
		}

		/* Free window */
		if (td->win) {
			Infowin_set(td->win);
			(void)Infowin_nuke();
			FREE(td->win);
		}

		/* Free term */
		/*(void)term_nuke(t); already freed before we got here? */
	}

	/* Free colors */
	Infoclr_set(xor);
	(void)Infoclr_nuke();
	FREE(xor);

	for (i = 0; i < 256; ++i)
	{
		Infoclr_set(clr[i]);
		(void)Infoclr_nuke();
		FREE(clr[i]);
	}

	/* Close link to display */
	(void)Metadpy_nuke();
}

#endif /* USE_X11 */

