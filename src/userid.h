/* File: userid.h */

/* Purpose: extern declarations (variables and functions) */
#ifndef INCLUDED_USERID_H
#define INCLUDED_USERID_H

/* userid.c */
extern void safe_setuid_drop(void);
extern void safe_setuid_grab(void);
extern void init_setuid(void);


/*
 * Hack -- conditional (or "bizarre") externs
 */

#ifdef SET_UID
# ifndef HAS_USLEEP
/* util.c */
extern int usleep(huge usecs);
# endif	/* HAS_USLEEP */
extern void user_name(char *buf, int id);
#endif /* SET_UID */

#endif /* INCLUDED_USERID_H */

