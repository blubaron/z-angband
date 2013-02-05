/* File: dungeon.c */

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
#include "button.h"

#define TY_CURSE_CHANCE 100


/*
 * Return a "feeling" (or NULL) about an item.  Method 1 (Heavy).
 */
static byte value_check_aux1(const object_type *o_ptr)
{
	/* Artifacts */
	if (FLAG(o_ptr, TR_INSTA_ART))
	{
		/* Cursed / Worthless */
		if (cursed_p(o_ptr) || !o_ptr->cost) return FEEL_TERRIBLE;

		/* Normal */
		return FEEL_SPECIAL;
	}

	/* Ego-Items */
	if (ego_item_p(o_ptr))
	{
		ego_item_type *e_ptr = &e_info[o_ptr->e_idx];

		/* Ego items that are inherently cursed are "worthless" */
		if (FLAG(e_ptr, TR_CURSED))
		{
			return FEEL_WORTHLESS;
		}
		/* Ego items that are cursed but not inherently are "tainted" */
		else if (cursed_p(o_ptr))
		{
			return FEEL_TAINTED;
		}

		/* Normal */
		return FEEL_EXCELLENT;
	}

	/* Broken items */
	if (!o_ptr->cost) return FEEL_BROKEN;

	/* Good bonus */
	if ((o_ptr->to_a > 0) || (o_ptr->to_h + o_ptr->to_d > 0))
	{
		/* Cursed good item? */
		if (cursed_p(o_ptr)) return FEEL_DUBIOUS;

		/* Normal good item */
		return FEEL_GOOD;
	}

	/* Cursed items */
	if (cursed_p(o_ptr)) return FEEL_CURSED;

	/* Worthless is "bad" */
	if (!object_value(o_ptr)) return FEEL_BAD;

	/* Default to "average" */
	return FEEL_AVERAGE;
}


/*
 * Return a "feeling" (or NULL) about an item.  Method 2 (Light).
 */
static byte value_check_aux2(const object_type *o_ptr)
{
	/* Cursed items (all of them) */
	if (cursed_p(o_ptr)) return FEEL_WEAK_CURSED;

	/* Broken items (all of them) */
	if (o_ptr->cost <= 0) return FEEL_WEAK_BAD;

	/* Artifacts -- except cursed/broken ones */
	if (FLAG(o_ptr, TR_INSTA_ART)) return FEEL_WEAK_GOOD;

	/* Worthless is "bad" */
	if (object_value(o_ptr) < 1) return FEEL_WEAK_BAD;

	/* Good armor bonus */
	if (o_ptr->to_a > 0) return FEEL_WEAK_GOOD;

	/* Good weapon bonuses */
	if (o_ptr->to_h + o_ptr->to_d > 0) return FEEL_WEAK_GOOD;

	/* Bad bonuses */
	if (item_tester_hook_armour(o_ptr) && o_ptr->to_a < 0) return FEEL_WEAK_BAD;
	if (item_tester_hook_weapon(o_ptr) &&
			  o_ptr->to_a + o_ptr->to_h + o_ptr->to_d < 0) return FEEL_WEAK_BAD;

	/* Ego-Items -- except cursed/broken ones */
	if (ego_item_p(o_ptr)) return FEEL_WEAK_GOOD;

	/* No feeling */
	return FEEL_NONE;
}

/*
 * Psuedo-id the item
 */
void sense_item(object_type *o_ptr, bool heavy, bool wield, bool msg)
{
	byte feel;

	int slot;

	bool okay = FALSE;

	/* Valid "tval" codes */
	switch (o_ptr->tval)
	{
		case TV_SHOT:
		case TV_ARROW:
		case TV_BOLT:
		case TV_BOW:
		case TV_DIGGING:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_SWORD:
		case TV_BOOTS:
		case TV_GLOVES:
		case TV_HELM:
		case TV_CROWN:
		case TV_SHIELD:
		case TV_CLOAK:
		case TV_SOFT_ARMOR:
		case TV_HARD_ARMOR:
		case TV_DRAG_ARMOR:
		{
			okay = TRUE;
			break;
		}
		case TV_FIGURINE:
		{
			if (!heavy)
				okay = TRUE;
			break;
		}
		default:
		{
			/* Skip */
			return;
		}
	}

	/* We know about it already, do not tell us again */
	if (o_ptr->info & (OB_SENSE)) return;

	/* It is fully known, no information needed */
	if (object_known_p(o_ptr)) return;

	/* Occasional failure on inventory items */
	if (!wield && !one_in_(5)) return;

	/* Good luck */
	if ((p_ptr->muta3 & MUT3_GOOD_LUCK) && !one_in_(13))
	{
		heavy = TRUE;
	}

	/* Check for a feeling */
	feel = (heavy ? value_check_aux1(o_ptr) : value_check_aux2(o_ptr));

	/* Skip non-changes */
	if (feel == o_ptr->feeling) return;

	/* hack the knowledge flag */
	if (cursed_p(o_ptr)) o_ptr->kn_flags[2] |= TR2_CURSED;

	/* Bad luck */
	if ((p_ptr->muta3 & MUT3_BAD_LUCK) && !one_in_(13))
	{
		switch (feel)
		{
			case FEEL_TERRIBLE:
			{
				feel = FEEL_SPECIAL;
				break;
			}
			case FEEL_WORTHLESS:
			{
				feel = FEEL_EXCELLENT;
				break;
			}
			case FEEL_CURSED:
			{
				feel = one_in_(3) ? FEEL_AVERAGE : FEEL_GOOD;
				break;
			}
			case FEEL_AVERAGE:
			{
				feel = one_in_(2) ? FEEL_BAD : FEEL_GOOD;
				break;
			}
			case FEEL_GOOD:
			case FEEL_BAD:
			{
				feel = one_in_(3) ? FEEL_AVERAGE : FEEL_CURSED;
				break;
			}
			case FEEL_EXCELLENT:
			{
				feel = FEEL_WORTHLESS;
				break;
			}
			case FEEL_SPECIAL:
			{
				feel = FEEL_TERRIBLE;
				break;
			}
			case FEEL_WEAK_BAD:
			case FEEL_WEAK_GOOD:
			{
				feel = one_in_(3) ? FEEL_WEAK_GOOD : FEEL_WEAK_BAD;
				break;
			}
			case FEEL_WEAK_CURSED:
			{
				feel = one_in_(3) ? FEEL_AVERAGE : FEEL_WEAK_CURSED;
				break;
			}
		}
	}

	/* Stop everything */
	if (disturb_minor) disturb(FALSE);

	/* Message */
	if (msg)
	{
		/* Message (equipment) */
		if (wield)
		{
			slot = GET_ARRAY_INDEX(p_ptr->equipment, o_ptr);

			msgf("You feel the %v (%c) you are %s %s %s...",
				   OBJECT_FMT(o_ptr, FALSE, 0), I2A(slot),
				   describe_use(slot),
				   ((o_ptr->number == 1) ? "is" : "are"),
				   game_inscriptions[feel]);
		}

		/* Message (inventory) */
		else
		{
			slot = get_item_position(p_ptr->inventory, o_ptr);

			msgf("You feel the %v (%c) in your pack %s %s...",
				   OBJECT_FMT(o_ptr, FALSE, 0), I2A(slot),
				   ((o_ptr->number == 1) ? "is" : "are"),
				   game_inscriptions[feel]);
		}
	}

	/* We have "felt" it */
	o_ptr->info |= (OB_SENSE);

	/* Set the "inscription" */
	o_ptr->feeling = feel;

  if ((feel == FEEL_AVERAGE) && !(p_ptr->muta3 & MUT3_BAD_LUCK)) {
    /* if we don't have bad luck and the item is average, just go ahead and 
     * id it. */
    identify_item(o_ptr);
  }

  /* Notice changes */
	notice_item();
}


/*
 * Sense the inventory
 *
 *   Class 0 = Warrior --> fast and heavy
 *   Class 1 = Mage    --> slow and light
 *   Class 2 = Priest  --> fast but light
 *   Class 3 = Rogue   --> okay and heavy
 *   Class 4 = Ranger  --> slow but heavy  (changed!)
 *   Class 5 = Paladin --> slow but heavy
 */
