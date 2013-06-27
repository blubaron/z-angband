/*
 * File: cmd-context.c
 * Purpose: Show player and terrain context menus.
 *
 * Copyright (c) 2011 Brett Reid
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

void do_cmd_cast_wrapper(void);
bool do_cmd_center_map(void);
object_type *chest_check(int x, int y);

int context_menu_command(int mx, int my);
int context_menu_object(const object_type *o_ptr);
void textui_cmd_destroy_menu(int item);
void roff_obj_aux(const object_type *o_ptr);
void set_get_item_object(const object_type *obj);
bool item_tester_hook_useable(const object_type *o_ptr);
bool item_tester_hook_activate(const object_type *o_ptr);
bool item_tester_refill_lantern(const object_type *o_ptr);
bool item_tester_refill_torch(const object_type *o_ptr);

bool player_is_caster(void)
{
	if (p_ptr->spell.realm[0]) {
		return (TRUE);
	}
	if (p_ptr->rp.pclass == CLASS_MINDCRAFTER) {
		return (TRUE);
	}

  return (FALSE);
}

bool player_can_cast(void)
{
	if (p_ptr->rp.pclass == CLASS_MINDCRAFTER) {
		if (query_timed(TIMED_CONFUSED) || p_ptr->csp < 1) {
			return (FALSE);
		}
	} else
	if (p_ptr->spell.realm[0]) {
		if (query_timed(TIMED_BLIND) || no_lite()
			|| query_timed(TIMED_CONFUSED) || p_ptr->csp < 1)
		{
			return (FALSE);
		}
	}
	return (TRUE);
}
bool player_can_cast_from(const object_type *o_ptr)
{
	if (p_ptr->spell.realm[0] == o_ptr->tval-TV_BOOKS_MIN +1) {
    return (TRUE);
  }
	if (p_ptr->spell.realm[1] == o_ptr->tval-TV_BOOKS_MIN +1) {
    return (TRUE);
  }
  return (FALSE);
}
bool player_can_read(void)
{
	if (query_timed(TIMED_BLIND) || no_lite()
		|| query_timed(TIMED_CONFUSED))
	{
		return (FALSE);
	}

  return (TRUE);
}
bool player_can_study(void)
{
	if (p_ptr->spell.realm[0]) {
		if (query_timed(TIMED_BLIND) || no_lite()
			|| query_timed(TIMED_CONFUSED) || p_ptr->csp < 1)
		{
			return (FALSE);
		}
	}
	if (p_ptr->rp.pclass == CLASS_MINDCRAFTER) {
		return (FALSE);
	}

  return (TRUE);
}

bool player_can_fire(void)
{
	object_type *j_ptr;
	/* Get the "bow" (if any) */
	j_ptr = &p_ptr->equipment[EQUIP_BOW];

	/* Require a launcher */
	if (j_ptr->tval) {
		/* see if we have ammo for the launcher */
		if (player_has(p_ptr->ammo_tval, 0)) {
			return(TRUE);
		}
	}

	return (FALSE);
}

bool player_has_power(void)
{
	int i;
	const mutation_type *mut_ptr;

	/* Look for racial powers */
	for (i = 0; i < MAX_RACE_POWERS; i++) {
		mut_ptr = &race_powers[i];
		if (mut_ptr->which == p_ptr->rp.prace) {
			if (mut_ptr->level <= p_ptr->lev) {
				return (TRUE);
			}
		}
	}
	/* Look for appropriate mutations */
	for (i = 0; i < MUT_PER_SET; i++) {
		mut_ptr = &mutations[i];

		if (p_ptr->muta1 & mut_ptr->which) {
			if (mut_ptr->level <= p_ptr->lev) {
				return (TRUE);
			}
		}
	}
	return (FALSE);
}
bool player_can_use_power(void)
{
	int i;
	const mutation_type *mut_ptr;
	
	/* Not when we're confused */
	if (query_timed(TIMED_CONFUSED)) {
		return (FALSE);
	}

	/* Look for racial powers */
	for (i = 0; i < MAX_RACE_POWERS; i++) {
		mut_ptr = &race_powers[i];
		if (mut_ptr->which == p_ptr->rp.prace) {
			if (mut_ptr->level <= p_ptr->lev) {
				if (mut_ptr->cost <= p_ptr->csp) {
					return (TRUE);
				}
			}
		}
	}
	/* Look for appropriate mutations */
	for (i = 0; i < MUT_PER_SET; i++) {
		mut_ptr = &mutations[i];

		if (p_ptr->muta1 & mut_ptr->which) {
			if (mut_ptr->level <= p_ptr->lev) {
				if (mut_ptr->cost <= p_ptr->csp) {
					return (TRUE);
				}
			}
		}
	}

	return (FALSE);
}

bool player_has_pets(void)
{
	int i;
	monster_type *m_ptr;

	/* Process the monsters (backwards) */
	for (i = m_max - 1; i >= 1; i--)
	{
		/* Access the monster */
		m_ptr = &m_list[i];

		/* Ignore "dead" monsters */
		if (!m_ptr->r_idx) continue;

		/* Check for pets */
		if (is_pet(m_ptr)) {
			return (TRUE);
		}
	}
	/*if (p_ptr->pet_count) {
		return (TRUE)
	}*/

	return (FALSE);
}


