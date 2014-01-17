/* File: target.h */

/* Purpose: extern declarations (variables and functions) */
#ifndef INCLUDED_TARGET_H
#define INCLUDED_TARGET_H

extern bool target_able(int m_idx);
extern bool target_okay(void);
extern void target_set_monster(int m_idx);
extern void target_set_location(int x, int y);
extern bool target_set_grid(int x, int y);
extern bool target_look_grid(int x, int y, bool recall);
extern bool target_look_grid_prompt(int col, int row, int x, int y, char* pmt);
extern bool target_set_interactive(int mode, int x, int y);
extern bool target_set(int mode);
extern bool get_aim_dir(int *dp);
extern bool get_rep_dir(int *dp);
extern bool get_hack_dir(int *dp);

#endif
