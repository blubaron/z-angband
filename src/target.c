/* File: target.c */

/* Purpose: Text ui targeting functions */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"
#include "target.h"



/**
 * Draw a visible path over the squares between (x1,y1) and (x2,y2).
 *
 * The path consists of "*", which are white except where there is a
 * monster, object or feature in the grid.
 *
 * This routine has (at least) three weaknesses:
 * - remembered objects/walls which are no longer present are not shown,
 * - squares which (e.g.) the player has walked through in the dark are
 *   treated as unknown space.
 * - walls which appear strange due to hallucination aren't treated correctly.
 *
 * The first two result from information being lost from the dungeon arrays,
 * which requires changes elsewhere
 */
sint draw_path(sint path_n, coord *path_g, byte *a, char *c, int x1, int y1)
{
	sint i;
	int x, y;
	byte colour;

	bool on_screen;
	bool visible_object;
	cave_type *c_ptr;
	pcave_type *pc_ptr;

	/* No path, so do nothing. */
	if (path_n < 1) return 0;

	/* The starting square is never drawn, but notice if it is being
	 * displayed. In theory, it could be the last such square.
	 */
	on_screen = panel_contains(x1, y1);

	/* Draw the path. */
	for (i = 0; i < path_n; i++) {
		/* Find the co-ordinates on the level. */
		y = path_g[i].y;
		x = path_g[i].x;

		/*
		 * As path[] is a straight line and the screen is oblong,
		 * there is only section of path[] on-screen.
		 * If the square being drawn is visible, this is part of it.
		 * If none of it has been drawn, continue until some of it
		 * is found or the last square is reached.
		 * If some of it has been drawn, finish now as there are no
		 * more visible squares to draw.
		 */
		 if (panel_contains(x, y)) on_screen = TRUE;
		 else if (on_screen) break;
		 else continue;

		/* Find the position on-screen */
		move_cursor_relative(x,y);

		/* This square is being overwritten, so save the original. */
		if (a && c) {
			Term_what(Term->scr->cx, Term->scr->cy, a+i, c+i);
		}

		/*if (a && c && ta && tc) {
			if (Term_what(Term->scr->cx, Term->scr->cy, a+i, c+i)) {
				*(ta+i) = Term->src->ta[y][x]
				*(tc+i) = Term->src->tc[y][x]
			}
		}*/

		c_ptr = area(x, y);
		pc_ptr = parea(x, y);
		visible_object = FALSE;

		/* check if there is a visible object in the map grid */
		if (c_ptr->o_idx) {
			object_type *o_ptr;
			/* Scan all objects in the grid */
			OBJ_ITT_START (c_ptr->o_idx, o_ptr)
			{
				/* Known objects are yellow. */
				if ((o_ptr->info & OB_SEEN) && (!SQUELCH(o_ptr->k_idx) || FLAG(o_ptr, TR_SQUELCH))) {
					visible_object = TRUE;
					break;
				}
			}
			OBJ_ITT_END;
		}

		/* Choose a colour. */
		if (c_ptr->m_idx && m_list[c_ptr->m_idx].ml) {
			/* Visible monsters are red. Friendly monsters are green */
			monster_type *m_ptr = &(m_list[c_ptr->m_idx]);

			/* Mimics act as objects */
			if (m_ptr->smart & SM_MIMIC) 
				colour = TERM_YELLOW;
			else if (m_ptr->smart & (SM_PET|SM_FRIENDLY))
				colour = TERM_L_GREEN;
			else
				colour = TERM_L_RED;
		}

		else if (c_ptr->o_idx && visible_object)
			/* Known objects are yellow. */
			colour = TERM_YELLOW;

		else if ((pc_ptr->player & GRID_SEEN) && !cave_floor_grid(c_ptr))
			/* Known walls are blue. */
			colour = TERM_BLUE;

		else if (is_visible_trap(c_ptr))
			/* Traps are violet. */
			colour = TERM_VIOLET;

		else if (!(pc_ptr->player & GRID_SEEN))
			/* Unknown squares are grey. */
			colour = TERM_L_DARK;

		else
			/* Unoccupied squares are white. */
			colour = TERM_WHITE;

		/* Draw the path segment */
		Term_queue_char(Term->scr->cx, Term->scr->cy, colour, '*', 0, 0);
	}
	return i;
}


/**
 * Load the attr/char at each point along "path" which is on screen from
 * "a" and "c". This was saved in draw_path().
 */
void remove_path(sint path_n, coord *path_g, byte *a, char *c)
{
	sint i;
	for (i = 0; i < path_n; i++) {
		if (!panel_contains(path_g[i].x, path_g[i].y)) continue;
		move_cursor_relative(path_g[i].x, path_g[i].y);
		Term_queue_char(Term->scr->cx, Term->scr->cy, a[i], c[i], 0, 0);
	}
}



/*
 * Update (if necessary) and verify (if possible) the target.
 *
 * We return TRUE if the target is "okay" and FALSE otherwise.
 */
bool target_okay(void)
{
	/* Accept stationary targets */
	if (p_ptr->target_who < 0) return (TRUE);

	/* Check moving targets */
	if (p_ptr->target_who > 0)
	{
		/* Accept reasonable targets */
		if (target_able(p_ptr->target_who))
		{
			monster_type *m_ptr = &m_list[p_ptr->target_who];

			/* Acquire monster location */
			p_ptr->target_row = m_ptr->fy;
			p_ptr->target_col = m_ptr->fx;

			/* Good target */
			return (TRUE);
		}
	}

	/* Assume no target */
	return (FALSE);
}


/*
 * Set the target to a monster (or nobody)
 */
void target_set_monster(int m_idx)
{
	/* Acceptable target */
	if ((m_idx > 0) && target_able(m_idx)) {
		monster_type *m_ptr = &m_list[m_idx];

		/* Save target info */
		/*p_ptr->target_set = TRUE;*/
		p_ptr->target_who = m_idx;
		p_ptr->target_row = m_ptr->fy;
		p_ptr->target_col = m_ptr->fx;
	} else

	/* Clear target */
	{
		/* Reset target info */
		/*p_ptr->target_set = FALSE;*/
		p_ptr->target_who = 0;
		p_ptr->target_row = 0;
		p_ptr->target_col = 0;
	}
}


/*
 * Set the target to a location
 */
void target_set_location(int x, int y)
{
	/* Legal target */
	if (in_bounds2(x, y)) {
		/* Save target info */
		/*p_ptr->target_set = TRUE;*/
		p_ptr->target_who = -1;
		p_ptr->target_row = y;
		p_ptr->target_col = x;
	} else

	/* Clear target */
	{
		/* Reset target info */
		/*p_ptr->target_set = FALSE;*/
		p_ptr->target_who = 0;
		p_ptr->target_row = 0;
		p_ptr->target_col = 0;
	}
}

bool target_set_grid(int x, int y)
{
	cave_type *c_ptr = area(x,y);
	/*pcave_type *pc_ptr = parea(x,y);*/
	int m_idx = c_ptr->m_idx;

	/* Acceptable target */
	if ((m_idx > 0) && target_able(m_idx)) {
		monster_type *m_ptr = &m_list[m_idx];

		/* Save target info */
		/*p_ptr->target_set = TRUE;*/
		p_ptr->target_who = m_idx;
		p_ptr->target_row = m_ptr->fy;
		p_ptr->target_col = m_ptr->fx;

		/* Set up target information */
		monster_race_track(m_ptr->r_idx);
		health_track(m_idx);

		return TRUE;
	} else
	/* Legal location */
	if (in_bounds2(x, y)) {
		/* Save target info */
		/*p_ptr->target_set = TRUE;*/
		p_ptr->target_who = -1;
		p_ptr->target_row = y;
		p_ptr->target_col = x;

		/* reset health tracker */
		health_track(0);
		if (c_ptr->o_idx) {
			object_type *o_ptr = &(o_list[c_ptr->o_idx]);
			if (o_ptr->info & (OB_SEEN)) {
				if (o_ptr->k_idx) {
					object_kind_track(o_ptr->k_idx);
				}
			}
		}
		return TRUE;
	} else
	/* Clear target */
	{
		/* Reset target info */
		/*p_ptr->target_set = FALSE;*/
		p_ptr->target_who = 0;
		p_ptr->target_row = 0;
		p_ptr->target_col = 0;

		health_track(0);
	}
	return FALSE;
}