static void sense_inventory(void)
{
	int i;
	bool heavy;

	object_type *o_ptr;

	long difficulty;


	/*** Check for "sensing" ***/

	/* No sensing when confused */
	if (query_timed(TIMED_CONFUSED)) return;

	/* Analyze the class */
	switch (p_ptr->rp.pclass)
	{
		case CLASS_WARRIOR:
		{
			/* Good (heavy) sensing */
			difficulty = 9000L;

			/* Done */
			break;
		}

		case CLASS_MAGE:
		case CLASS_HIGH_MAGE:
		{
			/* Very bad (light) sensing */
			difficulty = 240000L;

			/* Done */
			break;
		}

		case CLASS_PRIEST:
		{
			/* Good (light) sensing */
			difficulty = 10000L;

			/* Done */
			break;
		}

		case CLASS_ROGUE:
		{
			/* Okay sensing */
			difficulty = 20000L;

			/* Done */
			break;
		}

		case CLASS_RANGER:
		{
			/* Bad (heavy) sensing */
			difficulty = 95000L;

			/* Done */
			break;
		}

		case CLASS_PALADIN:
		{
			/* Bad (heavy) sensing */
			difficulty = 77777L;

			/* Done */
			break;
		}

		case CLASS_WARRIOR_MAGE:
		{
			/* Bad sensing */
			difficulty = 75000L;

			/* Done */
			break;
		}

		case CLASS_MINDCRAFTER:
		{
			/* Bad sensing */
			difficulty = 55000L;

			/* Done */
			break;
		}

		case CLASS_CHAOS_WARRIOR:
		{
			/* Bad (heavy) sensing */
			difficulty = 80000L;

			/* Done */
			break;
		}

		case CLASS_MONK:
		{
			/* Okay sensing */
			difficulty = 20000L;
			heavy = FALSE;

			/* Done */
			break;
		}

		default:
		{
			/* Paranoia */
			difficulty = 0;
		}
	}

	/*
	 * Scale difficulty depending on sensing ability
	 * This can be affected by objects.
	 */
	difficulty /= (p_ptr->skills[SKILL_SNS] > 0 ? p_ptr->skills[SKILL_SNS] : 1);

	/* Rescale larger by a facter of 25 */
	difficulty *= 25;

	/* Sensing gets better as you get more experienced */
	difficulty /= p_ptr->lev * p_ptr->lev + 40;

	/* Does it work? */
	if (!(one_in_(difficulty))) return;

	/* Heavy sensing? */
	heavy = class_info[p_ptr->rp.pclass].heavy_sense;

	/* Hack: force heavy sensing for humans */
	if (p_ptr->rp.pclass == RACE_HUMAN) heavy = TRUE;

	/*** Sense everything ***/

	/* Scan Equipment */
	for (i = 0; i < EQUIP_MAX; i++)
	{
		o_ptr = &p_ptr->equipment[i];

		/* Skip empty slots */
		if (!o_ptr->k_idx) continue;

		sense_item(o_ptr, heavy, TRUE, TRUE);
	}

	/* Scan inventory */
	OBJ_ITT_DFS_START (p_ptr->inventory, o_ptr)
	{
		sense_item(o_ptr, heavy, FALSE, TRUE);
    /* HACK only sense one layer of pack contents - Brett */
    if (o_ptr->tval == TV_CONTAINER) {
      object_type *o2_ptr;
	    OBJ_ITT_START (o_ptr->contents_o_idx, o2_ptr)
	    {
    		sense_item(o2_ptr, heavy, FALSE,TRUE);
	    }
	    OBJ_ITT_END;
    }
	}
	OBJ_ITT_END;
}


/*
 * Go to any level (ripped off from wiz_jump)
 */
static void pattern_teleport(void)
{
	int min_level = 0;
	int max_level = 99;

	/* Ask for level */
	if (get_check("Teleport level? "))
	{
		char tmp_val[160];

		/* Only downward in ironman mode */
		if (ironman_downward)
			min_level = p_ptr->depth;

		/* Maximum level */
		if (p_ptr->depth > 100)
			max_level = dungeon()->max_level;
		else if (p_ptr->depth == 100)
			max_level = 100;

		/* Default */
		strnfmt(tmp_val, 160, "%d", p_ptr->depth);

		/* Ask for a level */
		if (!get_string(tmp_val, 11, "Teleport to level (%d-%d): ",
						min_level, max_level)) return;

		/* Extract request */
		p_ptr->cmd.arg = atoi(tmp_val);
	}
	else if (get_check("Normal teleport? "))
	{
		teleport_player(200);
		return;
	}
	else
	{
		return;
	}

	/* Paranoia */
	if (p_ptr->cmd.arg < min_level) p_ptr->cmd.arg = min_level;

	/* Paranoia */
	if (p_ptr->cmd.arg > max_level) p_ptr->cmd.arg = max_level;

	/* Accept request */
	msgf("You teleport to dungeon level %d.", p_ptr->cmd.arg);

	/* Change level */
	p_ptr->depth = p_ptr->cmd.arg;

	/* Leaving */
	p_ptr->state.leaving = TRUE;
}


static void wreck_the_pattern(void)
{
	int px = p_ptr->px;
	int py = p_ptr->py;

	int to_ruin, r_y, r_x;

	if (area(px, py)->feat == FEAT_PATTERN_XTRA2)
	{
		/* Ruined already */
		return;
	}

	msgf("You bleed on the Pattern!");
	msgf("Something terrible happens!");

	if (!query_timed(TIMED_INVULN))
		take_hit(damroll(10, 8), "corrupting the Pattern");

	to_ruin = rand_range(35, 80);

	while (to_ruin--)
	{
		scatter(&r_x, &r_y, px, py, 4);

		if ((area(r_x, r_y)->feat >= FEAT_PATTERN_START) &&
			(area(r_x, r_y)->feat < FEAT_PATTERN_XTRA2))
		{
			cave_set_feat(r_x, r_y, FEAT_PATTERN_XTRA2);
		}
	}

	cave_set_feat(px, py, FEAT_PATTERN_XTRA2);
}


/*
 * Returns TRUE if we are on the Pattern...
 */
static bool pattern_effect(void)
{
	cave_type *c_ptr = area(p_ptr->px, p_ptr->py);

	if ((c_ptr->feat < FEAT_PATTERN_START) ||
		(c_ptr->feat > FEAT_PATTERN_XTRA2))
		return FALSE;

	if ((p_ptr->rp.prace == RACE_AMBERITE) && (query_timed(TIMED_CUT) > 0) && one_in_(10))
	{
		wreck_the_pattern();
	}

	if (c_ptr->feat == FEAT_PATTERN_END)
	{
		(void)clear_poisoned();
		(void)clear_image();
		(void)clear_stun();
		(void)clear_cut();
		(void)clear_blind();
		(void)clear_afraid();
		(void)do_res_stat(A_STR);
		(void)do_res_stat(A_INT);
		(void)do_res_stat(A_WIS);
		(void)do_res_stat(A_DEX);
		(void)do_res_stat(A_CON);
		(void)do_res_stat(A_CHR);
		(void)restore_level();
		(void)hp_player(1000);
		cave_set_feat(p_ptr->px, p_ptr->py, FEAT_PATTERN_OLD);
		msgf("This section of the Pattern looks less powerful.");
	}


	/*
	 * We could make the healing effect of the
	 * Pattern center one-time only to avoid various kinds
	 * of abuse, like luring the win monster into fighting you
	 * in the middle of the pattern...
	 */

	else if (c_ptr->feat == FEAT_PATTERN_OLD)
	{
		/* No effect */
	}
	else if (c_ptr->feat == FEAT_PATTERN_XTRA1)
	{
		pattern_teleport();
	}
	else if (c_ptr->feat == FEAT_PATTERN_XTRA2)
	{
		if (!query_timed(TIMED_INVULN))
			take_hit(200, "walking the corrupted Pattern");
	}
	else
	{
		if ((p_ptr->rp.prace == RACE_AMBERITE) && one_in_(2))
			return TRUE;
		else if (!query_timed(TIMED_INVULN))
			take_hit(damroll(1, 3), "walking the Pattern");
	}

	return TRUE;
}


/*
 * Regenerate hit points				-RAK-
 */
static void regenhp(int percent)
{
	u32b new_chp, new_chp_frac;
	int old_chp;

	/* Hack: Don't do this for liches. */
	if (p_ptr->state.lich) return;

	/* Save the old hitpoints */
	old_chp = p_ptr->chp;

	/* Extract the new hitpoints */
	new_chp = ((u32b)p_ptr->mhp) * percent + PY_REGEN_HPBASE;
	p_ptr->chp += (s16b)(new_chp >> 16);	/* div 65536 */

	/* check for overflow */
	if ((p_ptr->chp < 0) && (old_chp > 0)) p_ptr->chp = MAX_SHORT;
	new_chp_frac = (new_chp & 0xFFFF) + p_ptr->chp_frac;	/* mod 65536 */
	if (new_chp_frac >= 0x10000L)
	{
		p_ptr->chp_frac = (u16b)(new_chp_frac - 0x10000L);
		p_ptr->chp++;
	}
	else
	{
		p_ptr->chp_frac = (u16b)new_chp_frac;
	}

	/* Fully healed */
	if (p_ptr->chp >= p_ptr->mhp)
	{
		p_ptr->chp = p_ptr->mhp;
		p_ptr->chp_frac = 0;
	}

	/* Notice changes */
	if (old_chp != p_ptr->chp)
	{
		/* Redraw */
		p_ptr->redraw |= (PR_HP);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);
	}
}


/*
 * Regenerate mana points				-RAK-
 */
static void regenmana(int percent)
{
	u32b new_mana, new_mana_frac;
	int old_csp;

	old_csp = p_ptr->csp;
	new_mana = ((u32b)p_ptr->msp) * percent + PY_REGEN_MNBASE;
	p_ptr->csp += (s16b)(new_mana >> 16);	/* div 65536 */

	/* check for overflow */
	if ((p_ptr->csp < 0) && (old_csp > 0))
	{
		p_ptr->csp = MAX_SHORT;
	}
	new_mana_frac = (new_mana & 0xFFFF) + p_ptr->csp_frac;	/* mod 65536 */
	if (new_mana_frac >= 0x10000L)
	{
		p_ptr->csp_frac = (u16b)(new_mana_frac - 0x10000L);
		p_ptr->csp++;
	}
	else
	{
		p_ptr->csp_frac = (u16b)(new_mana_frac);
	}

	/* Must set frac to zero even if equal */
	if (p_ptr->csp >= p_ptr->msp)
	{
		p_ptr->csp = p_ptr->msp;
		p_ptr->csp_frac = 0;
	}

	/* Redraw mana */
	if (old_csp != p_ptr->csp)
	{
		/* Redraw */
		p_ptr->redraw |= (PR_MANA);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);
		p_ptr->window |= (PW_SPELL);
	}
}


