/* File: z-term.h */

/*
 * Copyright (c) 1997 Ben Harrison
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 */

#ifndef INCLUDED_Z_TERM_H
#define INCLUDED_Z_TERM_H

#include "h-basic.h"


/*
 * A term_win is a "window" for a Term
 *
 *	- Cursor Useless/Visible codes
 *	- Cursor Location (see "Useless")
 *
 *	- Array[h] -- Access to the attribute array
 *	- Array[h] -- Access to the character array
 *
 *	- Array[h*w] -- Attribute array
 *	- Array[h*w] -- Character array
 *
 *  - Pointer to the next window in the stack.
 *
 * Note that the attr/char pair at (x,y) is a[y][x]/c[y][x]
 * and that the row of attr/chars at (0,y) is a[y]/c[y]
 */

typedef struct term_win term_win;

struct term_win
{
	bool cu, cv;
	byte cx, cy;

	byte **a;
	char **c;

	byte *va;
	char *vc;

	byte **ta;
	char **tc;

	byte *vta;
	char *vtc;
	
	term_win *next;
};



/*
 * An actual "term" structure
 *
 *	- Extra "user" info (used by application)
 *
 *	- Extra "data" info (used by implementation)
 *
 *
 *	- Flag "user_flag"
 *	  An extra "user" flag (used by application)
 *
 *
 *	- Flag "data_flag"
 *	  An extra "data" flag (used by implementation)
 *
 *
 *	- Flag "active_flag"
 *	  This "term" is "active"
 *
 *	- Flag "mapped_flag"
 *	  This "term" is "mapped"
 *
 *	- Flag "total_erase"
 *	  This "term" should be fully erased
 *
 *	- Flag "fixed_shape"
 *	  This "term" is not allowed to resize
 *
 *	- Flag "icky_corner"
 *	  This "term" has an "icky" corner grid
 *
 *	- Flag "soft_cursor"
 *	  This "term" uses a "software" cursor
 *
 *	- Flag "always_pict"
 *	  Use the "Term_pict()" routine for all text
 *
 *	- Flag "higher_pict"
 *	  Use the "Term_pict()" routine for special text
 *
 *	- Flag "always_text"
 *	  Use the "Term_text()" routine for invisible text
 *
 *	- Flag "unused_flag"
 *	  Reserved for future use
 *
 *	- Flag "never_bored"
 *	  Never call the "TERM_XTRA_BORED" action
 *
 *	- Flag "never_frosh"
 *	  Never call the "TERM_XTRA_FROSH" action
 *
 *
 *	- Value "attr_blank"
 *	  Use this "attr" value for "blank" grids
 *
 *	- Value "char_blank"
 *	  Use this "char" value for "blank" grids
 *
 *
 *	- Ignore this pointer
 *
 *	- Keypress Queue -- various data
 *
 *	- Keypress Queue -- pending keys
 *
 *
 *	- Window Width (max 255)
 *	- Window Height (max 255)
 *
 *	- Minimum modified row
 *	- Maximum modified row
 *
 *	- Minimum modified column (per row)
 *	- Maximum modified column (per row)
 *
 *
 *	- Displayed screen image
 *	- Requested screen image
 *
 *
 *	- Hook for init-ing the term
 *	- Hook for nuke-ing the term
 *
 *	- Hook for user actions
 *
 *	- Hook for extra actions
 *
 *	- Hook for placing the cursor
 *
 *	- Hook for drawing some blank spaces
 *
 *	- Hook for drawing a string of chars using an attr
 *
 *	- Hook for drawing a sequence of special attr/char pairs
 */

typedef struct term term;

struct term
{
	vptr user;

	vptr data;

	bool user_flag;

	bool data_flag;

	bool active_flag;
	bool mapped_flag;
	bool total_erase;
	bool fixed_shape;
	bool icky_corner;
	bool soft_cursor;
	bool always_pict;
	bool higher_pict;
	bool always_text;
	bool unused_flag;
	bool never_bored;
	bool never_frosh;

	byte attr_blank;
	char char_blank;
	byte attr_special;
	char char_big;
	char char_cave;

	char *key_queue;

	u16b key_head;
	u16b key_tail;
	u16b key_xtra;
	u16b key_size;

	char *mouse_queue;
	u16b *mouse_x_queue;
	u16b *mouse_y_queue;

	byte mouse_head;
	byte mouse_tail;
	byte mouse_size;

	byte wid;
	byte hgt;

	byte y1;
	byte y2;

	byte *x1;
	byte *x2;

	term_win *old;
	term_win *scr;

	void (*init_hook) (term *t);
	void (*nuke_hook) (term *t);

	errr (*user_hook) (int n);

