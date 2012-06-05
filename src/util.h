/* File: util.h */

/* Purpose: extern declarations (variables and functions) */
#ifndef INCLUDED_UTIL_H
#define INCLUDED_UTIL_H

/* util.c */
extern bool assert_helper(cptr expr, cptr file, int line, bool result);
extern int count_bits(u32b x);

extern bool is_a_vowel(int ch);
extern void request_command(int shopping);
extern void repeat_push(int what);
extern bool repeat_pull(int *what);
extern void repeat_clear(void);
extern void repeat_check(void);

#endif /* INCLUDED_UTIL_H */