/*
 * Regenerate the monsters (once per 100 game turns)
 *
 * XXX XXX XXX Should probably be done during monster turns.
 */
static void regen_monsters(void)
{
	int i, frac;


	/* Regenerate everyone */
	for (i = 1; i < m_max; i++)
	{
		/* Check the i'th monster */
		monster_type *m_ptr = &m_list[i];
		monster_race *r_ptr = &r_info[m_ptr->r_idx];


		/* Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Skip imprisoned monsters */
		if (m_ptr->imprisoned) continue;

		/* Allow regeneration (if needed) */
		if (m_ptr->hp < m_ptr->maxhp)
		{
			/* Hack -- Base regeneration */
			frac = m_ptr->maxhp / 100;

			/* Hack -- Minimal regeneration rate */
			if (!frac) frac = 1;

			/* Hack -- Some monsters regenerate quickly */
			if (FLAG(r_ptr, RF_REGENERATE)) frac *= 2;

			/* Hack -- Regenerate */
			m_ptr->hp += frac;

			/* Do not over-regenerate */
			if (m_ptr->hp > m_ptr->maxhp) m_ptr->hp = m_ptr->maxhp;

			/* Redraw (later) if needed */
			if (p_ptr->health_who == i) p_ptr->redraw |= (PR_HEALTH);
		}
	}
}


void notice_lite_change(object_type *o_ptr)
{
	/* Hack -- notice interesting fuel steps */
	if ((o_ptr->timeout < 100) || (!(o_ptr->timeout % 100)))
	{
		/* Notice changes */
		notice_equip();
	}

	/* Hack -- Special treatment when blind */
	if (query_timed(TIMED_BLIND))
	{
		/* Hack -- save some light for later */
		if (o_ptr->timeout == 0) o_ptr->timeout++;
	}

	/* The light is now out */
	else if (o_ptr->timeout == 0)
	{
		disturb(FALSE);
		msgf("Your light has gone out!");

		/* Calculate torch radius */
		p_ptr->update |= (PU_TORCH);
	}

	/* The light is getting dim */
	else if ((o_ptr->timeout < 100) && (!(o_ptr->timeout % 10)))
	{
		if (disturb_minor) disturb(FALSE);
		msgf("Your light is growing faint.");
	}
}

static bool item_tester_unsensed(const object_type *o_ptr)
{
	object_kind *k_ptr = &k_info[o_ptr->k_idx];

	/* Check to see if we have identified the item */
	if (object_known_p(o_ptr)) return (FALSE);

	/* Cannot re-sense items that already have a strong-pseudo feeling */
	if (o_ptr->feeling >= FEEL_BROKEN && o_ptr->feeling <= FEEL_TAINTED) return (FALSE);

	/* Cannot sense flavoured items */
	if (k_ptr->flavor) return (FALSE);

	return (TRUE);
}


/*
 * Forcibly pseudo-identify an object in the inventory
 * (or on the floor)
 */
bool psychometry(void)
{
	object_type *o_ptr;
	byte feel;
	cptr q, s;

	/* Only un-id'ed items */
	item_tester_hook = item_tester_unsensed;

	/* Get an item */
	q = (p_ptr->rp.pclass == CLASS_MINDCRAFTER ? "Meditate on which item? " : "Focus on which item? ");
	s = "You have nothing appropriate.";

	o_ptr = get_item(q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR), (USE_INVEN));

	/* Not a valid item */
	if (!o_ptr) return (FALSE);

	/* Check for a feeling */
	feel = value_check_aux1(o_ptr);

	/* Skip non-feelings */
	if (!feel)
	{
		msgf("You do not find anything unusual about the %v.",
				   OBJECT_FMT(o_ptr, FALSE, 0));
		return TRUE;
	}

	msgf("You feel that the %v %s %s...",
			   OBJECT_FMT(o_ptr, FALSE, 0),
			   ((o_ptr->number == 1) ? "is" : "are"),
			   game_inscriptions[feel]);

	/* We have "felt" it */
	o_ptr->info |= (OB_SENSE);

	/* hack the knowledge flag */
	if (cursed_p(o_ptr)) o_ptr->kn_flags[2] |= TR2_CURSED;

	/* "Inscribe" it */
	o_ptr->feeling = feel;

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);

	/* Notice changes */
	notice_item();

	/* Something happened */
	return (TRUE);
}


/*
 * If player has inscribed the object with "!!", let him know when it's
 * recharged. -LM-
 */
static void recharged_notice(const object_type *o_ptr)
{
	cptr s;

	/* No inscription */
	if (!o_ptr->inscription) return;

	/* Find a '!' */
	s = strchr(quark_str(o_ptr->inscription), '!');

	/* Process notification request. */
	while (s)
	{
		/* Find another '!' */
		if (s[1] == '!')
		{
			/* Count the item */
			if (o_ptr->number == 1)
			{
				/* One item has recharged */
				msgf("Your %v has recharged.", OBJECT_FMT(o_ptr, FALSE, 0));
			}
			else
			{
				object_kind *k_ptr = &k_info[o_ptr->k_idx];

				/* Find out how many are recharging still */
				int power = (o_ptr->timeout + (k_ptr->pval - 1)) / k_ptr->pval;

				/* Watch the top */
				if (power > o_ptr->number) power = o_ptr->number;

				/* All are charged now */
				if (power == 0)
				{
					msgf("All %d of your %v are charged.", o_ptr->number,
						OBJECT_FMT(o_ptr, FALSE, 0));
				}
				/* One is charged */
				else if (o_ptr->number - power == 1)
				{
					msgf("One of your %v is charged.",
						OBJECT_FMT(o_ptr, FALSE, 0));
				}
				/* Some are charged, some are recharging */
				else
				{
					msgf("%d of your %v are charged.", o_ptr->number - power,
						OBJECT_FMT(o_ptr, FALSE, 0));
				}
			}

			/* Disturb the player */
			disturb(FALSE);

			/* Done. */
			return;
		}

		/* Keep looking for '!'s */
		s = strchr(s + 1, '!');
	}
}

/*
 * Damaging liquids have damage adjusted for resistance in
 * two ways: first, these functions are used to calculate an
 * additive adjustment, then normal damage resistance.  The
 * effect is that having some resistance reduces the damage
 * enough that often these features do no damage.  This is
 * good; it makes the terrain less difficult to deal with.
 */
static int dam_adjust(int resistance)
{
	if (!resistance) return 0;  /* Won't matter */

	else return (2*(100/resistance)-2);
}

static int dam_fire_adjust(void)
{
	return dam_adjust(res_fire_lvl());
}

static int dam_acid_adjust(void)
{
	return dam_adjust(res_acid_lvl());
}

static int dam_pois_adjust(void)
{
	return dam_adjust(res_pois_lvl());
}

static int dam_cold_adjust(void)
{
	return dam_adjust(res_cold_lvl());
}

static int dam_elec_adjust(void)
{
	return dam_adjust(res_elec_lvl());
}

/*
 * Handle certain things once every 10 game turns
 */