	errr (*xtra_hook) (int n, int v);

	errr (*curs_hook) (int x, int y);

	errr (*wipe_hook) (int x, int y, int n);

	errr (*text_hook) (int x, int y, int n, byte a, cptr s);

	void (*resize_hook) (void);

	errr (*pict_hook) (int x, int y, int n, const byte *ap, const char *cp,
					   const byte *tap, const char *tcp);

	/* draw a cave grid */
	errr (*cave_hook) (int x, int y, int n, 
		const void **grid, const int cx, const int cy); /* grid is actually cave_type** */

	/* override the map display */
	void (*view_map_hook)(term *t);

	/* ui override hooks */
	bool (*get_aim_dir_hook)(int *dp);
	bool (*get_check_hook)(bool def, bool esc, cptr prompt);
	bool (*get_string_hook)(char *buf, int len, cptr prompt);
	bool (*askfor_hook)(char *buf, int len); /* uses text grid to display */
	bool (*get_com_hook)(cptr prompt, char *command);
	bool (*get_com_m_hook)(cptr prompt, char *command);
	s32b (*get_quantity_big_hook)(cptr prompt, s32b initial, s32b max);
	s16b (*get_quantity_hook)(cptr prompt, s16b initial, s16b max);
	u32b (*get_number_hook)(cptr prompt, u32b initial);
	void *(*get_item_hook)(cptr pmt, cptr str, int mode); /* return value is actually object_type */
	void (*store_hook)(const void *f1_ptr, const void *st_ptr); /* f1 is actually a pointer to field_type, st_ptr is store_type */
	void (*bldg_hook)(const void *f_ptr, const void *b_ptr); /* f is actually a pointer to field_type, b_ptr is store_type */
	void (*character_hook)(void);
	void (*destroy_hook)(void);
	void (*inven_hook)(void);
	void (*equip_hook)(void);
	void (*browse_hook)(void);
	void (*term_fresh_hook)(void);
	bool (*target_set_interactive_hook)(int mode, int x, int y);
	void (*redraw_stuff_hook)(void);
	void (*window_stuff_hook)(void);
	bool (*player_birth_hook)(void);

};





/**** Available Constants ****/


/*
 * Definitions for the "actions" of "Term_xtra()"
 *
 * These values may be used as the first parameter of "Term_xtra()",
 * with the second parameter depending on the "action" itself.  Many
 * of the actions shown below are optional on at least one platform.
 *
 * The "TERM_XTRA_EVENT" action uses "v" to "wait" for an event
 * The "TERM_XTRA_SHAPE" action uses "v" to "show" the cursor
 * The "TERM_XTRA_FROSH" action uses "v" for the index of the row
 * The "TERM_XTRA_SOUND" action uses "v" for the index of a sound
 * The "TERM_XTRA_ALIVE" action uses "v" to "activate" (or "close")
 * The "TERM_XTRA_LEVEL" action uses "v" to "resume" (or "suspend")
 * The "TERM_XTRA_DELAY" action uses "v" as a "millisecond" value
 *
 * The other actions do not need a "v" code, so "zero" is used.
 */
#define TERM_XTRA_EVENT	1		/* Process some pending events */
#define TERM_XTRA_FLUSH 2		/* Flush all pending events */
#define TERM_XTRA_SHAPE 4		/* Set cursor shape (optional) */
#define TERM_XTRA_FROSH 5		/* Flush one row (optional) */
#define TERM_XTRA_FRESH 6		/* Flush all rows (optional) */
#define TERM_XTRA_NOISE 7		/* Make a noise (optional) */
#define TERM_XTRA_SOUND 8		/* Make a sound (optional) */
#define TERM_XTRA_BORED 9		/* Handle stuff when bored (optional) */
#define TERM_XTRA_REACT 10		/* React to global changes (optional) */
#define TERM_XTRA_ALIVE 11		/* Change the "hard" level (optional) */
#define TERM_XTRA_LEVEL 12		/* Change the "soft" level (optional) */
#define TERM_XTRA_DELAY 13		/* Delay some milliseconds (optional) */


/*** Color constants ***/


/*
 * Angband "attributes" (with symbols, and base (R,G,B) codes)
 *
 * The "(R,G,B)" codes are given in "fourths" of the "maximal" value,
 * and should "gamma corrected" on most (non-Macintosh) machines.
 */
