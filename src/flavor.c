/* File: flavor.c */

/* Purpose: Object flavor code */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"

object_kind_flavor *flavor_info;

/*
 * Max sizes of the following arrays
 */
#define MAX_SYLLABLES 164		/* Used with scrolls (see below) */


/*
 * Syllables for scrolls (must be 1-4 letters each)
 */

static cptr syllables[MAX_SYLLABLES] =
{
	"a", "ab", "ag", "aks", "ala", "an", "ankh", "app",
	"arg", "arze", "ash", "aus", "ban", "bar", "bat", "bek",
	"bie", "bin", "bit", "bjor", "blu", "bot", "bu",
	"byt", "comp", "con", "cos", "cre", "dalf", "dan",
	"den", "der", "doe", "dok", "eep", "el", "eng", "er", "ere", "erk",
	"esh", "evs", "fa", "fid", "flit", "for", "fri", "fu", "gan",
	"gar", "glen", "gop", "gre", "ha", "he", "hyd", "i",
	"ing", "ion", "ip", "ish", "it", "ite", "iv", "jo",
	"kho", "kli", "klis", "la", "lech", "man", "mar",
	"me", "mi", "mic", "mik", "mon", "mung", "mur", "nag", "nej",
	"nelg", "nep", "ner", "nes", "nis", "nih", "nin", "o",
	"od", "ood", "org", "orn", "ox", "oxy", "pay", "pet",
	"ple", "plu", "po", "pot", "prok", "re", "rea", "rhov",
	"ri", "ro", "rog", "rok", "rol", "sa", "san", "sat",
	"see", "sef", "seh", "shu", "ski", "sna", "sne", "snik",
	"sno", "so", "sol", "sri", "sta", "sun", "ta", "tab",
	"tem", "ther", "ti", "tox", "trol", "tue", "turs", "u",
	"ulk", "um", "un", "uni", "ur", "val", "viv", "vly",
	"vom", "wah", "wed", "werg", "wex", "whon", "wun", "xi",
	"yerg", "yp", "zun", "tri", "blaa", "jah", "bul", "on",
	"foo", "ju", "xuxu"
};

/*
 * Certain items, if aware, are known instantly
 * This function is used only by "flavor_init()"
 */
static bool object_easy_know(int i)
{
	object_kind *k_ptr = &k_info[i];

	if (FLAG(k_ptr, TR_EASY_KNOW)) return (TRUE);

	/* Nope */
	return (FALSE);
}


void get_table_name(char *out_string, bool quotes)
{
	int testcounter = 2;

	int len = 0;
	char Syllable[80];
	bool choice = one_in_(3);

	/* Empty string */
	out_string[0] = 0;

	if (quotes)
	{
		strnfcat(out_string, 18, &len, "'");
	}

	while (testcounter--)
	{
		if (choice) (void)get_rnd_line("dwarvish.txt", 0, Syllable);
		else (void)get_rnd_line("elvish.txt", 0, Syllable);
		strnfcat(out_string, 18, &len, "%s", Syllable);
	}

	if (quotes)
	{
		out_string[1] = toupper(out_string[1]);
		strnfcat(out_string, 18, &len, "'");
	}
	else
	{
		out_string[0] = toupper(out_string[0]);
	}
}


static void flavor_assign_fixed(void)
{
	int i,j;
	object_kind_flavor *fl_ptr;
	object_kind *k_ptr;

	for (i = 1; i < z_info->flavor_max; i++) {
		if (flavor_info[i].sval == SV_ANY)
			continue;
		fl_ptr = &(flavor_info[i]);
		for (j = 0; j < z_info->k_max; j++) {
		  k_ptr = &k_info[j];
			if ((k_ptr->tval == fl_ptr->tval)
			  && (k_ptr->sval == fl_ptr->sval))
			{
				k_ptr->flavor = i;
				fl_ptr->k_idx = j;
			}
		}
	}
}


static errr flavor_assign_random(byte tval)
{
	int i,j;
	int flavor_count = 0;
	int choice;
	object_kind_flavor *fl_ptr;
	/*object_kind *k_ptr;*/

	/* Count the random flavors for the given tval */
	for (i = 1; i < z_info->flavor_max; i++) {
		fl_ptr = &(flavor_info[i]);
		if ((fl_ptr->tval == tval) && (fl_ptr->sval == SV_ANY)
		  && (fl_ptr->k_idx == 0))
			flavor_count++;
	}

	for (i = 0; i < z_info->k_max; i++) {
		if (k_info[i].tval != tval || k_info[i].flavor)
			continue;

		if (k_info[i].flags[3] & TR3_NO_FLAVOR)
			continue;

		/* HACK - Ordinary food is "boring" */
		if ((tval == TV_FOOD) && (k_info[i].sval > SV_FOOD_MIN_FOOD))
			continue;

		if (!flavor_count) {
			msgf("Not enough flavors for tval %d.", tval);
			message_flush();
			return (PARSE_ERROR_GENERIC);
		}

		choice = randint0(flavor_count);
	
		for (j = 1; j < z_info->flavor_max; j++) {
			fl_ptr = &(flavor_info[j]);
			if ((fl_ptr->tval == tval) && (fl_ptr->sval == SV_ANY)
			  && (fl_ptr->k_idx == 0))
			{
				if (choice == 0) {
					k_info[i].flavor = j;
					fl_ptr->k_idx = i;
					flavor_count--;
					break;
				}

				choice--;
			}
		}
	}
	/* Success */
	return (0);
}


