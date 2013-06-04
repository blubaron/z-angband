/*
 * File: cmd-menu.c
 * Purpose: Show a menu of all commands.
 *
 * Copyright (c) 2010 Andi Sidwell
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
/*#include "cave.h"
#include "cmds.h"
#include "files.h"
#include "game-cmd.h"
#include "keymap.h"
#include "textui.h"
#include "wizard.h"
#include "target.h"
#include "squelch.h"
#include "object/object.h"
#include "monster/mon-lore.h"
#include "monster/mon-util.h"*/

#define OPT(x) (x)

/*
 * Holds a generic command - if cmd is set to other than CMD_NULL 
 * it simply pushes that command to the game, otherwise the hook 
 * function will be called.
 */
struct cmd_info
{
	const char *desc;
	keycode_t key_original;
	keycode_t key_roguelike;
	void (*hook)(void);
	bool (*prereq)(void);
};

static struct cmd_info cmd_item[] =
{
	{ "Inscribe an object", '{', 0, NULL, NULL },
	{ "Uninscribe an object", '}', 0, NULL, NULL },
	{ "Wear/wield an item", 'w', 0, NULL, NULL },
	{ "Take off/unwield an item", 't', 'T', NULL, NULL },
	{ "Examine an item", 'I', 0, NULL, NULL},//, textui_obj_examine },
	{ "Drop an item", 'd', 0, NULL, NULL },
	{ "Fire your missile weapon", 'f', 't', NULL, NULL},//, NULL, player_can_fire_msg },
	{ "Use an item (incl. staff)", 'u', 'Z', NULL, NULL },
	{ "Aim a wand", 'a', 'z', NULL, NULL },
	{ "Zap a rod", 'z', 'a', NULL, NULL },
	{ "Activate an object", 'A', 0, NULL, NULL },
	{ "Eat some food", 'E', 0, NULL, NULL },
	{ "Quaff a potion",  'q', 0, NULL, NULL },
	{ "Read a scroll", 'r', 0, NULL, NULL},//, NULL, player_can_read_msg },
	{ "Fuel your light source", 'F', 0, NULL, NULL},//, NULL, player_can_refuel_msg },
	{ "Throw an item", 'v', 0, NULL, NULL}//, textui_cmd_throw, NULL }
};

/* General actions */
static struct cmd_info cmd_action[] =
{
	{ "Search for traps/doors", 's', 0, NULL, NULL },
	{ "Disarm a trap or chest", 'D', 0, NULL, NULL },
	{ "Rest for a while", 'R', 0, NULL, NULL},//, textui_cmd_rest, NULL },
	{ "Use a power", 'U', 0, NULL, NULL },
	{ "Look around", 'l', 'x', NULL, NULL},//, do_cmd_look, NULL },
	{ "Target monster or location", '*', 0, NULL, NULL},//, do_cmd_target, NULL },
	{ "**Target closest monster", '\'', 0, NULL, NULL},//, do_cmd_target_closest, NULL },
	{ "Alter terrain", '+', 0, NULL, NULL },
	{ "Dig a tunnel", 'T', KTRL('T'), NULL, NULL },
	{ "Go up staircase", '<', 0, NULL, NULL },
	{ "Go down staircase", '>', 0, NULL, NULL },
	{ "Toggle search mode", 'S', '#', NULL, NULL },
	{ "Open a door or a chest", 'o', 0, NULL, NULL },
	{ "Close a door", 'c', 0, NULL, NULL },
	{ "Jam a door shut", 'j', 'S', NULL, NULL },
	/*{ "**Bash a door open", 'B', 'f', NULL, NULL },*/
	{ "**Repeat Last Spell/Ranged Attack", 'h', '\t', NULL, NULL},//, textui_cmd_fire_at_nearest, NULL },
	{ "**Fire at nearest target", 'h', '\t', NULL, NULL}//, textui_cmd_fire_at_nearest, NULL }
};

/* Item management commands */
static struct cmd_info cmd_item_manage[] =
{
	{ "Organize packs", 'O', 0, NULL, NULL },
	{ "Display equipment listing", 'e', 0, do_cmd_equip, NULL },
	{ "Display inventory listing", 'i', 0, do_cmd_inven, NULL },
	{ "Pick up objects", 'g', 0, NULL },
	{ "Destroy an item", 'k', KTRL('D'), NULL, NULL},//, textui_cmd_destroy, NULL },	
	{ "Ignore an item type", 'K', 'O', NULL, NULL},//, textui_cmd_toggle_ignore, NULL },
	{ "Ungnore an item type", KTRL('U'), 0, NULL, NULL}//, do_cmd_message_one, NULL }
};

