/* File: birth-modify.c */

/* Purpose: level a starting player character to level 45 */

/*
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

#include "wild.h"

/* player is jumping to the end game */
void player_birth_jump_end_game1(void)
{
	int i, j;
  int tv;
  object_type *q_ptr;

  if (!jump_end_game) return;

  /* replace any rations of food with scrolls of satisfy hunger */
  q_ptr = player_has(TV_FOOD, SV_FOOD_RATION);
  if (q_ptr) {
    q_ptr->number = 0;
   	delete_held_object(&p_ptr->inventory, q_ptr);
  }
	q_ptr =
		object_prep(lookup_kind(TV_SCROLL, SV_SCROLL_SATISFY_HUNGER));
	q_ptr->number = 10;
	object_aware(q_ptr); object_known(q_ptr);
	/* These objects give no score */
	q_ptr->info |= OB_NO_EXP;	(void)inven_carry(q_ptr);

  /* replace any torches with an ego lantern */
  q_ptr = player_has(TV_LITE, SV_LITE_TORCH);
  if (q_ptr) {
    q_ptr->number = 0;
   	delete_held_object(&p_ptr->inventory, q_ptr);
		q_ptr =
			object_prep(lookup_kind(TV_LITE, SV_LITE_LANTERN));

    apply_magic(q_ptr, 80, 30, OC_FORCE_GOOD);

    object_aware(q_ptr);
		object_known(q_ptr);

    object_mental(q_ptr);

	  /* Save all the known flags */
	  q_ptr->kn_flags[0] = q_ptr->flags[0];
	  q_ptr->kn_flags[1] = q_ptr->flags[1];
	  q_ptr->kn_flags[2] = q_ptr->flags[2];
	  q_ptr->kn_flags[3] = q_ptr->flags[3];
		/* These objects give no score */
		q_ptr->info |= OB_NO_EXP;

		(void)inven_carry(q_ptr);
  }

  /* create good items out of any existing equipment */
  OBJ_ITT_START(p_ptr->inventory, q_ptr)
  {

    if (((q_ptr->tval >= TV_THROWN) && (q_ptr->tval <= TV_DRAG_ARMOR))
      || (q_ptr->tval == TV_AMULET)|| (q_ptr->tval == TV_RING))
    {
		  apply_magic(q_ptr, 90, 20, OC_FORCE_GOOD);
      identify_item(q_ptr);
	    /* Save all the known flags */
	    q_ptr->kn_flags[0] = q_ptr->flags[0];
	    q_ptr->kn_flags[1] = q_ptr->flags[1];
	    q_ptr->kn_flags[2] = q_ptr->flags[2];
	    q_ptr->kn_flags[3] = q_ptr->flags[3];
		  object_aware(q_ptr); object_known(q_ptr); object_mental(q_ptr);
    }
  }
  OBJ_ITT_END;

  /* max player stats */
	for (i = 0; i < A_MAX; i++) {
		/* Obtain a "bonus" for "race" and "class" */
		j = rp_ptr->r_adj[i] + cp_ptr->c_adj[i];
		p_ptr->stat[i].cur = p_ptr->stat[i].max = 280 + j*10;
	}

  /* give the player enough xp to get level 45 */
  p_ptr->exp = player_exp[43] * p_ptr->expfact / 100L;
  p_ptr->max_exp = player_exp[43] * p_ptr->expfact / 100L;
  /* run check_experience() later */

  /* give the player 2 million gold */
  p_ptr->au = 3500000;
	rebirth_ptr->au = 3500000;

  /* give spellcasters all of their books */
  if (p_ptr->spell.realm[0]) {
    tv = TV_LIFE_BOOK + p_ptr->spell.realm[0] - 1;
    for (i = 1; i < 4; i++) {
		  q_ptr = object_prep(lookup_kind(tv, i));

	    object_aware(q_ptr); object_known(q_ptr);
    	q_ptr->info |= OB_NO_EXP;	(void)inven_carry(q_ptr);
    }
  }
  if (p_ptr->spell.realm[1]) {
    tv = TV_LIFE_BOOK + p_ptr->spell.realm[1] - 1;
    for (i = 1; i < 4; i++) {
		  q_ptr = object_prep(lookup_kind(tv, i));

	    object_aware(q_ptr); object_known(q_ptr);
    	q_ptr->info |= OB_NO_EXP;	(void)inven_carry(q_ptr);
    }
  }

  /* give the player 4 rods of alchemy */
	q_ptr = object_prep(lookup_kind(TV_ROD, SV_ROD_ALCHEMY));
	q_ptr->number = 4;

	object_aware(q_ptr); object_known(q_ptr);
	q_ptr->info |= OB_NO_EXP;	(void)inven_carry(q_ptr);

  /* give the player a bunch of (30) healing potions */
	q_ptr = object_prep(lookup_kind(TV_POTION, SV_POTION_HEALING));
	q_ptr->number = 30;

	object_aware(q_ptr); object_known(q_ptr);
	q_ptr->info |= OB_NO_EXP;	(void)inven_carry(q_ptr);

  /* give spellcasters some restore mana potions */
  if (p_ptr->spell.realm[0]) {
		q_ptr = object_prep(lookup_kind(TV_POTION, SV_POTION_RESTORE_MANA));
		q_ptr->number = 10;

		object_aware(q_ptr); object_known(q_ptr);
		q_ptr->info |= OB_NO_EXP;	(void)inven_carry(q_ptr);
  }
  /* give the player some restore level potions */
	q_ptr = object_prep(lookup_kind(TV_POTION, SV_POTION_RESTORE_EXP));
	q_ptr->number = 6;

	object_aware(q_ptr); object_known(q_ptr);
	q_ptr->info |= OB_NO_EXP;	(void)inven_carry(q_ptr);

  /* fill the rest of the inventory with good/great items */
  j = 0;
  OBJ_ITT_START(p_ptr->inventory, q_ptr)
  {
    j++; /* count the number of items in the inventory */
  }
  OBJ_ITT_END;
  if (j < 23) {
    j = 23-j-1;
    for (i = 0; i < j; i++) {
		  q_ptr = make_object(80, 20, NULL);
      identify_item(q_ptr);
	    /* Save all the known flags */
	    q_ptr->kn_flags[0] = q_ptr->flags[0];
	    q_ptr->kn_flags[1] = q_ptr->flags[1];
	    q_ptr->kn_flags[2] = q_ptr->flags[2];
	    q_ptr->kn_flags[3] = q_ptr->flags[3];
		  object_aware(q_ptr); object_known(q_ptr); object_mental(q_ptr);
		  q_ptr->info |= OB_NO_EXP;	(void)inven_carry(q_ptr);
    }
    /* add a backpack */
	  q_ptr = object_prep(lookup_kind(TV_CONTAINER, 17));
    identify_item(q_ptr);
	  /* Save all the known flags */
	  q_ptr->kn_flags[0] = q_ptr->flags[0];
	  q_ptr->kn_flags[1] = q_ptr->flags[1];
	  q_ptr->kn_flags[2] = q_ptr->flags[2];
	  q_ptr->kn_flags[3] = q_ptr->flags[3];
		object_aware(q_ptr); object_known(q_ptr); object_mental(q_ptr);
		q_ptr->info |= OB_NO_EXP;	(void)inven_carry(q_ptr);
  }
  /* give the player all flavor/monster knowledge */
  for (i = 1; i < z_info->k_max; i++) {
    /*if (k_info[i].flavor) {*/
      k_info[i].info |= OK_AWARE;
    /*}*/
  }
  for (i = 1; i < z_info->r_max; i++) {
    r_info[i].r_flags[6] |= RF6_LIBRARY;
	  r_info[i].r_sights = 1000;
  }
}