static void process_world(void)
{
	int i;
	s32b regen_amount;
	bool cave_no_regen = FALSE;
	int upkeep_factor = 0;

	u16b x, y;

	object_type *o_ptr;
	int temp;
	object_kind *k_ptr;
  feature_type *f_ptr;

	cave_type *c_ptr = area(p_ptr->px, p_ptr->py);
	const mutation_type *mut_ptr;

	int depth = base_level();
	int adj_depth = (p_ptr->depth ? p_ptr->depth : MIN(20, depth));

	/* Announce the level feeling */
	if ((turn - old_turn == 1000) && (p_ptr->depth)) do_cmd_feeling();

	/* Every 10 game turns */
	if (turn % 10) return;

	/*** Attempt timed autosave ***/
	if (autosave_t && autosave_freq)
	{
		if (!(turn % ((s32b)autosave_freq * 10)))
			do_cmd_save_game(TRUE);
	}

	if (p_ptr->state.mon_fight)
	{
		msgf("You hear noise.");
	}

	/*** Handle the wilderness/town (sunshine) ***/

	/* While in town/wilderness */
	if (!p_ptr->depth)
	{
		/* Hack -- Daybreak/Nighfall/change of ambient light in wilderness */
		if (!(turn % TOWN_HALF_HOUR))
		{
			bool dawn = FALSE;
			bool dusk = FALSE;

			/* Redraw time on status bar */
			p_ptr->redraw |= (PR_TIME);

			/* Check for dawn */
			dawn = (!(turn % TOWN_DAY));
			dusk = (!dawn && !(turn % TOWN_HALF_DAY));

			/* Day breaks */
			if (dawn)
			{
				/* Message */
				msgf("The sun has risen.");
			}
			else if (dusk)
			{
				/* Message */
				msgf("The sun has fallen.");
			}
			if (dawn || dusk)
				disturb(FALSE);

			p_ptr->update |= PU_TORCH;
			update_stuff();

			/* Light up or darken the area */
			for (y = p_ptr->min_hgt; y < p_ptr->max_hgt; y++)
			{
				for (x = p_ptr->min_wid; x < p_ptr->max_wid; x++)
				{
					if (dawn || dusk) light_dark_square(x, y, dawn);
					/* Hack! */
					else lite_spot(x,y);
				}
			}

			if (dawn || dusk)
			{

				/* Update the monsters */
				p_ptr->update |= (PU_MONSTERS | PU_VIEW);

				/* Redraw map */
				p_ptr->redraw |= (PR_MAP);

				/* Window stuff */
				p_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);
			}
		}
	}


	/*** Process the monsters ***/

	/* Check for creature generation. */
	if (one_in_(MAX_M_ALLOC_CHANCE) && !current_quest)
	{
		/* Make a new monster */
		(void)alloc_monster(MAX_SIGHT + 5, FALSE, 0);
	}

	/* Hack -- Check for creature regeneration */
	if (!(turn % 100)) regen_monsters();

	/*** Damage over Time ***/

	/* Take damage from poison */
	if (query_timed(TIMED_POISONED) && !query_timed(TIMED_INVULN))
	{
		/* Take damage */
		take_hit(1, "poison");
	}

	/* (Vampires) Take damage from sunlight */
	if (FLAG(p_ptr, TR_HURT_LITE))
	{
		if (!p_ptr->depth && !(FLAG(p_ptr, TR_RES_LITE)) &&
			!(FLAG(p_ptr, TR_IM_LITE)) &&
			!query_timed(TIMED_INVULN) &&
			(!((turn / TOWN_HALF_DAY) % 2)))
		{
			if (c_ptr->info & CAVE_GLOW)
			{
				/* Can't regenerate well */
				cave_no_regen = TRUE;
			}
		}

		o_ptr = &p_ptr->equipment[EQUIP_LITE];

		if (o_ptr->tval &&
			(o_ptr->sval != SV_LITE_TORCH) &&
			(o_ptr->sval != SV_LITE_LANTERN) &&
			!(FLAG(p_ptr, TR_RES_LITE)) &&
			!(FLAG(p_ptr, TR_IM_LITE)))
		{
			char ouch[280];

			msgf("The %v scorches your undead flesh!", OBJECT_FMT(o_ptr, FALSE, 0));

			cave_no_regen = TRUE;

			strnfmt(ouch, 280, "wielding %v", OBJECT_FMT(o_ptr, TRUE, 0));

			if (!query_timed(TIMED_INVULN)) take_hit(1, ouch);
		}
	}

  f_ptr = &(f_info[c_ptr->feat]);
  if ((f_ptr->flags & FF_DAMAGING) && !query_timed(TIMED_INVULN)) {
    int damage = 0;
		cptr message;
		//cptr hit_from;
    if (f_ptr->flags & FF_DEEP) {
      if (f_ptr->flags2 & FF_FIERY) {
		    damage = resist(adj_depth-dam_fire_adjust(), res_fire_lvl);
		    if (FLAG(p_ptr, TR_FEATHER))
		    {
			    damage = damage / 5;

			    message = "The heat of the %s burns you!";
		    } else {
			    message = "The %s burns you!";
		    }
      } else
      if (f_ptr->flags2 & FF_ICY) {
		    damage = resist(adj_depth-dam_cold_adjust(), res_cold_lvl);
		    if (FLAG(p_ptr, TR_FEATHER))
		    {
			    damage = damage / 5;

			    message = "The cold of the %s chills you!";
		    } else {
			    message = "The %s freezes you!";
		    }
      } else
      if (f_ptr->flags2 & FF_ACID) {
		    damage = resist(adj_depth-dam_acid_adjust(), res_acid_lvl);
		    if (FLAG(p_ptr, TR_FEATHER))
		    {
			    damage = damage / 5;

			    message = "The fumes of the %s burn you!";
		    } else {
			    message = "The %s burns you!";
		    }
      } else
      if (f_ptr->flags2 & FF_ELEC) {
		    damage = resist(adj_depth-dam_elec_adjust(), res_elec_lvl);
		    if (FLAG(p_ptr, TR_FEATHER))
		    {
			    damage = damage / 5;

			    message = "The heat of the %s shocks you!";
		    } else {
			    message = "The %s shocks you!";
		    }
      } else
      if (f_ptr->flags2 & FF_POISON) {
		    damage = resist((adj_depth / 2 + 1) - dam_pois_adjust(), res_pois_lvl);
		    if (FLAG(p_ptr, TR_FEATHER)) {
			    damage = damage / 5;
		    }
		    message = "The fumes of the %s poison you!";
      } else
      {
		    damage = (adj_depth / 3 + 1);
		    message = "The %s hurts you!";
      }
    } else 
    if (!(FLAG(p_ptr, TR_FEATHER))) {
      if (f_ptr->flags2 & FF_FIERY) {
		    damage = resist((adj_depth / 2 + 1)-dam_fire_adjust(), res_fire_lvl);
		    message = "The %s burns you!";
      } else
      if (f_ptr->flags2 & FF_ICY) {
		    damage = resist((adj_depth / 2 + 1)-dam_cold_adjust(), res_cold_lvl);
		    message = "The %s freezes you!";
      } else
      if (f_ptr->flags2 & FF_ACID) {
		    damage = resist((adj_depth / 2 + 1)-dam_acid_adjust(), res_acid_lvl);
		    message = "The %s burns you!";
      } else
      if (f_ptr->flags2 & FF_ELEC) {
		    damage = resist((adj_depth / 2 + 1)-dam_elec_adjust(), res_elec_lvl);
		    message = "The %s shocks you!";
      } else
      if (f_ptr->flags2 & FF_POISON) {
		    damage = resist((adj_depth / 4 + 1) - dam_pois_adjust(), res_pois_lvl);
		    message = "The fumes of the %s poison you!";
      } else
      {
		    damage = (adj_depth / 6 + 1);
		    message = "The %s hurts you!";
      }
    }
 		if (damage && (f_ptr->flags & FF_WILD) && FLAG(p_ptr, TR_WILD_WALK)) {
      damage = 0;
    }
 		if (damage && (f_ptr->flags & FF_ROCKY) && FLAG(p_ptr, TR_PASS_WALL)) {
      damage = 0;
    }
 		if (damage) {
			/* Take damage */
			msgf(message, f_name + f_ptr->name);
			take_hit(damage, f_name + f_ptr->name);

			cave_no_regen = TRUE;
		}
  }

  if ((f_ptr->flags & FF_LIQUID) && (f_ptr->flags & FF_DEEP)
    && !(FLAG(p_ptr, TR_FEATHER))) {
 		if (p_ptr->total_weight >
			((adj_str_wgt[p_ptr->stat[A_STR].ind] * 100) / 2))
		{
			/* Take damage */
			msgf("You are drowning!");
			take_hit(randint1(adj_depth + 1), "drowning");
			cave_no_regen = TRUE;
		}
  }

#if (0)
  if ((c_ptr->feat == FEAT_SHAL_LAVA) && !(FLAG(p_ptr, TR_FEATHER)))
	{
		int damage = resist((adj_depth / 2 + 1)-dam_fire_adjust(), res_fire_lvl);

		if (damage)
		{
			/* Take damage */
			msgf("The lava burns you!");
			take_hit(damage, "shallow lava");
			cave_no_regen = TRUE;
		}
	}

	else if (c_ptr->feat == FEAT_DEEP_LAVA)
	{
		int damage = resist(adj_depth-dam_fire_adjust(), res_fire_lvl);
		cptr message;
		cptr hit_from;

		if (FLAG(p_ptr, TR_FEATHER))
		{
			damage = damage / 5;

			message = "The heat burns you!";
			hit_from = "flying over deep lava";
		}
		else
		{
			message = "The lava burns you!";
			hit_from = "deep lava";
		}

		if (damage)
		{
			/* Take damage */
			msgf(message);
			take_hit(damage, hit_from);

			cave_no_regen = TRUE;
		}
	}

	if ((c_ptr->feat == FEAT_SHAL_ACID) && !(FLAG(p_ptr, TR_FEATHER)))
	{
		int damage = resist(adj_depth / 2 + 1-dam_acid_adjust(), res_acid_lvl);

		if (damage)
		{
			/* Take damage */
			msgf("The acid burns you!");
			take_hit(damage, "shallow acid");
			cave_no_regen = TRUE;
		}
	}

	else if (c_ptr->feat == FEAT_DEEP_ACID)
	{
		int damage = resist(adj_depth-dam_acid_adjust(), res_acid_lvl);
		cptr message;
		cptr hit_from;

		if (FLAG(p_ptr, TR_FEATHER))
		{
			damage = damage / 5;

			message = "The fumes burn you!";
			hit_from = "flying over deep acid";
		}
		else
		{
			message = "The acid burns you!";
			hit_from = "deep acid";
		}

		if (damage)
		{
			/* Take damage */
			msgf(message);
			take_hit(damage, hit_from);

			cave_no_regen = TRUE;
		}
	}

	if ((c_ptr->feat == FEAT_SHAL_SWAMP) &&	!(FLAG(p_ptr, TR_FEATHER)) &&
		!FLAG(p_ptr, TR_WILD_WALK))
	{
		int damage = resist(adj_depth / 4 + 1-dam_pois_adjust(), res_pois_lvl);

		/* Hack - some resistance will save you */
		if (damage && damage >= p_ptr->depth / 4)
		{
			/* Take damage */
			msgf("The plants poison you!");
			take_hit(damage, "swamp");
			cave_no_regen = TRUE;
		}
	}

	else if ((c_ptr->feat == FEAT_DEEP_SWAMP) && !query_timed(TIMED_INVULN) &&
		!FLAG(p_ptr, TR_WILD_WALK))
	{
		int damage = resist(adj_depth / 2 - dam_pois_adjust(), res_pois_lvl);
		cptr message;
		cptr hit_from;

		if (FLAG(p_ptr, TR_FEATHER))
		{
			damage = damage / 5;

			message = "The fumes poison you!";
			hit_from = "flying over thick swamp";
		}
		else
		{
			message = "The fumes poison you!";
			hit_from = "thick swamp";
		}

		if (damage)
		{
			/* Take damage */
			msgf(message);
			take_hit(damage, hit_from);

			cave_no_regen = TRUE;
		}
	}
	else if (((c_ptr->feat == FEAT_DEEP_WATER) ||
			  (c_ptr->feat == FEAT_OCEAN_WATER)) &&
			  !(FLAG(p_ptr, TR_FEATHER)))
	{
		if (p_ptr->total_weight >
			((adj_str_wgt[p_ptr->stat[A_STR].ind] * 100) / 2))
		{
			/* Take damage */
			msgf("You are drowning!");
			take_hit(randint1(adj_depth + 1), "drowning");
			cave_no_regen = TRUE;
		}
	}
