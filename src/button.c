/*
 * File: mouse.c
 * Purpose: Mousebutton handling code
 *
 * Copyright (c) 2007 Nick McConnell
 * 2d buttons, changes from angband Copyright (c) 2012 Brett Reid
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

#include "angband.h"
#include "button.h"

/*** Constants ***/

/**
 * Maximum length of a mouse button label
 */
#define MAX_MOUSE_LABEL 10


/*** Types ***/

/**
 * Maximum number of mouse buttons
 */
#define MAX_MOUSE_BUTTONS  20

typedef struct _button_mouse_1d
{
	char label[MAX_MOUSE_LABEL]; /*!< Label on the button */
	int left;                    /*!< Column containing the left edge of the button */
	int right;                   /*!< Column containing the right edge of the button */
	keycode_t key;               /*!< Keypress corresponding to the button */
} button_mouse_1d;

typedef struct _button_backup
{
	struct _button_backup *next; /* the button set that will be restored after this one */
	button_mouse *buttons;       /* the list of buttons */
	button_mouse *buttons_end;   /* the end of the list of buttons */
	button_mouse_1d *buttons_1d; /* the list of 1d buttons */
	int num;                     /* the number of buttons in the list */
	int num_1d;                  /* the number of 1d buttons in the list */
	int length_1d;               /* the length of the row of 1d buttons */
} button_backup;

/*** Variables ***/

static button_mouse *button_list_root;
static button_mouse *button_list_end;
static button_mouse_1d *button_1d_list;
static button_backup *button_backups;

static int button_num;
static int button_1d_start_x;
static int button_1d_start_y;
static int button_1d_length;
static int button_1d_num;
keycode_t mouse_press;

/*
 * Hooks for making and unmaking buttons
 */
button_add_2d_f button_add_2d_hook;
button_add_1d_f button_add_1d_hook;
button_kill_f button_kill_hook;
button_print_f button_print_hook;
button_get_f button_get_hook;



/*** Code ***/

/*
 * The mousebutton code. Buttons should be created when neccessary and
 * destroyed when no longer necessary.  By default, 1d buttons occupy the section
 * of the bottom line between the status display and the location display
 * in normal screen mode, and the bottom line after any prompt in alternate
 * screen mode.
 *
 * Individual ports may (and preferably will) handle this differently using
 * button_add_gui and button_kill_gui.
 */

/*
 * Add a button
 */
int button_add_1d_text(const char *label, char keypress)
{
	int i;
	int length = strlen(label);

	/* Check the label length */
	if (length > MAX_MOUSE_LABEL)
	{
		bell("Label too long - button abandoned!");
		return 0;
	}

	/* Check we haven't already got a button for this keypress */
	for (i = 0; i < button_1d_num; i++)
		if (button_1d_list[i].key == keypress)
			return 0;

	/* Make the button */
	button_1d_length += length;
	my_strcpy(button_1d_list[button_1d_num].label, label, MAX_MOUSE_LABEL);
	button_1d_list[button_1d_num].left = button_1d_length - length + 1;
	button_1d_list[button_1d_num].right = button_1d_length;
	button_1d_list[button_1d_num++].key = keypress;

	/* Redraw */
	p_ptr->redraw |= (PR_BUTTONS);

	/* Return the size of the button */
	return (length);
}

/*
 * Add a button
 */
int button_add_1d(const char *label, char keypress)
{
	if (button_add_1d_hook) {
		int res = (*button_add_1d_hook) (label, keypress);
		if (res) { // if res != 0
			return res;
		}
	}
	return button_add_1d_text(label, keypress);
}

button_mouse* button_add_start(int top, int left, keycode_t keypress)
{
	button_mouse* button = RNEW(button_mouse);

	button->label = NULL;
	button->left = left;
	button->right = left+1;
	button->top = top;
	button->bottom = top;
	button->next = 0;
	button->mods = 0;
	button->key = keypress;

	return button;
}

