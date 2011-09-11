/* File: wild4.c */

/* Purpose: Wilderness changes by Brett Reid */

/*
 * Copyright (c) 2008 Brett Reid
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


/* A bit more descriptive name for dungeons - Brett */
/* This used to be in create dungeons, but there it
 * was not catching everything */
void name_dungeons()
{
	int i, j, b;
  char buf[T_NAME_LEN + 1];
	int len;

  place_type *pl_ptr;
  dun_type *d_ptr;

  /* Scan all places */
	for (i = 0; i < place_count; i++)
	{
		pl_ptr = &place[i];
    if ((pl_ptr->type == PL_TOWN_FRACT) || (pl_ptr->type == PL_TOWN_OLD))
    {
      continue;
    } else
    if (pl_ptr->type == PL_DUNGEON)
    {
      /* Get a normal 'elvish' name */
	    get_table_name(buf, FALSE);

	    /* Get length */
	    len = strlen(buf) - 1;

      d_ptr = pl_ptr->dungeon;
      switch (d_ptr->habitat) {
      case RF7_DUN_DARKWATER:
        {
    		  strcpy(pl_ptr->name, "Sewers");
          break;
        }
      case RF7_DUN_LAIR:
        {
    		  strcpy(pl_ptr->name, "Lair");
          break;
        }
      case RF7_DUN_TEMPLE:
        {
		      if ((len < T_NAME_LEN - 10))
		      {
            if (one_in_(3)) {
			        strnfmt(pl_ptr->name, T_NAME_LEN + 1, "%s Monastary", buf);
            } else
            if (one_in_(2)) {
			        strnfmt(pl_ptr->name, T_NAME_LEN + 1, "Temple of %s", buf);
            } else 
            {
			        strnfmt(pl_ptr->name, T_NAME_LEN + 1, "%s Temple", buf);
            }
		      }
		      else
		      {
			      // Use a default name
			      strnfmt(pl_ptr->name, T_NAME_LEN + 1, "Evil Temple", buf);
		      }
          break;
        }
      case RF7_DUN_TOWER:
        {
		      if ((len < T_NAME_LEN - 9))
		      {
            if (one_in_(2)) {
			        strnfmt(pl_ptr->name, T_NAME_LEN + 1, "Tower of %s", buf);
            } else {
			        strnfmt(pl_ptr->name, T_NAME_LEN + 1, "%s Tower", buf);
            }
		      }
		      else
		      {
			      // Use a default name
			      strnfmt(pl_ptr->name, T_NAME_LEN + 1, "Evil Tower", buf);
		      }
          break;
        }
      case RF7_DUN_RUIN:
        {
		      if ((len < T_NAME_LEN - 9) && (one_in_(2)))
		      {
  	        strnfmt(pl_ptr->name, T_NAME_LEN + 1, "Ruins of %s", buf);
		      }
		      else
		      {
			      // Use a default name
			      strnfmt(pl_ptr->name, T_NAME_LEN + 1, "Ruins", buf);
		      }
          break;
        }
      case RF7_DUN_GRAVE:
        {
		      if ((len < T_NAME_LEN - 9) && (one_in_(2)))
		      {
  	        strnfmt(pl_ptr->name, T_NAME_LEN + 1, "%s Cemetary", buf);
		      }
		      else
		      {
			      // Use a default name
			      strnfmt(pl_ptr->name, T_NAME_LEN + 1, "Graveyard", buf);
		      }
          break;
        }
      case RF7_DUN_CAVERN:
        {
		      if ((len < T_NAME_LEN - 9))
          { // use the dwarvish name generator?
            if (one_in_(4)) 
            {
			        strnfmt(pl_ptr->name, T_NAME_LEN + 1, "Caves of %s", buf);
            } else
            if (one_in_(3)) 
            {
			        strnfmt(pl_ptr->name, T_NAME_LEN + 1, "%s Caves", buf);
            } else
            if (one_in_(2)) 
            {
			        strnfmt(pl_ptr->name, T_NAME_LEN + 1, "%s's Hole", buf);
            } else
            {
			        strnfmt(pl_ptr->name, T_NAME_LEN + 1, "%s Cavern", buf);
            }
		      }
		      else
		      {
			      // Use a default name
			      strnfmt(pl_ptr->name, T_NAME_LEN + 1, "Cavern", buf);
		      }
          break;
        }
      case RF7_DUN_PLANAR:
        {
    		  strcpy(pl_ptr->name, "Planar Gate");
          break;
        }
      case RF7_DUN_HELL:
        {
    		  strcpy(pl_ptr->name, "Gate to Hell");
          break;
        }
      case RF7_DUN_HORROR:
        {
    		  strcpy(pl_ptr->name, "Dungeon of Horror");
          break;
        }
      case RF7_DUN_MINE:
        {
		      if ((len < T_NAME_LEN - 9))
		      { // use the dwarvish name generator?
            if (one_in_(2)) {
			        strnfmt(pl_ptr->name, T_NAME_LEN + 1, "Mines of %s", buf);
            } else {
			        strnfmt(pl_ptr->name, T_NAME_LEN + 1, "%s Mine", buf);
            }
		      }
		      else
		      {
			      // Use a default name
			      strnfmt(pl_ptr->name, T_NAME_LEN + 1, "Mine", buf);
		      }
          break;
        }
      case RF7_DUN_CITY:
        {
		      if ((len < T_NAME_LEN - 4))
		      {
            if (one_in_(2)) {
			        strnfmt(pl_ptr->name, T_NAME_LEN + 1, "Bal %s", buf);
            } else {
			        strnfmt(pl_ptr->name, T_NAME_LEN + 1, "%s Fel", buf);
            }
		      }
		      else
		      {
			      // Use a default name
			      strnfmt(pl_ptr->name, T_NAME_LEN + 1, "Outlaw Town", buf);
		      }
          break;
        }
        // Default - keep the name given before dungeon init ("Dungeon")
      }
    } else
    if (pl_ptr->type == PL_QUEST_PIT)
    {
    	quest_type *q_ptr;
	    q_ptr = &quest[pl_ptr->quest_num];
      if (pl_ptr->quest_num && q_ptr->type == QUEST_TYPE_WILD) {
        switch (q_ptr->data.wld.data) {
        case 0:
          {
	          (void)strnfmt(pl_ptr->name, T_NAME_LEN, "Undead Camp");
            break;
          }
        case 1:
          {
	          (void)strnfmt(pl_ptr->name, T_NAME_LEN, "Orc Camp");
            break;
          }
        case 2:
          {
	          (void)strnfmt(pl_ptr->name, T_NAME_LEN, "Troll Camp");
            break;
          }
        case 3:
          {
	          (void)strnfmt(pl_ptr->name, T_NAME_LEN, "Giant Camp");
            break;
          }
        case 4:
          {
	          (void)strnfmt(pl_ptr->name, T_NAME_LEN, "Dragon Camp");
            break;
          }
        default:
          {
	          (void)strnfmt(pl_ptr->name, T_NAME_LEN, "Evil Camp");
          }
        }
      }
    } else
		{
    }
	}
}