#endif
	/* Spectres -- take damage when moving through walls */
	/*
	 * Added: ANYBODY takes damage if inside through walls
	 * without wraith form -- NOTE: Spectres will never be
	 * reduced below 0 hp by being inside a stone wall; others
	 * WILL BE!
	 */
	if (cave_wall_grid(c_ptr))
	{
		if (!query_timed(TIMED_INVULN) && !query_timed(TIMED_WRAITH_FORM) &&
			((p_ptr->chp > (depth / 10)) ||
			 !(FLAG(p_ptr, TR_PASS_WALL))))
		{
			cptr dam_desc;
			int dmg = (depth / 5);

			cave_no_regen = TRUE;

			if (FLAG(p_ptr, TR_PASS_WALL))
			{
				msgf("Your molecules feel disrupted!");
				dam_desc = "density";
				dmg /= 2;
			}
			else
			{
				msgf("You are being crushed!");
				dam_desc = "solid rock";
			}

			take_hit(dmg+1, dam_desc);
		}
	}

	/*
	 * Fields you are standing on may do something.
	 */
	field_script(c_ptr, FIELD_ACT_PLAYER_ON, "");

	/* Nightmare mode activates the TY_CURSE at midnight */
	if (ironman_nightmare)
	{
		s32b len = TOWN_DAY;
		s32b tick = turn % len + len / 4;

		int hour = (24 * tick / len) % 24;
		int min = (1440 * tick / len) % 60;
		int prev_min = (1440 * (tick - 10) / len) % 60;

		/* Require exact minute */
		if (min != prev_min)
		{
			/* Every 15 minutes after 11:00 pm */
			if ((hour == 23) && !(min % 15))
			{
				/* Disturbing */
				disturb(FALSE);

				switch (min / 15)
				{
					case 0:
					{
						msgf("You hear a distant bell toll ominously.");
						break;
					}
					case 1:
					{
						msgf("A distant bell sounds twice.");
						break;
					}
					case 2:
					{
						msgf("A distant bell sounds three times.");
						break;
					}
					case 3:
					{
						msgf("A distant bell tolls four times.");
						break;
					}
				}
			}

			/* TY_CURSE activates at mignight! */
			if (!hour && !min)
			{
				int count = 0;

				disturb(TRUE);
				msgf
					("A distant bell tolls many times, fading into an deathly silence.");
				(void)activate_ty_curse(FALSE, &count);
			}
		}
	}

	/* Take damage from cuts */
	if (query_timed(TIMED_CUT) && !query_timed(TIMED_INVULN))
	{
		/* Mortal wound or Deep Gash */
		if (query_timed(TIMED_CUT) > 200)
		{
			i = 3;
		}

		/* Severe cut */
		else if (query_timed(TIMED_CUT) > 100)
		{
			i = 2;
		}

		/* Other cuts */
		else
		{
			i = 1;
		}

		/* Take damage */
		take_hit(i, "a fatal wound");
	}


	/*** Check the Food, and Regenerate ***/

	/* Digest normally */
	if (p_ptr->food < PY_FOOD_MAX)
	{
		/* Every 100 game turns */
		if (!(turn % 100))
		{
			/* Basic digestion rate based on speed */
			if (p_ptr->pspeed > 199) i = 49;
			else if (p_ptr->pspeed < 0) i = 1;
			else
				i = extract_energy[p_ptr->pspeed];

			i *= 2;

			/* Regeneration takes more food */
			if (FLAG(p_ptr, TR_REGEN)) i += 30;

			/* Some specific mutations increase food requirement */
			if (p_ptr->muta3 & (MUT3_RESILIENT)) i += 20;
			if (p_ptr->muta1 & (MUT1_EAT_ROCK)) i += 20;

			/* Slow digestion takes less food */
			if (FLAG(p_ptr, TR_SLOW_DIGEST)) i -= 10;

			/* Slow healing gives some benefit... */
			if (FLAG(p_ptr, TR_SLOW_HEAL)) i -= 5;

			/* Wasting disease - almost no digestion */
			if (p_ptr->muta2 & MUT2_WASTING) i -= 50;

			/* Minimal digestion */
			if (i < 1) i = 1;

			/* Digest some food */
			(void)set_food(p_ptr->food - i);
		}
	}

	/* Digest quickly when gorged */
	else
	{
		/* Digest a lot of food */
		(void)set_food(p_ptr->food - 100);
	}

	/* Starve to death (slowly) */
	if (p_ptr->food < PY_FOOD_STARVE)
	{
		/* Calculate damage */
		i = (PY_FOOD_STARVE - p_ptr->food) / 10;

		/* Take damage */
		if (!query_timed(TIMED_INVULN)) take_hit(i, "starvation");
	}

	/* Default regeneration */
	regen_amount = PY_REGEN_NORMAL;

	/* Getting Weak */
	if (p_ptr->food < PY_FOOD_WEAK)
	{
		/* Lower regeneration */
		if (p_ptr->food < PY_FOOD_STARVE)
		{
			regen_amount = 0;
		}
		else if (p_ptr->food < PY_FOOD_FAINT)
		{
			regen_amount = PY_REGEN_FAINT;
		}
		else
		{
			regen_amount = PY_REGEN_WEAK;
		}

		/* Getting Faint */
		if (p_ptr->food < PY_FOOD_FAINT)
		{
			/* Faint occasionally */
			if (!query_timed(TIMED_PARALYZED) && (randint0(100) < 10))
			{
				/* Message */
				msgf("You faint from the lack of food.");
				disturb(TRUE);

				/* Hack -- faint (bypass free action) */
				(void)inc_paralyzed(randint1(5));
			}
		}
	}


	/* Are we walking the pattern? */
	if (pattern_effect())
	{
		cave_no_regen = TRUE;
	}
	else
	{
		/* Regeneration ability */
		if (FLAG(p_ptr, TR_REGEN))
		{
			regen_amount = regen_amount * 2;
		}

		if (FLAG(p_ptr, TR_SLOW_HEAL))
		{
			regen_amount = regen_amount / 4;
		}

	}


	/* Searching or Resting */
	if ((p_ptr->state.searching == SEARCH_MODE_SEARCH)|| p_ptr->state.resting)
	{
		regen_amount = regen_amount * 2;
	}

	if (total_friends && use_upkeep)
	{
		if (total_friends > 1 + (p_ptr->lev / cp_ptr->pet_upkeep_div))
		{
			upkeep_factor = total_friend_levels;

			/* Bounds checking */
			if (upkeep_factor > 95) upkeep_factor = 95;
			if (upkeep_factor < 5) upkeep_factor = 5;
		}
	}

	/* Regenerate the mana */
	if (p_ptr->csp < p_ptr->msp)
	{
		if (upkeep_factor)
		{
			s16b upkeep_regen = (((100 - upkeep_factor) * regen_amount) / 100);
			regenmana(upkeep_regen);
		}
		else
		{
			regenmana(regen_amount);
		}
	}

	/* Poisoned or cut yields no healing */
	if (query_timed(TIMED_POISONED) || query_timed(TIMED_CUT)) regen_amount = 0;

	/* Special floor -- Pattern, in a wall -- yields no healing */
	if (cave_no_regen) regen_amount = 0;

	/* Regenerate Hit Points */
	if (p_ptr->chp < p_ptr->mhp)
	{
		regenhp(regen_amount);
	}


	/*** Timeout Various Things ***/
	if (query_timed(TIMED_XTRA_FAST)) (void)inc_xtra_fast(-1);  /* Do this one first */
	if (query_timed(TIMED_XTRA_INVIS)) (void)inc_tim_xtra_invisible(-1);  /* Do first also */
	if (query_timed(TIMED_IMAGE)) (void)inc_image(-1);
	if (query_timed(TIMED_BLIND)) (void)inc_blind(-1);
	if (query_timed(TIMED_ESP)) (void)inc_tim_esp(-1);
	if (query_timed(TIMED_INFRA)) (void)inc_tim_infra(-1);
	if (query_timed(TIMED_PARALYZED)) (void)inc_paralyzed(-1);
	if (query_timed(TIMED_CONFUSED)) (void)inc_confused(-1);
	if (query_timed(TIMED_AFRAID)) (void)inc_afraid(-1);
	if (query_timed(TIMED_FAST)) (void)inc_fast(-1);
	if (query_timed(TIMED_SLOW)) (void)inc_slow(-1);
	if (query_timed(TIMED_PROTEVIL)) (void)inc_protevil(-1);
	if (query_timed(TIMED_INVULN)) (void)inc_invuln(-1);
	if (query_timed(TIMED_WRAITH_FORM)) (void)inc_wraith_form(-1);
	if (query_timed(TIMED_HERO)) (void)inc_hero(-1);
	if (query_timed(TIMED_SHERO)) (void)inc_shero(-1);
	if (query_timed(TIMED_BLESSED)) (void)inc_blessed(-1);
	if (query_timed(TIMED_SHIELD)) (void)inc_shield(-1);
	if (query_timed(TIMED_OPPOSE_ACID)) (void)inc_oppose_acid(-1);
	if (query_timed(TIMED_OPPOSE_ELEC)) (void)inc_oppose_elec(-1);
	if (query_timed(TIMED_OPPOSE_FIRE)) (void)inc_oppose_fire(-1);
	if (query_timed(TIMED_OPPOSE_COLD)) (void)inc_oppose_cold(-1);
	if (query_timed(TIMED_OPPOSE_POIS)) (void)inc_oppose_pois(-1);
	if (query_timed(TIMED_OPPOSE_CONF)) (void)inc_oppose_conf(-1);
	if (query_timed(TIMED_OPPOSE_BLIND)) (void)inc_oppose_blind(-1);
	if (query_timed(TIMED_ETHEREALNESS)) (void)inc_etherealness(-1);
	if (query_timed(TIMED_LUMINOSITY)) (void)inc_luminosity(-1);
	if (query_timed(TIMED_SEE_INVIS)) (void)inc_tim_invis(-1);
	if (query_timed(TIMED_STR)) (void)inc_tim_str(-1);
	if (query_timed(TIMED_CHR)) (void)inc_tim_chr(-1);
	if (query_timed(TIMED_SUST_ALL)) (void)inc_tim_sust_all(-1);
	if (query_timed(TIMED_IMMUNE_ACID)) (void)inc_immune_acid(-1);
	if (query_timed(TIMED_IMMUNE_FIRE)) (void)inc_immune_fire(-1);
	if (query_timed(TIMED_IMMUNE_COLD)) (void)inc_immune_cold(-1);
	if (query_timed(TIMED_IMMUNE_ELEC)) (void)inc_immune_elec(-1);
	if (query_timed(TIMED_SH_FIRE)) (void)inc_sh_fire(-1);
	if (query_timed(TIMED_SH_COLD)) (void)inc_sh_cold(-1);
	if (query_timed(TIMED_SH_ELEC)) (void)inc_sh_elec(-1);
	if (query_timed(TIMED_SH_ACID)) (void)inc_sh_acid(-1);
	if (query_timed(TIMED_SH_FEAR)) (void)inc_sh_fear(-1);
	if (query_timed(TIMED_INVIS)) (void)inc_tim_invisible(-1);


	/* Quest timeouts */
	for (i = 0; i < z_info->q_max; i++)
	{
		if (quest[i].timeout && turn > quest[i].timeout)
		{
			trigger_quest_fail(i);
		}
	}


	/*** Poison and Stun and Cut ***/

	/* Poison */
	if (query_timed(TIMED_POISONED))
	{
		int adjust = adj_con_fix[p_ptr->stat[A_CON].ind] + 1;

		/* Apply some healing */
		(void)inc_poisoned(-adjust);
	}

	/* Stun */
	if (query_timed(TIMED_STUN))
	{
		int adjust = adj_con_fix[p_ptr->stat[A_CON].ind] + 1;

		/* Apply some healing */
		(void)inc_stun(-adjust);
	}

	/* Cut */
	if (query_timed(TIMED_CUT))
	{
		int adjust = adj_con_fix[p_ptr->stat[A_CON].ind] + 1;

		if (FLAG(p_ptr, TR_SLOW_HEAL))
			adjust /= 2;

		/* Hack -- Truly "mortal" wound */
		if (query_timed(TIMED_CUT) > 1000) adjust = 0;

		/* Apply some healing */
		(void)inc_cut(-adjust);
	}

	/*** Process mutation effects ***/
	for (i = MUT_PER_SET; i < MUT_PER_SET * 2; i++)
	{
		mut_ptr = &mutations[i];

		/*
		 * Do we have this mutation and
		 * is it truly a randomly activating one?
		 */
		if (player_has_mut(i) && (mut_ptr->chance > 0))
		{
			mutation_random_aux(mut_ptr);
		}
	}


	/*** Process Inventory ***/

	/* Handle experience draining */
	if (FLAG(p_ptr, TR_DRAIN_EXP))
	{
		if ((randint0(100) < 10) && (p_ptr->exp > 0))
		{
			p_ptr->exp--;
			p_ptr->max_exp--;
			check_experience();
		}
	}

	/* Handle stat draining */
	if (FLAG(p_ptr, TR_DRAIN_STATS))
	{
		if (one_in_(250))
		{
			int stat = randint0(6);

			if (p_ptr->stat[stat].cur > 30)
			{
				p_ptr->stat[stat].cur--;
				p_ptr->update |= (PU_BONUS);
			}
		}
	}

	/* Process equipment */
	for (i = 0; i < EQUIP_MAX; i++)
	{
		/* Get the object */
		o_ptr = &p_ptr->equipment[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Apply extra scripts */
		apply_object_trigger(TRIGGER_TIMED, o_ptr, "");

		/* TY Curse */
		if ((FLAG(o_ptr, TR_TY_CURSE)) && one_in_(TY_CURSE_CHANCE))
		{
			int count = 0;

			(void)activate_ty_curse(FALSE, &count);
		}

		/* Auto-curse */
		if ((FLAG(o_ptr, TR_AUTO_CURSE)) &&
				!(FLAG(o_ptr, TR_CURSED)) && one_in_(1000))
		{
			msgf("There is a malignant black aura surrounding you...");
			SET_FLAG(o_ptr, TR_CURSED);
			o_ptr->feeling = FEEL_NONE;
		}

		/*
		 * Hack: Uncursed teleporting items (e.g. Trump Weapons)
		 * can actually be useful!
		 */
		if ((FLAG(o_ptr, TR_TELEPORT)) && one_in_(100))
		{
			if (cursed_p(o_ptr) && !(FLAG(p_ptr, TR_NO_TELE)))
			{
				disturb(FALSE);

				/* Teleport player */
				teleport_player(40);
			}
			else
			{
				if (!disturb_other || (o_ptr->inscription &&
									   (strchr
										(quark_str(o_ptr->inscription), '.'))))
				{
					/* Do nothing */
					/* msgf("Teleport aborted.") */ ;
				}
				else if (get_check("Teleport? "))
				{
					disturb(FALSE);
					teleport_player(50);
				}
			}
		}

		/* Recharge activatable objects */
		if (o_ptr->timeout > 0)
		{
			/* Lights are special */
			if (o_ptr->tval == TV_LITE)
			{
				/* Artifact lights decrease timeout */
				if (FLAG(o_ptr, TR_INSTA_ART))
				{
					/* Recharge */
					o_ptr->timeout--;

					if (!o_ptr->timeout)
					{
						recharged_notice(o_ptr);

						/* Notice changes */
						notice_equip();
					}
				}
				else if (!(FLAG(o_ptr, TR_LITE)))
				{
					/* Normal lights that are not everburning */
					o_ptr->timeout--;

					/* Notice interesting fuel steps */
					notice_lite_change(o_ptr);
				}
			}

			/* Notice changes */
			else
			{
				/* Recharge */
				o_ptr->timeout--;

				if (!o_ptr->timeout)
				{
					recharged_notice(o_ptr);

					/* Notice changes */
					notice_equip();
				}
			}
		}
	}

	/*
	 * Recharge rods.  Rods now use timeout to control charging status,
	 * and each charging rod in a stack decreases the stack's timeout by
	 * one per turn. -LM-
	 */
	OBJ_ITT_START (p_ptr->inventory, o_ptr)
	{
		k_ptr = &k_info[o_ptr->k_idx];

		/* Must have a timeout */
		if (!o_ptr->timeout) continue;

		/* Examine all charging rods or stacks of charging rods. */
		if (o_ptr->tval == TV_ROD)
		{
			/* Determine how many rods are charging. */
			temp = (o_ptr->timeout + (k_ptr->pval - 1)) / k_ptr->pval;
			if (temp > o_ptr->number) temp = o_ptr->number;

			/* Decrease timeout by that number. */
			o_ptr->timeout -= temp;

			/* Boundary control. */
			if (o_ptr->timeout < 0) o_ptr->timeout = 0;

			/* Notice changes, provide message if object is inscribed. */
			if (temp > (o_ptr->timeout + (k_ptr->pval - 1)) / k_ptr->pval)
			{
				recharged_notice(o_ptr);

				/* Notice changes */
				notice_inven();
			}
		}
	}
	OBJ_ITT_END;

	/* Feel the inventory */
	sense_inventory();

	/* Sometimes, check for inscribed "reminder" items */
	if (turn % 2000 == 0)
		inventory_remind();


	/*** Process Objects ***/

	/* Process objects */
	for (i = 1; i < o_max; i++)
	{
		/* Access object */
		o_ptr = &o_list[i];

		/* Skip dead objects */
		if (!o_ptr->k_idx) continue;

		/* Exit if not in dungeon */
		if (!(o_ptr->ix || o_ptr->iy)) continue;

		field_script(area(o_ptr->ix, o_ptr->iy),
				   FIELD_ACT_OBJECT_ON, "p", LUA_OBJECT(o_ptr));

		if (!o_ptr->timeout) continue;

		/* Recharge rods on the ground.  No messages. */
		if (o_ptr->tval == TV_ROD)
		{
			/* Charge it */
			o_ptr->timeout -= o_ptr->number;

			/* Boundary control. */
			if (o_ptr->timeout < 0) o_ptr->timeout = 0;
		}
	}

	/*
	 * Cycle ultra-quick R"bool"G to prevent periodic patterns
	 * in the illumination in a forest after dark.
	 */

	quick_rand_add();


	/*** Involuntary Movement ***/

	/* Delayed Word-of-Recall */
	if (query_timed(TIMED_WORD_RECALL))
	{
		//place_type *pl_ptr;

		/* Count down towards recall */
		*(get_timed_ptr(TIMED_WORD_RECALL)) = query_timed(TIMED_WORD_RECALL) - 1;

		p_ptr->redraw |= (PR_STATUS);

		/* Hack - no recalling in the middle of the wilderness */
		/*if ((!p_ptr->depth) && (!p_ptr->place_num))
		{
			if (!query_timed(TIMED_WORD_RECALL))
				msgf("You feel a momentary yank downwards, but it passes.");
			return;
		}

		pl_ptr = &place[p_ptr->place_num];*/

		/* No recalling if there's no dungeon, etc. */
		/*if (!pl_ptr->dungeon || !(pl_ptr->dungeon->recall_depth))
		{
			if (!query_timed(TIMED_WORD_RECALL))
				msgf("You feel a momentary yank downwards, but it passes.");
			return;
		}*/

		/* Activate the recall */
		if (!query_timed(TIMED_WORD_RECALL))
		{
			/* Disturbing! */
			disturb(FALSE);

			/* Leaving */
			p_ptr->state.leaving = TRUE;

			/* Determine the level */
			if (p_ptr->depth)
			{
				msgf("You feel yourself yanked upwards!");

				p_ptr->depth = 0;
			}
			else if (p_ptr->place_num)
			{
				dun_type *d_ptr = dungeon();
		    /* No recalling if there's no dungeon, etc. */
		    if (!d_ptr || !(d_ptr->recall_depth))
		    {
			    if (!query_timed(TIMED_WORD_RECALL))
				    msgf("You feel a momentary yank downwards, but it passes.");
			    return;
		    }

				msgf("You feel yourself yanked downwards!");

				/* Not lower than bottom of dungeon */
				p_ptr->depth = d_ptr->recall_depth;

				/* Go down at least to the start of the dungeon */
				if (p_ptr->depth < d_ptr->min_level)
				{
					p_ptr->depth = d_ptr->min_level;
				}

				/* Nightmare mode makes recall more dangerous */
				if (ironman_nightmare && one_in_(666))
				{
					if (p_ptr->depth < 50)
					{
						p_ptr->depth *= 2;
					}
					else if (p_ptr->depth < 99)
					{
						p_ptr->depth = (p_ptr->depth + 99) / 2;
					}
					else if (p_ptr->depth > 100)
					{
						p_ptr->depth = MAX_DEPTH - 1;
					}

					if (p_ptr->depth > d_ptr->max_level)
					{
						p_ptr->depth = d_ptr->max_level;
					}
				}
      } else if (p_ptr->home_place_num && p_ptr->home_store_num)
      {
        /* Move the player from its current wilderness location to its primary home
         * Copied from building_magetower() - Brett */

        place_type *pl_ptr2 = &place[p_ptr->home_place_num];
				store_type *st_ptr2 = &pl_ptr2->store[p_ptr->home_store_num];

				msgf("You feel yourself pulled home!");

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
		    msgf("You lose concentration wondering where to go and the tension leaves the air around you...");
		    p_ptr->redraw |= (PR_STATUS);
			}

			/* Sound */
			sound(SOUND_TPLEVEL);
		}
	}
}