int button_add_end(button_mouse* button,
                   const char *label, keycode_t keypress,
                   int bottom, int right)
{
	if (bottom) button->bottom = bottom;
	if (!right && label) {
		right = strlen(label);
		right = button->left + right;
	}
	if (right) button->right = right;
	if (keypress) button->key = keypress;

	if (label) {
		if (button->label) {
			(void)string_free(button->label);
		}
		button->label = string_make(label);
	}
	if (button_add_2d_hook) {
		int res = (*button_add_2d_hook) (button);
		if (res) { // if res != 0
			return res;
		}
	}
	if (!button_list_root)
		button_list_root = button;
	if (button_list_end)
		button_list_end->next = button;
	button_list_end = button;
	button->next = 0;
	button_num++;

	/* Redraw */
	p_ptr->redraw |= (PR_BUTTONS);

	return 0;
}

int button_add_2d(int top, int left, int bottom, int right,
               const char *label, keycode_t keypress)
{
	button_mouse *bttn = button_add_start(top, left, 0);
	if (bttn) {
		return button_add_end(bttn, label, keypress, bottom, right);
	}
	return -1;
}

/*
 * Make a backup of the current buttons
 */
bool button_backup_all(void)
{
	button_backup *newbackup;

	newbackup = RNEW(button_backup);
	if(!newbackup) {
		return FALSE;
	}
	/* copy the 2d buttons */
	newbackup->buttons = button_list_root;
	newbackup->buttons_end = button_list_end;
	newbackup->num = button_num;
	button_list_root = NULL;
	button_list_end = NULL;

	/* copy the 1d buttons */
	if (button_num == 0) {
		newbackup->buttons_1d = NULL;
		newbackup->num_1d = 0;
		newbackup->length_1d = 0;
	} else {
		newbackup->buttons_1d = C_RNEW(button_1d_num, button_mouse_1d);
		if (!(newbackup->buttons_1d)) {
			/* free the 2d buttons TODO */
			FREE(newbackup);
			return FALSE;
		}
		/* Straight memory copy */
		(void)C_COPY(newbackup->buttons_1d, button_1d_list, button_1d_num, button_mouse_1d);

		newbackup->num_1d = button_1d_num;
		newbackup->length_1d = button_1d_length;
	}

	/* push the backup onto the stack */
	newbackup->next = button_backups;
	button_backups = newbackup;

	/* Redraw */
	p_ptr->redraw |= (PR_BUTTONS);

	return TRUE;
}


/*
 * Restore the buttons from backup
 */
void button_restore(void)
{
	int i = 0;

	/* Remove the current lot */
	button_kill_all();

	if (button_backups) {
		button_backup *next;

		/* restore the 2d buttons */
		button_list_root = button_backups->buttons;
		button_list_end = button_backups->buttons_end;
		button_num = button_backups->num;

		/* restore the 1d buttons */
		if (button_backups->buttons_1d) {
			/* straight memory copy */
			if (button_backups->num_1d > MAX_MOUSE_BUTTONS) {
				(void)C_COPY(button_1d_list, button_backups->buttons_1d, MAX_MOUSE_BUTTONS, button_mouse);
				button_1d_num = MAX_MOUSE_BUTTONS;
				button_1d_length = button_backups->length_1d;
				/* modify the length of the button row based on the buttons that were not copied */
				for (i = MAX_MOUSE_BUTTONS; i < button_backups->num_1d; i++) {
					button_1d_length -= strlen(button_backups->buttons_1d[i].label);
				}
			} else {
				(void)C_COPY(button_1d_list, button_backups->buttons_1d, button_backups->num_1d, button_mouse_1d);
				button_1d_num = button_backups->num_1d;
				button_1d_length = button_backups->length_1d;
			}
			FREE(button_backups->buttons_1d);
		}
		/* remove the backup from the stack */
		next = button_backups->next;
		if (button_backups->buttons_1d) {
			FREE(button_backups->buttons_1d);
		}
		FREE(button_backups);
		button_backups = next;
	}
			
	/* signal that the buttons need to be redrawn */
	p_ptr->redraw |= (PR_BUTTONS);
}


