/* File: run.c */

/* Purpose: running code */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"
#include "script.h"

/*
 * Check for a "known wall"
 */
static int see_wall(int x, int y)
{
	pcave_type *pc_ptr;

	feature_type *f_ptr;

	/* Illegal grids are "walls" */
	if (!in_boundsp(x, y)) return (TRUE);

	pc_ptr = parea(x, y);

	f_ptr = &f_info[pc_ptr->feat];

	/* Return block-los status */
	//return (f_ptr->flags & FF_BLOCK);
	return (!(f_ptr->flags & (FF_PWALK|FF_MWALK)));
}


/*
 * Check for an "unknown corner"
 */
static int see_nothing(int x, int y)
{
	cave_type *c_ptr;
	pcave_type *pc_ptr;

	/* Illegal grids are unknown */
	if (!in_boundsp(x, y)) return (FALSE);

	c_ptr = area(x, y);
	pc_ptr = parea(x, y);

	/* Memorized grids are always known */
	if (pc_ptr->feat) return (FALSE);

	/* Non-floor grids are unknown */
	if (cave_wall_grid(c_ptr)) return (TRUE);

	/* Viewable door/wall grids are known */
	if (player_can_see_grid(pc_ptr)) return (FALSE);

	/* Default */
	return (TRUE);
}

/*
 * Check for "interesting" terrain
 */
static int see_interesting(int x, int y)
{
	cave_type *c_ptr;
	pcave_type *pc_ptr;
	object_type *o_ptr;

	/* Illegal grids are boring */
	if (!in_boundsp(x, y)) return (FALSE);

	c_ptr = area(x, y);
	pc_ptr = parea(x, y);

	/* Check for a monster */
	if (c_ptr->m_idx)
	{
		monster_type *m_ptr = &m_list[c_ptr->m_idx];

		/* If it's visible, it's interesting */
		if (m_ptr->ml) return (TRUE);
	}

	/* Check for objects */
	OBJ_ITT_START(c_ptr->o_idx, o_ptr)
	{
		/* If it's squelched, it's not interesting */
		if (SQUELCH(o_ptr->k_idx) && !FLAG(o_ptr, TR_SQUELCH)) continue;

		/* If it's visible, it's interesting */
		if (o_ptr->info & OB_SEEN) return (TRUE);
	}
	OBJ_ITT_END;

	/* Check for traps */
	if (is_visible_trap(c_ptr)) return (TRUE);

	/* Check for building */
	if (is_build(c_ptr)) return (TRUE);

	/* Check memorized grids */
	if (pc_ptr->feat)
	{
		feature_type *feat;
		feat = &(f_info[pc_ptr->feat]);
		/* damaging terrain is interesting if the player does not have the 
		 * resistance, but is not interesting if the terrain is shallow (not deep)
		 * and the player has levitation.
		 */
		if (feat->flags & FF_DAMAGING) {
			if (!(feat->flags & FF_DEEP) && FLAG(p_ptr, TR_FEATHER)) {
				return (FALSE);
			}
			if ((feat->flags2 & FF_FIERY) && !res_fire_lvl()) {
				return (TRUE);
			}
			if ((feat->flags2 & FF_ICY) && !res_cold_lvl()) {
				return (TRUE);
			}
			if ((feat->flags2 & FF_ACID) && !res_acid_lvl()) {
				 return (TRUE);
			}
			if ((feat->flags2 & FF_ELEC) && !res_elec_lvl()) {
				return (TRUE);
			}
			if ((feat->flags2 & FF_POISON) && !res_pois_lvl()) {
				return (TRUE);
			}
		}
		/* non damaging liquid terrains are only interesting if the terrain
		 * is deep and the player does not have levitation.
		 */
		if ((feat->flags & FF_LIQUID) && (feat->flags & FF_DEEP)) {
			if (!FLAG(p_ptr, TR_FEATHER)) return (TRUE);
		}
		/* see if the terrain has a flag that is generally interesting */
		if ((feat->flags & (FF_MASK_INTERESTING))
			|| (feat->flags2 & (FF_MASK_INTERESTING2))) {
			/* if so, check if we are ignoring certain types of the generally
			 * interesting flags.
			 */
			if ((find_ignore_doors) 
				&& !((feat->flags & (FF_MASK_INTERESTING)) & ~FF_CLOSEABLE)) {
				/* we are ignoring open doors */
				return (FALSE);
			}
			if ((find_ignore_stairs)
				&& !((feat->flags & (FF_MASK_INTERESTING)) & ~(FF_EXIT_UP | FF_EXIT_DOWN))) {
				/* we are ignoring stairs */
				return (FALSE);
			}
			return (TRUE);
		}

	}

	/* Boring */
	return (FALSE);
}


#define RUN_MODE_START        0  /* Beginning of run */
#define RUN_MODE_OPEN         1  /* Running in a room or open area */
#define RUN_MODE_CORRIDOR     2  /* Running in a corridor */
#define RUN_MODE_WALL         3  /* Running along a wall */
#define RUN_MODE_FINISH       4  /* End of run */
#define RUN_MODE_ROAD         5  /* Run along a wilderness road */
#define RUN_MODE_WATER        6  /* Swim along a wilderness waterway */
#define RUN_MODE_PATH         7  /* Follow a precomputed path */