/*
 * Prepare the "variable" part of the "k_info" array.
 *
 * The "color"/"metal"/"type" of an item is its "flavor".
 * For the most part, flavors are assigned randomly each game.
 *
 * Initialize descriptions for the "colored" objects, including:
 * Rings, Amulets, Staffs, Wands, Rods, Food, Potions, Scrolls.
 *
 * The first 4 entries for potions are fixed (Water, Apple Juice,
 * Slime Mold Juice, Unused Potion).
 *
 * Scroll titles are always between 6 and 14 letters long.  This is
 * ensured because every title is composed of whole words, where every
 * word is from 1 to 8 letters long (one or two syllables of 1 to 4
 * letters each), and that no scroll is finished until it attempts to
 * grow beyond 15 letters.  The first time this can happen is when the
 * current title has 6 letters and the new word has 8 letters, which
 * would result in a 6 letter scroll title.
 *
 * Hack -- make sure everything stays the same for each saved game
 * This is accomplished by the use of a saved "random seed", as in
 * "town_gen()".  Since no other functions are called while the special
 * seed is in effect, so this function is pretty "safe".
 */
void flavor_init(void)
{
	int i, j;
	char buf[26];
	char *end = buf + 1;

	/* Hack -- Use the "simple" RNG */
	Rand_quick = TRUE;

	/* Hack -- Induce consistant flavors */
	Rand_value = seed_flavor;

  /* Create names for any flavor that does not have a name */
	for (i = 1; i < z_info->flavor_max; i++) {
		char buf[80];
		object_kind_flavor *fl_ptr = &(flavor_info[i]);
		if (fl_ptr->name || !(fl_ptr->idx)) {
		  continue;
		}
		/* Get a new title */
		while (TRUE) {
			char tmp[80];
			int buf_len = 0;
			bool okay = TRUE;

			/* Start a new title */
			buf[0] = '\0';

			/* Collect words until done */
			while (TRUE) {
				int q, s;
				int len = 0;

				/* Start a new word */
				tmp[0] = '\0';

				/* Choose one or two syllables */
				s = ((randint0(100) < 30) ? 1 : 2);

				/* Add a one or two syllable word */
				for (q = 0; q < s; q++) {
					/* Add the syllable */
					strnfcat(tmp, 80, &len, syllables[randint0(MAX_SYLLABLES)]);
				}

				/* Stop before getting too long */
				if (strlen(buf) + 1 + strlen(tmp) > 15)
					break;

				/* Add a space + word */
				strnfcat(buf, 80, &buf_len, " %s", tmp);
			}
			/* Check for duplicate names (if any other flavor has this name) */
			for (j = 1; j < z_info->flavor_max; j++) {
				object_kind_flavor *fl2_ptr = &(flavor_info[j]);
				if ((j != i) && fl2_ptr->name
				  && (strcmp(buf+1, fl2_ptr->name) == 0))
				{
					okay = FALSE;
				}
			}
			if (okay)
				break;
		}
		/* Save the title */
		fl_ptr->name = string_make(buf+1);

	}

	/* Hack -- Induce consistant flavors */
	Rand_value = seed_flavor;

	flavor_assign_fixed();

	flavor_assign_random(TV_RING);
	flavor_assign_random(TV_AMULET);
	flavor_assign_random(TV_STAFF);
	flavor_assign_random(TV_WAND);
	flavor_assign_random(TV_ROD);
	flavor_assign_random(TV_FOOD);
	flavor_assign_random(TV_POTION);

	flavor_assign_random(TV_SCROLL);

	/* Hack -- Use the "complex" RNG */
	Rand_quick = FALSE;

	/* Analyze every object */
	for (i = 1; i < z_info->k_max; i++)
	{
		object_kind *k_ptr = &k_info[i];

		/* Skip "empty" objects */
		if (!k_ptr->name) continue;

		/* No flavor yields aware */
		if (!k_ptr->flavor) k_ptr->info |= OK_AWARE;
	}
}

void clear_flavor_assignments()
{
	int i;

	/* R */

	/* Note: To reset any flavor names that were not given in
	 * flavor.txt, any name that is all lower case is considered
	 * to have been generated by flavor_init and will be removed.
	 * However, nothing in flavor.txt is all lower case, so this
	 * behavior is not currently a problem. And we can avoid testing
	 * each sylable to see if a name is completely a collection
	 * of sylables and spaces.
	 */

	/* Check every flavor */
	for (i = 1; i < z_info->flavor_max; i++) {
		flavor_info[i].k_idx = 0;
		/* check if the flavor's name was generated*/
		/* if it was, remove it */
		if (flavor_info[i].name) {
			char *s = flavor_info[i].name;
			s32b len = strlen(flavor_info[i].name);
			int j;
			bool remove = TRUE;
			
			for (j=0; j <= len; j++) {
				if (isupper(s++)) {
					/* generated names do not have an upper
					 * case letter, so this cannot be one */
					remove = FALSE;
					break;
				}
			}
#if 0
			if (remove) {
				while (*s) {
					for (j = 0; j < MAX_SYLLABLES; j++) {
						len = strlen(syllables[j])
						if (strncmp(s, syllables[j], len) == 0) {
							if (len > bestlen) {
								best = j;
								bestlen = len;
							}
						}
					}
					if (bestlen > 0) {
						s += besetlen;
					} else {
						/* no syllable was found so this was not
						 * generated, so go to the next flavor */
						 remove = FALSE;
						 break;
					}
					if (*s == ' ') {
						s++;
					}
					if (*s == '\0') {
						/* the whole name was comprised of syllables
						* so remove the name */
						 remove = TRUE;
						break;
					}
				}
			}
#endif
			if (remove) {
				ZFREE(flavor_info[i].name);
			}
		}
	}

	/* Check every object */
	for (i = 1; i < z_info->k_max; i++) {
		object_kind *k_ptr = &k_info[i];

		/* Skip "empty" objects */
		if (!k_ptr->name) continue;

		/* Remove "flavor" (if any) */
		if (k_ptr->flavor) {
			k_ptr->info &= ~OK_AWARE;
			k_ptr->flavor = 0;
		}
	}
}