void player_birth_jump_end_game2(void)
{
  int i, j;
	place_type *pl_ptr;
	dun_type *d_ptr;
  void wild_discover(int wx, int wy);

  if (!jump_end_game) return;
  p_ptr->state.skip_more = TRUE;

  /* reveal all of the wilderness map and mark all towns visited */
	for (i = 0; i < max_wild - 1; i++) {
		for (j = 0; j < max_wild - 1; j++) {
			/* Memorise the location */
			wild_discover(i,j);
		}
	}

	/* Scan all places */
	for (i = 0; i < place_count; i++) {
		pl_ptr = &place[i];
		if ((pl_ptr->type == PL_TOWN_FRACT) || (pl_ptr->type == PL_TOWN_OLD)) {
      pl_ptr->seen = TRUE;
      /* if a town has not already been allocated, allocate it */
      if (!(pl_ptr->region)) {
        draw_city(pl_ptr);
      }
      /* if the place has a dungeon, set the recall depth */
      if (pl_ptr->dungeon) {
        d_ptr = pl_ptr->dungeon;
        d_ptr->recall_depth = 98 - ((98 - d_ptr->min_level) % d_ptr->level_change_step);
        if (d_ptr->recall_depth > d_ptr->max_level) {
          d_ptr->recall_depth = d_ptr->max_level;
        }
        if (d_ptr->recall_depth < d_ptr->min_level) {
          d_ptr->recall_depth = d_ptr->min_level;
        }
      }
      /* if the place has a mage tower, record aura with it */
      for (j = 0; j < pl_ptr->numstores; j++) {
        if ((pl_ptr->store[j].type == BUILD_MAGETOWER0)
          || (pl_ptr->store[j].type == BUILD_MAGETOWER1))
        {
          pl_ptr->store[j].data = 1;
        }
      }
      /* remove quests with a level < 40 */
    } else
    if (pl_ptr->type == PL_DUNGEON) {
      d_ptr = pl_ptr->dungeon;
      /* remove guards from the outside of a dungeon */
      d_ptr->flags &= ~DF_GUARDED;
      /* give the player recall levels to the bottom of each dungeon (max 98) */
      d_ptr->recall_depth = 98 - ((98 - d_ptr->min_level) % d_ptr->level_change_step);
      if (d_ptr->recall_depth > d_ptr->max_level) {
        d_ptr->recall_depth = d_ptr->max_level;
      }
      if (d_ptr->recall_depth < d_ptr->min_level) {
        d_ptr->recall_depth = d_ptr->min_level;
      }
      pl_ptr->seen = TRUE;
    }
  }

  /* clear the region, so the proper one is setup later */
  set_region(0);

  /* advance any levels given in player birth */
  check_experience();
  p_ptr->chp = p_ptr->mhp;
  p_ptr->csp = p_ptr->msp;

  /* put the player in the capital, on the stairs */
	pl_ptr = &place[p_ptr->capital_place_num];
  if (p_ptr->capital_dun_num) {
    p_ptr->wilderness_x = pl_ptr->x * 16 + pl_ptr->store[p_ptr->capital_dun_num].x;
    p_ptr->wilderness_y = pl_ptr->y * 16 + pl_ptr->store[p_ptr->capital_dun_num].y;
  } else
  {
    p_ptr->wilderness_x = pl_ptr->x * 16 + pl_ptr->store[p_ptr->capital_store_num].x;
    p_ptr->wilderness_y = pl_ptr->y * 16 + pl_ptr->store[p_ptr->capital_store_num].y;
  }
  change_level(0);
  move_wild();

  /* give the player the final quests */
  for (i = 0; i < z_info->q_max; i++) {
    if (quest[i].x_type == QX_KILL_WINNER) {
     	quest[i].flags |= QUEST_FLAG_KNOWN;
      quest[i].status = QUEST_STATUS_TAKEN;
    } else
    if (quest[i].type == QUEST_TYPE_FIND_PLACE) {
      /* mark the quest finished as everything is known */
      quest[i].status = QUEST_STATUS_FINISHED;
    } else
    if (quest[i].type == QUEST_TYPE_WILD) {
      /* skip camp quests */
      continue;
    } else
    /* remove quests with a level < 60 */
    if (quest[i].level < 60) {
      if (quest[i].type == QUEST_TYPE_FIND_ITEM) {
        /* add the artifact to the player so it is not lost */
        int a_idx = quest[i].data.fit.a_idx;
        object_type *o_ptr;
        o_ptr = create_named_art(a_idx);
        identify_item(o_ptr);
	      /* Save all the known flags */
	      o_ptr->kn_flags[0] = o_ptr->flags[0];
	      o_ptr->kn_flags[1] = o_ptr->flags[1];
	      o_ptr->kn_flags[2] = o_ptr->flags[2];
	      o_ptr->kn_flags[3] = o_ptr->flags[3];
		    object_aware(o_ptr); object_known(o_ptr);
		    object_mental(o_ptr);	(void)inven_carry(o_ptr);
      }
      if ((quest[i].type == QUEST_TYPE_BOUNTY)
        || (quest[i].type == QUEST_TYPE_DEFEND))
      {
        /* unmark the race as a questor */
        int r_idx = quest[i].data.bnt.r_idx;
        r_info[r_idx].flags[0] &= ~(RF0_QUESTOR);
        r_info[r_idx].r_pkills += quest[i].data.bnt.cur_num;
        r_info[r_idx].r_pkills += quest[i].data.bnt.max_num;
        if (FLAG( &(r_info[r_idx]), RF_UNIQUE)) {
          r_info[i].max_num = 0;
        }
      } else
      if (quest[i].type == QUEST_TYPE_DUNGEON) {
        /* unmark the race as a questor */
        int r_idx = quest[i].data.dun.r_idx;
        r_info[r_idx].flags[0] &= ~(RF0_QUESTOR);
        r_info[r_idx].r_pkills += quest[i].data.dun.cur_num;
        r_info[r_idx].r_pkills += quest[i].data.dun.max_num;
        if (FLAG( &(r_info[r_idx]), RF_UNIQUE)) {
          r_info[i].max_num = 0;
        }
      }
     	quest[i].flags |= QUEST_FLAG_KNOWN;
      quest[i].status = QUEST_STATUS_FINISHED;
    }
  }

  /* kill off most of the underlevel uniques */
  for (i = 0; i < z_info->r_max; i++) {
    if (FLAG( &(r_info[i]), RF_UNIQUE) && (r_info[i].level < 70)
      && !(FLAG( &(r_info[i]), RF_QUESTOR))
      && !(FLAG( &(r_info[i]), RF_FRIENDLY))
      && (r_info[i].max_num > 0))
    {
      if (30+randint1(80) > r_info[i].level) {
        r_info[i].r_pkills = 1;
        r_info[i].max_num = 0;
      }
    }
  }
  p_ptr->state.skip_more = FALSE;
}
