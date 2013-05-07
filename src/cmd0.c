/* File: cmd0.c */

/* Purpose: Angband game engine */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"
#include "script.h"

void do_cmd_messages_reverse(void);
int context_menu_player(int mx, int my);
int context_menu_cave(int cy, int cx, int adjacent, int mx, int my);


/*
 * Verify use of "wizard" mode
 */
static bool enter_wizard_mode(void)
{
	/* Ask first time */
#if 0
	if (!(p_ptr->state.noscore & 0x0002))
#else
	if (!p_ptr->state.noscore)
#endif
	{
		/* Mention effects */
		msgf("Wizard mode is for debugging and experimenting.");
		msgf("The game will not be scored if you enter wizard mode.");
		message_flush();

		/* Verify request */
		if (!get_check("Are you sure you want to enter wizard mode? "))
		{
			return (FALSE);
		}

		/* Mark savefile */
		p_ptr->state.noscore |= 0x0002;
	}

	/* Success */
	return (TRUE);
}


#ifdef ALLOW_WIZARD

/*
 * Verify use of "debug" commands
 */
static bool enter_debug_mode(void)
{
	/* Ask first time */
#if 0
	if (!(p_ptr->state.noscore & 0x0008))
#else
	if (!p_ptr->state.noscore)
#endif
	{
		/* Mention effects */
		msgf("The debug commands are for debugging and experimenting.");
		msgf("The game will not be scored if you use debug commands.");
		message_flush();

		/* Verify request */
		if (!get_check("Are you sure you want to use debug commands? "))
		{
			return (FALSE);
		}

		/* Mark savefile */
		p_ptr->state.noscore |= 0x0008;
	}

	/* Success */
	return (TRUE);
}

/*
 * Hack -- Declare the Debug Routines
 */
extern void do_cmd_debug(void);

#endif /* ALLOW_WIZARD */


#ifdef ALLOW_BORG

/*
 * Verify use of "borg" commands
 */
static bool enter_borg_mode(void)
{
	/* Ask first time */
	if (!(p_ptr->state.noscore & 0x0040))
	{
		/* Mention effects */
		msgf("The borg commands are for debugging and experimenting.");
		msgf("The game will not be scored if you use borg commands.");
		message_flush();

		/* Verify request */
		if (!get_check("Are you sure you want to use borg commands? "))
		{
			return (FALSE);
		}

		/* Mark savefile */
		p_ptr->state.noscore |= 0x0040;
	}

	/* Success */
	return (TRUE);
}

#endif /* ALLOW_BORG */


void do_cmd_cast_wrapper(void)
{
	/* Cast a spell  - from process_command*/
	if (FLAG(p_ptr, TR_NO_MAGIC)) {
		cptr which_power = "magic";
		if (p_ptr->rp.pclass == CLASS_MINDCRAFTER)
			which_power = "psionic powers";
		else if (mp_ptr->spell_book == TV_LIFE_BOOK)
			which_power = "prayer";

		msgf("An anti-magic shell disrupts your %s!", which_power);
		p_ptr->state.energy_use = 0;
	} else {
		if (p_ptr->rp.pclass == CLASS_MINDCRAFTER)
			do_cmd_mindcraft();
		else
			do_cmd_cast();
	}
}