bool target_look_grid(int x, int y, bool recall)
{
	cave_type *c_ptr = area(x,y);
	pcave_type *pc_ptr = parea(x,y);
	int m_idx = c_ptr->m_idx;

	/* Hack -- hallucination */
	if (query_timed(TIMED_IMAGE)) {
		prtf(0,0, "You see something strange");
		return FALSE;
	}

	if (m_idx && m_list[m_idx].r_idx && m_list[m_idx].ml) {
		monster_type *m_ptr = &(m_list[m_idx]);
		char m_name[80];

		if ((m_ptr->smart & SM_MIMIC) && mimic_desc(m_name, &(r_info[m_ptr->r_idx]))) {
			health_track(0);
			prtf(0, 0, "You see a %s", m_name);
		} else
		/* show monster info */
		{
			/* Set up target information */
			monster_race_track(m_ptr->r_idx);
			health_track(m_idx);
			if (recall) {
				/* show the recall info */
				/* Save */
				screen_save();

				/* Recall on screen */
				screen_roff_mon(m_ptr->r_idx, 0);

				/* Pause */
				(void)inkey();

				/* Restore */
				screen_load();
			} else {
				prtf(0, 0, "You see %v", MONSTER_FMT(m_ptr, 0x08));
			}
			return TRUE;
		}
	} else {
		health_track(0);
		/* mention what is in the grid cell */
		if (c_ptr->o_idx) {
			object_type *o_ptr = &(o_list[c_ptr->o_idx]);
			if (o_ptr->info & (OB_SEEN)) {
				if (o_ptr->k_idx) {
					object_kind_track(o_ptr->k_idx);
				}
				if (recall) {
					/* show more info if we know of it */
					prtf(0, 0, "You see %v", OBJECT_FMT(o_ptr, TRUE, 3));
				} else {
					prtf(0, 0, "You see %v", OBJECT_FMT(o_ptr, TRUE, 3));
				}
			}
		} else
		if (c_ptr->fld_idx) {
			cptr name = NULL, s3;
			char fld_name[41];
			field_type *f_ptr = &(fld_list[c_ptr->fld_idx]);
			field_thaum *t_ptr = &(t_info[f_ptr->t_idx]);

			if (f_ptr->info & FIELD_INFO_MARK) {
				/* See if it has a special name */
				field_script_single(f_ptr, FIELD_ACT_LOOK, ":s", LUA_RETURN(name));
				if (name) {
					/* Copy the string into the temp buffer */
					strncpy(fld_name, name, 40);

					/* Anything there? */
					if (!fld_name[0]) {
						/* Default to field name */
						strncpy(fld_name, t_ptr->name, 40);
					}

					/* Free string allocated to hold return value */
					string_free(name);
				} else {
					/* Default to field name */
					strncpy(fld_name, t_ptr->name, 40);
				}
				s3 = is_a_vowel(fld_name[0]) ? "an " : "a ";

				/* Describe the field */
				prtf(0, 0, "You see %s%s", s3, fld_name);
			}
		} else
		if (pc_ptr->feat && (pc_ptr->player & GRID_KNOWN)) {
			cptr name, s3;
			name = f_name + f_info[pc_ptr->feat].name;
			if (f_info[pc_ptr->feat].flags & FF_OBJECT) {
				/* Pick proper indefinite article */
				s3 = (is_a_vowel(name[0])) ? "an " : "a ";
			} else  {
				s3 = "";
			}
			prtf(0, 0, "You see %s%s", s3, name);
				
		}
	}
	return FALSE;
}

bool target_look_grid_prompt(int col, int row, int x, int y, char* pmt)
{
	cave_type *c_ptr = area(x,y);
	pcave_type *pc_ptr = parea(x,y);
	int m_idx = c_ptr->m_idx;

	/* Hack -- hallucination */
	if (query_timed(TIMED_IMAGE)) {
		prtf(col,row, "%s something strange:", pmt);
		return FALSE;
	}

	if (m_idx && m_list[m_idx].r_idx && m_list[m_idx].ml) {
		monster_type *m_ptr = &(m_list[m_idx]);
		char m_name[80];

		if ((m_ptr->smart & SM_MIMIC) && mimic_desc(m_name, &(r_info[m_ptr->r_idx]))) {
			health_track(0);
			prtf(col,row, "%s a %s:", pmt, m_name);
		} else
		/* show monster info */
		{
			/* Set up target information */
			monster_race_track(m_ptr->r_idx);
			health_track(m_idx);
			prtf(0, 0, "%s %v:", pmt, MONSTER_FMT(m_ptr, 0x08));
			return TRUE;
		}
	} else {
		health_track(0);
		/* mention what is in the grid cell */
		if (c_ptr->o_idx) {
			object_type *o_ptr = &(o_list[c_ptr->o_idx]);
			if (o_ptr->info & (OB_SEEN)) {
				if (o_ptr->k_idx) {
					object_kind_track(o_ptr->k_idx);
				}
				prtf(col,row, "%s %v:", pmt, OBJECT_FMT(o_ptr, TRUE, 3));
			}
		} else
		if (c_ptr->fld_idx) {
			cptr name = NULL, s3;
			char fld_name[41];
			field_type *f_ptr = &(fld_list[c_ptr->fld_idx]);
			field_thaum *t_ptr = &(t_info[f_ptr->t_idx]);

			if (f_ptr->info & FIELD_INFO_MARK) {
				/* See if it has a special name */
				field_script_single(f_ptr, FIELD_ACT_LOOK, ":s", LUA_RETURN(name));
				if (name) {
					/* Copy the string into the temp buffer */
					strncpy(fld_name, name, 40);

					/* Anything there? */
					if (!fld_name[0]) {
						/* Default to field name */
						strncpy(fld_name, t_ptr->name, 40);
					}

					/* Free string allocated to hold return value */
					string_free(name);
				} else {
					/* Default to field name */
					strncpy(fld_name, t_ptr->name, 40);
				}
				s3 = is_a_vowel(fld_name[0]) ? "an " : "a ";

				/* Describe the field */
				prtf(0, 0, "%s %s%s:", pmt, s3, fld_name);
			}
		} else
		if (pc_ptr->feat && (pc_ptr->player & GRID_KNOWN)) {
			cptr name, s3;
			name = f_name + f_info[pc_ptr->feat].name;
			if (f_info[pc_ptr->feat].flags & FF_OBJECT) {
				/* Pick proper indefinite article */
				s3 = (is_a_vowel(name[0])) ? "an " : "a ";
			} else  {
				s3 = "";
			}
			prtf(col,row, "%s %s%s:", pmt, s3, name);
				
		}
	}
	return FALSE;
}


/*
 * Sorting hook -- comp function -- by "distance to player"
 *
 * We use "u" and "v" to point to arrays of "x" and "y" positions,
 * and sort the arrays by double-distance to the player.
 */