bool obj_is_food(const object_type *o_ptr)
{
	return (o_ptr->tval == TV_FOOD);
}
bool obj_is_potion(const object_type *o_ptr)
{
	return (o_ptr->tval == TV_POTION);
}
bool obj_is_wand(const object_type *o_ptr)
{
	return (o_ptr->tval == TV_WAND);
}
bool obj_is_scroll(const object_type *o_ptr)
{
	return (o_ptr->tval == TV_SCROLL);
}
bool obj_is_staff(const object_type *o_ptr)
{
	return (o_ptr->tval == TV_STAFF);
}
bool obj_is_rod(const object_type *o_ptr)
{
	return (o_ptr->tval == TV_ROD);
}
bool obj_has_charges(const object_type *o_ptr)
{
	return (o_ptr->pval > 0);
}
bool obj_is_activatable(const object_type *o_ptr)
{
	return (!o_ptr->timeout);
}
bool obj_can_zap(const object_type *o_ptr)
{
	/* A single rod is still charging */
	if ((o_ptr->number == 1) && (o_ptr->timeout)) {
		return FALSE;
	} else
	/* A stack of rods lacks enough energy. */
	if ((o_ptr->number > 1)
		&& (o_ptr->timeout > (o_ptr->number - 1) * k_info[o_ptr->k_idx].pval))
	{
		return FALSE;
	}
	return TRUE;
}
bool obj_can_fire(const object_type *o_ptr)
{
	object_type *j_ptr;
	/* Get the "bow" (if any) */
	j_ptr = &p_ptr->equipment[EQUIP_BOW];

	/* Require a launcher */
	if (j_ptr->tval) {
		/* see if we have ammo for the launcher */
		return (o_ptr->tval == p_ptr->ammo_tval);
	}

	return (FALSE);
}
bool obj_can_refill(const object_type *o_ptr)
{
	object_type *q_ptr;

	/* Get the light */
	q_ptr = &p_ptr->equipment[EQUIP_LITE];

	/* It is nothing */
	if (q_ptr->tval != TV_LITE) {
		return (FALSE);
  } else

	/* It's a lamp */
	if (q_ptr->sval == SV_LITE_LANTERN) {
		return item_tester_refill_lantern(o_ptr);
  } else

	/* It's a torch */
	if (q_ptr->sval == SV_LITE_TORCH) {
		return item_tester_refill_torch(o_ptr);
  }
  return (FALSE);
}

int context_menu_player_2(int mx, int my)
{
	menu_type *m;
	rect_region r;
	int selected;
	char *labels;

	m = menu_dynamic_new();
	if (!m) {
		return 0;
	}

	labels = (char*)string_make(lower_case);
	m->selections = labels;

	menu_dynamic_add_label(m, "Knowledge", '~', 1, labels);
	menu_dynamic_add_label(m, "Show Map", 'M', 2, labels);
	menu_dynamic_add_label(m, "^Show Messages", 'P', 3, labels);
	menu_dynamic_add_label(m, "Show Monster List", '[', 9, labels);
	menu_dynamic_add_label(m, "Show Object List", ']', 10, labels);
	menu_dynamic_add_label(m, "Toggle Searching", 'S', 4, labels);
	if (player_has_pets()) {
		menu_dynamic_add_label(m, "Pet Commands", 'p', 5, labels);
	}
	menu_dynamic_add_label(m, "Destroy an item", 'k', 6, labels);
	menu_dynamic_add_label(m, "Options", '=', 8, labels);
	menu_dynamic_add_label(m, "Commands", '?', 7, labels);
	menu_dynamic_add_label(m, "Back", ' ', 11, labels);

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

	if (selected == 1) {
		/* show knowledge screen */
		Term_keypress('~');//,0);
	} else
	if (selected == 2) {
		/* Toggle show map */
		do_cmd_view_map();
	} else
	if (selected == 3) {
		/* Toggle show messages */
		Term_keypress(KTRL('p'));//,0);/*XXX should be ('p', KC_MOD_CONTROL);*/
	} else
	if (selected == 4) {
		/* Toggle search mode */
		Term_keypress('S');//,0);
	} else
	if (selected == 5) {
		/* show pet menu */
		do_cmd_pet();
	} else
	if (selected == 6) {
		/* destroy/squelch an item */
		Term_keypress('k');//,0);
	} else
	if (selected == 7) {
		/* show the commands */
		context_menu_command(mx,my);
	} else
	if (selected == 8) {
		/* show options screen */
		Term_keypress('=');//,0);
	} else
	if (selected == 9) {
		/* show the monster list */
		p_ptr->cmd.dir = 5;
		do_cmd_list_monster(0);
		//Term_keypress('[');//,0);
	} else
	if (selected == 10) {
		/* show the object list */
		p_ptr->cmd.dir = 5;
		do_cmd_list_object(0);
		//Term_keypress(']');//,0);
	} else
	if (selected == 11) {
		/* show the previous menu again */
		return 2;
	}

	return 1;
}