/*
 * Creates a description of the item "o_ptr", and stores it in "out_val".
 *
 * One can choose the "verbosity" of the description, including whether
 * or not the "number" of items should be described, and how much detail
 * should be used when describing the item.
 *
 * The given "buf" must be 80 chars long to hold the longest possible
 * description, which can get pretty long, including incriptions, such as:
 * "no more Maces of Disruption (Defender) (+10,+10) [+5] (+3 to stealth)".
 * Note that the inscription will be clipped to keep the total description
 * under size - 1 chars (plus a terminator).
 *
 * Note the use of "object_desc_num()" and "object_desc_int()" as
 * hyper-efficient, portable, versions of some common "sprintf()" commands.
 *
 * Note that all ego-items (when known) append an "Ego-Item Name", unless
 * the item is also an artifact, which should NEVER happen.
 *
 * Note that all artifacts (when known) append an "Artifact Name", so we
 * have special processing for "Specials" (artifact Lites, Rings, Amulets).
 * The "Specials" never use "modifiers" if they are "known", since they
 * have special "descriptions", such as "The Necklace of the Dwarves".
 *
 * Special Lite's use the "k_info" base-name (Phial, Star, or Arkenstone),
 * plus the artifact name, just like any other artifact, if known.
 *
 * Special Ring's and Amulet's, if not "aware", use the same code as normal
 * rings and amulets, and if "aware", use the "k_info" base-name (Ring or
 * Amulet or Necklace).  They will NEVER "append" the "k_info" name.  But,
 * they will append the artifact name, just like any artifact, if known.
 *
 * None of the Special Rings/Amulets are "EASY_KNOW", though they could be,
 * at least, those which have no "pluses", such as the three artifact lites.
 *
 * Hack -- Display "The One Ring" as "a Plain Gold Ring" until aware.
 *
 * If "pref" then a "numeric" prefix will be pre-pended.
 *
 * Mode:
 *   0 -- The Cloak of Death
 *   1 -- The Cloak of Death [1,+3]
 *   2 -- The Cloak of Death [1,+3] (+2 to Stealth)
 *   3 -- The Cloak of Death [1,+3] (+2 to Stealth) {nifty}
 */