/*
 * Parse and execute the current command
 * Give "Warning" on illegal commands.
 *
 * XXX XXX XXX Make some "blocks"
 */
void process_command(void);
void process_click(char press, int xpos, int ypos);



/*
 * Process the player
 *
 * Notice the annoying code to handle "pack overflow", which
 * must come first just in case somebody manages to corrupt
 * the savefiles by clever use of menu commands or something.
 */
static void process_player(void)
{
	/*** Check for interupts ***/

	/* Complete resting */
	if (p_ptr->state.resting < 0)
	{
		/* Basic resting */
		if (p_ptr->state.resting == -1)
		{
			/* Stop resting */
			if ((p_ptr->chp == p_ptr->mhp) && (p_ptr->csp >= p_ptr->msp))
			{
				disturb(FALSE);
			}
		}

		/* Complete resting */
		else if (p_ptr->state.resting == -2)
		{
			/* Stop resting */
			if ((p_ptr->chp == p_ptr->mhp) &&
				(p_ptr->csp == p_ptr->msp) &&
				!query_timed(TIMED_BLIND) && !query_timed(TIMED_CONFUSED) &&
				!query_timed(TIMED_POISONED) && !query_timed(TIMED_AFRAID) &&
				!query_timed(TIMED_STUN) && !query_timed(TIMED_CUT) &&
				!query_timed(TIMED_SLOW) && !query_timed(TIMED_PARALYZED) &&
				!query_timed(TIMED_IMAGE) && !query_timed(TIMED_WORD_RECALL))
			{
				disturb(FALSE);
			}
		}
	}

	/*** Handle "abort" ***/

	/* Check for "player abort" */
	if (p_ptr->state.running || p_ptr->cmd.rep || p_ptr->state.resting)
	{
		/* Do not wait */
		p_ptr->cmd.inkey_scan = TRUE;

		/* Check for a key */
		if (inkey())
		{
			/* Flush input */
			flush();

			/* Disturb */
			disturb(FALSE);

			/* Hack -- Show a Message */
			msgf("Cancelled.");
		}
	}

	/*** Handle actual user input ***/

	/* Repeat until energy is reduced */
	while (TRUE)
	{
		/* Notice stuff */
		notice_stuff();

		/* Update */
		handle_stuff();

		/* Place the cursor on the player */
		move_cursor_relative(p_ptr->px, p_ptr->py);

		/* Refresh (optional) */
		if (fresh_before) Term_fresh();

		/* Hack -- Pack Overflow */
		if (get_list_length(p_ptr->inventory) > INVEN_PACK)
		{
			int i = 0;

			object_type *o_ptr;

			/* Scan pack */
			OBJ_ITT_START (p_ptr->inventory, o_ptr)
			{
				/* Count items */
				i++;

				/* Does item need to be dropped? */
				if (i > INVEN_PACK)
				{
					/* Disturbing */
					disturb(FALSE);

					/* Warning */
					msgf("Your pack overflows!");

					/* Drop the excess item(s) */
					inven_drop(o_ptr, o_ptr->number);
				}

			}
			OBJ_ITT_END;

			/* Notice stuff */
			notice_stuff();

			/* Update */
			handle_stuff();
		}

		/* Assume free turn */
		p_ptr->state.energy_use = 0;


		/* Paralyzed or Knocked Out */
		if (query_timed(TIMED_PARALYZED) || (query_timed(TIMED_STUN) >= 100))
		{
			/* Take a turn */
			p_ptr->state.energy_use = 100;
		}

		/* Resting */
		else if (p_ptr->state.resting)
		{
			/* Timed rest */
			if (p_ptr->state.resting > 0)
			{
				/* Reduce rest count */
				p_ptr->state.resting--;

				/* Redraw the state */
				p_ptr->redraw |= (PR_STATE);
			}

			/* Take a turn */
			p_ptr->state.energy_use = 100;
		}

		/* Running */
		else if (p_ptr->state.running)
		{
			/* Take a step */
			run_step(0);
		}


		/* Repeated command */
		else if (p_ptr->cmd.rep)
		{
			/* Count this execution */
			p_ptr->cmd.rep--;

			/* Redraw the state */
			p_ptr->redraw |= (PR_STATE);

			/* Redraw stuff */
			redraw_stuff();

			/* Hack -- Assume messages were seen */
			msg_flag = FALSE;

			/* Clear the top line */
			clear_msg();

			/* Process the command */
			process_command();
		}

		/* Normal command */
		else
		{
			/* Place the cursor on the player */
			move_cursor_relative(p_ptr->px, p_ptr->py);

			/* Get a command (normal) */
			request_command(FALSE);

			if (p_ptr->cmd.cmd & 0x80) {
				/* the command is a mouse command */
				int x, y;
				char button;
				/* make sure we have the right mouse press, and clear until we do */
				while ((Term_getmousepress(&button, &x, &y)==0) && (button != p_ptr->cmd.cmd));
				if (button == p_ptr->cmd.cmd) {
					process_click(button, x ,y);
				}
			} else {
				/* Process the command */
				process_command();
			}
		}


		/*** Clean up ***/

		/* Significant */
		if (p_ptr->state.energy_use)
		{
			/* Use some energy */
			p_ptr->energy -= p_ptr->state.energy_use;

			/* Change stuff */
			change_stuff();

			/* Hack -- constant hallucination */
			if (query_timed(TIMED_IMAGE)) p_ptr->redraw |= (PR_MAP);
		}

		/* Hack -- notice death */
		if (!p_ptr->state.playing || p_ptr->state.is_dead) break;

		/* Handle "leaving" */
		if (p_ptr->state.leaving)
		{
			break;
		}
		/* Used up energy for this turn */
		if (p_ptr->state.energy_use) break;
	}
}


