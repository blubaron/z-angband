/* File: signals.h */

/* Purpose: extern declarations (variables and functions) */
#ifndef INCLUDED_SIGNALS_H
#define INCLUDED_SIGNALS_H

/* signals.c */
extern void signals_ignore_tstp(void);
extern void signals_handle_tstp(void);
extern void signals_init(void);

#endif /* INCLUDED_SIGNALS_H */