bool ang_sort_comp_distance(vptr u, vptr v, int a, int b)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	s16b *x = (s16b *)(u);
	s16b *y = (s16b *)(v);

	int da, db, kx, ky;

	/* Absolute distance components */
	kx = x[a];
	kx -= px;
	kx = ABS(kx);
	ky = y[a];
	ky -= py;
	ky = ABS(ky);

	/* Approximate Double Distance to the first point */
	da = ((kx > ky) ? (kx + kx + ky) : (ky + ky + kx));

	/* Absolute distance components */
	kx = x[b];
	kx -= px;
	kx = ABS(kx);
	ky = y[b];
	ky -= py;
	ky = ABS(ky);

	/* Approximate Double Distance to the first point */
	db = ((kx > ky) ? (kx + kx + ky) : (ky + ky + kx));

	/* Compare the distances */
	return (da <= db);
}


/*
 * Sorting hook -- swap function -- by "distance to player"
 *
 * We use "u" and "v" to point to arrays of "x" and "y" positions,
 * and sort the arrays by distance to the player.
 */
void ang_sort_swap_distance(vptr u, vptr v, int a, int b)
{
	s16b *x = (s16b *)(u);
	s16b *y = (s16b *)(v);

	s16b temp;

	/* Swap "x" */
	temp = x[a];
	x[a] = x[b];
	x[b] = temp;

	/* Swap "y" */
	temp = y[a];
	y[a] = y[b];
	y[b] = temp;
}



/*
 * Hack -- help "select" a location (see below)
 */
static s16b target_pick(int x1, int y1, int dx, int dy)
{
	int i, v;

	int x2, y2, x3, y3, x4, y4;

	int b_i = -1, b_v = 9999;


	/* Scan the locations */
	for (i = 0; i < temp_n; i++)
	{
		/* Point 2 */
		x2 = temp_x[i];
		y2 = temp_y[i];

		/* Directed distance */
		x3 = (x2 - x1);
		y3 = (y2 - y1);

		/* Verify quadrant */
		if (dx && (x3 * dx <= 0)) continue;
		if (dy && (y3 * dy <= 0)) continue;

		/* Absolute distance */
		x4 = ABS(x3);
		y4 = ABS(y3);

		/* Verify quadrant */
		if (dy && !dx && (x4 > y4)) continue;
		if (dx && !dy && (y4 > x4)) continue;

		/* Approximate Double Distance */
		v = ((x4 > y4) ? (x4 + x4 + y4) : (y4 + y4 + x4));

		/* XXX XXX XXX Penalize location */

		/* Track best */
		if ((b_i >= 0) && (v >= b_v)) continue;

		/* Track best */
		b_i = i;
		b_v = v;
	}

	/* Result */
	return (b_i);
}


/*
 * Hack -- determine if a given location is "interesting"
 */
static bool target_set_accept(int x, int y)
{
	int px = p_ptr->px;
	int py = p_ptr->py;

	cave_type *c_ptr;
	pcave_type *pc_ptr;
	feature_type *feat_ptr;

	object_type *o_ptr;
	field_type *f_ptr;

	byte feat;

	/* Player grid is always interesting */
	if ((y == py) && (x == px)) return (TRUE);


	/* Handle hallucination */
	if (query_timed(TIMED_IMAGE)) return (FALSE);

	/* paranoia */
	if (!in_boundsp(x, y)) return (FALSE);

	/* Examine the grid */
	c_ptr = area(x, y);
	pc_ptr = parea(x, y);

	/* Visible monsters */
	if (c_ptr->m_idx)
	{
		monster_type *m_ptr = &m_list[c_ptr->m_idx];

		/* Visible monsters */
		if (m_ptr->ml) return (TRUE);
	}

	/* Scan all objects in the grid */
	OBJ_ITT_START (c_ptr->o_idx, o_ptr)
	{
		/* Memorized object, not squelched */
		if ((o_ptr->info & OB_SEEN) && (!SQUELCH(o_ptr->k_idx) || FLAG(o_ptr, TR_SQUELCH))) return (TRUE);
	}
	OBJ_ITT_END;

	/* Scan all fields in the grid */
	FLD_ITT_START (c_ptr->fld_idx, f_ptr)
	{
		/* Memorized , lookable field */
		if ((f_ptr->info & (FIELD_INFO_MARK | FIELD_INFO_NO_LOOK)) ==
			FIELD_INFO_MARK) return (TRUE);
	}
	FLD_ITT_END;

	/* Interesting memorized features */
	feat = pc_ptr->feat;
	feat_ptr = &(f_info[pc_ptr->feat]);

	/* Ignore hidden terrain */
	if (feat_ptr->flags & FF_HIDDEN) return (FALSE);

	/* Notice the Pattern */
	if (feat_ptr->flags & FF_PATTERN) return (TRUE);

	/* Notice doors */
	if (feat_ptr->flags & FF_DOOR) return (TRUE);

	/* Notice stairs */
	if (feat_ptr->flags & FF_EXIT_UP) return (TRUE);
	if (feat_ptr->flags & FF_EXIT_DOWN) return (TRUE);

	/* Notice quest terrain */
	if (feat_ptr->flags & FF_QUEST) return (TRUE);

	/* Notice veins with treasure */
	if (feat_ptr->flags & FF_DIG_GOLD) return (TRUE);
	if (feat_ptr->flags & FF_DIG_CUSTOM) return (TRUE);

	/* Notice the Pattern */
	/*if (cave_pattern_grid(pc_ptr)) return (TRUE);

	/* Notice doors */
	/*if (feat == FEAT_OPEN) return (TRUE);
	if (feat == FEAT_BROKEN) return (TRUE);

	/* Notice stairs */
	/*if (feat == FEAT_LESS) return (TRUE);
	if (feat == FEAT_MORE) return (TRUE);

	/* Notice doors */
	/*if (feat == FEAT_CLOSED) return (TRUE);

	/* Notice veins with treasure */
	/*if (feat == FEAT_MAGMA_K) return (TRUE);
	if (feat == FEAT_QUARTZ_K) return (TRUE);

	/* Nope */
	return (FALSE);
}


/*
 * Prepare the "temp" array for "target_set"
 *
 * Return the number of target_able monsters in the set.
 */
static void target_set_prepare(int mode)
{
	int y, x;

	/* Reset "temp" array */
	temp_n = 0;

	/* Scan the current panel */
	for (y = p_ptr->panel_y1; y <= p_ptr->panel_y2; y++)
	{
		for (x = p_ptr->panel_x1; x <= p_ptr->panel_x2; x++)
		{
			cave_type *c_ptr;

			if (!in_bounds2(x, y)) continue;

			c_ptr = area(x, y);

			/* Require "interesting" contents */
			if (!target_set_accept(x, y)) continue;

			/* Require target_able monsters for "TARGET_KILL" */
			if ((mode & (TARGET_KILL)) && !target_able(c_ptr->m_idx)) continue;

			/* Require hostile creatures if "TARGET_HOST" is used */
			if ((mode & (TARGET_HOST))
				&& !is_hostile(&m_list[c_ptr->m_idx])) continue;

			/* Do not target unknown mimics if we want monsters */
			if ((mode & (TARGET_KILL | TARGET_HOST)) &&
				(m_list[c_ptr->m_idx].smart & SM_MIMIC))
			{
				continue;
			}

			/* Paranoia */
			if (temp_n >= TEMP_MAX) continue;

			/* Save the location */
			temp_x[temp_n] = x;
			temp_y[temp_n] = y;
			temp_n++;
		}
	}

	/* Set the sort hooks */
	ang_sort_comp = ang_sort_comp_distance;
	ang_sort_swap = ang_sort_swap_distance;

	/* Sort the positions */
	ang_sort(temp_x, temp_y, temp_n);
}