int context_menu_player(int mx, int my)
{
	menu_type *m;
	rect_region r;
	int selected;
	char *labels;

	cave_type *c_ptr = area(p_ptr->px,p_ptr->py);
	pcave_type *pc_ptr = parea(p_ptr->px,p_ptr->py);
	feature_type *feat;

	m = menu_dynamic_new();
	if (!m) {
		return 0;
	}

	labels = (char*)string_make(lower_case);
	m->selections = labels;

	feat  = &(f_info[c_ptr->feat]);

	menu_dynamic_add_label(m, "Use Item", 'u', 1, labels);
	/* if player can cast, add casting option */
	if (player_is_caster()) {
		if (player_can_cast()) {
			menu_dynamic_add_label(m, "Cast", 'm', 2, labels);
		} else {
			menu_dynamic_add_label(m, "$Cast", 'm', 2, labels);
		}
	}
	/* if player can use racial powers or mutations, add option */
	if (player_has_power()) {
		if (player_can_use_power()) {
			menu_dynamic_add_label(m, "Use Power", 'U', 16, labels);
		} else {
			menu_dynamic_add_label(m, "$Use Power", 'U', 16, labels);
		}
	}
	/* if player is on stairs add option to use them */
	if (feat->flags & FF_EXIT_UP) {
		menu_dynamic_add_label(m, "Go Up", '<', 11, labels);
	}
	if (feat->flags & FF_EXIT_DOWN) {
		menu_dynamic_add_label(m, "Go Down", '>', 12, labels);
	}
	menu_dynamic_add_label(m, "Search", 's', 3, labels);
	menu_dynamic_add_label(m, "Look", 'l', 6, labels);
	menu_dynamic_add_label(m, "Rest", 'R', 4, labels);
	menu_dynamic_add_label(m, "Inventory", 'i', 5, labels);
	/* if object under player add pickup option */
	if (c_ptr->o_idx) {
		object_type *o_ptr = &(o_list[c_ptr->o_idx]);
		//if (!squelch_item_ok(o_ptr)) {
  			menu_dynamic_add_label(m, "Floor", 'i', 13, labels);
			if (inven_carry_okay(o_ptr)) {
  				menu_dynamic_add_label(m, "Pickup", 'g', 14, labels);
			} else {
  				menu_dynamic_add_label(m, "$Pickup", 'g', 14, labels);
			}
		//}
	}
	menu_dynamic_add_label(m, "Character", 'C', 7, labels);
	/* XXX Don't show the keymap line until the keymap list is implemented, to
	 * avoid confusion as to what should be there */
	/*menu_dynamic_add(m, "Keymaps", 10);*/
	if (!OPT(center_player)) {
		menu_dynamic_add_label(m, "^Center Map", 'L', 15, labels);
	}
	menu_dynamic_add_label(m, "Other", ' ', 9, labels);

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
			/* use an item */
			do_cmd_use();
		} break;
	case 2:
		{
			/* Cast a spell */
			do_cmd_cast_wrapper();
		} break;
	case 3:
		{
			/* search */
			do_cmd_search();
		} break;
	case 4:
		{
			/* rest */
			do_cmd_rest();
		} break;
	case 5:
		{
			/* show inventory screen */
			Term_keypress('i');//,0);
		} break;
	case 6:
		{
			/* look mode */
			if (target_set(TARGET_LOOK)) {
			//if (target_set_interactive(TARGET_LOOK, p_ptr->px, p_ptr->py)) {
				msgf("Target Selected.");
			}
		} break;
	case 7:
		{
			/* show character screen */
			do_cmd_character();
		} break;
	case 9:
		{
			/* show another layer of menu options screen */
			int res;
			while ((res = context_menu_player_2(mx,my)) == 3);
			if (res == 2) return 3;
		} break;
	case 10:
		{
			/* show the commands */
			int res;
			while ((res = context_menu_command(mx,my)) == 3);
			if (res == 2) return 3;
		} break;
	case 11:
		{
			/* go up stairs */
			do_cmd_go_up();
		} break;
	case 12:
		{
			/* go down stairs */
			do_cmd_go_down();
		} break;
	case 13:
		{
			/* there is an item on the floor, show the inventory screen starting
			 * from the floor */
			do_cmd_inven_floor();
		} break;
	case 14:
		{
			/* pick the item up */
			//cmd_insert(CMD_PICKUP);
			//cmd_set_arg_item(cmd_get_top(), 0, -1);
			carry(TRUE);
		} break;
	case 15:
		{
			/* center the map on the player */
			/*panel_center(p_ptr->px, p_ptr->py);*/
			do_cmd_center_map();
		} break;
	case 16:
		{
			/* use character powers */
			do_cmd_racial_power();
		} break;

	}

	return 1;
}