/*
 * Add energy to player and monsters.
 * Those with the most energy move first.
 * (This prevents monsters like Morgoth getting double moves
 * when he is at a lower speed than the player.)
 */
static void process_energy(void)
{
	int i, speed, e;
	monster_type *m_ptr;

	/*** Apply energy to player ***/
	if (p_ptr->pspeed > 199) i = 49;
	else if (p_ptr->pspeed < 0) i = 1;
	else
		i = extract_energy[p_ptr->pspeed];

	p_ptr->energy += i;

	/* Give energy to all monsters */
	for (i = m_max - 1; i >= 1; i--)
	{
		/* Access the monster */
		m_ptr = &m_list[i];

		/* Ignore "dead" monsters */
		if (!m_ptr->r_idx) continue;

		/* Skip imprisoned monsters */
		if (m_ptr->imprisoned) continue;

		speed = m_ptr->mspeed;

		/* Monsters move quickly in Nightmare mode */
		if (ironman_nightmare)
		{
			speed = MIN(199, m_ptr->mspeed + 5);
		}

		e = extract_energy[speed];

		/* Give this monster some energy */
		m_ptr->energy += e;
	}

	/* Can the player move? */
	while (p_ptr->energy >= 100 && !p_ptr->state.leaving)
	{
		/* process monster with even more energy first */
		process_monsters(p_ptr->energy + 1);

		/* Process the player while still alive */
		if (!p_ptr->state.leaving)
		{
			process_player();
		}
	}

	/* Process the fields */
	process_fields();
}

