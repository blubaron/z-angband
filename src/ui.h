/* File: ui.h */

/* Purpose: extern declarations (variables and functions) */
#ifndef INCLUDED_UI_H
#define INCLUDED_UI_H

/* ui.c */
extern void center_string(char *buf, uint max, cptr fmt, va_list *vp);
extern void binary_fmt(char *buf, uint max, cptr fmt, va_list *vp);
extern void fmt_clean(char *buf);
extern bool ang_sort_comp_hook_string(const vptr u, const vptr v, int a, int b);
extern void ang_sort_swap_hook_string(const vptr u, const vptr v, int a, int b);
extern int get_player_choice(cptr *choices, int num, int col, int wid,
                             cptr helpfile, void (*hook) (cptr));
extern int get_player_sort_choice(cptr *choices, int num, int col, int wid,
                                  cptr helpfile, void (*hook) (cptr));
extern bool display_action_menu(menu_action *options, int select, bool scroll,
						 int (*disp)(int), cptr prompt);
extern void bell(cptr reason);
extern void sound(int num);
extern int color_char_to_attr(char c);
extern char attr_to_color_char(byte c);
extern void screen_save(void);
extern void screen_load(void);
extern int fmt_offset(cptr str1, cptr str2);
extern int len_cstr(cptr str);
extern void put_cstr(int col, int row, cptr str, int clear);
extern void put_fstr(int col, int row, cptr str, ...);
extern void prtf(int col, int row, cptr str, ...);
extern void roff(cptr str, ...);
extern void wrap_froff(ang_file *fff, char *buf, int margin, int rowmax);
extern void froff(ang_file *fff, cptr str, ...);
extern void clear_from(int row);
extern void clear_msg(void);
extern void clear_row(int row);
extern void clear_region(int x, int y1, int y2);
extern bool askfor_aux(char *buf, int len);
extern bool get_string(char *buf, int len, cptr str, ...);
extern bool get_check(cptr prompt, ...);
extern bool get_check_ext(bool def, bool esc, cptr prompt, ...);
extern bool get_com(cptr prompt, char *command);
extern bool get_com_m(cptr prompt, char *command);
extern s16b get_quantity(cptr prompt, s16b initial, s16b max);
extern s32b get_quantity_big(cptr prompt, s32b initial, s32b max);
extern u32b get_number(cptr prompt, u32b initial);
extern void pause_line(int row);
extern int get_keymap_dir(char ch);

#endif /* INCLUDED_UI_H */