int context_menu_cave(int cy, int cx, int adjacent, int mx, int my)
{
	menu_type *m;
	rect_region r;
	int selected;
	char *labels;

	cave_type *c_ptr = area(cx,cy);
	pcave_type *pc_ptr = parea(cx,cy);
	feature_type *feat;
	object_type *o_ptr;

	/* paranoia */
	if (!in_boundsp(cx,cy)) return 0;

	m = menu_dynamic_new();
	if (!m) {
		return 0;
	}

	labels = (char*)string_make(lower_case);
	m->selections = labels;

	feat  = &(f_info[c_ptr->feat]);

	menu_dynamic_add_label(m, "Look At", 'l', 1, labels);
	if (c_ptr->m_idx) {
		menu_dynamic_add_label(m, "Recall Info", '/', 18, labels);
	}
	menu_dynamic_add_label(m, "Use Item On", 'u', 2, labels);
	if (player_can_cast()) {
		menu_dynamic_add_label(m, "Cast On", 'm', 3, labels);
	}
	if (adjacent) {
		if (c_ptr->m_idx) {
			menu_dynamic_add_label(m, "Attack", '+', 4, labels);
		} else {
			menu_dynamic_add_label(m, "Alter", '+', 4, labels);
		}
		if (c_ptr->o_idx) {
			o_ptr = chest_check(cx,cy);
			if (o_ptr && o_ptr->pval) {
				//if (!squelch_item_ok(o_ptr)) {
					if (object_known_p(o_ptr)) {
						if (chest_traps[o_ptr->pval]) {
							menu_dynamic_add_label(m, "Disarm Chest", 'D', 5, labels);
							menu_dynamic_add_label(m, "Open Chest", 'o', 8, labels);
						} else {
							menu_dynamic_add_label(m, "Open Disarmed Chest", 'o', 8, labels);
						}
					} else {
						menu_dynamic_add_label(m, "Open Chest", 'o', 8, labels);
					}
				//}
			}
		}
		if (is_visible_trap(c_ptr)) {
			menu_dynamic_add_label(m, "Disarm", 'D', 5, labels);
			menu_dynamic_add_label(m, "Jump Onto", 'W', 6, labels);
		}
		if (pc_ptr->feat) {
			if ((feat->flags & FF_CLOSEABLE)
				|| ((feat->flags & FF_BROKEN) && (feat->flags & FF_DOOR)))
			{
				menu_dynamic_add_label(m, "Close", 'c', 7, labels);
			}
			if (feat->flags & FF_CLOSED) {
				menu_dynamic_add_label(m, "Open", 'o', 8, labels);
				menu_dynamic_add_label(m, "Bash Open", 'B', 9, labels);
				menu_dynamic_add_label(m, "Lock", 'D', 5, labels);
				menu_dynamic_add_label(m, "Jam", 'j', 10, labels);
			}
			if (feat->flags & FF_DIG) {
				menu_dynamic_add_label(m, "Tunnel", 'T', 11, labels);
			}
		}
		menu_dynamic_add_label(m, "Search", 's', 12, labels);
		menu_dynamic_add_label(m, "Walk Towards", ';', 14, labels);
	} else {
		menu_dynamic_add_label(m, "Pathfind To", ',', 13, labels);
		menu_dynamic_add_label(m, "Walk Towards", ';', 14, labels);
		menu_dynamic_add_label(m, "Run Towards", '.', 15, labels);
	}
	if (player_can_fire()) {
		menu_dynamic_add_label(m, "Fire On", 'f', 16, labels);
	}
	menu_dynamic_add_label(m, "Throw To", 'v', 17, labels);

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

	/* if there is a monster, draw a target path, which will be erased by the
	 * screen load below */
	if (c_ptr->m_idx && m_list[c_ptr->m_idx].ml) {
		sint path_n;
		coord path_g[2*MAX_RANGE+1];

		/* Find the path. */
		path_n = project_path(path_g, p_ptr->px, p_ptr->py, cx, cy, PROJECT_THRU);
		/* Draw the path. */
		draw_path(path_n, path_g, NULL, NULL, p_ptr->px, p_ptr->py);
	}

	menu_layout(m, &r);
	rect_region_erase_bordered(&r);

	/* display the prompt for the context menu */
	target_look_grid_prompt(0, 0, cx, cy,
		format("($UEnter to select command$Y\n$V, $UESC$ to cancel$Y%c$V) You see", ESCAPE));

	/* Hack - redraw stuff to show the target health bar */
	health_redraw();

	/* show the menu and pick from it */
	selected = menu_dynamic_select(m);

	menu_dynamic_free(m);
	string_free(labels);

	button_restore();
	screen_load();

	if (selected == 1) {
		/* look at the spot */
		if (target_set_interactive(TARGET_LOOK, cx, cy)) {
			msgf("Target Selected.");
		}
	} else
	if (selected == 2) {
		/* use an item on the spot */
		p_ptr->cmd.dir = 5;
		p_ptr->cmd.cmd = 'u';
		do_cmd_use();
		/*cmd_insert(CMD_USE_AIMED);
		cmd_set_arg_target(cmd_get_top(), 1, DIR_TARGET);*/
	} else
	if (selected == 3) {
		/* cast a spell on the spot */
		p_ptr->cmd.dir = 5;
		p_ptr->cmd.cmd = 'm';
		do_cmd_cast_wrapper();
		/*if (textui_obj_cast_ret() >= 0) {
			cmd_set_arg_target(cmd_get_top(), 1, DIR_TARGET);
		}*/
	} else
	if (selected == 4) {
		/* attack a spot adjacent to the player */
		p_ptr->cmd.dir = coords_to_dir(cx, cy);
		p_ptr->cmd.arg = 16;
		p_ptr->cmd.cmd = '+';
		do_cmd_alter();
		/*cmd_insert(CMD_ALTER);
		cmd_set_arg_direction(cmd_get_top(), 0, coords_to_dir(cy,cx));*/
	} else
	if (selected == 5) {
		/* disarm an adjacent trap or chest */
		p_ptr->cmd.dir = coords_to_dir(cx, cy);
		p_ptr->cmd.arg = 1;
		p_ptr->cmd.cmd = 'D';
		do_cmd_disarm();
		/*cmd_insert(CMD_DISARM);
		cmd_set_arg_direction(cmd_get_top(), 0, coords_to_dir(cy,cx));*/
	} else
	if (selected == 6) {
		/* walk onto an adjacent spot even if there is a trap there */
		bool orig_disarm = easy_disarm;
		easy_disarm = always_pickup;
		p_ptr->cmd.dir = coords_to_dir(cx, cy);
		p_ptr->cmd.arg = 1;
		p_ptr->cmd.cmd = 'W';
		do_cmd_walk(always_pickup);
		easy_disarm = orig_disarm;
		/*cmd_insert(CMD_JUMP);
		cmd_set_arg_direction(cmd_get_top(), 0, coords_to_dir(cy,cx));*/
	} else
	if (selected == 7) {
		/* close a door */
		p_ptr->cmd.dir = coords_to_dir(cx, cy);
		/*p_ptr->cmd.arg = 1;*/
		p_ptr->cmd.cmd = 'c';
		do_cmd_close();
		/*cmd_insert(CMD_CLOSE);
		cmd_set_arg_direction(cmd_get_top(), 0, coords_to_dir(cy,cx));*/
	} else
	if (selected == 8) {
		/* open a door or chest */
		p_ptr->cmd.dir = coords_to_dir(cx, cy);
		/*p_ptr->cmd.arg = 1;*/
		p_ptr->cmd.cmd = 'o';
		do_cmd_open();
		/*cmd_insert(CMD_OPEN);
		cmd_set_arg_direction(cmd_get_top(), 0, coords_to_dir(cy,cx));*/
	} else
	if (selected == 9) {
		/* bash a door */
		p_ptr->cmd.dir = coords_to_dir(cx, cy);
		/*p_ptr->cmd.arg = 1;*/
		p_ptr->cmd.cmd = 'o';
		do_cmd_open();
		/*p_ptr->cmd.cmd = 'B';
		do_cmd_bash();*/
		/*cmd_insert(CMD_BASH);
		cmd_set_arg_direction(cmd_get_top(), 0, coords_to_dir(cy,cx));*/
	} else
	if (selected == 10) {
		/* jam a door */
		p_ptr->cmd.dir = coords_to_dir(cx, cy);
		/*p_ptr->cmd.arg = 1;*/
		p_ptr->cmd.cmd = 'j';
		do_cmd_spike();
		/*cmd_insert(CMD_JAM);
		cmd_set_arg_direction(cmd_get_top(), 0, coords_to_dir(cy,cx));*/
	} else
	if (selected == 11) {
		/* Tunnel in a direction */
		p_ptr->cmd.dir = coords_to_dir(cx, cy);
		p_ptr->cmd.arg = 16;
		p_ptr->cmd.cmd = 'T';
		do_cmd_tunnel();
		/*cmd_insert(CMD_TUNNEL);
		cmd_set_arg_direction(cmd_get_top(), 0, coords_to_dir(cy,cx));*/
	} else
	if (selected == 12) {
		/* Search */
		/*p_ptr->cmd.arg = 1;*/
		p_ptr->cmd.cmd = 's';
		do_cmd_search();
		/*cmd_insert(CMD_SEARCH);*/
	} else
	if (selected == 13) {
		/* pathfind to the spot */
		/*p_ptr->cmd.arg = 16;*/
		p_ptr->cmd.cmd = ',';
		do_cmd_pathfind(cx,cy);
		/*cmd_insert(CMD_PATHFIND);
		cmd_set_arg_point(cmd_get_top(), 0, cx, cy);*/
	} else
	if (selected == 14) {
		/* walk towards the spot */
		p_ptr->cmd.dir = coords_to_dir(cx, cy);
		p_ptr->cmd.cmd = ';';
		do_cmd_walk(always_pickup);
		/*cmd_insert(CMD_WALK);
		cmd_set_arg_direction(cmd_get_top(), 0, coords_to_dir(cy,cx));*/
	} else
	if (selected == 15) {
		/* run towards the spot */
		p_ptr->cmd.dir = coords_to_dir(cx, cy);
		p_ptr->cmd.cmd = '.';
		do_cmd_run();
		/*cmd_insert(CMD_RUN);
		cmd_set_arg_direction(cmd_get_top(), 0, coords_to_dir(cy,cx));*/
	} else
	if (selected == 16) {
		/* Fire ammo towards the spot */
		p_ptr->cmd.dir = 5;
		p_ptr->cmd.cmd = 'f';
		do_cmd_fire();
		/*cmd_insert(CMD_FIRE);
		cmd_set_arg_target(cmd_get_top(), 1, DIR_TARGET);*/
	} else
	if (selected == 17) {
		/* throw an item towards the spot */
		p_ptr->cmd.dir = 5;
		p_ptr->cmd.cmd = 'v';
		do_cmd_throw();
		/*cmd_insert(CMD_THROW);
 		cmd_set_arg_target(cmd_get_top(), 1, DIR_TARGET);*/
	} else
	if (selected == 18) {
		/* recall monster Info */
		monster_type *m_ptr = &m_list[c_ptr->m_idx];
		if (m_ptr) {

			/* Save screen */
			screen_save();
			button_backup_all(TRUE);

			/* Recall on screen */
			screen_roff_mon(m_ptr->r_idx, 0);

			/* wait for a key or mouse press */
			inkey();

			/* Load screen */
			button_restore();
			screen_load();
		}
	}

	return 1;
}