/*
 * Examine a grid, return a keypress.
 *
 * The "mode" argument contains the "TARGET_LOOK" bit flag, which
 * indicates that the "space" key should scan through the contents
 * of the grid, instead of simply returning immediately.  This lets
 * the "look" command get complete information, without making the
 * "target" command annoying.
 *
 * The "info" argument contains the "commands" which should be shown
 * inside the "[xxx]" text.  This string must never be empty, or grids
 * containing monsters will be displayed with an extra comma.
 *
 * Note that if a monster is in the grid, we update both the monster
 * recall info and the health bar info to track that monster.
 *
 * This function must handle blindness/hallucination.
 */
static int target_set_aux(int x, int y, int mode, cptr info)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	cave_type *c_ptr = area(x, y);
	pcave_type *pc_ptr = parea(x, y);

	cptr s1, s2, s3;
	char s4[30];
	
	bool boring;
	bool seen = FALSE;

	int feat;

	int query;

	object_type *o_ptr;
	field_type *f_ptr;

	/* Repeat forever */
	while (1)
	{
		/* Paranoia */
		query = ' ';

		/* backup and clear buttons */
		button_backup_all(TRUE);

		/* Assume boring */
		boring = TRUE;

		/* Default */
		s1 = "You see ";
		s2 = "";
		s3 = "";
		
		/* Create "s4" string, containing the vector, if in show_coords, 
		 * otherwise just show distance */
		if (show_coords)
		{
			if (p_ptr->px == x && p_ptr->py == y)
			{
				sprintf(s4, "0");
			}
			else if (p_ptr->px == x)
			{
				sprintf(s4, "%d%c",
						ABS(p_ptr->py - y), 
						(p_ptr->py > y ? 'N' : 'S')	);
			}
			else if (p_ptr->py == y)
			{
				sprintf(s4, "%d%c",
						ABS(p_ptr->px - x), 
						(p_ptr->px > x ? 'W' : 'E')	);
			}
			else
			{
				sprintf(s4, "%d:%d%c %d%c",
						distance(x,y,p_ptr->px, p_ptr->py),
						ABS(p_ptr->py - y), 
						(p_ptr->py > y ? 'N' : 'S'),
						ABS(p_ptr->px - x), 
						(p_ptr->px > x ? 'W' : 'E')	);
			}
			
		} else
			sprintf(s4, "%d", distance(x,y,p_ptr->px, p_ptr->py));

		/* Hack -- under the player */
		if ((y == py) && (x == px))
		{
			/* Description */
			s1 = "You are ";

			/* Preposition */
			s2 = "on ";
		}


		/* Hack -- hallucination */
		if (query_timed(TIMED_IMAGE))
		{
			cptr name = "something strange";

			/* Display a message */
			prtf(0, 0, "%s%s%ssomething strange [%s] (%s)", s1, s2, s3, name, info, s4);
			move_cursor_relative(x, y);
			query = inkey_m();

			/* restore previous buttons */
			button_restore();

			/* Stop on everything but "return" */
			if ((query != '\r') && (query != '\n')) break;

			/* Repeat forever */
			continue;
		}


		/* Actual monsters */
		if (c_ptr->m_idx)
		{
			monster_type *m_ptr = &m_list[c_ptr->m_idx];
			monster_race *r_ptr = &r_info[m_ptr->r_idx];

			/* Visible */
			if (m_ptr->ml)
			{
				bool recall = FALSE;

				char m_name[80];

				/* Not boring */
				boring = FALSE;

				/* Check for mimics + obtain object description */
				if ((m_ptr->smart & SM_MIMIC) && mimic_desc(m_name, r_ptr))
				{
					/* Describe the object */
					s3 = "a ";
					prtf(0, 0, "%s%s%s%s [%s] (%s)", s1, s2, s3, m_name, info, s4);
					move_cursor_relative(x, y);
					query = inkey_m();

					/* restore previous buttons */
					button_restore();

					/* Always stop at "normal" keys */
					if ((query != '\r') && (query != '\n')
						&& (query != ' ')) break;

					/* Sometimes stop at "space" key */
					if ((query == ' ') && !(mode & TARGET_LOOK)) break;

					/* Change the intro */
					s1 = "It is ";

					/* Preposition */
					s2 = "on ";
				}

				/* Normal monsters */
				else
				{
					/* Hack -- track this monster race */
					monster_race_track(m_ptr->r_idx);

					/* Hack -- health bar for this monster */
					health_track(c_ptr->m_idx);

					/* Hack -- handle stuff */
					handle_stuff();

					/* Interact */
					while (1)
					{
						/* Recall */
						if (recall)
						{
							/* Save */
							screen_save();

							/* Recall on screen */
							screen_roff_mon(m_ptr->r_idx, 0);

							/* Hack -- Complete the prompt (again) */
							roff("  [$Xr,%s] (%s)", info, s4);

							/* Command */
							query = inkey_m();

							/* Restore */
							screen_load();

							/* If we had a mouse press, consume it and clear the recall info */
							if (query & 0x80) {
								char b;
								int mx, my;
								Term_getmousepress(&b, &mx, &my);
								query = 'r';
							}
						}

						/* Normal */
						else
						{
							cptr attitude;

							if (is_pet(m_ptr))
								attitude = " (pet) ";
							else if (is_friendly(m_ptr))
								attitude = " (friendly) ";
							else
								attitude = " ";

							/* Describe, and prompt for recall */
							prtf(0, 0, "%s%s%s%v (%s)%s[$Xr,%s] (%s)",
									s1, s2, s3, MONSTER_FMT(m_ptr, 0x08),
									look_mon_desc(c_ptr->m_idx), attitude,
									info, s4);

							/* Place cursor */
							move_cursor_relative(x, y);

							/* Command */
							query = inkey_m();
						}

						/* restore previous buttons */
						button_restore();

						/* Normal commands */
						if (query != 'r') break;

						/* backup previous buttons again */
						button_backup_all(TRUE);

						/* Toggle recall */
						recall = !recall;
					}

					/* Always stop at "normal" keys */
					if ((query != '\r') && (query != '\n')
						&& (query != ' ')) break;

					/* Sometimes stop at "space" key */
					if ((query == ' ') && !(mode & (TARGET_LOOK))) break;

					/* backup previous buttons again */
					button_backup_all(TRUE);

					/* Change the intro */
					s1 = "It is ";

					/* Hack -- take account of gender */
					if (FLAG(r_ptr, RF_FEMALE)) s1 = "She is ";
					else if (FLAG(r_ptr, RF_MALE)) s1 = "He is ";

					/* Use a preposition */
					s2 = "carrying ";

					/* Scan all objects being carried */
					OBJ_ITT_START (m_ptr->hold_o_idx, o_ptr)
					{
						/* Describe the object */
						prtf(0, 0, "%s%s%s%v [%s] (%s)", s1, s2, s3,
							 OBJECT_FMT(o_ptr, TRUE, 3), info, s4);
						move_cursor_relative(x, y);
						query = inkey_m();

						/* restore previous buttons */
						button_restore();

						/* Always stop at "normal" keys */
						if ((query != '\r') && (query != '\n')
							&& (query != ' '))
						{
							return (query);
						}

						/* Sometimes stop at "space" key */
						if ((query == ' ') && !(mode & (TARGET_LOOK)))
						{
							return (query);
						}

						/* backup previous buttons again */
						button_backup_all(TRUE);

						/* Change the intro */
						s2 = "also carrying ";
					}
					OBJ_ITT_END;

					/* Use a preposition */
					s2 = "on ";
				}
			}
		}

		/* Scan all objects in the grid */
		if (easy_floor)
		{
			int floor_num;

			object_type *o_ptr = test_floor(&floor_num, c_ptr, 0x02);

			/* Any items there? */
			if (o_ptr)
			{
				/* Not boring */
				boring = FALSE;

				while (1)
				{
					if (floor_num == 1)
					{
						/* Describe the object */
						prtf(0, 0, "%s%s%s%v [%s] (%s)",
								s1, s2, s3, OBJECT_FMT(o_ptr, TRUE, 3), info, s4);
					}
					else
					{
						/* Message */
						prtf(0, 0, "%s%s%sa pile of %d items [%c,%s] (%s)",
								s1, s2, s3, floor_num,
								rogue_like_commands ? 'x' : 'l', info, s4);
					}

					move_cursor_relative(x, y);

					/* Command */
					query = inkey_m();

					/* restore previous buttons */
					button_restore();

					/* Display list of items (query == "el", not "won") */
					if ((floor_num > 1)
						&& (query == (rogue_like_commands ? 'x' : 'l')))
					{
						/* Save screen */
						screen_save();

						/* Display */
						show_list(c_ptr->o_idx, FALSE);

						/* Prompt */
						prtf(0, 0, "Hit any key to continue");

						/* Wait */
						(void)inkey();

						/* Load screen */
						screen_load();
					}
					else
					{
						/* Stop */
						break;
					}

					/* backup previous buttons again */
					button_backup_all(TRUE);
				}

				/* Stop */
				break;
			}
		}

		/* Scan all objects in the grid */
		OBJ_ITT_START (c_ptr->o_idx, o_ptr)
		{
			/* Describe it */
			if (o_ptr->info & OB_SEEN)
			{
				/* Not boring */
				boring = FALSE;

				/* Describe the object */
				prtf(0, 0, "%s%s%s%v [%s] (%s)", s1, s2, s3,
					 OBJECT_FMT(o_ptr, TRUE, 3), info, s4);
				move_cursor_relative(x, y);
				query = inkey_m();

				/* restore previous buttons */
				button_restore();

				/* Always stop at "normal" keys */
				if ((query != '\r') && (query != '\n') && (query != ' '))
				{
					return (query);
				}

				/* Sometimes stop at "space" key */
				if ((query == ' ') && !(mode & TARGET_LOOK))
				{
					return (query);
				}

				/* backup previous buttons again */
				button_backup_all(TRUE);

				/* Change the intro */
				s1 = "It is ";

				/* Plurals */
				if (o_ptr->number != 1) s1 = "They are ";

				/* Preposition */
				s2 = "on ";
			}
		}
		OBJ_ITT_END;

		/* Scan all fields in the grid */
		FLD_ITT_START (c_ptr->fld_idx, f_ptr)
		{
			field_thaum *t_ptr = &t_info[f_ptr->t_idx];

			char fld_name[41];

			/* Do not describe this field */
			if (f_ptr->info & FIELD_INFO_NO_LOOK) continue;

			/* Describe if if is visible and known. */
			if (f_ptr->info & FIELD_INFO_MARK)
			{
				char *name = NULL;

				/* See if it has a special name */
				field_script_single(f_ptr, FIELD_ACT_LOOK, ":s", LUA_RETURN(name));

				if (name)
				{
					/* Copy the string into the temp buffer */
					strncpy(fld_name, name, 40);

					/* Anything there? */
					if (!fld_name[0])
					{
						/* Default to field name */
						strncpy(fld_name, t_ptr->name, 40);
					}

					/* Free string allocated to hold return value */
					string_free(name);
				}
				else
				{
					/* Default to field name */
					strncpy(fld_name, t_ptr->name, 40);
				}

				/* Not boring */
				boring = FALSE;

				s3 = is_a_vowel(fld_name[0]) ? "an " : "a ";

				/* Describe the field */
				prtf(0, 0, "%s%s%s%s [%s] (%s)", s1, s2, s3, fld_name, info, s4);
				move_cursor_relative(x, y);
				query = inkey_m();

				/* restore previous buttons */
				button_restore();

				/* Always stop at "normal" keys */
				if ((query != '\r') && (query != '\n') && (query != ' '))
				{
					return (query);
				}

				/* Sometimes stop at "space" key */
				if ((query == ' ') && !(mode & TARGET_LOOK))
				{
					return (query);
				}

				/* backup previous buttons again */
				button_backup_all(TRUE);

				/* Change the intro */
				s1 = "It is ";

				/* Preposition */
				s2 = "on ";

				/* Hack - we've seen a field here */
				seen = TRUE;
			}
		}
		FLD_ITT_END;

		/* Sometimes a field stops the feat from being mentioned */
		if (fields_have_flags(c_ptr, FIELD_INFO_NFT_LOOK))
		{
			/*
			 * Only if we know about the field will it stop the
			 * feat from being described.
			 */

			/* If we have seen something */
			if (seen)
			{
				if ((query != '\r') && (query != '\n'))
				{
					/* Just exit */
					break;
				}
				else
				{
					/* Back for more */
					continue;
				}
			}
		}

		/* Get memorised terrain feature */
		feat = pc_ptr->feat;

		/* Terrain feature if needed */
		if (boring || (feat >= FEAT_OPEN))
		{
			bool perma = cave_perma_grid(pc_ptr);
			bool wall = cave_wall_grid(pc_ptr);
			cptr name = f_name + f_info[feat].name;

			/* Hack -- handle unknown grids */
			if (feat == FEAT_NONE && !(pc_ptr->player & GRID_KNOWN)) 
				name = "unknown grid";
			else if (feat == FEAT_NONE)
			{
				/* feat = FEAT_NONE only because we can't see into
				 * this grid right now, but we know what it is.
				 */
				feat = c_ptr->feat;
				perma = cave_perma_grid(c_ptr);
				wall = cave_wall_grid(c_ptr);
				name = f_name + f_info[feat].name;
			}


			/* Pick a prefix for the pattern and stairs */
			if (*s2 && perma)
			{
				s2 = "on ";
			}
			else if (*s2 && wall)
			{
				s2 = "in ";
			}

			if (f_info[feat].flags & FF_OBJECT)
			//if (f_info[feat].flags & FF_OVERLAY)
			{
				/* Pick proper indefinite article */
				s3 = (is_a_vowel(name[0])) ? "an " : "a ";
			}
			else
			{
				s3 = "";
			}

			/* Display a message */
			if (p_ptr->state.wizard)
				prtf(0, 0, "%s%s%s%s [%s] (%d:%d) (%s)", s1, s2, s3, name,
						info, y, x, s4);
			else
				prtf(0, 0, "%s%s%s%s [%s] (%s)", s1, s2, s3, name, info, s4);
			move_cursor_relative(x, y);
			query = inkey_m();

			/* restore previous buttons */
			button_restore();

			/* Always stop at "normal" keys */
			if ((query != '\r') && (query != '\n') && (query != ' ')) break;

			/* backup previous buttons again */
			button_backup_all(TRUE);
		}

		/* restore previous buttons */
		button_restore();

		/* Stop on everything but "return" */
		if ((query != '\r') && (query != '\n')) break;
	}

	/* Keep going */
	return (query);
}