void object_desc(char *buf, const object_type *o_ptr, int pref, int mode,
                 int max)
{
	cptr basenm, modstr;
	int power;

	bool aware = FALSE;
	bool known = FALSE;

	bool append_name = FALSE;

	bool show_weapon = FALSE;
	bool show_armour = FALSE;

	cptr s;

	object_type *bow_ptr;

	/* damage dice, damage sides, damage bonus, energy */
	int dd, ds, db, energy_use;
	int tmul;
	long avgdam;

	int len = 0;

	object_kind *k_ptr = &k_info[o_ptr->k_idx];

	monster_race *r_ptr = &r_info[o_ptr->pval];

	/* See if the object is "aware" */
	if (object_aware_p(o_ptr)) aware = TRUE;

	/* See if the object is "known" */
	if (object_known_p(o_ptr)) known = TRUE;

	/* Artifacts are not "aware' unless "known" */
	if ((FLAG(o_ptr, TR_INSTA_ART)) && !known) aware = FALSE;

	/* Extract default "base" string */
	basenm = get_object_name(o_ptr);

	/* Assume no "modifier" string */
	modstr = "";

	/* Empty description */
	buf[0] = '\0';

	/* Analyze the object */
	switch (o_ptr->tval)
	{
		case TV_SKELETON:
		case TV_BOTTLE:
		case TV_JUNK:
		case TV_SPIKE:
		case TV_FLASK:
		case TV_CHEST:
		case TV_CONTAINER:
		case TV_SPIRIT:
		{
			/* Some objects are easy to describe */
			break;
		}

		case TV_FIGURINE:
		case TV_STATUE:
		{
			/* Figurines/Statues */
			cptr tmp = mon_race_name(r_ptr);

			char idol_name[512];

			if (!FLAG(r_ptr, RF_UNIQUE))
			{
				strnfmt(idol_name, 512, "%s%s",
						(is_a_vowel(*tmp) ? "an " : "a "), tmp);

				modstr = idol_name;
			}
			else
			{
				modstr = tmp;
			}

			break;
		}

		case TV_SHOT:
		case TV_BOLT:
		case TV_ARROW:
		case TV_BOW:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_SWORD:
		case TV_DIGGING:
		{
			/* Missiles/ Bows/ Weapons */
			show_weapon = TRUE;
			break;
		}

		case TV_BOOTS:
		case TV_GLOVES:
		case TV_CLOAK:
		case TV_CROWN:
		case TV_HELM:
		case TV_SHIELD:
		case TV_SOFT_ARMOR:
		case TV_HARD_ARMOR:
		case TV_DRAG_ARMOR:
		{
			/* Armour */
			show_armour = TRUE;
			break;
		}

		case TV_LITE:
		{
			/* Lites (including a few "Specials") */
			break;
		}

		case TV_AMULET:
		{
			/* Amulets (including a few "Specials") */

			/* Known artifacts */
			if ((FLAG(k_ptr, TR_INSTA_ART)) && aware) break;

			/* Color the object */
			if (o_ptr->flags[3] & TR3_NO_FLAVOR) break;
			if (k_ptr->flavor)
				modstr = flavor_info[k_ptr->flavor].name;
			if (aware) append_name = TRUE;

			if (((plain_descriptions) && (aware)) || (o_ptr->info & OB_STOREB))
				basenm = "& Amulet~";
			else
				basenm = "& # Amulet~";
			break;
		}

		case TV_RING:
		{
			/* Rings (including a few "Specials") */

			/* Known artifacts */
			if ((FLAG(k_ptr, TR_INSTA_ART)) && aware) break;

			/* Color the object */
			if (o_ptr->flags[3] & TR3_NO_FLAVOR) break;
			if (k_ptr->flavor)
				modstr = flavor_info[k_ptr->flavor].name;
			if (aware) append_name = TRUE;

			if (((plain_descriptions) && (aware)) || (o_ptr->info & OB_STOREB))
				basenm = "& Ring~";
			else
				basenm = "& # Ring~";

			/* Hack -- The One Ring */
			if (!aware && (o_ptr->sval == SV_RING_POWER)) modstr = "Plain Gold";

			break;
		}

		case TV_STAFF:
		{
			/* Color the object */
			if (o_ptr->flags[3] & TR3_NO_FLAVOR) break;
			if (k_ptr->flavor)
				modstr = flavor_info[k_ptr->flavor].name;
			if (aware) append_name = TRUE;
			/* Hack: unusual plurals */
			if (((plain_descriptions) && (aware)) || (o_ptr->info & OB_STOREB))
			{
				if (o_ptr->number == 1)
					basenm = "& Staff~";
				else
					basenm = "& Stave~";
			}
			else
			{
				if (o_ptr->number == 1)
					basenm = "& # Staff~";
				else
					basenm = "& # Stave~";
			}
			break;
		}

		case TV_WAND:
		{
			/* Color the object */
			if (o_ptr->flags[3] & TR3_NO_FLAVOR) break;
			if (k_ptr->flavor)
				modstr = flavor_info[k_ptr->flavor].name;
			if (aware) append_name = TRUE;
			if (((plain_descriptions) && (aware)) || (o_ptr->info & OB_STOREB))
				basenm = "& Wand~";
			else
				basenm = "& # Wand~";
			break;
		}

		case TV_ROD:
		{
			/* Color the object */
			if (o_ptr->flags[3] & TR3_NO_FLAVOR) break;
			if (k_ptr->flavor)
				modstr = flavor_info[k_ptr->flavor].name;
			if (aware) append_name = TRUE;
			if (((plain_descriptions) && (aware)) || (o_ptr->info & OB_STOREB))
				basenm = "& Rod~";
			else
				basenm = "& # Rod~";
			break;
		}

		case TV_SCROLL:
		{
			/* Color the object */
			if (o_ptr->flags[3] & TR3_NO_FLAVOR) break;
			if (k_ptr->flavor)
				modstr = flavor_info[k_ptr->flavor].name;
			if (aware) append_name = TRUE;
			if (((plain_descriptions) && (aware)) || (o_ptr->info & OB_STOREB))
				basenm = "& Scroll~";
			else
				basenm = "& Scroll~ titled \"#\"";
			break;
		}

		case TV_POTION:
		{
			/* Color the object */
			if (o_ptr->flags[3] & TR3_NO_FLAVOR) break;
			if (k_ptr->flavor)
				modstr = flavor_info[k_ptr->flavor].name;
			if (aware) append_name = TRUE;
			if (((plain_descriptions) && (aware)) || (o_ptr->info & OB_STOREB))
				basenm = "& Potion~";
			else
				basenm = "& # Potion~";
			break;
		}

		case TV_FOOD:
		{
			/* Ordinary food is "boring" */
			if (o_ptr->sval >= SV_FOOD_MIN_FOOD) break;
			if (o_ptr->flags[3] & TR3_NO_FLAVOR) break;

			/* Color the object */
			if (k_ptr->flavor)
				modstr = flavor_info[k_ptr->flavor].name;
			if (aware) append_name = TRUE;
			if (((plain_descriptions) && (aware)) || (o_ptr->info & OB_STOREB))
				basenm = "& Mushroom~";
			else
				basenm = "& # Mushroom~";
			break;
		}

			/*** Magic Books ***/

		case TV_LIFE_BOOK:
		{
			modstr = basenm;
			if (mp_ptr->spell_book == TV_LIFE_BOOK)
				basenm = "& Book~ of Life Magic #";
			else
				basenm = "& Life Spellbook~ #";
			break;
		}

		case TV_SORCERY_BOOK:
		{
			modstr = basenm;
			if (mp_ptr->spell_book == TV_LIFE_BOOK)
				basenm = "& Book~ of Sorcery #";
			else
				basenm = "& Sorcery Spellbook~ #";
			break;
		}

		case TV_NATURE_BOOK:
		{
			modstr = basenm;
			if (mp_ptr->spell_book == TV_LIFE_BOOK)
				basenm = "& Book~ of Nature Magic #";
			else
				basenm = "& Nature Spellbook~ #";
			break;
		}

		case TV_CHAOS_BOOK:
		{
			modstr = basenm;
			if (mp_ptr->spell_book == TV_LIFE_BOOK)
				basenm = "& Book~ of Chaos Magic #";
			else
				basenm = "& Chaos Spellbook~ #";
			break;
		}

		case TV_DEATH_BOOK:
		{
			modstr = basenm;
			if (mp_ptr->spell_book == TV_LIFE_BOOK)
				basenm = "& Book~ of Death Magic #";
			else
				basenm = "& Death Spellbook~ #";
			break;
		}

		case TV_CONJ_BOOK:
		{
			modstr = basenm;
			if (mp_ptr->spell_book == TV_LIFE_BOOK)
				basenm = "& Book~ of Conjuration #";
			else
				basenm = "& Conjuration Spellbook~ #";
			break;
		}

		case TV_ARCANE_BOOK:
		{
			modstr = basenm;
			if (mp_ptr->spell_book == TV_LIFE_BOOK)
				basenm = "& Book~ of Arcane Magic #";
			else
				basenm = "& Arcane Spellbook~ #";
			break;
		}


		case TV_ILLUSION_BOOK:
		{
			modstr = basenm;
			if (mp_ptr->spell_book == TV_LIFE_BOOK)
				basenm = "& Book~ of Illusion Magic #";
			else
				basenm = "& Illusion Spellbook~ #";
			break;
		}

		case TV_GOLD:
		{
			/* Hack -- Gold/Gems */
			strcpy(buf, basenm);
			return;
		}

		default:
		{
			/* Used in the "inventory" routine */
			strcpy(buf, "(nothing)");
			return;
		}
	}

	/* The object "expects" a "number" */
	if (basenm[0] == '&')
	{
		/* Skip the ampersand (and space) */
		s = basenm + 2;

		/* No prefix */
		if (!pref)
		{
			/* Nothing */
		}

		/* Hack -- None left */
		else if (o_ptr->number <= 0)
		{
			strnfcat(buf, max, &len, "no more ");
		}

		/* Extract the number */
		else if (o_ptr->number > 1)
		{
			strnfcat(buf, max, &len, "%d ", o_ptr->number);
		}

		/* Hack -- The only one of its kind */
		else if (known && (FLAG(o_ptr, TR_INSTA_ART)))
		{
			strnfcat(buf, max, &len, "The ");
		}

		/* A single one, with a vowel in the modifier */
		else if ((*s == '#') && (is_a_vowel(modstr[0])))
		{
			strnfcat(buf, max, &len, "an ");
		}

		/* A single one, with a vowel */
		else if (is_a_vowel(*s))
		{
			strnfcat(buf, max, &len, "an ");
		}

		/* A single one, without a vowel */
		else
		{
			strnfcat(buf, max, &len, "a ");
		}
	}

	/* Hack -- objects that "never" take an article */
	else
	{
		/* No ampersand */
		s = basenm;

		/* No pref */
		if (!pref)
		{
			/* Nothing */
		}

		/* Hack -- all gone */
		else if (o_ptr->number <= 0)
		{
			strnfcat(buf, max, &len, "no more ");
		}

		/* Prefix a number if required */
		else if (o_ptr->number > 1)
		{
			strnfcat(buf, max, &len, "%d ", o_ptr->number);
		}

		/* Hack -- The only one of its kind */
		else if (known && (FLAG(o_ptr, TR_INSTA_ART)))
		{
			strnfcat(buf, max, &len, "The ");
		}

		/* Hack -- single items get no prefix */
		else
		{
			/* Nothing */
		}
	}

	/* Copy the string */
	while (*s)
	{
		/* Pluralizer */
		if (*s == '~')
		{
			/* Add a plural if needed */
			if (o_ptr->number != 1)
			{
				/* Get previous character */
				char k = s[-1];

				/* XXX XXX XXX Mega-Hack */

				/* Hack -- "Cutlass-es" and "Torch-es" */
				if ((k == 's') || (k == 'h'))
				{
					strnfcat(buf, max, &len, "es");
				}
				else
				{
					/* Add an 's' */
					strnfcat(buf, max, &len, "s");
				}
			}
		}

		/* Modifier */
		else if (*s == '#')
		{
			/* Insert the modifier */
			strnfcat(buf, max, &len, "%s", modstr);
		}

		/* Normal */
		else
		{
			/* Copy character */
			strnfcat(buf, max, &len, "%c", *s);
		}

		s++;
	}

	/* Append the "kind name" to the "base name" */
	if (append_name)
	{
		strnfcat(buf, max, &len, " of %s", get_object_name(o_ptr));
	}


	/* Hack -- Append "Artifact" or "Special" names */
	/* Mega-Hack!  The One Ring's xtra name is suppressed, and it cannot be renamed! */
	if (known && (o_ptr->tval != TV_RING || o_ptr->sval != SV_RING_POWER))
	{
		if (o_ptr->inscription && strchr(quark_str(o_ptr->inscription), '#'))
		{
			/* Find the '#' */
			cptr str = strchr(quark_str(o_ptr->inscription), '#');

			/* Add the false name */
			strnfcat(buf, max, &len, " %s" CLR_DEFAULT, &str[1]);
		}

		/* Is it a new artifact or ego item? */
		else if (o_ptr->xtra_name)
		{
			strnfcat(buf, max, &len, " %s", quark_str(o_ptr->xtra_name));
		}
	}

	/* No more details wanted */
	if (mode < 1) return;

  /* Hack so that bonuses are not displayed - Brett */
  /* but this prevents the possibility of containers from being dropped trapped? does anyone care? */
	if (o_ptr->tval == TV_CONTAINER)
	{
    return;
  }

	/* Hack -- Chests must be described in detail */
	if (o_ptr->tval == TV_CHEST)
	{
		/* Not searched yet */
		if (!known)
		{
			/* Nothing */
		}

		/* May be "empty" */
		else if (!o_ptr->pval)
		{
			strnfcat(buf, max, &len, " (empty)");
		}

		/* May be "disarmed" */
		else if (o_ptr->pval < 0)
		{
			if (chest_traps[0 - o_ptr->pval])
			{
				strnfcat(buf, max, &len, " (disarmed)");
			}
			else
			{
				strnfcat(buf, max, &len, " (unlocked)");
			}
		}

		/* Describe the traps, if any */
		else
		{
			/* Describe the traps */
			switch (chest_traps[o_ptr->pval])
			{
				case 0:
				{
					strnfcat(buf, max, &len, " (Locked)");
					break;
				}
				case CHEST_LOSE_STR:
				{
					strnfcat(buf, max, &len, " (Poison Needle)");
					break;
				}
				case CHEST_LOSE_CON:
				{
					strnfcat(buf, max, &len, " (Poison Needle)");
					break;
				}
				case CHEST_POISON:
				{
					strnfcat(buf, max, &len, " (Gas Trap)");
					break;
				}
				case CHEST_PARALYZE:
				{
					strnfcat(buf, max, &len, " (Gas Trap)");
					break;
				}
				case CHEST_EXPLODE:
				{
					strnfcat(buf, max, &len, " (Explosion Device)");
					break;
				}
				case CHEST_SUMMON:
				{
					strnfcat(buf, max, &len, " (Summoning Runes)");
					break;
				}
				default:
				{
					strnfcat(buf, max, &len, " (Multiple Traps)");
					break;
				}
			}
		}
	}


	/* Display the item like a weapon */
	if (FLAG(o_ptr, TR_SHOW_MODS)) show_weapon = TRUE;

	/* Display the item like a weapon */
	if (o_ptr->to_h && o_ptr->to_d) show_weapon = TRUE;

	/* Display the item like armour */
	if ((o_ptr->ac) && (o_ptr->tval != TV_WAND) && (o_ptr->tval != TV_CONTAINER)) show_armour = TRUE;

	/* Dump base weapon info */
	switch (o_ptr->tval)
	{
		case TV_SHOT:
		case TV_BOLT:
		case TV_ARROW:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_SWORD:
		case TV_DIGGING:
		{
			/* Missiles and Weapons */

			/* Append a "damage" string */
			strnfcat(buf, max, &len, " (%dd%d)", o_ptr->dd, o_ptr->ds);

			/* All done */
			break;
		}

		case TV_BOW:
		{
			/* Bows get a special "damage string" */

			/* Extract the "base power" */
      if ((o_ptr->dd ==0) && (o_ptr->ds > 0)) {
					power = o_ptr->ds;
      } else
			switch (o_ptr->sval)
			{
				case SV_SLING:
				{
					power = 2;
					break;
				}
				case SV_SHORT_BOW:
				{
					power = 2;
					break;
				}
				case SV_LONG_BOW:
				{
					if (p_ptr->stat[A_STR].use >= 160)
					{
						power = 3;
					}
					else
					{
						/* hack- weak players cannot use a longbow well */
						power = 2;
					}
					break;
				}
				case SV_LIGHT_XBOW:
				{
					power = 4;
					break;
				}
				case SV_HEAVY_XBOW:
				{
					power = 5;
					break;
				}
				default:
				{

					msgf("Unknown firing multiplier.");
					power = 0;
          if (o_ptr->sval < 10) {
  			    p_ptr->ammo_mult = (o_ptr->sval%10);
          } else
          if (o_ptr->sval < 20) {
            if (p_ptr->stat[A_STR].use >= o_ptr->weight*3/2) {
    			    power = (o_ptr->sval%10)+2;
            } else {
    			    power = (o_ptr->sval%10)+1;
            }
          } else
          if (o_ptr->sval < 30) {
  			    power = (o_ptr->sval%10)+2;
          } else
          {
  			    power = (o_ptr->sval%10)+(o_ptr->sval/10);
          }
          if (power < 2) power = 2;
        }
			}

			/* Apply the "Extra Might" flag */
			if (FLAG(o_ptr, TR_XTRA_MIGHT)) power++;

			/* Append a special "damage" string */
			strnfcat(buf, max, &len, " (x%d)", power);

			/* All done */
			break;
		}
	}

	/* Hack: Humans who normally have Heavy sense get to see weapon bonuses without ID */
	/* Add the weapon bonuses */
	if (known ||
		(p_ptr->rp.prace == RACE_HUMAN && class_info[p_ptr->rp.pclass].heavy_sense &&
		 o_ptr->feeling != FEEL_NONE))
	{
		/* Show the tohit/todam on request */
		if (show_weapon)
		{
			strnfcat(buf, max, &len, " (%+d,%+d%%)", o_ptr->to_h,
					deadliness_calc(o_ptr->to_d) - 100);
		}

		/* Show the tohit if needed */
		else if (o_ptr->to_h)
		{
			strnfcat(buf, max, &len, " (%+d)", o_ptr->to_h);
		}

		/* Show the todam if needed */
		else if (o_ptr->to_d)
		{
			strnfcat(buf, max, &len, " (%+d%%)", o_ptr->to_d * 5);
		}
	}


	bow_ptr = &p_ptr->equipment[EQUIP_BOW];

	/* if have a firing weapon + ammo matches bow */
	if (bow_ptr->k_idx && (p_ptr->ammo_tval == o_ptr->tval))
	{
		/* See if the bow is "known" - then set damage bonus */
		if (object_known_p(bow_ptr))
		{
			db = bow_ptr->to_d;
		}
		else
		{
			db = 0;
		}

		/* effect of player */
		db += p_ptr->dis_to_d;

		/* effect of ammo */
		if (known) db += o_ptr->to_d;

		dd = o_ptr->dd;
		ds = o_ptr->ds;

		/* effect of damage dice x2 */
		avgdam = avg_dam(db, dd, ds);

		/* Bow properties */
		energy_use = p_ptr->bow_energy;
		tmul = p_ptr->ammo_mult;

		/* Get extra "power" from "extra might" */
		if (FLAG(p_ptr, TR_XTRA_MIGHT)) tmul++;

		/* launcher multiplier */
		avgdam *= tmul;

		/* display (shot damage/ avg damage) */
		strnfcat(buf, max, &len, " (%d/", avgdam / 200);

		tmul = p_ptr->num_fire;
		if (tmul == 0)
		{
			strnfcat(buf, max, &len, "0)");
		}
		else
		{
			/* calc effects of energy  x2 */
			avgdam *= (1 + p_ptr->num_fire);

			/* rescale */
			avgdam /= 4 * energy_use;
			strnfcat(buf, max, &len, "%d)", avgdam);
		}
	}

	/* Add the armor bonuses */
	if (known ||
		(p_ptr->rp.prace == RACE_HUMAN && class_info[p_ptr->rp.pclass].heavy_sense &&
		 o_ptr->feeling != FEEL_NONE))
	{
		/* Show the armor class info */
		if (show_armour)
		{
			strnfcat(buf, max, &len, " [%d,%+d]", o_ptr->ac,  o_ptr->to_a);
		}

		/* No base armor, but does increase armor */
		else if (o_ptr->to_a)
		{
			strnfcat(buf, max, &len, " [%+d]", o_ptr->to_a);
		}
	}

	/* Hack -- always show base armor */
	else if (show_armour)
	{
		strnfcat(buf, max, &len, " [%d]", o_ptr->ac);
	}


	/* No more details wanted */
	if (mode < 2) return;


	/*
	 * Hack -- Wands and Staffs have charges.  Make certain how many charges
	 * a stack of staffs really has is clear. -LM-
	 */
	if (known && ((o_ptr->tval == TV_STAFF) || (o_ptr->tval == TV_WAND)))
	{
		/* Dump " (N charges)" */
		strnfcat(buf, max, &len, " (");

		/* Clear explaination for staffs. */
		if ((o_ptr->tval == TV_STAFF) && (o_ptr->number > 1))
		{
			strnfcat(buf, max, &len, "%dx ", o_ptr->number);
		}

		if (o_ptr->pval == 1)
		{
			strnfcat(buf, max, &len, "%d charge)", o_ptr->pval);
		}
		else
		{
			strnfcat(buf, max, &len, "%d charges)", o_ptr->pval);
		}
	}
	/* Hack -- Rods have a "charging" indicator.  Now that stacks of rods may
	 * be in any state of charge or discharge, this now includes a number. -LM-
	 */
	else if (o_ptr->tval == TV_ROD)
	{
		/* Hack -- Dump " (# charging)" if relevant */
		if (o_ptr->timeout)
		{
			/* Stacks of rods display an exact count of charging rods. */
			if (o_ptr->number > 1)
			{
				/* Paranoia. */
				if (k_ptr->pval == 0) k_ptr->pval = 1;

				/*
				 * Find out how many rods are charging, by dividing
				 * current timeout by each rod's maximum timeout.
				 * Ensure that any remainder is rounded up.  Display
				 * very discharged stacks as merely fully discharged.
				 */
				power = (o_ptr->timeout + (k_ptr->pval - 1)) / k_ptr->pval;
				if (power > o_ptr->number) power = o_ptr->number;

				/* Display prettily. */
				strnfcat(buf, max, &len, " (%d charging)", power);
			}

			/* "one Rod of Perception (1 charging)" would look tacky. */
			else
			{
				strnfcat(buf, max, &len, " (charging)");
			}
		}
	}

	/* Hack -- Process Lanterns/Torches */
	else if (o_ptr->tval == TV_LITE)
	{
		if (FLAG(o_ptr, TR_LITE))
		{
			/* Hack - tell us when lites of everburning are "empty" */
			if ((o_ptr->sval <= SV_LITE_LANTERN) && !o_ptr->timeout)
			{
				strnfcat(buf, max, &len, " (empty)");
			}
		}
		else
		{
			/* Hack -- Turns of light for normal lites */
			strnfcat(buf, max, &len, " (with %d turns of light)", o_ptr->timeout);
		}
	}


	/* Dump "pval" flags for wearable items */
	if (known && (FLAG(o_ptr, TR_PVAL_MASK)))
	{
		/* Start the display */
		strnfcat(buf, max, &len, " (%+d", o_ptr->pval);

		/* Do not display the "pval" flags */
		if (FLAG(o_ptr, TR_HIDE_TYPE))
		{
			/* Nothing */
		}

		/* Speed */
		else if (FLAG(o_ptr, TR_SPEED))
		{
			/* Dump " to speed" */
			strnfcat(buf, max, &len, " to speed");
		}

		/* Attack speed */
		else if (FLAG(o_ptr, TR_BLOWS))
		{
			if (ABS(o_ptr->pval) == 1)
			{
				/* Add " attack" */
				strnfcat(buf, max, &len, " attack");
			}
			else
			{
				/* Add "attacks" */
				strnfcat(buf, max, &len, " attacks");
			}
		}

		/* Finish the display */
		strnfcat(buf, max, &len, ")");
	}

	/* Indicate charging objects, but not rods. */
	if (known && o_ptr->timeout && (o_ptr->tval != TV_ROD)
		&& (o_ptr->tval != TV_LITE))
	{
		/* Hack -- Dump " (charging)" if relevant */
		strnfcat(buf, max, &len, " (charging)");
	}


	/* No more details wanted */
	if (mode < 3) return;

	/* Use the standard inscription if available */
	if (o_ptr->inscription)
	{
		cptr tmp = quark_str(o_ptr->inscription);

		/* A '#' indicates a fake name, and a '}' suppresses the remainder
		   of the inscription.  Give no inscription at all if one of these
		   comes first.  */
		if ((*tmp != '#') && (*tmp != '}'))
		{

			/* Append the inscription */
			strnfcat(buf, max, &len, " {");

			/* Scan for the '#' character which marks a fake name. */
			/* Also scan for the '}' character which suppresses the rest
			   of the inscription during display. */
			while(*tmp && (*tmp != '#') && (*tmp != '}'))
			{
				strnfcat(buf, max, &len, "%c", *tmp);

				tmp++;
			}

			/* Finish the inscription */
			strnfcat(buf, max, &len, CLR_DEFAULT "}");
		}
	}

	/* Use the game-generated "feeling" otherwise, if available */
	else if (o_ptr->feeling)
	{
		/* Append the inscription */
		strnfcat(buf, max, &len, " {%s" CLR_DEFAULT "}", game_inscriptions[o_ptr->feeling]);
	}

	/* Note "cursed" if the item is known to be cursed */
	else if (cursed_p(o_ptr) && (known || (o_ptr->info & (OB_SENSE))))
	{
		/* Append the inscription */
		strnfcat(buf, max, &len, " {cursed}");
	}

	/* Mega-Hack -- note empty wands/staffs */
	else if (!known && (o_ptr->info & (OB_EMPTY)))
	{
		/* Append the inscription */
		strnfcat(buf, max, &len, " {empty}");
	}
	/* Note worthless if we know the object kind is worthless */
	else if (!aware && object_worthless_p(o_ptr))
	{
		/* Append the inscription */
		strnfcat(buf, max, &len, " {worthless}");
	}

	/* Note "cursed?" if the object is known to sometimes be cursed */
	else if (!aware && object_maybecursed_p(o_ptr))
	{
		/* Append the inscription */
		strnfcat(buf, max, &len, " {cursed?}");
	}

	/* Note "tried" if the object has been tested unsuccessfully */
	else if (!aware && object_tried_p(o_ptr))
	{
		/* Append the inscription */
		strnfcat(buf, max, &len, " {tried}");
	}

	/* Note the discount, if any */
	else if (o_ptr->discount)
	{
		/* Append the inscription */
		strnfcat(buf, max, &len, " {%d%% off}", o_ptr->discount);
	}
}