/* pick the context menu options appropiate for the item */
int context_menu_object(const object_type *o_ptr)
{
	menu_type *m;
	rect_region r;
	int selected;
	int location = 0;
	char *labels;
	char header[120];
	s16b *list;

	m = menu_dynamic_new();
	if (!m || !o_ptr) {
		return 0;
	}
	object_desc(header, o_ptr, TRUE, 2, sizeof(header));

	list = look_up_list((object_type*)o_ptr);
	if (list) {
		if (list == &(p_ptr->inventory)) {
			location = USE_INVEN;
		} else
		if (list == &(area(p_ptr->px, p_ptr->py)->o_idx)) {
			location = USE_FLOOR;
		} else
		{
			/* check if in a container */
			location = USE_INVEN;
		}
	} else
	if (GET_ARRAY_INDEX(p_ptr->equipment, o_ptr) >= EQUIP_WIELD) {
		location = USE_EQUIP;
	}

	labels = (char*)string_make(lower_case);
	m->selections = labels;

	menu_dynamic_add_label(m, "Inspect", 'I', 1, labels);

	if (item_tester_hook_is_book(o_ptr)) {
		if (player_can_cast_from(o_ptr)) {
			if (player_can_cast()) {
				menu_dynamic_add_label(m, "Cast", 'm', 8, labels);
			} else {
				menu_dynamic_add_label(m, "$Cast", 'm', 8, labels);
			}
			if (player_can_study()) {
				menu_dynamic_add_label(m, "Study", 'G', 10, labels);
			} else {
				menu_dynamic_add_label(m, "$Study", 'G', 10, labels);
			}
		}
		if (player_is_caster() && player_can_read()) {
			menu_dynamic_add_label(m, "Browse", 'b', 9, labels);
		}
	} else
	if (item_tester_hook_useable(o_ptr)) {
		if (obj_is_wand(o_ptr)) {
			if (obj_has_charges(o_ptr)) {
				menu_dynamic_add_label(m, "Aim", 'a', 8, labels);
			} else {
				menu_dynamic_add_label(m, "$Aim", 'a', 8, labels);
			}
		} else
		if (obj_is_rod(o_ptr)) {
			if (obj_can_zap(o_ptr)) {
				menu_dynamic_add_label(m, "Zap", 'z', 8, labels);
			} else {
				menu_dynamic_add_label(m, "$Zap", 'z', 8, labels);
			}
		} else
		if (obj_is_staff(o_ptr)) {
			if (obj_has_charges(o_ptr)) {
				menu_dynamic_add_label(m, "Use", 'u', 8, labels);
			} else {
				menu_dynamic_add_label(m, "$Use", 'u', 8, labels);
			}
		} else
		if (obj_is_scroll(o_ptr)) {
			if (player_can_read()) {
				menu_dynamic_add_label(m, "Read", 'r', 8, labels);
			} else {
				menu_dynamic_add_label(m, "$Read", 'r', 8, labels);
			}
		} else
		if (obj_is_potion(o_ptr)) {
			menu_dynamic_add_label(m, "Quaff", 'q', 8, labels);
		} else
		if (obj_is_food(o_ptr)) {
			menu_dynamic_add_label(m, "Eat", 'E', 8, labels);
		} else
		if (item_tester_hook_activate(o_ptr)) {
			if (obj_is_activatable(o_ptr)) {
				menu_dynamic_add_label(m, "Activate", 'A', 8, labels);
			} else {
				menu_dynamic_add_label(m, "$Activate", 'A', 8, labels);
			}
		} else
		{
			menu_dynamic_add_label(m, "Use", 'U', 8, labels);
		}
	} else
	if (item_tester_hook_ammo(o_ptr)) {
		if (obj_can_fire(o_ptr)) {
			menu_dynamic_add_label(m, "Fire", 'f', 15, labels);
		} else {
			menu_dynamic_add_label(m, "$Fire", 'f', 15, labels);
		}
	}
	if (obj_can_refill(o_ptr)) {
		menu_dynamic_add_label(m, "Refill", 'F', 11, labels);
	}
	if (item_tester_hook_wear(o_ptr)) {
		if (location == USE_EQUIP) {
			menu_dynamic_add_label(m, "Take off", 't', 3, labels);
		} else
		if (location == USE_INVEN) {
			if (item_tester_hook_armour(o_ptr)) {
				menu_dynamic_add_label(m, "Wear", 'w', 2, labels);
			} else {
		 		menu_dynamic_add_label(m, "Wield", 'w', 2, labels);
			}
			/*menu_dynamic_add_label(m, "Equip", 'w', 2, labels);*/
		}
	}
	if ((location == USE_INVEN) || (location == USE_EQUIP)) {
		menu_dynamic_add_label(m, "Drop", 'd', 6, labels);
		if (o_ptr->number > 1) {
			menu_dynamic_add_label(m, "Drop All", 'd', 13, labels);
		}
	} else
	if (location == USE_FLOOR) {
		if (inven_carry_okay(o_ptr)) {
			menu_dynamic_add_label(m, "Pickup", 'g', 7, labels);
		} else {
			menu_dynamic_add_label(m, "$Pickup", 'g', 7, labels);
		}
	}
	menu_dynamic_add_label(m, "Throw", 'v', 12, labels);

	/*if (obj_has_inscrip(o_ptr)) {*/
	if (o_ptr->inscription) {
		menu_dynamic_add_label(m, "Uninscribe", '}', 5, labels);
	} else {
		menu_dynamic_add_label(m, "Inscribe", '{', 4, labels);
	}
	menu_dynamic_add_label(m, "Destroy", 'k', 14, labels);
#if 0
	if (object_is_squelched(o_ptr)) {
		menu_dynamic_add_label(m, "Unignore", 'k', 14, labels);
	} else {
		menu_dynamic_add_label(m, "Ignore", 'k', 14, labels);
	}
#endif

	/* work out display region */
	r.width = menu_dynamic_longest_entry(m) + 3 + 2; /* +3 for tag, 2 for pad */
	r.col = Term->wid - r.width - 1;
	r.row = 1;
	r.page_rows = m->count;

	/* Hack -- no flush needed */
	msg_flag = FALSE;

	screen_save();
	button_backup_all(TRUE);

	/* Recall object */
	roff_obj_aux(o_ptr);

	menu_layout(m, &r);
	rect_region_erase_bordered(&r);

	prtf(0, 0, "($UEnter to select$Y\n$V, $UESC$Y%c$V) Command for %s:", ESCAPE, header);
	selected = menu_dynamic_select(m);

	menu_dynamic_free(m);
	string_free(labels);

	button_restore();
	screen_load();

	if (selected == 1) {
		/* inspect it */
		identify_fully_aux(o_ptr);
		return 2;
	} else
	if (selected == 2) {
		/* wield the item */
		set_get_item_object(o_ptr);
 		p_ptr->cmd.cmd = 'w';
		do_cmd_wield();
 		/*cmd_insert(CMD_WIELD);
		cmd_set_arg_item(cmd_get_top(), 0, slot);*/
	} else
	if (selected == 3) {
		/* take the item off */
		set_get_item_object(o_ptr);
 		p_ptr->cmd.cmd = 't';
		do_cmd_takeoff();
 		/*cmd_insert(CMD_TAKEOFF);
		cmd_set_arg_item(cmd_get_top(), 0, slot);*/
	} else
	if (selected == 4) {
		/* inscribe the item */
		set_get_item_object(o_ptr);
 		p_ptr->cmd.cmd = '{';
		do_cmd_inscribe();
 		/*cmd_insert(CMD_INSCRIBE);
		cmd_set_arg_item(cmd_get_top(), 0, slot);*/
	} else
	if (selected == 5) {
		/* uninscribe the item */
		set_get_item_object(o_ptr);
 		p_ptr->cmd.cmd = '}';
		do_cmd_uninscribe();
 		/*cmd_insert(CMD_UNINSCRIBE);
		cmd_set_arg_item(cmd_get_top(), 0, slot);*/
	} else
	if (selected == 6) {
		/* drop the item */
		set_get_item_object(o_ptr);
 		p_ptr->cmd.cmd = 'd';
		do_cmd_drop();
 		/*cmd_insert(CMD_DROP);
		cmd_set_arg_item(cmd_get_top(), 0, slot);*/
	} else
	if (selected == 7) {
		/* pick the item up */
		p_ptr->cmd.cmd = 'g';
		if (inven_carry_okay(o_ptr)) {
			py_pickup_aux((object_type*)o_ptr);
		} else {
			carry(TRUE);
		}
 		/*cmd_insert(CMD_PICKUP);
		cmd_set_arg_item(cmd_get_top(), 0, slot);*/
	} else
	if (selected == 8) {
		/* use the item */
		bool full = item_tester_full;
		item_tester_full = FALSE;
		if (player_can_cast_from(o_ptr)) {
			set_get_item_object(o_ptr);
 			p_ptr->cmd.cmd = 'm';
			do_cmd_cast();
		} else {
			set_get_item_object(o_ptr);
			p_ptr->cmd.cmd = 'u';
			do_cmd_use();
			/*cmd_insert(CMD_USE_ANY);
			cmd_set_arg_item(cmd_get_top(), 0, slot);*/
		}
		item_tester_full = full;
	} else
	if (selected == 9) {
		/* browse a spellbook */
 		p_ptr->cmd.cmd = 'b';
		do_cmd_browse_aux(o_ptr);
		/* copied from textui_spell_browse */
		/*textui_book_browse(o_ptr);*/
		return 2;
	} else
	if (selected == 10) {
		/* study a spell book */
 		p_ptr->cmd.cmd = 'G';
		do_cmd_study(FALSE, (object_type*)o_ptr);

	} else
	if (selected == 11) {
		/* use the item to refill a light source */
		set_get_item_object(o_ptr);
 		p_ptr->cmd.cmd = 'F';
		do_cmd_refill();
		/*cmd_insert(CMD_REFILL);
		cmd_set_arg_item(cmd_get_top(), 0, slot);*/
	} else
	if (selected == 12) {
		/* throw the item */
		set_get_item_object(o_ptr);
 		p_ptr->cmd.cmd = 'v';
		do_cmd_throw();
		/*cmd_insert(CMD_THROW);
		cmd_set_arg_item(cmd_get_top(), 0, slot);*/
	} else
	if (selected == 13) {
		/* drop all of the item stack */
		if (get_check(format("Drop %s? ", header))) {
			set_get_item_object(o_ptr);
			p_ptr->cmd.arg = o_ptr->number;
			p_ptr->cmd.cmd = 'd';
			do_cmd_drop();
			/*cmd_insert(CMD_DROP);
			cmd_set_arg_item(cmd_get_top(), 0, slot);
			cmd_set_arg_number(cmd_get_top(), 1, o_ptr->number);*/
		}
	} else
	if (selected == 14) {
		/* squelch or unsquelch the item */
		set_get_item_object(o_ptr);
 		p_ptr->cmd.cmd = 'k';
		do_cmd_destroy();
		/*textui_cmd_destroy_menu(slot);*/
	} else
	if (selected == 15) {
		/* fire some ammo */
		set_get_item_object(o_ptr);
 		p_ptr->cmd.cmd = 'f';
		do_cmd_fire();
	} else
	if (selected == -1) {
		/* this menu was canceled, tell whatever called us to display its menu again */
		return 3;
	}
	return 1;
}