bool use_main_menu; /* whether a port is using the textui menu bar */
int (*main_menu_bar_fn) (keycode_t); /* the button function for the textui menu bar */

bool add_main_buttons(void)
{
	int i, hgt;
	hgt = Term->hgt - 1;

	if (use_main_menu && main_menu_bar_fn) {
		button_set_fn(main_menu_bar_fn);
		/* setup the main menu bar 
		 * if the screen is big enough put in bottom left corner, otherwise put
		 * in blank line in center of info panel */
		if (hgt > 23) {
			button_add_2d(hgt-1, 0, hgt-1, 10, format("%s[%s MENU]",CLR_UMBER, ANGBAND_SYS), ESCAPE);
		} else {
			button_add_2d(ROW_BLANK, COL_BLANK, ROW_BLANK, COL_MAP, format("%s[%s MENU]",CLR_UMBER, ANGBAND_SYS), ESCAPE);
		}
		button_set_fn(NULL);
	}

	/* messages - done in mouse press func to use reverse messages screen */
	//button_add_2d(0, 0, 0, Term->wid-1, NULL, KTRL('P'));
	/* character screen */
	button_add_2d(ROW_RACE, COL_RACE, ROW_TITLE, COL_MAP, NULL, 'C');
	/* inventory */
	button_add_2d(ROW_EQUIPPY, COL_EQUIPPY, ROW_EQUIPPY+2, COL_MAP, NULL, 'i');
	/* target */
	button_add_2d(ROW_TARGET_NAME, COL_INFO, ROW_INFO, COL_MAP, NULL, 'l');
	/* search */
	button_add_2d(hgt, COL_STATE,hgt, COL_AFRAID, NULL, 'S');
	/* study */
	if (p_ptr->spell.realm[0]) {
		button_add_2d(hgt, COL_STUDY, hgt, COL_DEPTH, NULL, 'G');
	}
	/* map */
	if (Term->wid > COL_DEPTH+30)  {
		i = COL_DEPTH+20;
	} else {
		i = Term->wid;
	}
	button_add_2d(hgt, COL_DEPTH, hgt, i, NULL, 'M');
	/* time */
	if (Term->wid > COL_DEPTH+30)  {
		if (Term->wid > COL_DEPTH+35)  {
			i = COL_DEPTH+35;
		} else {
			i = Term->wid;
		}
		button_add_2d(hgt, COL_DEPTH+20, hgt, Term->wid, NULL, KTRL('T'));
	}
	/* wield */
	button_add_2d(ROW_AC, COL_AC, ROW_SP, COL_MAP>>1, NULL, 'w');
	/* takeoff */
	button_add_2d(ROW_AC, COL_MAP>>1, ROW_SP, COL_MAP, NULL, 't');
	/* use */
	button_add_2d(ROW_LEVEL, COL_GOLD, ROW_GOLD, COL_MAP>>1, NULL, 'u');
	/* cast */
	button_add_2d(ROW_LEVEL, COL_MAP>>1, ROW_GOLD, COL_MAP, NULL, 'm');

	return TRUE;
}

/*
 * Interact with the current dungeon level.
 *
 * This function will not exit until the level is completed,
 * the user dies, or the game is terminated.
 */
bool dungeon_init_level(void)
{
	bool check_autosave = FALSE;

	/* Not leaving */
	if (p_ptr->state.leaving)
	{
		p_ptr->state.leaving = FALSE;

		/* Hack - save game if asked */
		if (autosave_l) check_autosave = TRUE;
	}

	/* Reset the "command" vars */
	p_ptr->cmd.cmd = 0;
	p_ptr->cmd.new = 0;
	p_ptr->cmd.rep = 0;
	p_ptr->cmd.arg = 0;
	p_ptr->cmd.dir = 0;


	/* Cancel the target */
	p_ptr->target_who = 0;

	/* Cancel the health bar */
	health_track(0);

	/* create some mouse buttons for the main screen */
	add_main_buttons();

	/* Check visual effects.  Should this be here? */
	p_ptr->change |= (PC_SHIMMER | PC_REPAIR);

	/* Disturb */
	disturb(TRUE);

	/* Track maximum player level */
	if (p_ptr->max_lev < p_ptr->lev)
	{
		p_ptr->max_lev = p_ptr->lev;
	}

	/* Center the panel on the player */
	panel_center(p_ptr->px, p_ptr->py);

	/* Flush messages */
	message_flush();


	/* Enter "xtra" mode */

	character_xtra = TRUE;

	/* Window stuff */
	p_ptr->window |= (PW_SPELL | PW_PLAYER);

	/* Window stuff */
	p_ptr->window |= (PW_MONSTER | PW_MESSAGE | PW_VISIBLE);

	/* Redraw dungeon */
	p_ptr->redraw |= (PR_WIPE | PR_BASIC | PR_EXTRA | PR_EQUIPPY | PR_TIME);

	/* Redraw map */
	p_ptr->redraw |= (PR_MAP);

	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);

	/* Update stuff */
	p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);

	/* Calculate torch radius */
	p_ptr->update |= (PU_TORCH);

	/* Update */
	handle_stuff();

	/* Update stuff */
	p_ptr->update |= (PU_VIEW | PU_FLOW | PU_DISTANCE | PU_MON_LITE);

	/* Update */
	handle_stuff();

	/* Leave "xtra" mode */
	character_xtra = FALSE;

	/* Update stuff */
	p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);

	/* Notice changes */
	notice_item();

	/* Notice stuff */
	notice_stuff();

	/* Update */
	handle_stuff();

	/* Refresh */
	Term_fresh();

	/* Hack -- notice death or departure */
	if (!p_ptr->state.playing || p_ptr->state.is_dead) return FALSE;

	/* Print quest message if appropriate */
	quest_discovery();

	/*** Process this dungeon level ***/

	/* Hack - save game if asked */
	if (check_autosave)
	{
		do_cmd_save_game(TRUE);
	}
	return TRUE;
}
bool dungeon_play_frame(void)
{
	/* Hack -- Compact the monster list occasionally */
	if (m_cnt + 32 > z_info->m_max) compact_monsters(64);

	/* Hack -- Compress the monster list occasionally */
	if (m_cnt + 32 < m_max) compact_monsters(0);


	/* Hack -- Compact the object list occasionally */
	if (o_cnt + 32 > z_info->o_max) compact_objects(64);

	/* Hack -- Compress the object list occasionally */
	if (o_cnt + 32 < o_max) compact_objects(0);


	/* Hack -- Compact the field list occasionally */
	if (fld_cnt + 32 > z_info->fld_max) compact_fields(64);

	/* Hack -- Compress the field list occasionally */
	if (fld_cnt + 32 < fld_max) compact_fields(0);

	/*
	 * Add energy to player and monsters.
	 * Those with the most energy move first.
	 */
	process_energy();

	/* Notice */
	notice_stuff();

	/* Update  */
	handle_stuff();

	/* Hack -- Hilite the player */
	move_cursor_relative(p_ptr->px, p_ptr->py);

	/* Optional fresh */
	if (fresh_after) Term_fresh();

	/* Hack -- Notice death or departure */
	if (!p_ptr->state.playing || p_ptr->state.is_dead) return FALSE;

	/* Process all of the monsters */
	process_monsters(100);

	/* Reset monsters */
	reset_monsters();

	/* Notice */
	notice_stuff();

	/* Update */
	handle_stuff();

	/* Hack -- Hilite the player */
	move_cursor_relative(p_ptr->px, p_ptr->py);

	/* Optional fresh */
	if (fresh_after) Term_fresh();

	/* Hack -- Notice death or departure */
	if (!p_ptr->state.playing || p_ptr->state.is_dead) return FALSE;

	/* Handle "leaving" */
	if (p_ptr->state.leaving) return FALSE;

	/* Process the world */
	process_world();

	/* Hack -- Notice death or departure */
	if (!p_ptr->state.playing || p_ptr->state.is_dead) return FALSE;

	/* Handle "leaving" */
	if (p_ptr->state.leaving) return FALSE;

	/* Notice */
	notice_stuff();

	/* Update */
	handle_stuff();

	/* Hack -- Hilite the player */
	move_cursor_relative(p_ptr->px, p_ptr->py);

	/* Optional fresh */
	if (fresh_after) Term_fresh();

	/* Hack -- Notice death or departure */
	if (!p_ptr->state.playing || p_ptr->state.is_dead) return FALSE;

	/* Handle "leaving" */
	if (p_ptr->state.leaving) return FALSE;


	/* Count game turns */
	turn++;

	return TRUE;
}
void dungeon_close_level(void)
{
	/* remove all buttons, namely the main screen ones */
	button_kill_all();

	/* The dungeon is not ready */
	character_dungeon = FALSE;
}

