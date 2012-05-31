/* File: z-file.h */

/* Purpose: extern declarations (variables and functions) */
#ifndef INCLUDED_Z_FILE_H
#define INCLUDED_Z_FILE_H

/* z-file.c */
extern errr path_parse(char *buf, int max, cptr file);
extern void path_build(char *buf, int max, cptr path, cptr file);
extern FILE *my_fopen(cptr file, cptr mode);
extern FILE *my_fopen_temp(char *buf, int max);
extern errr my_fgets(FILE *fff, char *buf, huge n);
extern errr my_raw_fgets(FILE *fff, char *buf, huge n);
extern void my_fclose(FILE *fff);
extern errr fd_kill(cptr file);
extern errr fd_move(cptr file, cptr what);
extern int fd_make(cptr file, int mode);
extern int fd_open(cptr file, int flags);
extern errr fd_lock(int fd, int what);
extern errr fd_seek(int fd, huge n);
extern errr fd_read(int fd, char *buf, huge n);
extern errr fd_write(int fd, cptr buf, huge n);
extern errr fd_close(int fd);


/*
 * Hack -- conditional (or "bizarre") externs
 */
#if defined(MAC_MPW) || defined(MACH_O_CARBON)
/* main-mac.c, or its derivatives */
extern u32b _fcreator;
extern u32b _ftype;
# if defined(MAC_MPW) && defined(CARBON)
extern void convert_pathname(char *path);
# endif
# if defined(MACH_O_CARBON)
extern void fsetfileinfo(cptr path, u32b fcreator, u32b ftype);
# endif
#endif

#endif /* INCLUDED_Z-FILE_H */