void process_click(char press, int xpos, int ypos)
{
	char button = press & 0x7;
	bool shift = press & 16;
	bool ctrl = press & 32;
	bool alt = press & 64;
	bool meta = press & 8;
	int x,y;

	if (ypos == 0) {
		// special behavior for the top line
		if (button == 1) {
			// show previous messages
			do_cmd_messages_reverse();
		/*} else
		if (button == 2) {
			// show 
		*/
		}
		return;
	}

	y = ((ypos-ROW_MAP) / tile_height_mult) + p_ptr->panel_y1;
	x = ((xpos-COL_MAP) / tile_width_mult) + p_ptr->panel_x1;

	/* Check for a valid location */
	/*if (!in_bounds2(x, y)) return;*/
	if ((x == p_ptr->px) && (y == p_ptr->py)) {
		if (shift) {
			/* shift-click - cast magic / open inventory */
			if (button == 1) {
				do_cmd_cast_wrapper();
			} else
			if (button == 2) {
 				do_cmd_rest();
			}
		} else
		if (ctrl) {
			/* ctrl-click - use feature / use inventory item */
			if (button == 1) {
				do_cmd_use_terrain();
			} else
			if (button == 2) {
				do_cmd_use();
			}
		} else
		if (alt) {
			/* alt-click - Search  or show char screen */
			if (button == 1) {
 				do_cmd_search();
			} else
			if (button == 2) {
				Term_keypress('C');
			}
		} else
		{
			/* just-click - wait and try pickup, or show player context menu */
			if (button == 1) {
 				do_cmd_stay(TRUE);
			} else
			if (button == 2) {
				context_menu_player(xpos, ypos);
				//do_cmd_inven();
			} else
			/* temporary commands until context menus are implemented */
			if (button == 3) {
				do_cmd_use_terrain();
			} else
			if (button == 4) {
				do_cmd_use();
			} else
			if (button == 5) {
				do_cmd_cast_wrapper();
			}
		}
	} else
	if (button == 1) {
#if (0)
		if (query_timed(TIMED_CONFUSED)) {
			p_ptr->cmd.dir = randint0(8) + 1;
			do_cmd_walk(FALSE);
		} else
#endif
		{
			if (shift) {
				/* shift-click - run */
				p_ptr->cmd.dir = coords_to_dir(x, y);
				do_cmd_run();
			} else
			if (ctrl) {
				/* control-click - alter */
				p_ptr->cmd.dir = coords_to_dir(x, y);
				p_ptr->cmd.arg = 16;
				p_ptr->cmd.cmd = '+';
				do_cmd_alter();
			} else
			if (alt) {
				/* alt-click - look */
				do_cmd_locate();
			} else
			{
				if (mouse_path) {
					/* if the click was adjacent to the player, walk in that direction */
					if ((y-p_ptr->py >= -1) && (y-p_ptr->py <= 1)
							&& (x-p_ptr->px >= -1) && (x-p_ptr->px <= 1))
					{
						p_ptr->cmd.dir = coords_to_dir(x, y);
						do_cmd_walk(FALSE);
					} else {
						/* if not, pathfind to that spot */
						do_cmd_pathfind(x,y);
					}
				} else {
					/* move roughly in the clicked direction */
					p_ptr->cmd.dir = coords_to_dir_rough(x, y);
					do_cmd_walk(FALSE);
				}
			}
		}
	} else
	if (button == 2) {
		cave_type *c_ptr = area(x,y);
		pcave_type *pc_ptr = parea(x,y);
		/*bool doub = FALSE;*/
		int m_idx = c_ptr->m_idx;

		/*if ((p_ptr->target_row == y) && (p_ptr->target_col == x)) {
			doub = TRUE;
		}*/

		target_set_grid(x,y);

		if (shift) {
			/* shift-click - cast spell at target */
			p_ptr->cmd.dir = 5;
			do_cmd_cast_wrapper();
		} else
		if (ctrl) {
			/* control-click - fire at target */
			p_ptr->cmd.dir = 5;
			do_cmd_fire();
		} else
		if (alt) {
			/* alt-click - throw at target */
			/* maybe look with projectile path? */
			do_cmd_locate();
		} else
		{
			/* see if the click was adjacent to the player */
			if ((y-p_ptr->py >= -1) && (y-p_ptr->py <= 1)
				&& (x-p_ptr->px >= -1) && (x-p_ptr->px <= 1))
			{
				context_menu_cave(y,x, 1, xpos, ypos);
				/*if (doub) {
					if (m_idx) {
						target_look_grid(x, y, TRUE);
					} else {
						*//* if the click was adjacent to the player, alter in that direction */
						/*p_ptr->cmd.dir = coords_to_dir(x, y);
						p_ptr->cmd.arg = 16;
						p_ptr->cmd.cmd = '+';
						do_cmd_alter();
					}
				} else {
					target_look_grid(x, y, FALSE);
				}*/
			} else {
				context_menu_cave(y,x, 0, xpos, ypos);
				/*target_look_grid(x, y, doub);*/
			}
		}
	}

}

typedef char keycode_t;
extern bool use_main_menu; /* whether a port is using the textui menu bar */
extern int (*main_menu_bar_fn) (keycode_t); /* the button function for the textui menu bar */

/*
 * Parse and execute the current command
 * Give "Warning" on illegal commands.
 *
 * XXX XXX XXX Make some "blocks"
 */