/*
 * There are exactly 36 interesting
 * 2-move combinations. We only consider combinations where the
 * second move shares at least one component (N, E, S or W) with
 * the first move, plus orthogonal pairs (e.g. N-E).
 *
 * Since there are 36, we can use (64-bit) bit fields to simplify the tables.
 */


#define RUN_N_NW  0x0000000000000001L
#define RUN_N_N   0x0000000000000002L
#define RUN_N_NE  0x0000000000000004L
#define RUN_NE_NW 0x0000000000000008L
#define RUN_NE_N  0x0000000000000010L
#define RUN_NE_NE 0x0000000000000020L
#define RUN_NE_E  0x0000000000000040L
#define RUN_NE_SE 0x0000000000000080L
#define RUN_E_NE  0x0000000000000100L
#define RUN_E_E   0x0000000000000200L
#define RUN_E_SE  0x0000000000000400L
#define RUN_SE_NE 0x0000000000000800L
#define RUN_SE_E  0x0000000000001000L
#define RUN_SE_SE 0x0000000000002000L
#define RUN_SE_S  0x0000000000004000L
#define RUN_SE_SW 0x0000000000008000L
#define RUN_S_SE  0x0000000000010000L
#define RUN_S_S   0x0000000000020000L
#define RUN_S_SW  0x0000000000040000L
#define RUN_SW_SE 0x0000000000080000L
#define RUN_SW_S  0x0000000000100000L
#define RUN_SW_SW 0x0000000000200000L
#define RUN_SW_W  0x0000000000400000L
#define RUN_SW_NW 0x0000000000800000L
#define RUN_W_SW  0x0000000001000000L
#define RUN_W_W   0x0000000002000000L
#define RUN_W_NW  0x0000000004000000L
#define RUN_NW_SW 0x0000000008000000L
#define RUN_NW_W  0x0000000010000000L
#define RUN_NW_NW 0x0000000020000000L
#define RUN_NW_N  0x0000000040000000L
#define RUN_NW_NE 0x0000000080000000L
#define RUN_N_E   0x0000000100000000L
#define RUN_N_W   0x0000000200000000L
#define RUN_E_N   0x0000000400000000L
#define RUN_E_S   0x0000000800000000L
#define RUN_S_E   0x0000001000000000L
#define RUN_S_W   0x0000002000000000L
#define RUN_W_N   0x0000004000000000L
#define RUN_W_S   0x0000008000000000L
#define RUN_IMPOSSIBLE   0xFFFFFF0000000000L


/* All the moves that start with each direction */
#define RUN_N  (RUN_N_NW  | RUN_N_N  | RUN_N_NE  | RUN_N_E  | RUN_N_W)
#define RUN_NE (RUN_NE_NW | RUN_NE_N | RUN_NE_NE | RUN_NE_E | RUN_NE_SE)
#define RUN_E  (RUN_E_NE  | RUN_E_E  | RUN_E_SE  | RUN_E_N  | RUN_E_S)
#define RUN_SE (RUN_SE_NE | RUN_SE_E | RUN_SE_SE | RUN_SE_S | RUN_SE_SW)
#define RUN_S  (RUN_S_SE  | RUN_S_S  | RUN_S_SW  | RUN_S_E  | RUN_S_W)
#define RUN_SW (RUN_SW_SE | RUN_SW_S | RUN_SW_SW | RUN_SW_W | RUN_SW_NW)
#define RUN_W  (RUN_W_SW  | RUN_W_W  | RUN_W_NW  | RUN_W_N  | RUN_W_S)
#define RUN_NW (RUN_NW_SW | RUN_NW_W | RUN_NW_NW | RUN_NW_N | RUN_NW_NE)

#define TEST_NONE     0
#define TEST_WALL     1
#define TEST_UNSEEN   2
#define TEST_FLOOR    3
#define TEST_FLOOR_S  4
#define TEST_WALL_S   5

/*
 * The tests.
 *
 * Some of these are hard to understand. Don't worry too
 * much, unless you really need to modify the table.
 *
 * The order the groups of checks are in is important!
 * Don't reorder them unless you're sure you know what
 * you're doing. The order within each group is arbitrary.
 *
 * Each test consists of:
 *
 *   What type of square test to do (if any), and what
 *   square to test as an offset from the player.
 *
 *   What other moves must be valid (0 for none). If there
 *   are multiple moves listed, the test passes if any of
 *   the listed moves are valid.
 *
 *   What moves to remove from consideration if both the
 *   checks listed pass.
 */
//typedef long long _u64b;
typedef unsigned _int64 _u64b;