/*
 * Handle "target" and "look".
 *
 * Note that this code can be called from "get_aim_dir()".
 *
 * All locations must be on the current panel.
 *
 * Hack -- targeting/observing an "outer border grid" may induce
 * problems, so this is not currently allowed.
 *
 * The player can use the direction keys to move among "interesting"
 * grids in a heuristic manner, or the "space", "+", and "-" keys to
 * move through the "interesting" grids in a sequential manner, or
 * can enter "location" mode, and use the direction keys to move one
 * grid at a time in any direction.  The "t" (set target) command will
 * only target a monster (as opposed to a location) if the monster is
 * target_able and the "interesting" mode is being used.
 *
 * The current grid is described using the "look" method above, and
 * a new command may be entered at any time, but note that if the
 * "TARGET_LOOK" bit flag is set (or if we are in "location" mode,
 * where "space" has no obvious meaning) then "space" will scan
 * through the description of the current grid until done, instead
 * of immediately jumping to the next "interesting" grid.  This
 * allows the "target" command to retain its old semantics.
 *
 * The "*", "+", and "-" keys may always be used to jump immediately
 * to the next (or previous) interesting grid, in the proper mode.
 *
 * The "return" key may always be used to scan through a complete
 * grid description (forever).
 *
 * This command will cancel any old target, even if used from
 * inside the "look" command. This has been changed, this only happens
 * with target_set(), not target_set_interactive(). Also, if possible,
 * the cursor starts on the current target with target_set_interactive().
 */