void process_command(void)
{
	/* Handle repeating the last command */
	repeat_check();

	/* Parse the command */
	switch (p_ptr->cmd.cmd)
	{
		case ESCAPE:
		{
			/* Show the system menu, if it is being used */
			if (use_main_menu && main_menu_bar_fn) {
				(*main_menu_bar_fn)(ESCAPE);
			}
			break;
		}
		case ' ':
		{
			/* Ignore */
			break;
		}

		case '\r':
		{
			/* Ignore return */
			break;
		}

		/*** Wizard Commands ***/

		case KTRL('W'):
		{
			/* Toggle Wizard Mode */
			if (p_ptr->state.wizard)
			{
				p_ptr->state.wizard = FALSE;
				msgf("Wizard mode off.");
			}
			else if (enter_wizard_mode())
			{
				p_ptr->state.wizard = TRUE;
				msgf("Wizard mode on.");
			}

			/* Update monsters */
			p_ptr->update |= (PU_MONSTERS);

			/* Redraw "title" */
			p_ptr->redraw |= (PR_TITLE);

			break;
		}


#ifdef ALLOW_WIZARD

		case KTRL('A'):
		{
			/* Enter debug mode */
			if (enter_debug_mode())
			{
				do_cmd_debug();
			}
			break;
		}

#endif /* ALLOW_WIZARD */


#ifdef ALLOW_BORG

		case KTRL('Z'):
		{
			/* Enter borg mode */
			if (enter_borg_mode())
			{
				do_cmd_borg();
			}

			break;
		}

#endif /* ALLOW_BORG */



		/*** Inventory Commands ***/

		case 'w':
		{
			/* Wear/wield equipment */
			do_cmd_wield();
			break;
		}

		case 't':
		{
			/* Take off equipment */
			do_cmd_takeoff();
			break;
		}

		case 'd':
		{
			/* Drop an item */
			do_cmd_drop();
			break;
		}

		case 'k':
		{
			/* Destroy an item */
			do_cmd_destroy();
			break;
		}

		case 'O':
		{
			/* Organize */
			do_cmd_organize();
			break;
		}

		case 'K':
		{
			do_cmd_squelch();
			break;
		}

		case KTRL('U'):
		{
			do_cmd_unsquelch();
			break;
		}

		case 'e':
		{
			/* Equipment list */
			do_cmd_equip();
			break;
		}

		case 'i':
		{
			/* Inventory list */
			do_cmd_inven();
			break;
		}


		/*** Various commands ***/

		case 'I':
		{
			/* Identify an object */
			do_cmd_observe();
			break;
		}

		case KTRL('I'):
		case KTRL('E'):
		{
			/* Hack -- toggle windows */
			toggle_inven_equip();
			break;
		}


		/*** Standard "Movement" Commands ***/

		case '+':
		{
			/* Alter a grid */
			do_cmd_alter();
			break;
		}

		case 'T':
		{
			/* Dig a tunnel */
			do_cmd_tunnel();
			break;
		}

		case ';':
		{
			/* Move (usually pick up things) */
			do_cmd_walk(FALSE);
			break;
		}

		case '-':
		{
			/* Move (usually do not pick up) */
			do_cmd_walk(TRUE);
			break;
		}


		/*** Running, Resting, Searching, Staying */

		case '.':
		{
			/* Begin Running -- Arg is Max Distance */
			do_cmd_run();
			break;
		}

		case ',':
		{
			/* Stay still (usually pick things up) */
			do_cmd_stay(always_pickup);
			break;
		}

		case 'g':
		{
			/* Stay still (usually do not pick up) */
			do_cmd_stay(!always_pickup);
			break;
		}

		case 'R':
		{
			/* Rest -- Arg is time */
			do_cmd_rest();
			break;
		}

		case 's':
		{
			/* Search for traps/doors */
			do_cmd_search();
			break;
		}

		case 'S':
		{
			/* Toggle search mode */
			do_cmd_toggle_search();
			break;
		}


		/*** Stairs and Doors and Chests and Traps ***/

		case '<':
		{
			/* Go up staircase */
			do_cmd_go_up();
			break;
		}

		case '>':
		{
			/* Go down staircase */
			do_cmd_go_down();
			break;
		}

		case 'o':
		{
			/* Open a door or chest */
			do_cmd_open();
			break;
		}

		case 'c':
		{
			/* Close a door */
			do_cmd_close();
			break;
		}

		case 'j':
		{
			/* Jam a door with spikes */
			do_cmd_spike();
			break;
		}

		case 'D':
		{
			/* Disarm a trap or chest */
			do_cmd_disarm();
			break;
		}


		/*** Magic and Prayers ***/

		case 'G':
		{
			/* Gain new spells/prayers */
			do_cmd_study(FALSE, NULL);
			break;
		}

		case 'b':
		{
			/* Browse a book */
			do_cmd_browse();
			break;
		}

		case 'm':
		{
			/* Cast a spell */
			do_cmd_cast_wrapper();
			break;
		}

		case 'p':
		{
			/* Issue a pet command */
			do_cmd_pet();
			break;
		}

		/*** Use various objects ***/

		case '{':
		{
			/* Inscribe an object */
			do_cmd_inscribe();
			break;
		}

		case '}':
		{
			/* Uninscribe an object */
			do_cmd_uninscribe();
			break;
		}

		case 'A':
		{
			/* Activate an artifact */
			do_cmd_activate();
			break;
		}

		case 'E':
		{
			/* Eat some food */
			do_cmd_eat_food();
			break;
		}

		case 'F':
		{
			/* Fuel your lantern/torch */
			do_cmd_refill();
			break;
		}

		case 'f':
		{
			/* Fire an item */
			do_cmd_fire();
			break;
		}

		case 'v':
		{
			/* Throw an item */
			do_cmd_throw();
			break;
		}

		case 'a':
		{
			/* Aim a wand */
			do_cmd_aim_wand();
			break;
		}

		case 'z':
		{
			/* Zap a rod */
			do_cmd_zap_rod();
			break;
		}

		case 'q':
		{
			/* Quaff a potion */
			do_cmd_quaff_potion();
			break;
		}

		case 'r':
		{
			/* Read a scroll */
			do_cmd_read_scroll();
			break;
		}

		case 'u':
		{
			/* Use an item (including staves) */
			do_cmd_use();
			break;
		}

		case 'U':
		{
			/* Use racial power */
			do_cmd_racial_power();
			break;
		}


		/*** Looking at Things (nearby or on map) ***/

		case 'M':
		{
			/* Full dungeon map */
			do_cmd_view_map();
			break;
		}

		case 'L':
		{
			/* Locate player on map */
			do_cmd_locate();
			break;
		}

		case 'l':
		{
			/* Look around */
			do_cmd_look();
			break;
		}

		case '*':
		{
			/* Target monster or location */
			do_cmd_target();
			break;
		}



		/*** Help and Such ***/

		case '?':
		{
			/* Help */
			do_cmd_help();
			break;
		}

		case '/':
		{
			/* Identify symbol */
			do_cmd_query_symbol();
			break;
		}

		case 'C':
		{
			/* Character description */
			do_cmd_character();
			break;
		}


		/*** System Commands ***/

		case '!':
		{
			/* Hack -- User interface */
			(void)Term_user(0);
			break;
		}

		case '"':
		{
			/* Single line from a pref file */
			do_cmd_pref();
			break;
		}

		case '@':
		{
			/* Interact with macros */
			do_cmd_macros();
			break;
		}

		case '$':
		{
			/* Access lists */
			do_cmd_list();
			break;
		}
		
		case '%':
		{
			/* Interact with visuals */
			do_cmd_visuals();
			break;
		}

		case '&':
		{
			/* Interact with colors */
			do_cmd_colors();
			break;
		}

		case '=':
		{
			/* Interact with options */
			do_cmd_options(OPT_FLAG_SERVER | OPT_FLAG_PLAYER);
			do_cmd_redraw();
			break;
		}


		/*** Misc Commands ***/

		case ':':
		{
			/* Take notes */
			do_cmd_note();
			break;
		}

		case 'V':
		{
			/* Version info */
			do_cmd_version();
			break;
		}

		case KTRL('F'):
		{
			/* Repeat level feeling */
			do_cmd_feeling();
			break;
		}

		case KTRL('P'):
		{
			/* Show previous messages */
			do_cmd_messages();
			break;
		}

		case 'Q':
		{
			/* Show quest status -KMW- */
			do_cmd_checkquest();
			break;
		}

		case KTRL('R'):
		{
			/* Redraw the screen */
			do_cmd_redraw();
			break;
		}

		case KTRL('S'):
		{
			/* Hack -- Save and don't quit */
			do_cmd_save_game(FALSE);
			break;
		}

		case KTRL('T'):
		{
			/* Get the time of day */
			do_cmd_time();
			break;
		}

		case KTRL('X'):
		{
			/* Save and quit */
			do_cmd_save_and_exit();
			break;
		}

		case KTRL('Q'):
		{
			/* Quit (commit suicide) */
			do_cmd_suicide();
			break;
		}

		case '~':
		case '|':
		{
			/* Check artifacts, uniques, objects, quests etc. */
			do_cmd_knowledge();
			break;
		}

		case '(':
		{
			/* Load "screen dump" */
			do_cmd_load_screen();
			break;
		}

		case ')':
		{
			/* Save "screen dump" */
			do_cmd_save_screen();
			break;
		}

		default:
		{
			/* Hack -- Unknown command */
			if (one_in_(20))
			{
				char error_m[1024];
				sound(SOUND_ILLEGAL);
				if (!get_rnd_line("error.txt", 0, error_m))
					msgf(error_m);
			}
			else
				prtf(0, 0, "Type '?' for help.");
			break;
		}
	}
}