/* Information access commands */
static struct cmd_info cmd_info[] =
{
	{ "Help", '?', 0, do_cmd_help, NULL },
	{ "Browse a book", 'b', 'P', NULL, NULL},//, textui_spell_browse, NULL },
	{ "Gain new spells", 'G', 0, NULL, NULL},//, textui_obj_study, player_can_study_msg },
	{ "Cast a spell", 'm', 0, NULL, NULL},//, textui_obj_cast, player_can_cast_msg },
	{ "Pet Menu", 'p', 0, NULL, NULL},//, textui_obj_cast, player_can_cast_msg },
	{ "Full dungeon map", 'M', 0, NULL, NULL},//, do_cmd_view_map, NULL },
	{ "Display visible monster list", '[', 0, NULL, NULL},//, do_cmd_monlist, NULL },
	{ "Display visible item list", ']', 0, NULL, NULL},//, do_cmd_itemlist, NULL },
	{ "Locate player on map", 'L', 'W', NULL, NULL},//, do_cmd_locate, NULL },
	{ "Identify symbol", '/', 0, NULL, NULL},//, do_cmd_query_symbol, NULL },
	{ "Character description", 'C', 0, do_cmd_character, NULL },
	{ "List quests", 'Q', 0, NULL, NULL},//, textui_browse_knowledge, NULL },
	{ "Check knowledge", '~', 0, NULL, NULL},//, textui_browse_knowledge, NULL },
	{ "Repeat level feeling", KTRL('F'), 0, NULL, NULL},//, do_cmd_feeling, NULL },
	{ "Show previous messages", KTRL('P'), 0, NULL, NULL},//, do_cmd_messages, NULL }
	{ "Time of Day", KTRL('T'), 0, NULL, NULL}
};

/* Utility/assorted commands */
static struct cmd_info cmd_util[] =
{
	{ "Center map", KTRL('L'), '@', NULL, NULL},//, do_cmd_center_map, NULL },
	{ "Toggle inv/equip", KTRL('E'), 0, toggle_inven_equip }, /* XXX */

	{ "Redraw the screen", KTRL('R'), 0, do_cmd_redraw, NULL },
	{ "Version info", 'V', 0, do_cmd_version, NULL },
	{ "Options menu", '=', 0, NULL, NULL},//, do_cmd_xxx_options, NULL },

	{ "Save and don't quit", KTRL('S'), 0, NULL, NULL },
	{ "Save and quit", KTRL('X'), 0, NULL, NULL },
	{ "Quit (commit suicide)", KTRL('Q'), 0, NULL, NULL},//, textui_cmd_suicide, NULL },

	{ "Load \"screen dump\"", '(', 0, do_cmd_load_screen, NULL },
	{ "Save \"screen dump\"", ')', 0, do_cmd_save_screen, NULL }
};

/* Commands that shouldn't be shown to the user */ 
static struct cmd_info cmd_hidden[] =
{
	{ "Take notes", ':', 0, do_cmd_note, NULL },
	{ "Load a single pref line", '"', 0, do_cmd_pref, NULL },
	{ "**Enter a store", '_', 0, NULL, NULL },
	{ "Walk", ';', 0, NULL, NULL },
	{ "**Walk into a trap", 'W', '-', NULL, NULL },
	{ "Start running", '.', ',', NULL, NULL },
	{ "Stand still", ',', '.', NULL, NULL },

	{ "Repeat previous command", 'n', KTRL('V'), NULL, NULL },
	{ "**Do autopickup", KTRL('G'),0, NULL, NULL },
	{ "Toggle wizard mode", KTRL('W'), 0, NULL, NULL},//, do_cmd_wizard, NULL },

/*#ifdef ALLOW_DEBUG*/
#ifdef ALLOW_WIZARD
	{ "Debug mode commands", KTRL('A'), 0, NULL, NULL},//, do_cmd_try_debug, NULL },
#endif
#ifdef ALLOW_BORG
	{ "Borg commands", KTRL('Z'), 0, NULL, NULL},//, do_cmd_try_borg, NULL }
#endif
};



/*
 * A categorised list of all the command lists.
 */
typedef struct
{
	const char *name;
	struct cmd_info *list;
	size_t len;
} command_list;