/*
 * Remove a button
 */
int button_kill_text(char keypress)
{
	int i, j, length;

	button_mouse *testee = button_list_root, *next;
	button_mouse *prev = NULL;
	/* Find the 2d button */
	while (testee) {
		next = testee->next;
		if (testee->key == keypress) {
			/* close the gap in the list */
			if (prev) {
				prev->next = next;
			}
			if (button_list_root == testee) {
				button_list_root = next;
			}
			if (button_list_end == testee) {
				button_list_end = prev;
			}
			/* free the memory */
			if (testee->label)
				string_free(testee->label);
			FREE(testee);
			button_num--;
			/* Redraw */
			p_ptr->redraw |= (PR_BUTTONS);
			return -1;
		} else {
			prev = testee;
		}
		testee = next;
	}

	/* Find the 1d button */
	for (i = 0; i < button_1d_num; i++)
		if (button_1d_list[i].key == keypress) break;

	/* No such button */
	if (i == button_1d_num)
	{
		return 0;
	}

	/* Find the length */
	length = button_1d_list[i].right - button_1d_list[i].left + 1;
	button_1d_length -= length;

	/* Move each button up one */
	for (j = i; j < button_1d_num - 1; j++)
	{
		button_1d_list[j] = button_1d_list[j + 1];

		/* Adjust length */
		button_1d_list[j].left -= length;
		button_1d_list[j].right -= length;
	}

	/* Wipe the data */
	button_1d_list[button_1d_num].label[0] = '\0';
	button_1d_list[button_1d_num].left = 0;
	button_1d_list[button_1d_num].right = 0;
	button_1d_list[button_1d_num--].key = 0;

	/* Redraw */
	p_ptr->redraw |= (PR_BUTTONS);
	//redraw_stuff(p_ptr);

	/* Return the size of the button */
	return (length);
}

/*
 * Kill a button
 */
int button_kill(char keypress)
{
	if (button_kill_hook) {
		int res = (*button_kill_hook) (keypress);
		if (res) { // if res != 0
			return res;
		}
	}
	return button_kill_text (keypress);
}

/*
 * Kill all buttons
 */
void button_kill_all(void)
{
	int i;

	button_mouse *bttn = button_list_root, *bnext;
	while (bttn) {
		bnext = bttn->next;
		if (button_kill_hook) {
			(void)(*button_kill_hook) (bttn->key);
		} else {
			if (bttn->label) {
				(void)string_free(bttn->label);
			}
			FREE(bttn);
		}
		bttn = bnext;
	}
	button_list_root = NULL;
	button_list_end = NULL;
	button_num = 0;

	/* Paranoia */
	if (!button_kill_hook) return;

	/* One by one */
	if (button_kill_hook) {
		for (i = button_num - 1; i >= 0; i--) {
			int res = (*button_kill_hook) (button_1d_list[i].key);
			if (res==0) {
				button_1d_list[i].label[0] = '\0';
				button_1d_list[i].left = 0;
				button_1d_list[i].right = 0;
				button_1d_list[i].key = 0;
			}
		}
	} else {
		for (i = 0; i < button_1d_num; i--) {
			button_1d_list[i].label[0] = '\0';
			button_1d_list[i].left = 0;
			button_1d_list[i].right = 0;
			button_1d_list[i].key = 0;
		}
	}
	button_1d_length = 0;
	button_1d_num = 0;
}


/*
 * Initialise buttons.
 */