#if 0
/* pick the context menu options appropiate for a store */
int context_menu_store(struct store *store, const int oid, int mx, int my)
{
	menu_type *m;
	rect_region r;
	int selected;
	char *labels;
	object_type *o_ptr;

	m = menu_dynamic_new();
	if (!m || !store) {
		return 0;
	}

	/* Get the actual object */
	o_ptr = &store->stock[oid];

	labels = string_make(lower_case);
	m->selections = labels;

	menu_dynamic_add_label(m, "Inspect Inventory", 'I', 1, labels);
	if (store->sidx == STORE_HOME) {
		/*menu_dynamic_add(m, "Stash One", 2);*/
		menu_dynamic_add_label(m, "Stash", 'd', 3, labels);
		menu_dynamic_add_label(m, "Examine", 'x', 4, labels);
		menu_dynamic_add_label(m, "Take", 'p', 6, labels);
		if (o_ptr->number > 1) {
			menu_dynamic_add_label(m, "Take One", 'o', 5, labels);
		}
	} else {
		/*menu_dynamic_add(m, "Sell One", 2);*/
		menu_dynamic_add_label(m, "Sell", 'd', 3, labels);
		menu_dynamic_add_label(m, "Examine", 'x', 4, labels);
		menu_dynamic_add_label(m, "Buy", 'p', 6, labels);
		if (o_ptr->number > 1) {
			menu_dynamic_add_label(m, "Buy One", 'o', 5, labels);
		}
	}
	menu_dynamic_add_label(m, "Exit", '`', 7, labels);


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
	region_erase_bordered(&r);

	prt("(Enter to select, ESC) Command:", 0, 0);
	selected = menu_dynamic_select(m);

	menu_dynamic_free(m);
	string_free(labels);

	button_restore();
	screen_load();

	if (selected == 1) {
		Term_keypress('I', 0);
	} else
	if (selected == 2) {
		Term_keypress('s', 0);
		/* oid is store item we do not know item we want to sell here */
		/*if (store->sidx == STORE_HOME) {
			cmd_insert(CMD_STASH);
		} else {
			cmd_insert(CMD_SELL);
		}
		cmd_set_arg_item(cmd_get_top(), 0, oid);
		cmd_set_arg_number(cmd_get_top(), 1, 1);*/
	} else
	if (selected == 3) {
		Term_keypress('s', 0);
	} else
	if (selected == 4) {
		Term_keypress('x', 0);
	} else
	if (selected == 5) {
		if (store->sidx == STORE_HOME) {
			cmd_insert(CMD_RETRIEVE);
		} else {
			cmd_insert(CMD_BUY);
		}
		cmd_set_arg_choice(cmd_get_top(), 0, oid);
		cmd_set_arg_number(cmd_get_top(), 1, 1);
	} else
	if (selected == 6) {
		Term_keypress('p', 0);
	} else
	if (selected == 7) {
		Term_keypress(ESCAPE, 0);
	}
	return 1;
}