bool target_set_interactive(int mode, int x, int y)
{
	int i, d, m, t, bd;

	bool done = FALSE;

	bool flag = TRUE;

	char query;

	char info[80];

	cave_type *c_ptr;

	int wid, hgt;

	/* These are used for displaying the path to the target */
	int path_drawn = 0;
	sint path_n = 0;
	coord path_g[2*MAX_RANGE+1];
	char old_path_char[MAX_RANGE];
	byte old_path_attr[MAX_RANGE];

	m = 0;
	/* Prepare the "temp" array */
	target_set_prepare(mode);

	if (p_ptr->target_who < 0) {
		/* we have a targeted spot, so start on that spot */
		flag = FALSE;
		y = p_ptr->target_row;
		x = p_ptr->target_col;
	} else
	if (temp_n && (p_ptr->target_who > 0)
		&& target_able(p_ptr->target_who))
	{
		/* check if the current target is in the list */
		for (m = 0; m < temp_n; m++) {
			y = temp_y[m];
			x = temp_x[m];

			/* Access */
			c_ptr = area(x, y);
			if (c_ptr->m_idx && (c_ptr->m_idx == p_ptr->target_who)) {
				/* if so, start with it selected */
				break;
			}
		}
		if (m >= temp_n) {
			/* Current target not in list start near the player */
			m = 0;
			y = p_ptr->py;
			x = p_ptr->px;

			/* Cancel target */
			p_ptr->target_who = 0;

			/* Cancel tracking */
			/* health_track(0); */
		}
	} else
	{
		/* see if the desired point is on an interesting grid */
		for (m = 0; m < temp_n; m++) {
			if ((temp_y[m] == y) &&(temp_x[m] == x)) {
				break;
			}
		}
		if (m >= temp_n) {
			flag = FALSE;
		}
	}

	/* Interact */
	while (!done)
	{
		/* Interesting grids */
		if (flag && temp_n)
		{
			y = temp_y[m];
			x = temp_x[m];

			/* Access */
			c_ptr = area(x, y);

			if (target_able(c_ptr->m_idx))
			{
				strcpy(info, "$Xq,$Xt,$Xp,$Xo,$X+,$X-,<dir>");
			}

			/* Dis-allow target */
			else
			{
				strcpy(info, "$Xq,$Xp,$Xo,$X+,$X-,<dir>");
			}

			/* Draw the path in "target" mode. If there is one */
			if (mode & (TARGET_KILL)) {
				/* draw so the path overlay will look right */
				Term_fresh();
				/* Find the path. */
				path_n = project_path(path_g, p_ptr->px, p_ptr->py, x, y, PROJECT_THRU);
				/* Draw the path */
				path_drawn = draw_path(path_n, path_g, old_path_attr, old_path_char, p_ptr->px, p_ptr->py);
			}

			/* Describe and Prompt */
			query = target_set_aux(x, y, mode, info);

			/* Remove the path */
			if (path_drawn) {
				remove_path(path_n, path_g, old_path_attr, old_path_char);
				Term_fresh();
				path_drawn = 0;
			}

			/* Cancel tracking */
			/* health_track(0); */

			/* Assume no "direction" */
			d = 0;

			/* Analyze the mouse press */
			if (query & 0x80) {
				char b;
				int mx, my, tx, ty;

				Term_getmousepress(&b, &mx, &my);
				/*mods = b & 0x78*/
				b = b & 0x07;
				if (b == 1) {
					ty = ((my-ROW_MAP) / tile_height_mult) + p_ptr->panel_y1;
					tx = ((mx-COL_MAP) / tile_width_mult) + p_ptr->panel_x1;
					if ((tx == x) && (ty == y)) {
						target_set_grid(x, y);
						done = TRUE;
					} else {
						x = tx;
						y = ty;
						d = 5;
						/* see if there is something interesting here */
						for (i = 0; i < temp_n; i++) {
							if ((x == temp_x[i]) && (y == temp_y[i])) {
								m = i;
								break;
							}
						}
						/* if not, leave this mode */
						if (i == temp_n) {
							flag = FALSE;
						}
					}
				} else
				{
					p_ptr->target_who = 0;
					done = TRUE;
				}
			} else
			/* Analyze the key press */
			switch (query)
			{
				case ESCAPE:
				case 'q':
				{
					p_ptr->target_who = 0;
					done = TRUE;
					break;
				}

				case 't':
				case '.':
				case '5':
				case '0':
				{
					if (target_able(c_ptr->m_idx))
					{
						health_track(c_ptr->m_idx);
						p_ptr->target_who = c_ptr->m_idx;
						p_ptr->target_row = y;
						p_ptr->target_col = x;
						done = TRUE;
					}
					else
					{
						bell("Illegal target!");
					}
					break;
				}

				case ' ':
				case '*':
				case '+':
				{
					if (++m == temp_n)
					{
						m = 0;
						if (!expand_list) done = TRUE;
					}
					break;
				}

				case '-':
				{
					if (m-- == 0)
					{
						m = temp_n - 1;
						if (!expand_list) done = TRUE;
					}
					break;
				}

				case 'p':
				{
					/* Recenter the map around the player */
					verify_panel();

					/* Recalculate interesting grids */
					target_set_prepare(mode);

					y = p_ptr->py;
					x = p_ptr->px;

					/* Fall through */
				}

				case 'o':
				{
					flag = FALSE;
					break;
				}

				case 'm':
				{
					break;
				}

				default:
				{
					/* Extract the action (if any) */
					d = get_keymap_dir(query);

					if (!d) bell("Illegal command for target mode!");
					break;
				}
			}

			/* Hack -- move around */
			if (d)
			{
				/* Modified to scroll to monster */
				int x2 = x;
				int y2 = y;

				/* Find a new monster */
				i = target_pick(temp_x[m], temp_y[m], ddx[d], ddy[d]);

				/* Request to target past last interesting grid */
				while (flag && (i < 0))
				{
					/* Note the change */
					if (change_panel(ddx[d], ddy[d]))
					{
						int v = temp_y[m];
						int u = temp_x[m];

						/* Recalculate interesting grids */
						target_set_prepare(mode);

						/* Look at interesting grids */
						flag = TRUE;

						/* Find a new monster */
						i = target_pick(u, v, ddx[d], ddy[d]);

						/* Use that grid */
						if (i >= 0) m = i;
					}

					/* Nothing interesting */
					else
					{
						int dx = ddx[d];
						int dy = ddy[d];

						/* Get size */
						get_map_size(&wid, &hgt);

						/* Restore previous position */
						panel_center(x2, y2);

						/* Recalculate interesting grids */
						target_set_prepare(mode);

						/* Look at boring grids */
						flag = FALSE;

						/* Move */
						x = x2 + dx;
						y = y2 + dy;

						/* Apply the motion */
						if (panel_center(x, y)) target_set_prepare(mode);

						/* Slide into legality */
						if (x < p_ptr->min_wid) x = p_ptr->min_wid;
						else if (x >= p_ptr->max_wid) x = p_ptr->max_wid - 1;

						/* Slide into legality */
						if (y < p_ptr->min_hgt) y = p_ptr->min_hgt;
						else if (y >= p_ptr->max_hgt) y = p_ptr->max_hgt - 1;
					}
				}

				/* Use that grid */
				m = i;
			}
		}

		/* Arbitrary grids */
		else
		{
			/* Default prompt */
			strcpy(info, "$Xq,$Xt,$Xp,$Xm,$X+,$X-,<dir>");

			/* Draw the path in "target" mode. If there is one */
			if (mode & (TARGET_KILL)) {
				/* draw so the path overlay will look right */
				Term_fresh();
				/* Find the path. */
				path_n = project_path(path_g, p_ptr->px, p_ptr->py, x, y, PROJECT_THRU);
				/* Draw the path */
				path_drawn = draw_path(path_n, path_g, old_path_attr, old_path_char, p_ptr->px, p_ptr->py);
			}

			/* Describe and Prompt (enable "TARGET_LOOK") */
			query = target_set_aux(x, y, mode | TARGET_LOOK, info);

			/* Remove the path */
			if (path_drawn) {
				remove_path(path_n, path_g, old_path_attr, old_path_char);
				Term_fresh();
				path_drawn = 0;
			}

			/* Cancel tracking */
			/* health_track(0); */

			/* Assume no direction */
			d = 0;

			/* Analyze the mouse press */
			if (query & 0x80) {
				char b;
				int mx, my, tx, ty;

				Term_getmousepress(&b, &mx, &my);
				/*mods = b & 0x78*/
				b = b & 0x07;
				if (b == 1) {
					ty = ((my-ROW_MAP) / tile_height_mult) + p_ptr->panel_y1;
					tx = ((mx-COL_MAP) / tile_width_mult) + p_ptr->panel_x1;
					if ((tx == x) && (ty == y)) {
						target_set_grid(x, y);
						done = TRUE;
					} else {
						x = tx;
						y = ty;
						d = 5;
						/* see if there is something interesting here */
						for (i = 0; i < temp_n; i++) {
							if ((x == temp_x[i]) && (y == temp_y[i])) {
								m = i;
								flag = TRUE;
								break;
							}
						}
					}
				} else
				{
					p_ptr->target_who = 0;
					done = TRUE;
				}
			} else
			/* Analyze the keypress */
			switch (query)
			{
				case ESCAPE:
				case 'q':
				{
					p_ptr->target_who = 0;
					done = TRUE;
					break;
				}

				case 't':
				case '.':
				case '5':
				case '0':
				{
					p_ptr->target_who = -1;
					p_ptr->target_row = y;
					p_ptr->target_col = x;
					done = TRUE;
					break;
				}

				case ' ':
				case '*':
				case '+':
				case '-':
				{
					break;
				}

				case 'p':
				{
					/* Recenter the map around the player */
					verify_panel();

					/* Recalculate interesting grids */
					target_set_prepare(mode);

					y = p_ptr->py;
					x = p_ptr->px;

					/* Fall through */
				}

				case 'o':
				{
					break;
				}

				case 'm':
				{
					flag = TRUE;

					m = 0;
					bd = 999;

					/* Pick a nearby monster */
					for (i = 0; i < temp_n; i++)
					{
						t = distance(x, y, temp_x[i], temp_y[i]);

						/* Pick closest */
						if (t < bd)
						{
							m = i;
							bd = t;
						}
					}

					/* Nothing interesting */
					if (bd == 999) flag = FALSE;

					break;
				}

				default:
				{
					/* Extract the action (if any) */
					d = get_keymap_dir(query);

					if (!d) bell("Illegal command for target mode!");
					break;
				}
			}

			/* Handle "direction" */
			if (d)
			{
				int dx = ddx[d];
				int dy = ddy[d];

				/* Move */
				x += dx;
				y += dy;

				if (panel_center(x, y)) target_set_prepare(mode);

				/* Slide into legality */
				if (x < p_ptr->min_wid) x = p_ptr->min_wid;
				else if (x >= p_ptr->max_wid) x = p_ptr->max_wid - 1;

				/* Slide into legality */
				if (y < p_ptr->min_hgt) y = p_ptr->min_hgt;
				else if (y >= p_ptr->max_hgt) y = p_ptr->max_hgt - 1;
			}
		}
	}

	/* Forget */
	temp_n = 0;

	/* Clear the top line */
	clear_msg();

	/* Recenter the map around the player */
	verify_panel();

	/* Failure to set target */
	if (!p_ptr->target_who) return (FALSE);

	/* Success */
	return (TRUE);
}