static command_list cmds_all[] =
{
	{ "Items",           cmd_item,        NUM_ELEMENTS(cmd_item) },
	{ "Action", cmd_action,      NUM_ELEMENTS(cmd_action) },
	{ "Item Management",    cmd_item_manage, NUM_ELEMENTS(cmd_item_manage) },
	{ "Information",     cmd_info,        NUM_ELEMENTS(cmd_info) },
	{ "Utility",         cmd_util,        NUM_ELEMENTS(cmd_util) },
	{ "Misc.",          cmd_hidden,      NUM_ELEMENTS(cmd_hidden) }
};




/*** Menu functions ***/

/* Display an entry on a command menu */
static void cmd_sub_entry(menu_type *menu, int oid, bool cursor, int row, int col, int width)
{
	byte attr;
	keycode_t key;
	const struct cmd_info *commands = menu_priv(menu);

	(void)width;
	if (commands[oid].prereq && !(*commands[oid].prereq)()) {
		attr = (cursor ? curs_attrs[0][1] : curs_attrs[0][0]);
	} else {
		attr = (cursor ? curs_attrs[1][1] : curs_attrs[1][0]);
	}

	/* Write the description */
	Term_putstr(col, row, -1, attr, commands[oid].desc);

	/* Include keypress */
	Term_addch(attr, ' ');
	Term_addch(attr, '(');

	/* Get readable version */
	if (OPT(rogue_like_commands) && commands[oid].key_roguelike) {
		key = commands[oid].key_roguelike;
	} else {
		key = commands[oid].key_original;
	}
	if (KTRL(key) == key) {
		Term_addch(attr, '^');
		Term_addch(attr, UN_KTRL(key));
	} else {
		Term_addch(attr, key);
	}

	Term_addch(attr, ')');
}

/*
 * Display a list of commands.
 */
static bool show_cmd_menu(command_list *list, int mx, int my)
{
	menu_type menu;
	menu_iter commands_menu = { NULL, NULL, cmd_sub_entry, NULL, NULL };
	rect_region area = { 23, 4, 37, 13 };

	ui_event evt;
	struct cmd_info *selection = NULL;

	(void)mx; (void)my;
	if (list->len > area.page_rows) {
		area.page_rows = list->len;
	}

	/* Set up the menu */
	menu_init(&menu, MN_SKIN_SCROLL, &commands_menu);
	menu_setpriv(&menu, list->len, list->list);
	menu_layout(&menu, &area);

	/* Set up the screen */
	screen_save();
	window_make(area.col-2, area.row-1, area.col+area.width+2, area.row+area.page_rows);

	/* Select an entry */
	evt = menu_select(&menu, 0, TRUE);

	/* Load de screen */
	screen_load();

	/*if (evt.type == EVT_SELECT)*/
	if (evt == EVT_SELECT) {
		selection = &(list->list[menu.cursor]);
	}

	if (selection) {
		if (selection->hook) {
			(*(selection->hook))();
		} else
		if (OPT(rogue_like_commands) && selection->key_roguelike) {
			Term_keypress(selection->key_roguelike);
		} else
		if (selection->key_original) {
			Term_keypress(selection->key_original);
		}
		return FALSE;
	}

	return TRUE;
}



static bool cmd_list_action(menu_type *m, const ui_event *event, int oid)
{
	/*if (event->type == EVT_SELECT)*/
	if (*event == EVT_SELECT) {
		command_list *list = menu_priv(m);
		return show_cmd_menu(&list[oid], 0, 0);
	} else
	{
		return FALSE;
	}
}

static void cmd_list_entry(menu_type *menu, int oid, bool cursor, int row, int col, int width)
{
	byte attr = (cursor ? TERM_L_BLUE : TERM_WHITE);
	command_list *list = menu_priv(menu);
	Term_putstr(col, row, -1, attr, list[oid].name);
	(void)width;
}

static menu_iter command_menu_iter =
{
	NULL,
	NULL,
	cmd_list_entry,
	cmd_list_action,
	NULL
};

/*
 * Display a list of command types, allowing the user to select one.
 */
/*errr command_menu(void)*/
errr context_menu_command(int mx, int my)
{
	rect_region area = { 21, 5, 37, 6 };
	int i;
	menu_type *command_menu;
	
	(void)mx; (void)my;

	command_menu = menu_new(MN_SKIN_SCROLL, &command_menu_iter);

	menu_setpriv(command_menu, NUM_ELEMENTS(cmds_all), cmds_all);
	menu_layout(command_menu, &area);

	/* Set up the screen */
	screen_save();
	window_make(19, 4, 58, 11);

	menu_select(command_menu, 0, TRUE);

	screen_load();

	FREE(command_menu);

	return 0;
}

