/* File: dungeon.c */

/* Purpose: Angband game engine */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */
/*
 * Seperation from dungeon.c (c) 2012 Brett Reid
 *
 * Angband Dual License
 */
#include "angband.h"
#include "game.h"

extern bool dungeon_init_level();
extern bool dungeon_play_frame();
extern void dungeon_close_level();
extern void cheat_death();

void cheat_death(void)
{
	/* Restore hit points */
	p_ptr->chp = p_ptr->mhp;
	p_ptr->chp_frac = 0;

	/* Restore spell points */
	p_ptr->csp = p_ptr->msp;
	p_ptr->csp_frac = 0;

	/* Hack -- Healing */
	(void)clear_blind();
	(void)clear_confused();
	(void)clear_poisoned();
	(void)clear_afraid();
	(void)clear_paralyzed();
	(void)clear_image();
	(void)clear_stun();
	(void)clear_cut();

	/* Hack"-- Prevent starvation */
	(void)set_food(PY_FOOD_MAX - 1);


	/* Hack -- cancel recall */
	if (query_timed(TIMED_WORD_RECALL)) {
		/* Message */
		msgf("A tension leaves the air around you...");
		message_flush();

		/* Hack -- Prevent recall */
		*(get_timed_ptr(TIMED_WORD_RECALL)) = 0;
		p_ptr->redraw |= (PR_STATUS);
	}

	/* Note cause of death XXX XXX XXX */
	(void)strcpy(p_ptr->state.died_from, "Cheating death");

	/* Do not die */
	p_ptr->state.is_dead = FALSE;
}

/*
 * Load some "user pref files"
 *
 * Modified by Arcum Dagsson to support
 * separate macro files for different realms.
 */
static void load_all_pref_files(void)
{
	/* Process global pref file */
	(void)process_pref_file("player.prf");

	/* Process race pref file */
	(void)process_pref_file("%s.prf", rp_ptr->title);

	/* Process class pref file */
	(void)process_pref_file("%s.prf", cp_ptr->title);

	/* Process character file */
	(void)process_pref_file("%s.prf", player_base);

	/* Access the "realm 1" pref file */
	if (p_ptr->spell.realm[0] != REALM_NONE)
	{
		/* Process that file */
		(void)process_pref_file("%s.prf", realm_names[p_ptr->spell.realm[0]]);
	}

	/* Access the "realm 2" pref file */
	if (p_ptr->spell.realm[1] != REALM_NONE)
	{
		/* Process that file */
		(void)process_pref_file("%s.prf", realm_names[p_ptr->spell.realm[1]]);
	}
}


/*
 * Actually play a game
 *
 * If the "new_game" parameter is true, then, after loading the
 * savefile, we will commit suicide, if necessary, to allow the
 * player to start a new game.
 */