bool target_set(int mode)
{
	/* Cancel target */
	/*p_ptr->target_who = 0;*/

	return target_set_interactive(mode, p_ptr->px, p_ptr->py);
}

/*
 * Sets the player's target to be the closest monster
 */
static bool target_closest(void)
{
	cave_type *c_ptr;

	target_set_prepare(TARGET_KILL | TARGET_HOST);

	/* Fail if there are no choosable targets */
	if (temp_n == 0) return (FALSE);

	/* Select target number 1 */
	c_ptr = area(temp_x[0], temp_y[0]);

	health_track(c_ptr->m_idx);
	p_ptr->target_who = c_ptr->m_idx;
	p_ptr->target_row = temp_y[0];
	p_ptr->target_col = temp_x[0];

	return (TRUE);
}

static cptr dir_desc[10] =
{
		"(none)",
		"southwest",
		"south",
		"southeast",
		"west",
		"(none)",
		"east",
		"northwest",
		"north",
		"northeast"
};

/*
 * Get an "aiming direction" from the user.
 *
 * The "dir" is loaded with 1,2,3,4,6,7,8,9 for "actual direction", and
 * "0" for "current target", and "-1" for "entry aborted".
 *
 * Note that "Force Target", if set, will pre-empt user interaction,
 * if there is a usable target already set.
 *
 * Note that confusion over-rides any (explicit?) user choice.
 */