static const struct
{
	int test;
	int dx, dy;
	_u64b test_mask;
	_u64b remove_mask;
} run_checks[] = {
/*
 * 0: Paranoia: Eliminate all never-used cases.
 */
{TEST_NONE, 0, 0, 0, RUN_IMPOSSIBLE},

/*
 * 1-4:
 * If the player starts out by moving into a branch corridor:
 *
 * ###    ###
 * b.@ -> b..
 * #.#    #@#
 * #a#    #a#
 *
 * we should realize that the player means to continue to the square marked
 * 'a', and we should ignore the possibility of moving to 'b' instead.
 *
 * To recognize this, we check for the presence of a floor tile that the
 * player might have come from next to, and remove any moves that would
 * result in "doubling back".
 *
 * This test is done *before* removing impossible moves, because
 * we need to know which direction the player came from.
 */
{TEST_FLOOR_S,  0, -1, RUN_S, RUN_NE | RUN_NW},
{TEST_FLOOR_S,  1,  0, RUN_W, RUN_NE | RUN_SE},
{TEST_FLOOR_S,  0,  1, RUN_N, RUN_SE | RUN_SW},
{TEST_FLOOR_S, -1,  0, RUN_E, RUN_NW | RUN_SW},

/* 5-28: Eliminate impossible moves */
{TEST_WALL, -2, -2, 0,        RUN_NW_NW},
{TEST_WALL, -1, -2, 0,        RUN_NW_N | RUN_N_NW},
{TEST_WALL,  0, -2, 0,        RUN_NW_NE | RUN_N_N | RUN_NE_NW},
{TEST_WALL,  1, -2, 0,        RUN_NE_N | RUN_N_NE},
{TEST_WALL,  2, -2, 0,        RUN_NE_NE},
{TEST_WALL,  2, -1, 0,        RUN_NE_E | RUN_E_NE},
{TEST_WALL,  2,  0, 0,        RUN_NE_SE | RUN_E_E | RUN_SE_NE},
{TEST_WALL,  2,  1, 0,        RUN_SE_E | RUN_E_SE},
{TEST_WALL,  2,  2, 0,        RUN_SE_SE},
{TEST_WALL,  1,  2, 0,        RUN_SE_S | RUN_S_SE},
{TEST_WALL,  0,  2, 0,        RUN_SE_SW | RUN_S_S | RUN_SW_SE},
{TEST_WALL, -1,  2, 0,        RUN_SW_S | RUN_S_SW},
{TEST_WALL, -2,  2, 0,        RUN_SW_SW},
{TEST_WALL, -2,  1, 0,        RUN_SW_W | RUN_W_SW},
{TEST_WALL, -2,  0, 0,        RUN_SW_NW | RUN_W_W | RUN_NW_SW},
{TEST_WALL, -2, -1, 0,        RUN_NW_W | RUN_W_NW},
{TEST_WALL, -1, -1, 0,        RUN_NW | RUN_N_W | RUN_W_N},
{TEST_WALL,  0, -1, 0,        RUN_N},
{TEST_WALL,  1, -1, 0,        RUN_NE | RUN_N_E | RUN_E_N},
{TEST_WALL,  1,  0, 0,        RUN_E},
{TEST_WALL,  1,  1, 0,        RUN_SE | RUN_S_E | RUN_E_S},
{TEST_WALL,  0,  1, 0,        RUN_S},
{TEST_WALL, -1,  1, 0,        RUN_SW | RUN_S_W | RUN_W_S},
{TEST_WALL, -1,  0, 0,        RUN_W},

/*
 * 29-32:
 * Allow the player to run in a pillared corridor with
 * a radius-2 light source, by removing some diagonal
 * moves into unknown squares. Example:
 *
 *  #.#
 * #...#
 * ##@##
 *
 * Note that this, like the previous set, can result
 * in missing an actual (but unusual) branch.
 * Generally the player will see the branch once we
 * move another step, but by that point it's too
 * late, so we keep going forward.
 */
{TEST_UNSEEN, -2, -2, RUN_N | RUN_W, RUN_NW_NW},
{TEST_UNSEEN,  2, -2, RUN_N | RUN_E, RUN_NE_NE},
{TEST_UNSEEN,  2,  2, RUN_S | RUN_E, RUN_SE_SE},
{TEST_UNSEEN, -2,  2, RUN_S | RUN_W, RUN_SW_SW},

/*
 * 33-40
 * Force the player to explore potential corners.
 *
 * Note: These tests will only be used if find_examine is
 * set to be true and the player has light radius = 1.
 *
 * Example:
 *
 *  A##
 *   .@
 *   .#
 *
 * In a situation like this, the player may run south-
 * west, but this would leave A unexplored if the
 * player has light radius 1.  (Radius 0, we don't bother
 * with exploring, and Radius 2 or more we won't have a
 * problem.)
 *
 * Thus, if A is unknown, prevent the player from running
 * starting with a move southwest, so long as moving west
 * is available.
 */
{TEST_UNSEEN, -2, -1, RUN_W, RUN_SW},
{TEST_UNSEEN, -2, 1, RUN_W, RUN_NW},
{TEST_UNSEEN, 2, -1, RUN_E, RUN_SE},
{TEST_UNSEEN, 2, 1, RUN_E, RUN_NE},
{TEST_UNSEEN, -1, -2, RUN_N, RUN_NE},
{TEST_UNSEEN, 1, -2, RUN_N, RUN_NW},
{TEST_UNSEEN, -1, 2, RUN_S, RUN_SE},
{TEST_UNSEEN, 1, 2, RUN_S, RUN_SW},

/*
 * Ensure that the player will take unknown corners by
 * preventing orthagonal moves to unknown squares when
 * a diagonal move exists.
 *
 * Example:
 *
 *  ?##
 *  ?.@
 *  ?.#
 *
 * In this situation we remove all the 'west' moves, because
 * they are to unknown squares, and it's possible to move
 * southwest instead.
 *
 * Note that this can miss a branch if the unknown square
 * is actually a floor square and cutting corners is
 * enabled.
 *
 * We have to be careful that we differentiate between these
 * two situations:
 *
 * ###       ##
 *  .@  vs   .@
 * #.#       .#
 *
 * The first we should NOT turn because there is obviously
 * a real branch. We handle this by (indirectly) checking
 * for the presence of a wall in the "unknown" direction,
 * in this case the wall SWW of the player.
 */
{TEST_UNSEEN, -1, -2, RUN_NE_N | RUN_NW_N, RUN_N_NW},
{TEST_UNSEEN,  0, -2, RUN_NE_N | RUN_NW_N, RUN_N_N},
{TEST_UNSEEN,  1, -2, RUN_NE_N | RUN_NW_N, RUN_N_NE},
{TEST_UNSEEN,  2, -1, RUN_NE_E | RUN_SE_E, RUN_E_NE},
{TEST_UNSEEN,  2,  0, RUN_NE_E | RUN_SE_E, RUN_E_E},
{TEST_UNSEEN,  2,  1, RUN_NE_E | RUN_SE_E, RUN_E_SE},
{TEST_UNSEEN,  1,  2, RUN_SE_S | RUN_SW_S, RUN_S_SE},
{TEST_UNSEEN,  0,  2, RUN_SE_S | RUN_SW_S, RUN_S_S},
{TEST_UNSEEN, -1,  2, RUN_SE_S | RUN_SW_S, RUN_S_SW},
{TEST_UNSEEN, -2,  1, RUN_NW_W | RUN_SW_W, RUN_W_SW},
{TEST_UNSEEN, -2,  0, RUN_NW_W | RUN_SW_W, RUN_W_W},
{TEST_UNSEEN, -2, -1, RUN_NW_W | RUN_SW_W, RUN_W_NW},

/* Prefer going straight over zig-zagging */
{TEST_NONE,  0,  0, RUN_N_N,  RUN_NE_NW | RUN_NW_NE},
{TEST_NONE,  0,  0, RUN_E_E,  RUN_NE_SE | RUN_SE_NE},
{TEST_NONE,  0,  0, RUN_S_S,  RUN_SE_SW | RUN_SW_SE},
{TEST_NONE,  0,  0, RUN_W_W,  RUN_NW_SW | RUN_SW_NW},

/* Prefer diagonals to 2-move orthogonal zig-zags */
{TEST_NONE,  0,  0, RUN_NE,  RUN_N_E | RUN_E_N},
{TEST_NONE,  0,  0, RUN_NW,  RUN_N_W | RUN_W_N},
{TEST_NONE,  0,  0, RUN_SE,  RUN_S_E | RUN_E_S},
{TEST_NONE,  0,  0, RUN_SW,  RUN_S_W | RUN_W_S},

/* Prefer moving diagonal then orthagonal over the reverse */
{TEST_NONE,  0,  0, RUN_NW_W, RUN_W_NW},
{TEST_NONE,  0,  0, RUN_NW_N, RUN_N_NW},
{TEST_NONE,  0,  0, RUN_NE_N, RUN_N_NE},
{TEST_NONE,  0,  0, RUN_NE_E, RUN_E_NE},
{TEST_NONE,  0,  0, RUN_SE_E, RUN_E_SE},
{TEST_NONE,  0,  0, RUN_SE_S, RUN_S_SE},
{TEST_NONE,  0,  0, RUN_SW_S, RUN_S_SW},
{TEST_NONE,  0,  0, RUN_SW_W, RUN_W_SW},
};