#define TERM_DARK     0  /* d */    /* 0 0 0 */
#define TERM_WHITE    1  /* w */    /* 4 4 4 */
#define TERM_SLATE    2  /* s */    /* 2 2 2 */
#define TERM_ORANGE   3  /* o */    /* 4 2 0 */
#define TERM_RED      4  /* r */    /* 3 0 0 */
#define TERM_GREEN    5  /* g */    /* 0 2 1 */
#define TERM_BLUE     6  /* b */    /* 0 0 4 */
#define TERM_UMBER    7  /* u */    /* 2 1 0 */
#define TERM_L_DARK   8  /* D */    /* 1 1 1 */
#define TERM_L_WHITE  9  /* W */    /* 3 3 3 */
#define TERM_VIOLET   10 /* v */    /* 4 0 4 */
#define TERM_YELLOW   11 /* y */    /* 4 4 0 */
#define TERM_L_RED    12 /* R */    /* 4 0 0 */
#define TERM_L_GREEN  13 /* G */    /* 0 4 0 */
#define TERM_L_BLUE   14 /* B */    /* 0 4 4 */
#define TERM_L_UMBER  15 /* U */    /* 3 2 1 */

#define TERM_PURPLE      16    /* p */
#define TERM_L_PURPLE    17    /* P */
#define TERM_TEAL        18    /* t */
#define TERM_MUD         19    /* m */
#define TERM_L_YELLOW    20    /* Y */
#define TERM_MAGENTA     21    /* i */
#define TERM_L_TEAL      22    /* T */
#define TERM_L_VIOLET    23    /* V */
#define TERM_L_PINK      24    /* I */
#define TERM_MUSTARD     25    /* M */
#define TERM_BLUE_SLATE  26    /* z */
#define TERM_DEEP_L_BLUE 27    /* Z */

#define TERM_SKIPA 255
#define TERM_SKIPC -1

#define TERM_BLANKA TERM_WHITE
#define TERM_BLANKC ' '

/* The following allow color 'translations' to support environments with a limited color depth
 * as well as translate colours to alternates for e.g. menu highlighting. */

#define ATTR_FULL        0    /* full color translation */
#define ATTR_MONO        1    /* mono color translation */
#define ATTR_VGA         2    /* 16 color translation */
#define ATTR_BLIND       3    /* "Blind" color translation */
#define ATTR_LIGHT       4    /* "Torchlit" color translation */
#define ATTR_DARK        5    /* "Dark" color translation */
#define ATTR_HIGH        6    /* "Highlight" color translation */
#define ATTR_METAL       7    /* "Metallic" color translation */
#define ATTR_MISC        8    /* "Miscellaneous" color translation - see misc_to_attr */

#define MAX_ATTR        9

/*
 * Maximum number of colours, and number of "basic" Angband colours
 */ 
#define MAX_COLORS        128
#define BASIC_COLORS    28



/**** Available Variables ****/

extern term *Term;

extern byte tile_width_mult; /* an attempt to use the graphics multiple of tilesize - Brett */
extern byte tile_height_mult;


/**** Available Functions ****/

extern errr Term_user(int n);
extern void Term_xtra(int n, int v);

extern void Term_queue_char(int x, int y, byte a, char c, byte ta, char tc);
extern void Term_big_queue_char(int x, int y, byte a, char c, byte ta, char tc);

extern void Term_queue_line(int x, int y, int n, byte *a, char *s, byte *ta,
							char *tc);
extern void Term_queue_str(int x, int y, int n, byte a, const char *s, byte ta,
							char tc);

extern void Term_fresh(void);
extern errr Term_set_cursor(int v);
extern void Term_gotoxy(int x, int y);
extern void Term_draw(int x, int y, byte a, char c);
extern void Term_addch(byte a, char c);
extern void Term_putch(int x, int y, byte a, char c);
extern void Term_big_putch(int x, int y, byte a, char c);
extern void Term_addstr(int n, byte a, const char *s);
extern void Term_putstr(int x, int y, int n, byte a, const char *s);
extern void Term_erase(int x, int y, int n);
extern void Term_clear(void);
extern void Term_redraw(void);
extern errr Term_redraw_section(int x1, int y1, int x2, int y2);

extern errr Term_get_cursor(int *v);
extern void Term_get_size(int *w, int *h);
extern errr Term_locate(int *x, int *y);
extern errr Term_what(int x, int y, byte *a, char *c);

extern void Term_flush(void);
extern errr Term_keypress(int k);
extern errr Term_mousepress(int button, int mods, int x, int y);
extern errr Term_key_push(int k);
extern errr Term_inkey(char *ch, bool wait, bool take);
extern errr Term_getmousepress(char *button, int *x, int *y);

extern void Term_save(void);
extern void Term_load(void);

extern errr Term_resize(int w, int h);

extern void Term_activate(term *t);

extern errr term_nuke(term *t);
extern errr term_init(term *t, int w, int h, int k);

extern bool Term_is_bigtiled(int x, int y);

#endif /* INCLUDED_Z_TERM_H */