void button_init(void)
{
	/* Prepare mouse button arrays */
	button_1d_list = C_ZNEW(MAX_MOUSE_BUTTONS, button_mouse_1d);
	button_backups = NULL;

	/* initialize the global numbers */
	button_1d_start_x = 0;
	button_1d_start_y = 0;
	button_1d_length = 0;
	button_1d_num = 0;

	button_num = 0;
	mouse_press = 0;

	/* Initialise the hooks */
	button_add_2d_hook = NULL;
	button_add_1d_hook = NULL;
	button_kill_hook = NULL;
	button_print_hook = NULL;
}

void button_hook(button_add_2d_f add, button_add_1d_f add_1d,
                 button_kill_f kill, button_print_f print,
                 button_get_f get)
{
	/* Initialise the hooks */
	button_add_2d_hook = add;
	button_add_1d_hook = add_1d;
	button_kill_hook = kill;
	button_print_hook = print;
	button_get_hook = get;
}

/*
 * Dispose of the button memory
 */
void button_free(void)
{
	button_backup *next;
	while (button_backups) {
		next = button_backups->next;
		if (button_backups->buttons) {
			button_mouse *bttn = button_backups->buttons, *bnext;
			while (bttn) {
				bnext = bttn->next;
				if (bttn->label)
					(void)string_free(bttn->label);
				FREE(bttn);
				bttn = bnext;
			}
			button_backups->buttons = NULL;
			button_backups->buttons_end = NULL;
		}
		if (button_backups->buttons_1d) {
			FREE(button_backups->buttons_1d);
		}
		FREE(button_backups);
		button_backups = next;
	}
	button_backups = NULL;
	if (button_list_root) {
		button_mouse *bttn = button_list_root, *bnext;
		while (bttn) {
			bnext = bttn->next;
			if (bttn->label)
				(void)string_free(bttn->label);
			FREE(bttn);
			bttn = bnext;
		}
		button_list_root = NULL;
		button_list_end = NULL;
	}
	if (button_1d_list) {
		FREE(button_1d_list);
		button_1d_list = NULL;
	}

	button_num = 0;
	button_1d_start_x = 0;
	button_1d_start_y = 0;
	button_1d_length = 0;
	button_1d_num = 0;
	mouse_press = 0;
}

/**
 * Return the character represented by a button at screen position (x, y),
 * or 0.
 */
char button_get_key(int x, int y)
{
	int i;

	/* first check 2d buttons */
	button_mouse *bttn = button_list_root;
	while (bttn) {
		if ((y >= bttn->top) && (y <= bttn->bottom)
				&& (x >= bttn->left) && (x <= bttn->right)) {
			if (bttn->key & 0x80) {
				mouse_press = bttn->key;
				return mouse_press;
			} else {
				mouse_press = 0;
				return bttn->key;
			}
		}
		bttn = bttn->next;
	}
	/* then check 1d buttons */
	for (i = 0; i < button_1d_num; i++) {
		if ((y == button_1d_start_y)
				&& (x >= button_1d_list[i].left + button_1d_start_x)
				&& (x <= button_1d_list[i].right + button_1d_start_x)) {
			if (button_1d_list[i].key & 0x80) {
				mouse_press = button_1d_list[i].key;
				return mouse_press;
			} else {
				mouse_press = 0;
				return button_1d_list[i].key;
			}
		}
	}

	return 0;
}

/**
 * Print the current button list at the specified `row` and `col`umn.
 */
size_t button_print(int row, int col)
{
	int j;
	button_mouse *bttn;
	if (button_print_hook) {
		size_t res = (*button_print_hook)(row, col);
		if (res) { // if res != 0
			return res;
		}
	}
	button_1d_start_x = col;
	button_1d_start_y = row;

	for (j = 0; j < button_1d_num; j++)
		put_fstr(col + button_1d_list[j].left, row, CLR_SLATE "%s", button_1d_list[j].label);

	/* print 2d buttons */
	bttn = button_list_root;
	while (bttn) {
		if (bttn->label && bttn->label[0]) {
			put_cstr(bttn->left, bttn->top, bttn->label, FALSE);
		}
		bttn = bttn->next;
	}

	return button_1d_length;
}