static const _u64b valid_dir_mask[10] = {
/* 0 */ 0,
/* 1 */ RUN_NW | RUN_W | RUN_SW | RUN_S | RUN_SE,
/* 2 */ RUN_SW | RUN_S | RUN_SE | RUN_E | RUN_W,
/* 3 */ RUN_SW | RUN_S | RUN_SE | RUN_E | RUN_NE,
/* 4 */ RUN_NW | RUN_W | RUN_SW | RUN_N | RUN_S,
/* 5 */ 0,
/* 6 */ RUN_SE | RUN_E | RUN_NE | RUN_N | RUN_S,
/* 7 */ RUN_NE | RUN_N | RUN_NW | RUN_W | RUN_SW,
/* 8 */ RUN_NE | RUN_N | RUN_NW | RUN_E | RUN_W,
/* 9 */ RUN_SE | RUN_E | RUN_NE | RUN_N | RUN_NW
};

static const _u64b basic_dir_mask[10] = {
/* 0 */ 0,
/* 1 */ RUN_SW,
/* 2 */ RUN_S,
/* 3 */ RUN_SE,
/* 4 */ RUN_W,
/* 5 */ 0,
/* 6 */ RUN_E,
/* 7 */ RUN_NW,
/* 8 */ RUN_N,
/* 9 */ RUN_NE
};

/*
 * Lists of walls that, if all set, mean we're
 * probably in a corridor.
 *
 * Generally we decide we're in a corridor if
 * two opposite squares are walls, or one square
 * and the two opposite corners, or all four
 * corners.
 */