bool get_aim_dir(int *dp)
{
	int dir, tx, ty;

	char command;

	cptr p;

	sint path_n = 0;
	coord path_g[2*MAX_RANGE+1];
	int path_drawn;
	char old_path_char[MAX_RANGE];
	byte old_path_attr[MAX_RANGE];

	/* see if we have a ui override */
	if (Term->get_aim_dir_hook) return (*(Term->get_aim_dir_hook))(dp);

	/* Initialize */
	*dp = 0;

	/* Global direction */
	dir = p_ptr->cmd.dir;

	/* Hack -- auto-target if requested */
	if (use_old_target && target_okay()) dir = 5;

	if (repeat_pull(dp))
	{
		/* Confusion? */

		/* Verify */
		if (!(*dp == 5 && !target_okay()))
		{
			/* Store direction */
			dir = *dp;
		}
		else
		{
			/* Invalid repeat - reset it */
			repeat_clear();
		}
	}

	tx = ty = -1;

	/* Ask until satisfied */
	while (!dir)
	{
		/* backup any previous buttons */
		button_backup_all(TRUE);

		/* Choose a prompt */
		if (target_okay()) {
			p = "Direction ($U'5' for target$Y5$V, $U'*' to re-target$Y*$V, $U'c' for closest$Yc$V, $UEscape to cancel$Y" ESCAPE_STR "$V)? ";
			/* show the path to target */
			/* Find the path. */
			path_n = project_path(path_g, p_ptr->px, p_ptr->py, p_ptr->target_col, p_ptr->target_row, PROJECT_THRU);
			/* Draw the path */
			path_drawn = draw_path(path_n, path_g, old_path_attr, old_path_char, p_ptr->px, p_ptr->py);
		} else {
			p = "Direction ($U'*' to choose a target$Y*$V, $U'c' for closest$Yc$V, $UEscape to cancel$Y" ESCAPE_STR "$V)? ";
		}

		/* Get a command (or Cancel) */
		if (!get_com_m(p, &command)) {
			/* restore any previous buttons */
			button_restore();
			break;
		}

		if (path_drawn) {
			remove_path(path_n, path_g, old_path_attr, old_path_char);
			Term_fresh();
			path_drawn = 0;
		}

		/* restore any previous buttons */
		button_restore();

		if (command & 0x80) {
			int x,y;
			char b;

			/* this was a mouse press, get the press (and translate it) */
			Term_getmousepress(&b,&x,&y);
			b = b & 0x07;
			if (b == 1) {
				y = ((y-ROW_MAP) / tile_height_mult) + p_ptr->panel_y1;
				x = ((x-COL_MAP) / tile_width_mult) + p_ptr->panel_x1;
				if ((x == tx) && (y == ty)) {
					dir = 5;
				} else {
					if (target_set_grid(x,y)) {
						tx = x;
						ty = y;
					}
				}
			} else
			if (b == 2) {
				break;
			} else
			if (b == 3) {
				y = ((y-ROW_MAP) / tile_height_mult) + p_ptr->panel_y1;
				x = ((x-COL_MAP) / tile_width_mult) + p_ptr->panel_x1;
				(void) target_look_grid(x,y,TRUE);
			} else
			if (b == 4) {
				dir = 8;
			} else
			if (b == 5) {
				dir = 2;
			} else
			if (b == 6) {
				dir = 4;
			} else
			if (b == 7) {
				dir = 6;
			} else
			{
				dir = 0;
			}

			/* Verify requested targets */
			if ((dir == 5) && !target_okay()) dir = 0;
		} else {
			/* Convert various keys to "standard" keys */
			switch (command)
			{
			case 'T':
			case 't':
			case '.':
			case '5':
			case '0':
			{
				/* Use current target */
				dir = 5;
				break;
			}

			case 'c':
			{
				if (target_closest()) dir = 5;
				else if (target_set(TARGET_KILL | TARGET_HOST)) dir = 5;
				break;
			}

			case '*':
			{
				/* Set new target */
				if (target_set(TARGET_KILL | TARGET_HOST)) dir = 5;
				break;
			}

			default:
			{
				/* Extract the action (if any) */
				dir = get_keymap_dir(command);

				break;
			}
			}

			/* Verify requested targets */
			if ((dir == 5) && !target_okay()) dir = 0;

			/* Error */
			if (!dir) bell("Illegal aim direction!");
		}
	}

	/* No direction */
	if (!dir) return (FALSE);

	/* Save the direction */
	p_ptr->cmd.dir = dir;

	/* Check for confusion */
	if (query_timed(TIMED_CONFUSED))
	{
		/* Random direction */
		dir = ddd[randint0(8)];
	}

	/* Notice confusion */
	if (p_ptr->cmd.dir != dir)
	{
		/* Warn the user */
		if (accessible_mode)
		{
			msgf ("You are confused.  You point %s.", dir_desc[dir]);
		}
		else
			msgf("You are confused.");
	}

	/* Save direction */
	(*dp) = dir;

	repeat_push(p_ptr->cmd.dir);

	/* A "valid" direction was entered */
	return (TRUE);
}


/*
 * Request a "movement" direction (1,2,3,4,6,7,8,9) from the user,
 * and place it into "cmd.dir", unless we already have one.
 *
 * This function should be used for all "repeatable" commands, such as
 * run, walk, open, close, disarm, spike, tunnel, etc, as well
 * as all commands which must reference a grid adjacent to the player,
 * and which may not reference the grid under the player.  Note that,
 * for example, it is no longer possible to "disarm" or "open" chests
 * in the same grid as the player.
 *
 * Direction "5" is illegal and will (cleanly) abort the command.
 *
 * This function tracks and uses the "global direction", and uses
 * that as the "desired direction", to which "confusion" is applied.
 */
bool get_rep_dir(int *dp)
{
	int dir;

	if (repeat_pull(dp))
	{
		/* Done, if not confused. */
		if (!query_timed(TIMED_CONFUSED)) return (TRUE);

		/* Standard confusion */
		if (randint0(100) < 75)
		{
			/* Random direction */
			dir = ddd[randint0(8)];
		}
		else
			dir = p_ptr->cmd.dir;

		/* Notice confusion */
		if (p_ptr->cmd.dir != dir)
		{
			/* Warn the user */
			if (accessible_mode)
			{
				msgf ("You are confused.  You stumble %s.", dir_desc[dir]);
			}
			else
				msgf("You are confused.");
		}

		/* Save direction */
		(*dp) = dir;

		return (TRUE);
	}

	/* Initialize */
	(*dp) = 0;

	/* Global direction */
	dir = p_ptr->cmd.dir;

	/* Get a direction */
	while (!dir)
	{
		char ch;

		/* Get a command (or Cancel) */
		if (!get_com("Direction (Escape to cancel)? ", &ch)) break;

		/* Look up the direction */
		dir = get_keymap_dir(ch);

		/* Oops */
		if (!dir) bell("Illegal repeatable direction!");
	}

	/* Aborted */
	if (!dir) return (FALSE);

	/* Save desired direction */
	p_ptr->cmd.dir = dir;

	/* Apply "confusion" */
	if (query_timed(TIMED_CONFUSED))
	{
		/* Standard confusion */
		if (randint0(100) < 75)
		{
			/* Random direction */
			dir = ddd[randint0(8)];
		}
	}

	/* Notice confusion */
	if (p_ptr->cmd.dir != dir)
	{
		/* Warn the user */
		if (accessible_mode)
		{
			msgf ("You are confused.  You stumble %s.", dir_desc[dir]);
		}
		else
			msgf("You are confused.");
	}

	/* Save direction */
	(*dp) = dir;

	repeat_push(dir);

	/* Success */
	return (TRUE);
}


bool get_hack_dir(int *dp)
{
	int dir;
	cptr p;
	char command;


	/* Initialize */
	(*dp) = 0;

	/* Global direction */
	dir = 0;

	/* (No auto-targeting) */

	/* Ask until satisfied */
	while (!dir)
	{
		/* Choose a prompt */
		if (!target_okay())
		{
			p = "Direction ('*' to choose a target, Escape to cancel)? ";
		}
		else
		{
			p = "Direction ('5' for target, '*' to re-target, Escape to cancel)? ";
		}

		/* Get a command (or Cancel) */
		if (!get_com(p, &command)) break;

		/* Convert various keys to "standard" keys */
		switch (command)
		{
				/* Use current target */
			case 'T':
			case 't':
			case '.':
			case '5':
			case '0':
			{
				dir = 5;
				break;
			}

				/* Set new target */
			case '*':
			{
				if (target_set(TARGET_KILL)) dir = 5;
				break;
			}

			default:
			{
				/* Look up the direction */
				dir = get_keymap_dir(command);

				break;
			}
		}

		/* Verify requested targets */
		if ((dir == 5) && !target_okay()) dir = 0;

		/* Error */
		if (!dir) bell("Illegal direction!");
	}

	/* No direction */
	if (!dir) return (FALSE);

	/* Save the direction */
	p_ptr->cmd.dir = dir;

	/* Check for confusion */
	if (query_timed(TIMED_CONFUSED))
	{
		/* XXX XXX XXX */
		/* Random direction */
		dir = ddd[randint0(8)];
	}

	/* Notice confusion */
	if (p_ptr->cmd.dir != dir)
	{
		/* Warn the user */
		if (accessible_mode)
		{
			msgf ("You are confused.  You point %s.", dir_desc[dir]);
		}
		else
			msgf("You are confused.");
	}

	/* Save direction */
	(*dp) = dir;

	/* A "valid" direction was entered */
	return (TRUE);
}