bool count_wilderness(int option)
{
  int bcount[MAX_CITY_BUILD];
	int i, j, b;
  int btotal, ctotal, dtotal, ftotal, ttotal;
  int unknowns;
  bool visited_town;
  bool known_only = FALSE;
	
  place_type *pl_ptr;
  store_type *st_ptr;

	// zero counters
  btotal = 0;
  ctotal = 0;
  dtotal = 0;
  ftotal = 0;
  ttotal = 0;
  unknowns = 0;

	for (i = 0; i < MAX_CITY_BUILD; i++)
  {
    bcount[i]=0;
  }

  // Scan all places
	for (i = 0; i < place_count; i++)
	{
    pl_ptr = &place[i];
    if (known_only && (pl_ptr->seen == FALSE)) {
		  visited_town = FALSE;
		  for (j = 0; j < pl_ptr->numstores; j++)
		  {
			  store_type *st_ptr = &pl_ptr->store[i];
			  /* Stores are not given coordinates until you visit a town */
        if (st_ptr->x != 0 && st_ptr->y != 0) {
          visited_town = TRUE;
          break;
        }
		  }
      if (visited_town == FALSE) {
        continue;
      }
    }
    if ((pl_ptr->type == PL_TOWN_FRACT) || (pl_ptr->type == PL_TOWN_OLD)
      || (pl_ptr->type == PL_FARM))
    {
		  b = pl_ptr->numstores;
      for (j = 0; j < b; j++)
      {
        st_ptr = &pl_ptr->store[j];
        if (st_ptr->type < MAX_CITY_BUILD)
        {
          bcount[st_ptr->type]++;
        } else {
          unknowns++;
        }
		  }
      ttotal++;
    } else
    if (pl_ptr->type == PL_DUNGEON)
    {
      dtotal++;
    } else
    if (pl_ptr->type == PL_QUEST_PIT)
    {
      ctotal++;
    } else
    if (pl_ptr->type == PL_QUEST_STAIR)
    {
      /* do not count this */
    } else
    if (pl_ptr->type == PL_TOWN_MINI)
    {
      /* do not count this */
    } else
    if (pl_ptr->type == PL_FARM)
    {
      ftotal++;
    } else
		{
      if (pl_ptr->type)
      unknowns++;
    }
	}

  // count wilderness block terrain types?

  // we have the count, now display the information onscreen
  {
	  FILE *fff;
	  char file_name[1024];
	  char tmp_str[256];
    cptr store_name;

    // Open a temporary file
	  fff = my_fopen_temp(file_name, 1024);

	  // Failure
	  if (!fff) return 0;
	  for (i = 0; i < MAX_CITY_BUILD; i++)
	  {
      if (i > 16) 
      {
		    store_name = t_info[i+31].name;
      }
      else
      if (i == 16) 
      {
		    store_name = &("Blank Buildings");
      }
      else
      if (i == 15) 
      {
		    store_name = &("Empty Lots");
      }
      else
      if (i > 9) 
      {
		    store_name = t_info[i+33].name;
      }
      else
      if (i == 9) 
      {
		    store_name = &("Dungeon Stairs");
      }
      else
      {
		    store_name = t_info[i+34].name;
      }
      strnfmt(tmp_str, 256, "%2d of %s\n", bcount[i], store_name);

      // Copy to the file
		  froff(fff, "%s", tmp_str);
	  }

    if (known_only) {
      strnfmt(tmp_str, 256, "%d Known Towns\n", ttotal);
		  froff(fff, "%s", tmp_str);

      //strnfmt(tmp_str, 256, "%d Known Farms\n", ftotal);
		  //froff(fff, "%s", tmp_str);

      strnfmt(tmp_str, 256, "%d Known Dungeons\n", dtotal);
		  froff(fff, "%s", tmp_str);

      strnfmt(tmp_str, 256, "%d Known Camps\n", ctotal);
		  froff(fff, "%s", tmp_str);

      strnfmt(tmp_str, 256, "%d Unknowns\n", unknowns);
		  froff(fff, "%s", tmp_str);
    } else {
      strnfmt(tmp_str, 256, "%d Towns\n", ttotal);
		  froff(fff, "%s", tmp_str);

      //strnfmt(tmp_str, 256, "%d Farms\n", ftotal);
		  //froff(fff, "%s", tmp_str);

      strnfmt(tmp_str, 256, "%d Dungeons\n", dtotal);
		  froff(fff, "%s", tmp_str);

      strnfmt(tmp_str, 256, "%d Camps\n", ctotal);
		  froff(fff, "%s", tmp_str);

      strnfmt(tmp_str, 256, "%d Unknowns\n", unknowns);
		  froff(fff, "%s", tmp_str);
    }
	  // Close the file
	  my_fclose(fff);

	  // Display the file contents
	  (void)show_file(file_name, "Building types", 0, 0);

	  // Remove the file
	  (void)fd_kill(file_name);
  }
  return 1;
}


/*
bool recreate_wilderness(int option)
{
	u32b seed;

  messages_free();
  messages_init();
	// Wipe everything - to keep inventory, comment out a line in wipe all list
	wipe_all_list();

	// Basic seed 
	seed = (time(NULL));

	// Use the complex RNG 
	Rand_quick = FALSE;

	// Seed the "complex" RNG 
	Rand_state_init(seed);
	// Reinitialise the quests 
	init_player_quests();
	// Create a new wilderness for the player
	create_wilderness();
	
  p_ptr->update |= (PU_VIEW | PU_MON_LITE);
	p_ptr->window |= (PW_MONSTER);

  // Count the buildings in the wilderness, displaying the totals
  count_wilderness(0);

  return 1;
}
void name_dungeons();
bool name_places(int option)
{
  name_dungeons();
  return TRUE;
}
/**/