static const _u64b corridor_test_mask[] = {
RUN_N | RUN_S,
RUN_E | RUN_W,
RUN_N | RUN_SE | RUN_SW,
RUN_E | RUN_NW | RUN_SW,
RUN_S | RUN_NE | RUN_NW,
RUN_W | RUN_SE | RUN_NE,
RUN_NW | RUN_NE | RUN_SE | RUN_SW
};


/*
 * Lists of walls that, if all set, mean we're probably running
 * alongside a wall.
 *
 * For horizontal or vertical walls this means that at least two walls
 * are present on one side along our path.
 *
 * ToDo: support diagonal walls
 */
static const _u64b wall_test_mask[10][6] =
{
/* 0 */ {0, 0, 0, 0, 0, 0},
/* 1 */ {0, 0, 0, 0, 0, 0},
/* 2 */ {
		RUN_E | RUN_SE,
		RUN_E | RUN_NE,
		RUN_W | RUN_SW,
		RUN_W | RUN_NW,
		RUN_SE | RUN_NE,
		RUN_SW | RUN_NW,
	},

/* 3 */ {0, 0, 0, 0, 0, 0},
/* 4 */ {
		RUN_S | RUN_SW,
		RUN_S | RUN_SE,
		RUN_N | RUN_NW,
		RUN_N | RUN_NE,
		RUN_SW | RUN_SE,
		RUN_NW | RUN_NE,
	},
/* 5 */ {0, 0, 0, 0, 0, 0},
/* 6 */ {
		RUN_S | RUN_SW,
		RUN_S | RUN_SE,
		RUN_N | RUN_NW,
		RUN_N | RUN_NE,
		RUN_SW | RUN_SE,
		RUN_NW | RUN_NE,
	},
/* 7 */ {0, 0, 0, 0, 0, 0},
/* 8 */ {
		RUN_E | RUN_SE,
		RUN_E | RUN_NE,
		RUN_W | RUN_SW,
		RUN_W | RUN_NW,
		RUN_SE | RUN_NE,
		RUN_SW | RUN_NW,
	},
/* 9 */ {0, 0, 0, 0, 0, 0},
};


/*
 * Determine the run algorithm to use.
 *
 * If we seem to be in a corridor, use the "follow" algorithm.
 * otherwise, use the "open" algorithm.
 *
 * Note that we delay selecting the mode to use until after
 * the player has moved the first step of the run.
 *
 * In general determining which algorithm to use is complicated.
 */
static void run_choose_mode(void)
{
	int px = p_ptr->px;
	int py = p_ptr->py;
	unsigned int i;
	_u64b wall_dirs = 0;

	/* if we have a path, use it */
	if (p_ptr->run.path) {
		p_ptr->run.mode = RUN_MODE_PATH;
		return;
	}

	/* Check valid dirs */
	for (i = 1; i < 10; i++)
	{
		if (see_wall(px + ddx[i], py + ddy[i]))
			wall_dirs |= basic_dir_mask[i];
	}

	/* Check for evidence we're in a corridor */
	for (i = 0; i < NUM_ELEMENTS(corridor_test_mask); i++)
	{
		/*
		 * If none of the elements in the mask are _not_
		 * set, we're in a corridor.
		 */
		if (!(~wall_dirs & corridor_test_mask[i]))
		{
			p_ptr->run.mode = RUN_MODE_CORRIDOR;
			return;
		}
	}

	/* Check for evidence we're following a wall */
	for (i = 0; i < NUM_ELEMENTS(wall_test_mask[p_ptr->run.cur_dir]); i++)
	{
		/*
		 * If none of the elements in the mask are _not_
		 * set, we're following a wall.
		 */
		if (!(~wall_dirs & wall_test_mask[p_ptr->run.cur_dir][i]))
		{
			p_ptr->run.mode = RUN_MODE_WALL;
			return;
		}
	}

	//if (!p_ptr->depth) {
		/* check if we are on a road, if so follow it */
	//	int x, y;
	//	x = ((u16b)p_ptr->wilderness_x / WILD_BLOCK_SIZE);
	//	y = ((u16b)p_ptr->wilderness_y / WILD_BLOCK_SIZE);
     
	//	if (wild[y][x].done.info & WILD_INFO_ROAD|WILD_INFO_TRACK) {
	//		p_ptr->run.mode = RUN_MODE_ROAD;
	//		return;
	//	} else
	//	if (wild[y][x].done.info & WILD_INFO_WATER) {
	//		p_ptr->run.mode = RUN_MODE_WATER;
	//		return;
	//	}
	//}

	/* Assume we're in the open */
	p_ptr->run.mode = RUN_MODE_OPEN;
}


/*
 * Check if anything interesting is nearby
 */
static int check_interesting(void)
{
	int px = p_ptr->px;
	int py = p_ptr->py;

	int i;

	for (i = 0; i < 8; i++)
	{
		/* Ignore the square we just moved off of */
		if (ddd[i] == p_ptr->run.old_dir)
			continue;

		/* Subtract rather than add so the previous check works */
		if (see_interesting(px - ddx_ddd[i], py - ddy_ddd[i]))
			return (TRUE);
	}

	return (FALSE);
}