bool game_init(bool new_game)
{
	int i;

	/* Verify main term */
	if (!angband_term[0])
	{
		quit("main window does not exist");
	}

	/* Make sure main term is active */
	Term_activate(angband_term[0]);

	if (!angband_term[0]) quit("Main term does not exist!");

	/* Hack -- Character is "icky" */
	screen_save();

	/* Initialise the resize hooks */
	angband_term[0]->resize_hook = resize_map;

	for (i = 1; i < 8; i++)
	{
		/* Does the term exist? */
		if (angband_term[i])
		{
			/* Add the redraw on resize hook */
			angband_term[i]->resize_hook = redraw_window;
		}
	}

	/* Verify minimum size */
	if ((Term->hgt < 24) || (Term->wid < 80))
	{
		quit("main window is too small");
	}

	/* Hack -- turn off the cursor */
	(void)Term_set_cursor(0);

	/* Attempt to load */
	if (!load_player())
	{
		/* Oops */
		quit("broken savefile");
	}

	/* Nothing loaded */
	if (!character_loaded)
	{
		/* Make new player */
		new_game = TRUE;

		/* The dungeon is not ready */
		character_dungeon = FALSE;
	}

	/* Hack -- Default base_name */
	if (!player_base[0])
	{
		strcpy(player_base, "PLAYER");
	}

	/* Init the RNG */
	if (Rand_quick || new_game)
	{
		u32b seed;

		/* Basic seed */
		seed = (time(NULL));

#ifdef SET_UID

		/* Mutate the seed on Unix machines */
		seed = ((seed >> 3) * (getpid() * 2));

#endif

		/* Use the complex RNG */
		Rand_quick = FALSE;

		/* Seed the "complex" RNG */
		Rand_state_init(seed);
	}

	/* Set or clear "rogue_like_commands" if requested */
	if (arg_force_original) rogue_like_commands = FALSE;
	if (arg_force_roguelike) rogue_like_commands = TRUE;

	/* Roll new character */
	if (new_game)
	{
		/* Initialize the panel bounds to prevent a crash (rr9) */
		verify_panel();

		/* Wipe everything */
		wipe_all_list();
		/* Use the line I removed from wipe_all_list */
		/* (to clear player inventory) - Brett */
		if (p_ptr->inventory) delete_object_list(&p_ptr->inventory);

		/* Hack -- seed for flavors */
		seed_flavor = randint0(0x10000000);

		/* Roll up a new character */
		player_birth();

		/* Hack -- enter the world */
		if ((p_ptr->rp.prace == RACE_VAMPIRE) ||
			(p_ptr->rp.prace == RACE_SKELETON) ||
			(p_ptr->rp.prace == RACE_ZOMBIE) ||
			(p_ptr->rp.prace == RACE_SPECTRE) ||
			(p_ptr->rp.prace == RACE_GHOUL))
		{
			/* Undead start just after midnight */
			turn =  TOWN_HALF_DAY + (6*TOWN_HOUR) + 1;
			turn_offset = turn-1;
		}
		else
		{
			turn = 1;
		}

		/* Create a new wilderness for the player */
		create_wilderness();

		/* The dungeon is ready */
		character_dungeon = TRUE;
	}

	/* Reset the visual mappings */
	reset_visuals();

	/* Normal machine (process player name) */
	if (savefile[0])
	{
		process_player_name(FALSE);
	}

	/* Weird machine (process player name, pick savefile name) */
	else
	{
		process_player_name(TRUE);
	}

	/* Hack - if note file exists, load it */
	if (!new_game && take_notes)
	{
		add_note_type(NOTE_ENTER_DUNGEON);
	}

	/* Flash a message */
	prtf(0, 0, "Please wait...");

	/* Flush the message */
	Term_fresh();


	/* Hack -- Ask to enter wizard mode */
	if (arg_wizard && !(p_ptr->state.noscore)) {
		msgf("Wizard mode is for debugging and experimenting.");
		msgf("The game will not be scored if you enter wizard mode.");
		message_flush();
		if (get_check("Are you sure you want to enter wizard mode? ")) {
			p_ptr->state.wizard = TRUE;
			p_ptr->state.noscore |= 0x0002;
		}
	}

	/* Flavor the objects */
	flavor_init();

	/* Load the "pref" files */
	load_all_pref_files();

	/*
	 * Set or clear "rogue_like_commands" if requested
	 * (Do it again, because of loading the pref files
	 *  can stomp on the options.)
	 */
	if (arg_force_original) rogue_like_commands = FALSE;
	if (arg_force_roguelike) rogue_like_commands = TRUE;

	/* Generate a dungeon level if needed */
	if (!character_dungeon) generate_cave();

	/* Character is now "complete" */
	character_generated = TRUE;

	/* Hack -- Character is no longer "icky" */
	screen_load();

	/* React to changes */
	Term_xtra(TERM_XTRA_REACT, 0);

	/* Start game */
	p_ptr->state.playing = TRUE;

	/* Hack -- Enforce "delayed death" */
	if (p_ptr->chp < 0) p_ptr->state.is_dead = TRUE;

	/* Enter "xtra" mode */
	character_xtra = TRUE;

	/* Resize / init the map */
	p_ptr->update |= (PU_MAP);

	/* Need to recalculate some transient things */
	p_ptr->update |= (PU_BONUS | PU_SPELLS | PU_WEIGHT);

	/* Update some stuff not stored in the savefile any more */
	p_ptr->update |= (PU_VIEW | PU_MON_LITE);

	/* Update stuff */
	update_stuff();

	/* Leave "xtra" mode */
	character_xtra = FALSE;

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_SPELL | PW_PLAYER);

	/* Window stuff */
	p_ptr->window |= (PW_MONSTER);

	/* Window stuff */
	window_stuff();

	/* Initialise inventory and equipment info for ports */
	Term_write_list(p_ptr->inventory, LIST_INVEN);
	Term_write_equipment();

	return TRUE;
}