/*
 * Wrapper around object_desc() for the '%v'
 * format option.  This allows object_desc() to be
 * called in a format string.
 *
 * The parameters are object_type (o_ptr), pref (int), mode(int).
 */
void object_fmt(char *buf, uint max, cptr fmt, va_list *vp)
{
	const object_type *o_ptr;
	int pref;
	int mode;

	/* Unused parameter */
	(void)fmt;

	/* Get the object */
	o_ptr = va_arg(*vp, const object_type*);

	/* Get the pref */
	pref = va_arg(*vp, int);

	/* Get the mode */
	mode = va_arg(*vp, int);

	object_desc(buf, o_ptr, pref, mode, max);
}


/*
 * Hack -- describe an item currently in a store's inventory
 * This allows an item to *look* like the player is "aware" of it
 */
void object_desc_store(char *buf, const object_type *o_ptr, int pref,
                       int mode, int size)
{
	byte hack_flavor;
	bool hack_info;
	byte info;

	/* Hack - we will reset the object to exactly like it was */
	object_type *q_ptr = (object_type *)o_ptr;

	/* Save the "flavor" */
	hack_flavor = k_info[o_ptr->k_idx].flavor;

	/* Save the object kind info flags */
	hack_info = k_info[o_ptr->k_idx].info;

	/* Save the "info" */
	info = o_ptr->info;

	/* Clear the flavor */
	k_info[o_ptr->k_idx].flavor = FALSE;

	/* If this is a shop item or in you are in wizard mode play with objects */
	if (q_ptr->info & OB_STOREB)
	{
		/* Make it known */
		q_ptr->info |= (OB_KNOWN);

		/* Force "aware" for description */
		k_info[o_ptr->k_idx].info |= OK_AWARE;
	}

	/* Describe the object */
	object_desc(buf, q_ptr, pref, mode, size);

	/* Restore "flavor" value */
	k_info[o_ptr->k_idx].flavor = hack_flavor;

	/* Restore object kind info */
	k_info[o_ptr->k_idx].info = hack_info;

	/* Restore the "info" */
	q_ptr->info = info;
}

/*
 * Wrapper around object_desc_store() for the '%v'
 * format option.  This allows object_desc() to be
 * called in a format string.
 *
 * The parameters are object_type (o_ptr), pref (int), mode(int).
 */
void object_store_fmt(char *buf, uint max, cptr fmt, va_list *vp)
{
	const object_type *o_ptr;
	int pref;
	int mode;

	/* Unused parameter */
	(void)fmt;

	/* Get the object */
	o_ptr = va_arg(*vp, const object_type*);

	/* Get the pref */
	pref = va_arg(*vp, int);

	/* Get the mode */
	mode = va_arg(*vp, int);

	object_desc_store(buf, o_ptr, pref, mode, max);
}