/*
 * The corridor running algorithm.
 *
 * We start by determining all two-move combinations that the player could possibly
 * make, where the first move shares at least one component (N, E, S, or W) with
 * the move made last turn.
 *
 * Then we use various tests (given in the run_checks[] table above) to remove
 * some of the possibilities.
 *
 * If, after all the checks, all the two-move combinations left start with the
 * same move, we use that move. Otherwise, we're at a branch, so we stop.
 *
 * See run_checks[] for more detailed comments on some of the situations we
 * test for.
 */
static void run_corridor(int starting)
{
	int px = p_ptr->px;
	int py = p_ptr->py;

	_u64b valid_dirs = 0;
	unsigned int i;

	/* Check if we're next to something interesting. If we are, stop. */
	if (check_interesting())
	{
		p_ptr->run.mode = RUN_MODE_FINISH;
		return;
	}

	/* Add all possibly-legal dirs depending on previous direction */
	valid_dirs = valid_dir_mask[p_ptr->run.old_dir];

	/* Do magic */
	for (i = 0; i < NUM_ELEMENTS(run_checks); i++)
	{
		bool ok = TRUE;
		/* Hack: Avoid certain tests if find_examine not set. */
		if (i > 32 && i <= 40 && !find_examine) continue;

		if (run_checks[i].remove_mask & valid_dirs)
		{
			int dx = run_checks[i].dx;
			int dy = run_checks[i].dy;

			u32b test_mask = run_checks[i].test_mask;

			/* Check mask if present */
			if (test_mask)
			{
				if (!(valid_dirs & test_mask))
					continue;
			}

			/* Check square if present */
			if (dx || dy)
			{

				switch (run_checks[i].test)
				{
				case TEST_NONE:
					break;

				case TEST_WALL:
					ok = see_wall(px + dx, py + dy) &&
					     !see_nothing(px + dx, py + dy);
					break;

				case TEST_UNSEEN:
					ok = see_nothing(px + dx, py + dy);
					break;

				case TEST_FLOOR:
					ok = !see_wall(px + dx, py + dy) &&
					     !see_nothing(px + dx, py + dy);
					break;

				case TEST_FLOOR_S:
					ok = starting &&
					     !see_wall(px + dx, py + dy) &&
					     !see_nothing(px + dx, py + dy);
					break;

				}
			}

			if (ok) valid_dirs &= ~run_checks[i].remove_mask;

		}
	}

	/* If there are no valid paths left, stop */
	if (!valid_dirs)
	{
		p_ptr->run.mode = RUN_MODE_FINISH;
		return;
	}

	p_ptr->run.cur_dir = 0;

	/* Check if we can go a single direction */
	if      (!(valid_dirs & ~RUN_SW)) p_ptr->run.cur_dir = 1;
	else if (!(valid_dirs & ~RUN_S))  p_ptr->run.cur_dir = 2;
	else if (!(valid_dirs & ~RUN_SE)) p_ptr->run.cur_dir = 3;
	else if (!(valid_dirs & ~RUN_W))  p_ptr->run.cur_dir = 4;
	else if (!(valid_dirs & ~RUN_E))  p_ptr->run.cur_dir = 6;
	else if (!(valid_dirs & ~RUN_NW)) p_ptr->run.cur_dir = 7;
	else if (!(valid_dirs & ~RUN_N))  p_ptr->run.cur_dir = 8;
	else if (!(valid_dirs & ~RUN_NE)) p_ptr->run.cur_dir = 9;

	/* If we can go multiple directions, we're at a branch. Stop. */
	if (!p_ptr->run.cur_dir)
	{
		p_ptr->run.mode = RUN_MODE_FINISH;
		return;
	}

	p_ptr->run.old_dir = p_ptr->run.cur_dir;

	/* XXX Don't-cut-corners code goes here */
}


/*
 * Run in an open area
 *
 * XXX Write this?
 */
static void run_open(void)
{
	int px = p_ptr->px;
	int py = p_ptr->py;

	int dir = p_ptr->run.old_dir;
	int dx = ddx[dir];
	int dy = ddy[dir];

	if (check_interesting())
	{
		p_ptr->run.mode = RUN_MODE_FINISH;
		return;
	}

	p_ptr->run.cur_dir = dir;

	if (see_wall(px + dx, py + dy))
	{
		p_ptr->run.mode = RUN_MODE_FINISH;
		return;
	}

	/* Hack: Reassign mode next turn if a more interesting one applies. */
	p_ptr->run.mode = RUN_MODE_START;
}


static const int run_wall_check[10][2][2] =
{
/* 0 */ {{0, 0}, {0, 0}},
/* 1 */ {{0, 0}, {0, 0}},
/* 2 */ {{4, 1}, {6, 3}},
/* 3 */ {{0, 0}, {0, 0}},
/* 4 */ {{2, 1}, {8, 7}},
/* 5 */ {{0, 0}, {0, 0}},
/* 6 */ {{2, 3}, {8, 9}},
/* 7 */ {{0, 0}, {0, 0}},
/* 8 */ {{4, 7}, {6, 9}},
/* 9 */ {{0, 0}, {0, 0}},
};


/*
 * Run alongside a wall
 *
 * XXX Write this
 */
