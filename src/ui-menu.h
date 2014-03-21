#ifndef INCLUDED_UI_MENU_H
#define INCLUDED_UI_MENU_H

#include "button.h"
#include "ui-region.h"
/* included for keycode_t definition */
typedef keycode_t ui_event;

/* as long as the menu code strips / stops the "natural" advanced
 * mouse presses, we can use the higher bits to mark selection and moves */
#define EVT_ESCAPE    ESCAPE
#define EVT_SELECT    (keycode_t)(128|64|32|16)
#define EVT_MOVE      (keycode_t)(128|64|32|8)
#define EVT_RESIZE    (keycode_t)(128|64|32|16|8)
#define EVT_NONE      (keycode_t)0
#define EVENT_EMPTY   (keycode_t)0

/*** Constants ***/

/* Colors for interactive menus */
enum
{
	CURS_UNKNOWN = 0,		/* Use gray / dark blue for cursor */
	CURS_KNOWN = 1			/* Use white / light blue for cursor */
};

/* Cursor colours for different states */
extern const byte curs_attrs[2][2];

/* Standard menu orderings */
extern const char lower_case[];			/* abc..z */
extern const char upper_case[];			/* ABC..Z */
extern const char all_letters[];		/* abc..zABC..Z */


/*
  Together, these classes define the constant properties of
  the various menu classes.

  A menu consists of:
   - menu_iter, which describes how to handle the type of "list" that's 
     being displayed as a menu
   - a menu_skin, which describes the layout of the menu on the screen.
   - various bits and bobs of other data (e.g. the actual list of entries)
 */
typedef struct menu_type menu_type;



/*** Predefined menu "skins" ***/

/**
 * Types of predefined skins available.
 */
typedef enum
{
	/*
	 * A simple list of actions with an associated name and id.
	 * Private data: an array of menu_action
	 */
	MN_ITER_ACTIONS = 1,

	/*
	 * A list of strings to be selected from - no associated actions.
	 * Private data: an array of const char *
	 */
	MN_ITER_STRINGS = 2
} menu_iter_id;


/**
 * Primitive menu item with bound action.
 */
/*typedef struct
{
	int flags;
	char tag;
	char *name;
	void (*action)(const char *title, int row);
} menu_action;*/


/**
 * Flags for menu_actions.
 */
/*#define MN_ACT_GRAYED     0x0001*/ /* Allows selection but no action */
/*#define MN_ACT_HIDDEN     0x0002*/ /* Row is hidden, but may be selected via tag */


/*
 * A function pointer used in displaying menus
 *
 * The function takes a number for the option chosen
 * and will return TRUE if the selection works, and FALSE
 * if the menu should stay up.
 */
typedef bool (*menu_select_type) (int option);

typedef struct menu_action menu_action;

struct menu_action
{
	/*char tag;*/					/* display tag */
	cptr text;					/* Option text */
	cptr help;					/* Help file to use */
	menu_select_type action;	/* Action to do */

	byte flags;					/* Flags controling option behaviour */
};

/* Menu seperator */
#define MENU_SEPERATOR {"", NULL, NULL, 0x00}

/* Menu terminator */
#define MENU_END {NULL, NULL, NULL, 0x00}

#define MN_ACTIVE		0x01	/* Available to choose */
#define MN_SELECT		0x02	/* Can 'select' action */
#define MN_CLEAR		0x04	/* Clear screen before calling */



/**
 * Underlying function set for displaying lists in a certain kind of way.
 */
typedef struct
{
	/* Returns menu item tag (optional) */
	char (*get_tag)(menu_type *menu, int oid);

	/*
	 * Validity checker (optional--all rows are assumed valid if not present)
 	 * Return values will be interpreted as: 0 = no, 1 = yes, 2 = hide.
 	 */
	int (*valid_row)(menu_type *menu, int oid);

	/* Displays a menu row */
	void (*display_row)(menu_type *menu, int oid, bool cursor,
			int row, int col, int width);

	/* Handle 'positive' events (selections or cmd_keys) */
	/* XXX split out into a select handler and a cmd_key handler */
	bool (*row_handler)(menu_type *menu, const ui_event *event, int oid);

	/* Called when the screen resizes */
	void (*resize)(menu_type *m);
} menu_iter;



/*** Menu skins ***/

/**
 * Identifiers for the kind of layout to use
 */
typedef enum
{
	MN_SKIN_SCROLL = 1,   /**< Ordinary scrollable single-column list */
	MN_SKIN_COLUMNS = 2   /**< Multicolumn view */
} skin_id;


/* Class functions for menu layout */
typedef struct
{
	/* Determines the cursor index given a (mouse) location */
	int (*get_cursor)(int row, int col, int n, int top, rect_region *loc);

	/* Displays the current list of visible menu items */
	void (*display_list)(menu_type *menu, int cursor, int *top, rect_region *);

	/* Specifies the relative menu item given the state of the menu */
	char (*get_tag)(menu_type *menu, int pos);

	/* Process a direction */
	ui_event (*process_dir)(menu_type *menu, int dir);
} menu_skin;



/*** Base menu structure ***/

/**
 * Flags for menu appearance & behaviour
 */
enum
{
	/* Tags are associated with the view, not the element */
	MN_REL_TAGS = 0x01,

	/* No tags -- movement key and mouse browsing only */
	MN_NO_TAGS = 0x02,