bool game_change_level(void)
{

	/* Notice */
	notice_stuff();

	/* Update */
	handle_stuff();

	/* Cancel the target */
	p_ptr->target_who = 0;

	/* Cancel the health bar */
	health_track(0);

	/* Forget the view */
	forget_view();

	/* Handle "quit and save" */
	if (!p_ptr->state.playing && !p_ptr->state.is_dead) return FALSE;

	/* Go to the new level */
	change_level(p_ptr->depth);

	/* XXX XXX XXX */
	message_flush();

	/* Accidental Death */
	if (p_ptr->state.playing && p_ptr->state.is_dead)
	{
		object_type *o_ptr;
		o_ptr = player_has(TV_SPIRIT, 0);
		if (o_ptr) {
			/* Message */
			msgf("As your conciousness fades, you feel engery coming from one of your items.");
			message_flush();

			cheat_death();
			p_ptr->used_ankhs++;

			/* remove the item from the player */
			if (o_ptr->number > 1) {
				o_ptr->number -= 1;
			} else {
				delete_held_object(&(p_ptr->inventory), o_ptr);
			}

			/* TODO create a death chest quest and move all items to death chest storage */

			/* move the player somewhere, same as recall for now TODO change */
			if (p_ptr->depth) {
				p_ptr->depth = 0;
				change_level(p_ptr->depth);
				msgf("You open your eyes and see that you are back on the surface.");
			} else
			if (p_ptr->home_place_num && p_ptr->home_store_num) {
				/* Move the player from its current wilderness location to its primary home
				 * Copied from building_magetower() - Brett */

				place_type *pl_ptr2 = &place[p_ptr->home_place_num];
				store_type *st_ptr2 = &pl_ptr2->store[p_ptr->home_store_num];

				msgf("You open your eyes and find yourself at home.");

				/* Move the player */
				p_ptr->px = pl_ptr2->x * 16 + st_ptr2->x;
				p_ptr->py = pl_ptr2->y * 16 + st_ptr2->y;

				p_ptr->wilderness_x = p_ptr->px;
				p_ptr->wilderness_y = p_ptr->py;

				/* Notice player location */
				Term_move_player();

				/* Remove all monster lights */
				lite_n = 0;

				/* Mark the entire view as forgotten */
				view_n = 0;

				/* Notice the move */
				move_wild();

				/* Check for new panel (redraw map) */
				verify_panel();

				/* Update stuff */
				p_ptr->update |= (PU_VIEW | PU_FLOW | PU_MON_LITE);

				/* Update the monsters */
				p_ptr->update |= (PU_DISTANCE);

				/* Window stuff */
				p_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);
			} else
			{
				p_ptr->depth = 0;
				change_level(p_ptr->depth);
				msgf("You open your eyes and see that you are back on the surface.");
			}

			/* Leaving */
			p_ptr->state.leaving = TRUE;

			/* restore player energy */
			p_ptr->energy = 100;
		} else
		/* Mega-Hack -- Allow player to cheat death */
		if ((p_ptr->state.wizard || cheat_live) && !get_check("Die? "))
		{
			/* Mark social class, reset age, if needed */
			if (p_ptr->rp.sc) p_ptr->rp.sc = p_ptr->rp.age = 0;

			/* Increase age */
			p_ptr->rp.age++;

			/* Mark savefile */
			p_ptr->state.noscore |= 0x0001;

			/* Message */
			msgf("You invoke wizard mode and cheat death.");
			message_flush();

			cheat_death();

			p_ptr->depth = 0;
			change_level(p_ptr->depth);

			/* Leaving */
			p_ptr->state.leaving = TRUE;
		}
	}

	/* Handle "death" */
	if (p_ptr->state.is_dead) return FALSE;

	/* Make a new level */
	generate_cave();

	/* Update panels */
	p_ptr->update |= (PU_MAP);

	update_stuff();

	/* we are going to play another level */
	return TRUE;
}

void play_game(bool new_game)
{
	static bool play_level = FALSE;
	bool play_game = FALSE;
	if (!game_init(new_game)) {
		return;
	}
	play_game = TRUE;
	while (play_game) {
		/* Process the level */
		if (!play_level && character_dungeon) {
			if (dungeon_init_level()) {
				play_level = TRUE;
			} else {
				dungeon_close_level();
			}
		}
		if (play_level) {
			/* Main loop */
			if (dungeon_play_frame()) {
				/* we are going to play another turn on this level */
				continue;
			} else {
				play_level = FALSE;
				dungeon_close_level();
				if (!game_change_level()) {
					play_game = FALSE;
				}
			}
		} else {
			if (!game_change_level()) {
				play_game = FALSE;
			}
		}
	}
	close_game();
}