static void run_wall(void)
{
	unsigned int i;

	int px = p_ptr->px;
	int py = p_ptr->py;

	int dir = p_ptr->run.old_dir;
	int dx = ddx[dir];
	int dy = ddy[dir];

	if (check_interesting())
	{
		p_ptr->run.mode = RUN_MODE_FINISH;
		return;
	}

	/* Check if we are coming up to an opening in the wall */
	for (i = 0; i < NUM_ELEMENTS(run_wall_check[dir]); i++)
	{
		int wall_dir  = run_wall_check[dir][i][0];
		int floor_dir = run_wall_check[dir][i][1];

		if ( see_wall(px + ddx[wall_dir ], py + ddy[wall_dir ]) &&
	            !see_wall(px + ddx[floor_dir], py + ddy[floor_dir]))
		{
			p_ptr->run.mode = RUN_MODE_FINISH;
			return;
		}
	}

	p_ptr->run.cur_dir = dir;

	if (see_wall(px + dx, py + dy))
	{
		p_ptr->run.mode = RUN_MODE_FINISH;
		return;
	}
}

/****** Pathfinding code ******/
/*    modified from angband   */
#define MARK_DISTANCE(c,d) if ((c <= MAX_PF_LENGTH) && (c > d)) { c = d; try_again = TRUE;}

int find_player_path(int x, int y)
{
	/* Maximum size around the player to consider in the pathfinder */
	#define MAX_PF_RADIUS 50

	/* Maximum distance to consider in the pathfinder */
	#define MAX_PF_LENGTH 250


	static int terrain[MAX_PF_RADIUS][MAX_PF_RADIUS];
	static char pf_result[MAX_PF_LENGTH];

	int ox, oy, ex, ey;
	int oxl, oyl, exl, eyl;
	static int dir_search[8] = {2,4,6,8,1,3,7,9};

	int i, j, k, dir;
	int cur_distance;
	bool try_again;
	cave_type *c_ptr;
	feature_type *feat;

	ox = MAX(p_ptr->px - MAX_PF_RADIUS / 2, p_ptr->min_wid);
	oy = MAX(p_ptr->py - MAX_PF_RADIUS / 2, p_ptr->min_hgt);

	ex = MIN(p_ptr->px + MAX_PF_RADIUS / 2 - 1, p_ptr->max_wid);
	ey = MIN(p_ptr->py + MAX_PF_RADIUS / 2 - 1, p_ptr->max_hgt);

	/* make sure the target is in our grid, regardless if the player can walk there */
	if ((x < ox) || (x >= ex) || (y < oy) || (y >= ey))	{
		bell("Target out of range.");
		return 0;
	}


	for (j = oy; j < ey; j++) {
		for (i = ox; i < ex; i++) {
			feat  = &(f_info[parea(i, j)->feat]);
			if ((parea(i, j)->feat == 0)
				|| (feat->flags & FF_PWALK)
				|| ((feat->flags & FF_PPASS) && (FLAG(p_ptr, TR_PASS_WALL)))) {
				terrain[j - oy][i - ox] = MAX_PF_LENGTH;
			} else {
				terrain[j - oy][i - ox] = -1;
			}
		}
	}

	terrain[y - oy][x - ox] = MAX_PF_LENGTH;
	terrain[p_ptr->py - oy][p_ptr->px - ox] = 1;

	/* another algorithm to try to reduce the number of passes */
	cur_distance = 2;
	for (dir = 1; dir < 10; dir++) {
		if (dir != 5) {
			MARK_DISTANCE(terrain[p_ptr->py - oy + ddy[dir]][p_ptr->px - ox + ddx[dir]], cur_distance);
		}
	}

	do {
		try_again = FALSE;

		for (k = 1; k < MAX_PF_RADIUS/2 - 1; k++) {
			oxl = MAX(p_ptr->px - k, ox+1);
			oyl = MAX(p_ptr->py - k, oy+1);
			exl = MIN(p_ptr->px + k, ex-1);
			eyl = MIN(p_ptr->py + k, ey-1);

			j = oyl;
			for (i = oxl; i <= exl; i++) {
				cur_distance = terrain[j - oy][i - ox] + 1;

				if ((cur_distance > 0) && (cur_distance < MAX_PF_LENGTH)) {
					for (dir = 1; dir < 10; dir++) {
						if (dir != 5) {
							 MARK_DISTANCE(terrain[j - oy + ddy[dir]][i - ox + ddx[dir]], cur_distance);
						}
					}
				}
			}
			j = eyl;
			for (i = oxl; i <= exl; i++) {
				cur_distance = terrain[j - oy][i - ox] + 1;

				if ((cur_distance > 0) && (cur_distance < MAX_PF_LENGTH)) {
					for (dir = 1; dir < 10; dir++) {
						if (dir != 5) {
							MARK_DISTANCE(terrain[j - oy + ddy[dir]][i - ox + ddx[dir]], cur_distance);
						}
					}
				}
			}
			i = oxl;
			for (j = oyl; j <= eyl; j++) {
				cur_distance = terrain[j - oy][i - ox] + 1;

				if ((cur_distance > 0) && (cur_distance < MAX_PF_LENGTH)) {
					for (dir = 1; dir < 10; dir++) {
						if (dir != 5) {
							MARK_DISTANCE(terrain[j - oy + ddy[dir]][i - ox + ddx[dir]], cur_distance);
						}
					}
				}
			}
			i = exl;
			for (j = oyl; j <= eyl; j++) {
				cur_distance = terrain[j - oy][i - ox] + 1;

				if ((cur_distance > 0) && (cur_distance < MAX_PF_LENGTH)) {
					for (dir = 1; dir < 10; dir++) {
						if (dir != 5) {
							MARK_DISTANCE(terrain[j - oy + ddy[dir]][i - ox + ddx[dir]], cur_distance);
						}
					}
				}
			}
			if (terrain[y - oy][x - ox] < 0) {
				try_again = FALSE;
				break;
			}
			if (terrain[y - oy][x - ox] < MAX_PF_LENGTH) {
				try_again = FALSE;
				break;
			}
		}
	} while (try_again);

	/* Failure */
	if (terrain[y - oy][x - ox] == MAX_PF_LENGTH)
	{
		bell("Target space unreachable.");
		return 0;
	}

	/* Success */
	i = x;
	j = y;

	p_ptr->run.path_index = 0;

	while ((i != p_ptr->px) || (j != p_ptr->py))
	{
		cur_distance = terrain[j - oy][i - ox] - 1;
		for (k = 0; k < 8; k++)
		{
			dir = dir_search[k];
			if (terrain[j - oy + ddy[dir]][i - ox + ddx[dir]] == cur_distance)
				break;
		}

		/* Should never happend */
		if (dir == 10)
		{
			bell("Wtf ?");
			return 0;
		}

		else if (dir == 5)
		{
			bell("Heyyy !");
			return 0;
		}
		else if (p_ptr->run.path_index >= MAX_PF_LENGTH)
		{
			bell("Path not found !");
			return 0;
		}

		pf_result[p_ptr->run.path_index++] = '0' + (char)(10 - dir);
		i += ddx[dir];
		j += ddy[dir];
	}

	p_ptr->run.path = pf_result;
	p_ptr->run.path_index--;

	return pf_result[p_ptr->run.path_index] - '0';
}

