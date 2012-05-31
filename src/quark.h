/* File: quark.h */

/* Purpose: extern declarations (variables and functions) */
#ifndef INCLUDED_QUARK_H
#define INCLUDED_QUARK_H

/* quark.c */
extern s16b quark_add(cptr str);
extern s16b quark_fmt(cptr str, ...);
extern void quark_remove(s16b *i);
extern void quark_dup(s16b i);
extern cptr quark_str(s16b i);
extern errr quarks_init(void);
extern errr quarks_free(void);

#endif /* INCLUDED_QUARK_H */