	/* Tags to be generated by the display function */
	MN_PVT_TAGS = 0x04,

	/* Tag selections can be made regardless of the case of the key pressed. 
	 * i.e. 'a' activates the line tagged 'A'. */
	MN_CASELESS_TAGS = 0x08,

	/* double tap (or keypress) for selection; single tap is cursor movement */
	MN_DBL_TAP = 0x10,

	/* no select events to be triggered */
	MN_NO_ACTION = 0x20
} menu_type_flags;


/* Base menu type */
struct menu_type
{
	/** Public variables **/
	char *header;
	char *title;
	char *prompt;

	/* Keyboard shortcuts for menu selection-- shouldn't overlap cmd_keys */
	const char *selections; 

	/* String of characters that when pressed, menu handler should be called */
	/* Mustn't overlap with 'selections' or some items may be unselectable */
	const char *cmd_keys;

  	/* auxiliary browser help function */
	void (*browse_hook)(int oid, void *db, const rect_region *loc);

	/* Flags specifying the behavior of this menu (from menu_type_flags) */
	int flags;


	/** Private variables **/

	/* Stored boundary, set by menu_layout().  This is used to calculate
	 * where the menu should be displayed on display & resize */
	rect_region boundary;

	int filter_count;        /* number of rows in current view */
	const int *filter_list;  /* optional filter (view) of menu objects */

	int count;               /* number of rows in underlying data set */
	void *menu_data;         /* the data used to access rows. */

	const menu_skin *skin;      /* menu display style functions */
	const menu_iter *row_funcs; /* menu skin functions */

	/* State variables */
	int cursor;              /* Currently selected row */
	int top;                 /* Position in list for partial display */
	rect_region active;      /* Subregion actually active for selection */

};



/*** Menu API ***/

/**
 * Allocate and return a new, initialised, menu.
 */
menu_type *menu_new(skin_id, const menu_iter *iter);
menu_type *menu_new_action(menu_action *acts, size_t n);


/**
 * Initialise a menu, using the skin and iter functions specified.
 */
void menu_init(menu_type *menu, skin_id skin, const menu_iter *iter);


/**
 * Given a predefined menu kind, return its iter functions.
 */
const menu_iter *menu_find_iter(menu_iter_id iter_id);


/**
 * Set menu private data and the number of menu items.
 *
 * Menu private data is then available from inside menu callbacks using
 * menu_priv().
 */
void menu_setpriv(menu_type *menu, int count, void *data);


/**
 * Return menu private data, set with menu_setpriv().
 */
void *menu_priv(menu_type *menu);


/*
 * Set a filter on what items a menu can display.
 *
 * Use this if your menu private data has 100 items, but you want to choose
 * which ones of those to display at any given time, e.g. in an inventory menu.
 * object_list[] should be an array of indexes to display, and n should be its
 * length.
 */
void menu_set_filter(menu_type *menu, const int object_list[], int n);


/**
 * Remove any filters set on a menu by menu_set_filer().
 */
void menu_release_filter(menu_type *menu);


/**
 * Ready a menu for display in the region specified.
 *
 * XXX not ready for dynamic resizing just yet
 */
bool menu_layout(menu_type *menu, const rect_region *loc);


/**
 * Display a menu.
 * If reset_screen is true, it will reset the screen to the previously saved
 * state before displaying.
 */
void menu_refresh(menu_type *menu, bool reset_screen);


/**
 * Run a menu.
 *
 * 'notify' is a bitwise OR of ui_event_type events that you want to
 * menu_select to return to you if they're not handled inside the menu loop.
 * e.g. if you want to handle key events without specifying a menu_iter->handle
 * function, you can set notify to EVT_KBRD, and any non-navigation keyboard
 * events will stop the menu loop and return them to you.
 *
 * Some events are returned by default, and else are EVT_ESCAPE and EVT_SELECT.
 * 
 * Event types that can be returned:
 *   EVT_ESCAPE: no selection; go back (by default)
 *   EVT_SELECT: menu->cursor is the selected menu item (by default)
 *   EVT_MOVE:   the cursor has moved
 *   EVT_KBRD:   unhandled keyboard events
 *   EVT_MOUSE:  unhandled mouse events  
 *   EVT_RESIZE: resize events
 * 
 * XXX remove 'notify'
 *
 * If popup is TRUE, the screen background is saved before starting the menu,
 * and restored before each redraw. This allows variably-sized information
 * at the bottom of the menu.
 */
ui_event menu_select(menu_type *menu, int notify, bool popup);

/**
 * Set the menu cursor to the next valid row.
 */
void menu_ensure_cursor_valid(menu_type *m);


/* Internal menu stuff that cmd-know needs because it's quite horrible */
bool menu_handle_mouse(menu_type *menu, const ui_event *in, ui_event *out);
bool menu_handle_keypress(menu_type *menu, const ui_event *in, ui_event *out);


/*** Dynamic menu handling ***/

menu_type *menu_dynamic_new(void);
void menu_dynamic_add(menu_type *m, const char *text, int value);
void menu_dynamic_add_label(menu_type *m, const char *text, const char label, int value, char *label_list);
void menu_dynamic_set_select(menu_type *m);
size_t menu_dynamic_longest_entry(menu_type *m);
int menu_dynamic_select(menu_type *m);
void menu_dynamic_free(menu_type *m);

#endif /* INCLUDED_UI_MENU_H */