static void run_path(bool starting)
{
	int px = p_ptr->px;
	int py = p_ptr->py;

	int dir = p_ptr->run.old_dir;
	int dx;// = ddx[dir];
	int dy;// = ddy[dir];

	if (starting) {
		p_ptr->run.path_index--;
	}

	if (!p_ptr->run.path) {
		p_ptr->run.path_size = 0;
		p_ptr->run.path_index = 0;
		p_ptr->run.mode = RUN_MODE_FINISH;
		return;
	}

	if (p_ptr->run.path_index < 0) {
		p_ptr->run.path = NULL;
		p_ptr->run.path_size = 0;
		p_ptr->run.path_index = 0;
		p_ptr->run.mode = RUN_MODE_FINISH;
		return;
	}

	if (check_interesting())
	{
		p_ptr->run.path = NULL;
		p_ptr->run.path_size = 0;
		p_ptr->run.path_index = 0;
		p_ptr->run.mode = RUN_MODE_FINISH;
		return;
	}
	dir = p_ptr->run.path[p_ptr->run.path_index--] - '0';
	dx = ddx[dir];
	dy = ddy[dir];

	p_ptr->run.cur_dir = dir;

	if (see_wall(px + dx, py + dy))
	{
		p_ptr->run.path = NULL;
		p_ptr->run.path_size = 0;
		p_ptr->run.path_index = 0;
		p_ptr->run.mode = RUN_MODE_FINISH;
		return;
	}
}

/*
 * Take one step along a run path
 */
void run_step(int dir)
{
	if (dir)
	{
		/* Don't start by running into a wall! */
		if (see_wall(p_ptr->px + ddx[dir], p_ptr->py + ddy[dir]))
		{
			/* Message */
			msgf("You cannot run in that direction.");

			/* Disturb */
			disturb(FALSE);

			/* Done */
			return;
		}

		/* Start by moving this direction */
		p_ptr->run.cur_dir = dir;
		p_ptr->run.old_dir = dir;

		/* In "start run" state */
		p_ptr->run.mode = RUN_MODE_START;
	}
	else
	{
		int starting = FALSE;

		/* If we've just started our run, determine the algorithm now */
		if (p_ptr->run.mode == RUN_MODE_START)
		{
			starting = TRUE;
			run_choose_mode();
		}

		/* Use the selected algorithm */
		switch (p_ptr->run.mode)
		{
		case RUN_MODE_OPEN:
			run_open();
			break;

		case RUN_MODE_CORRIDOR:
			run_corridor(starting);
			break;

		case RUN_MODE_WALL:
			run_wall();
			break;

		case RUN_MODE_FINISH:
			break;

		case RUN_MODE_ROAD:
			run_open();
			break;
		
		case RUN_MODE_WATER:
			run_open();
			break;

		case RUN_MODE_PATH:
			run_path(starting);
			break;

		default:
			msgf("Error: Bad run mode %i.", p_ptr->run.mode);
			msgf("Please submit a bug report.");
			disturb(FALSE);
			return;
		}
	}

	/* Check for end of run */
	if (p_ptr->run.mode == RUN_MODE_FINISH)
	{
		disturb(FALSE);

		return;
	}

	/* Decrease the run counter */
	p_ptr->state.running--;

	/* Use energy */
	p_ptr->state.energy_use = 100;

	/* Take a step */
	move_player(p_ptr->run.cur_dir, FALSE);
}