/* pick the context menu options appropiate for an item available in a store */
int context_menu_store_item(struct store *store, const int oid, int mx, int my)
{
	menu_type *m;
	rect_region r;
	int selected;
	char *labels;
	object_type *o_ptr;
	char header[120];

	/* Get the actual object */
	o_ptr = &store->stock[oid];


	m = menu_dynamic_new();
	if (!m || !store) {
		return 0;
	}
	object_desc(header, sizeof(header), o_ptr, ODESC_PREFIX | ODESC_BASE);

	labels = string_make(lower_case);
	m->selections = labels;

	menu_dynamic_add_label(m, "Examine", 'x', 4, labels);
	if (store->sidx == STORE_HOME) {
		menu_dynamic_add_label(m, "Take", 'p', 6, labels);
		if (o_ptr->number > 1) {
			menu_dynamic_add_label(m, "Take One", 'o', 5, labels);
		}
	} else {
		menu_dynamic_add_label(m, "Buy", 'p', 6, labels);
		if (o_ptr->number > 1) {
			menu_dynamic_add_label(m, "Buy One", 'o', 5, labels);
		}
	}

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
	region_erase_bordered(&r);

	prt(format("(Enter to select, ESC) Command for %s:", header), 0, 0);
	selected = menu_dynamic_select(m);

	menu_dynamic_free(m);
	string_free(labels);

	button_restore();
	screen_load();

	if (selected == 4) {
		Term_keypress('x', 0);
	} else
	if (selected == 5) {
		if (store->sidx == STORE_HOME) {
			cmd_insert(CMD_RETRIEVE);
		} else {
			cmd_insert(CMD_BUY);
		}
		cmd_set_arg_choice(cmd_get_top(), 0, oid);
		cmd_set_arg_number(cmd_get_top(), 1, 1);
	} else
	if (selected == 6) {
		Term_keypress('p', 0);
	}

	return 1;
}

#endif /* if 0 */
