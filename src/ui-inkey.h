/* File: inkey.h */

/* Purpose: extern declarations (variables and functions) */
#ifndef INCLUDED_UI_INKEY_H
#define INCLUDED_UI_INKEY_H

/* inkey.c */
extern void text_to_ascii(char *buf, cptr str);
extern void ascii_to_text(char *buf, cptr str);

extern sint macro_find_exact(cptr pat);
extern void macro_add(cptr pat, cptr act);
extern void flush(void);

extern char inkey(void);
extern char inkey_m(void);

#endif /* INCLUDED_UI_INKEY_H */

