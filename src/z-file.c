/* File: util.c */

/* Purpose: Angband utilities -BEN- */


#include "angband.h"
#include "z-file.h"

/*
 * The concept of the "file" routines below (and elsewhere) is that all
 * file handling should be done using as few routines as possible, since
 * every machine is slightly different, but these routines always have the
 * same semantics.
 *
 * In fact, perhaps we should use the "path_parse()" routine below to convert
 * from "canonical" filenames (optional leading tilde's, internal wildcards,
 * slash as the path seperator, etc) to "system" filenames (no special symbols,
 * system-specific path seperator, etc).  This would allow the program itself
 * to assume that all filenames are "Unix" filenames, and explicitly "extract"
 * such filenames if needed (by "path_parse()", or perhaps "path_canon()").
 *
 * Note that "path_temp" should probably return a "canonical" filename.
 *
 * Note that "my_fopen()" and "my_open()" and "my_make()" and "my_kill()"
 * and "my_move()" and "my_copy()" should all take "canonical" filenames.
 *
 * Note that "canonical" filenames use a leading "slash" to indicate an absolute
 * path, and a leading "tilde" to indicate a special directory, and default to a
 * relative path, but MSDOS uses a leading "drivename plus colon" to indicate the
 * use of a "special drive", and then the rest of the path is parsed "normally",
 * and MACINTOSH uses a leading colon to indicate a relative path, and an embedded
 * colon to indicate a "drive plus absolute path", and finally defaults to a file
 * in the current working directory, which may or may not be defined.
 *
 * We should probably parse a leading "~~/" as referring to "ANGBAND_DIR". (?)
 */


#ifdef ACORN


/*
 * Most of the "file" routines for "ACORN" should be in "main-acn.c"
 */


#else  /* ACORN */


#ifdef SET_UID

/*
 * Extract a "parsed" path from an initial filename
 * Normally, we simply copy the filename into the buffer
 * But leading tilde symbols must be handled in a special way
 * Replace "~user/" by the home directory of the user named "user"
 * Replace "~/" by the home directory of the current user
 */
errr path_parse(char *buf, int max, cptr file)
{
	cptr u, s;
	struct passwd *pw;
	char user[128];

	/* Assume no result */
	buf[0] = '\0';

	/* No file? */
	if (!file) return (-1);

	/* File needs no parsing */
	if (file[0] != '~')
	{
		strncpy(buf, file, max - 1);

		/* Terminate */
		buf[max - 1] = '\0';

		return (0);
	}

	/* Point at the user */
	u = file + 1;

	/* Look for non-user portion of the file */
	s = strstr(u, PATH_SEP);

	/* Hack -- no long user names */
	if (s && (s >= u + sizeof(user))) return (1);

	/* Extract a user name */
	if (s)
	{
		int i;
		for (i = 0; u < s; ++i) user[i] = *u++;
		user[i] = '\0';
		u = user;
	}

	/* Look up the "current" user */
	if (u[0] == '\0') u = getlogin();

	/* Look up a user (or "current" user) */
	if (u) pw = getpwnam(u);
	else
		pw = getpwuid(getuid());

	/* Nothing found? */
	if (!pw) return (1);

	/* Make use of the info */
	(void)strncpy(buf, pw->pw_dir, max - 1);

	/* Append the rest of the filename, if any */
	if (s) (void)strncat(buf, s, max - 1);

	/* Terminate */
	buf[max - 1] = '\0';

	/* Success */
	return (0);
}


#else  /* SET_UID */


/*
 * Extract a "parsed" path from an initial filename
 *
 * This requires no special processing on simple machines,
 * except for verifying the size of the filename.
 */
errr path_parse(char *buf, int max, cptr file)
{
	/* Accept the filename */
	(void)strnfmt(buf, max, "%s", file);

	/* Success */
	return (0);
}


#endif /* SET_UID */


#ifndef HAVE_MKSTEMP

/*
 * Hack -- acquire a "temporary" file name if possible
 *
 * This filename is always in "system-specific" form.
 */
static errr path_temp(char *buf, int max)
{
	cptr s;

	/*
	 * Temp file
	 *
	 * If the following line gives you a compile-time warning,
	 * then turn on the HAVE_MKSTEMP if you have mkstemp().
	 */
	s = tmpnam(NULL);

	/* Oops */
	if (!s) return (-1);

	/* Format to length */
	(void)strnfmt(buf, max, "%s", s);

	/* Success */
	return (0);
}

#endif /* !HAVE_MKSTEMP */


/*
 * Create a new path by appending a file (or directory) to a path.
 *
 * This requires no special processing on simple machines, except
 * for verifying the size of the filename, but note the ability to
 * bypass the given "path" with certain special file-names.
 *
 * Note that the "file" may actually be a "sub-path", including
 * a path and a file.
 *
 * Note that this function yields a path which must be "parsed"
 * using the "parse" function above.
 */
void path_build(char *buf, int max, cptr path, cptr file)
{
	/* Special file */
	if (file[0] == '~')
	{
		/* Use the file itself */
		(void)strnfmt(buf, max, "%s", file);
	}

	/* Absolute file, on "normal" systems */
	else if (prefix(file, PATH_SEP) && !streq(PATH_SEP, ""))
	{
		/* Use the file itself */
		(void)strnfmt(buf, max, "%s", file);
	}

	/* No path given */
	else if (!path[0])
	{
		/* Use the file itself */
		(void)strnfmt(buf, max, "%s", file);
	}

	/* Path and File */
	else
	{
		/* Build the new path */
		(void)strnfmt(buf, max, "%s%s%s", path, PATH_SEP, file);
	}
}


/*
 * Hack -- replacement for "fopen()"
 */
FILE *my_fopen(cptr file, cptr mode)
{
	char buf[1024];
	FILE *fff;

	/* Hack -- Try to parse the path */
	if (path_parse(buf, 1024, file)) return (NULL);

	/* Attempt to fopen the file anyway */
	fff = fopen(buf, mode);

#if defined(MAC_MPW) || defined(MACH_O_CARBON)

	/* Set file creator and type */
	if (fff && strchr(mode, 'w')) fsetfileinfo(buf, _fcreator, _ftype);

#endif

	/* Return open file or NULL */
	return (fff);
}


/*
 * Hack -- replacement for "fclose()"
 */
void my_fclose(FILE *fff)
{
	/* Require a file */
	if (!fff) return;

	/* Close, check for error */
	(void)fclose(fff);
}


#endif /* ACORN */


#ifdef HAVE_MKSTEMP

FILE *my_fopen_temp(char *buf, int max)
{
	int fd;

	/* Prepare the buffer for mkstemp */
	strncpy(buf, "/tmp/anXXXXXX", max);

	/* Secure creation of a temporary file */
	fd = mkstemp(buf);

	/* Check the file-descriptor */
	if (fd < 0) return (NULL);

	/* Return a file stream */
	return (fdopen(fd, "w"));
}

#else  /* HAVE_MKSTEMP */

FILE *my_fopen_temp(char *buf, int max)
{
	/* Generate a temporary filename */
	if (path_temp(buf, max)) return (NULL);

	/* Open the file */
	return (my_fopen(buf, "w"));
}

#endif /* HAVE_MKSTEMP */


/*
 * Hack -- replacement for "fgets()"
 *
 * Read a string, without a newline, to a file
 *
 * Process tabs, strip internal non-printables if told.
 */
static errr my_fgets_aux(FILE *fff, char *buf, huge n, bool strip)
{
	huge i = 0;

	char *s;

	char tmp[1024];

	/* Read a line */
	if (fgets(tmp, 1024, fff))
	{
		/* Convert weirdness */
		for (s = tmp; *s; s++)
		{
			/* Handle newline */
			if (*s == '\n')
			{
				/* Terminate */
				buf[i] = '\0';

				/* Success */
				return (0);
			}

			/* Handle tabs */
			else if (*s == '\t')
			{
				/* Hack -- require room */
				if (i + 8 >= n) break;

				/* Append a space */
				buf[i++] = ' ';

				/* Append some more spaces */
				while (!(i % 8)) buf[i++] = ' ';
			}

			/* Strip non-printables if asked */
			else if(!strip || isprint(*s))
			{
				/* Copy */
				buf[i++] = *s;

				/* Check length */
				if (i >= n) break;
			}
		}
	}

	/* Nothing */
	buf[0] = '\0';

	/* Failure */
	return (1);
}


/*
 * Hack -- replacement for "fgets()"
 *
 * Read a string, without a newline, to a file
 *
 * Process tabs, strip internal non-printables
 */
errr my_fgets(FILE *fff, char *buf, huge n)
{
	return (my_fgets_aux(fff, buf, n, TRUE));
}

/*
 * Hack -- replacement for "fgets()"
 *
 * Read a string, without a newline, to a file
 *
 * Process tabs, do not strip internal non-printables
 */
errr my_raw_fgets(FILE *fff, char *buf, huge n)
{
	return (my_fgets_aux(fff, buf, n, FALSE));
}




#ifdef ACORN


/*
 * Most of the "file" routines for "ACORN" should be in "main-acn.c"
 *
 * Many of them can be rewritten now that only "fd_open()" and "fd_make()"
 * and "my_fopen()" should ever create files.
 */


#else  /* ACORN */


/*
 * Code Warrior is a little weird about some functions
 */
#ifdef BEN_HACK
extern int open(const char *, int, ...);
extern int close(int);
extern int read(int, void *, unsigned int);
extern int write(int, const void *, unsigned int);
extern long lseek(int, long, int);
#endif /* BEN_HACK */


/*
 * The Macintosh is a little bit brain-dead sometimes
 */
#ifdef MACINTOSH
# define open(N,F,M) \
((M), open((char*)(N),F))
# define write(F,B,S) \
write(F,(char*)(B),S)
#endif /* MACINTOSH */


/*
 * Several systems have no "O_BINARY" flag
 */
#ifndef O_BINARY
# define O_BINARY 0
#endif /* O_BINARY */


/*
 * Hack -- attempt to delete a file
 */
errr fd_kill(cptr file)
{
	char buf[1024];

	/* Hack -- Try to parse the path */
	if (path_parse(buf, 1024, file)) return (-1);

	/* Remove */
	(void)remove(buf);

	/* Assume success XXX XXX XXX */
	return (0);
}


/*
 * Hack -- attempt to move a file
 */
errr fd_move(cptr file, cptr what)
{
	char buf[1024];
	char aux[1024];

	/* Hack -- Try to parse the path */
	if (path_parse(buf, 1024, file)) return (-1);

	/* Hack -- Try to parse the path */
	if (path_parse(aux, 1024, what)) return (-1);

	/* Rename */
	(void)rename(buf, aux);

	/* Assume success XXX XXX XXX */
	return (0);
}


/*
 * Hack -- attempt to open a file descriptor (create file)
 *
 * This function should fail if the file already exists
 *
 * Note that we assume that the file should be "binary"
 */
int fd_make(cptr file, int mode)
{
	char buf[1024];
	int fd;

	/* Hack -- Try to parse the path */
	if (path_parse(buf, sizeof(buf), file)) return (-1);

#if defined(MACINTOSH)

	/* Create the file, fail if exists, write-only, binary */
	fd = open(buf, O_CREAT | O_EXCL | O_WRONLY | O_BINARY);

#else

	/* Create the file, fail if exists, write-only, binary */
	fd = open(buf, O_CREAT | O_EXCL | O_WRONLY | O_BINARY, mode);

#endif

#if defined(MAC_MPW) || defined(MACH_O_CARBON)

	/* Set file creator and type */
	if (fd >= 0) fsetfileinfo(buf, _fcreator, _ftype);

#endif

	/* Return descriptor */
	return (fd);
}


/*
 * Hack -- attempt to open a file descriptor (existing file)
 *
 * Note that we assume that the file should be "binary"
 */
int fd_open(cptr file, int flags)
{
	char buf[1024];

	/* Hack -- Try to parse the path */
	if (path_parse(buf, 1024, file)) return (-1);

	/* Attempt to open the file */
	return (open(buf, flags | O_BINARY, 0));
}


/*
 * Hack -- attempt to lock a file descriptor
 *
 * Legal lock types -- F_UNLCK, F_RDLCK, F_WRLCK
 */
errr fd_lock(int fd, int what)
{
	/* XXX XXX */
	what = what ? what : 0;

	/* Verify the fd */
	if (fd < 0) return (-1);

#ifdef SET_UID

# ifdef USG

#  if defined(F_ULOCK) && defined(F_LOCK)

	/* Un-Lock */
	if (what == F_UNLCK)
	{
		/* Unlock it, Ignore errors */
		lockf(fd, F_ULOCK, 0);
	}

	/* Lock */
	else
	{
		/* Lock the score file */
		if (lockf(fd, F_LOCK, 0) != 0) return (1);
	}

#  endif

# else

#  if defined(LOCK_UN) && defined(LOCK_EX)

	/* Un-Lock */
	if (what == F_UNLCK)
	{
		/* Unlock it, Ignore errors */
		(void)flock(fd, LOCK_UN);
	}

	/* Lock */
	else
	{
		/* Lock the score file */
		if (flock(fd, LOCK_EX) != 0) return (1);
	}

#  endif

# endif

#endif

	/* Success */
	return (0);
}


/*
 * Hack -- attempt to seek on a file descriptor
 */
errr fd_seek(int fd, huge n)
{
	huge p;

	/* Verify fd */
	if (fd < 0) return (-1);

	/* Seek to the given position */
	p = lseek(fd, n, SEEK_SET);

	/* Failure */
	if (p != n) return (1);

	/* Success */
	return (0);
}


/*
 * Hack -- attempt to read data from a file descriptor
 */
errr fd_read(int fd, char *buf, huge n)
{
	/* Verify the fd */
	if (fd < 0) return (-1);

#ifndef SET_UID

	/* Read pieces */
	while (n >= 16384)
	{
		/* Read a piece */
		if (read(fd, buf, 16384) != 16384) return (1);

		/* Shorten the task */
		buf += 16384;

		/* Shorten the task */
		n -= 16384;
	}

#endif

	/* Read the final piece */
	if (read(fd, buf, n) != (int)n) return (1);

	/* Success */
	return (0);
}


/*
 * Hack -- Attempt to write data to a file descriptor
 */
errr fd_write(int fd, cptr buf, huge n)
{
	/* Verify the fd */
	if (fd < 0) return (-1);

#ifndef SET_UID

	/* Write pieces */
	while (n >= 16384)
	{
		/* Write a piece */
		if (write(fd, buf, 16384) != 16384) return (1);

		/* Shorten the task */
		buf += 16384;

		/* Shorten the task */
		n -= 16384;
	}

#endif

	/* Write the final piece */
	if (write(fd, buf, n) != (int)n) return (1);

	/* Success */
	return (0);
}


/*
 * Hack -- attempt to close a file descriptor
 */
errr fd_close(int fd)
{
	/* Verify the fd */
	if (fd < 0) return (-1);

	/* Close */
	(void)close(fd);

	/* XXX XXX XXX */
	return (0);
}


#endif /* ACORN */

/* below is the z-file.c from angband - Brett */
/*
 * File: z-file.c
 * Purpose: Low-level file (and directory) handling
 *
 * Copyright (c) 1997-2007 Ben Harrison, pelpel, Andrew Sidwell, Matthew Jones
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */
#include "angband.h"

#include <sys/types.h>

#ifdef WINDOWS
# include <windows.h>
# include <io.h>
# include <direct.h>
#endif

#ifdef MACH_O_CARBON
# include <Carbon/Carbon.h>
#endif

#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif

#ifdef HAVE_DIRENT_H
# include <sys/types.h>
# include <dirent.h>
#endif

#ifdef HAVE_STAT
# include <sys/stat.h>
# include <sys/types.h>
#endif

#ifdef WINDOWS
# define my_mkdir(path, perms) mkdir(path)
#elif defined(HAVE_MKDIR) || defined(MACH_O_CARBON)
# define my_mkdir(path, perms) mkdir(path, perms)
#else
# define my_mkdir(path, perms) -1
#endif
#if  0
/*
 * Player info
 */
int player_uid;
int player_egid;




/*
 * Drop permissions
 */
void safe_setuid_drop(void)
{
#ifdef SET_UID
# if defined(HAVE_SETRESGID)

	if (setresgid(-1, getgid(), -1) != 0)
		quit("setegid(): cannot drop permissions correctly!");

# else

	if (setegid(getgid()) != 0)
		quit("setegid(): cannot drop permissions correctly!");

# endif
#endif /* SET_UID */
}


/*
 * Grab permissions
 */
void safe_setuid_grab(void)
{
#ifdef SET_UID
# if defined(HAVE_SETRESGID)

	if (setresgid(-1, player_egid, -1) != 0)
		quit("setegid(): cannot grab permissions correctly!");

# elif defined(HAVE_SETEGID)

	if (setegid(player_egid) != 0)
		quit("setegid(): cannot grab permissions correctly!");

# endif
#endif /* SET_UID */
}




/*
 * Apply special system-specific processing before dealing with a filename.
 */
static void path_parse(char *buf, size_t max, const char *file)
{
	/* Accept the filename */
	my_strcpy(buf, file, max);
}



static void path_process(char *buf, size_t len, size_t *cur_len, const char *path)
{
#if defined(SET_UID) || defined(USE_PRIVATE_PATHS)

	/* Home directory on Unixes */
	if (path[0] == '~')
	{
		const char *s;
		const char *username = path + 1;

		struct passwd *pw;
		char user[128];

		/* Look for non-user portion of the file */
		s = strstr(username, PATH_SEP);
		if (s)
		{
			int i;

			/* Keep username a decent length */
			if (s >= username + sizeof(user)) return;

			for (i = 0; username < s; ++i) user[i] = *username++;
			user[i] = '\0';
			username = user;
		}

#ifndef MACH_O_CARBON

		/* Look up a user (or "current" user) */
		pw = username[0] ? getpwnam(username) : getpwuid(getuid());

#else /* MACH_O_CARBON */

		{
			/* On Macs getlogin() can incorrectly return root, so get the username via system frameworks */
			CFStringRef cfusername = CSCopyUserName(TRUE);
			CFIndex cfbufferlength = CFStringGetMaximumSizeForEncoding(CFStringGetLength(cfusername), kCFStringEncodingUTF8) + 1;
			char *macusername = mem_alloc(cfbufferlength);
			CFStringGetCString(cfusername, macusername, cfbufferlength, kCFStringEncodingUTF8);
			CFRelease(cfusername);

			/* Look up the user */
			pw = getpwnam(macusername);
			mem_free(macusername);
		}
#endif /* !MACH_O_CARBON */

		if (!pw) return;

		/* Copy across */
		strnfcat(buf, len, cur_len, "%s%s", pw->pw_dir, PATH_SEP);
		if (s) strnfcat(buf, len, cur_len, "%s", s);
	}
	else

#endif

	{
		strnfcat(buf, len, cur_len, "%s", path);
	}
}



/*
 * Create a new path string by appending a 'leaf' to 'base'.
 *
 * On Unixes, we convert a tidle at the beginning of a basename to mean the
 * directory, complicating things a little, but better now than later.
 *
 * Remember to free the return value.
 */
size_t path_build(char *buf, size_t len, const char *base, const char *leaf)
{
	size_t cur_len = 0;
	buf[0] = '\0';

	if (!leaf || !leaf[0])
	{
		if (base && base[0])
			path_process(buf, len, &cur_len, base);

		return cur_len;
	}


	/*
	 * If the leafname starts with the seperator,
	 *   or with the tilde (on Unix),
	 *   or there's no base path,
	 * We use the leafname only.
	 */
#if defined(SET_UID) || defined(USE_PRIVATE_PATHS)
	if ((!base || !base[0]) || prefix(leaf, PATH_SEP) || leaf[0] == '~')
#else
	if ((!base || !base[0]) || prefix(leaf, PATH_SEP))
#endif
	{
		path_process(buf, len, &cur_len, leaf);
		return cur_len;
	}


	/* There is both a relative leafname and a base path from which it is relative */
	path_process(buf, len, &cur_len, base);
	strnfcat(buf, len, &cur_len, "%s", PATH_SEP);
	path_process(buf, len, &cur_len, leaf);

	return cur_len;
}

#endif /* if 0 */

/*** File-handling API ***/

/* On Windows, fwrite() and fread() are broken. */
#if defined(WINDOWS) || defined(SET_UID)
# define HAVE_WRITE
# define HAVE_READ
#endif

/* Some defines for compatibility between various build platforms */
#ifndef S_IRUSR
#define S_IRUSR S_IREAD
#endif

#ifndef S_IWUSR
#define S_IWUSR S_IWRITE
#endif

/* if the flag O_BINARY is not defined, it is not needed , but we still
 * need it defined so it will compile */
#ifndef O_BINARY
#define O_BINARY 0
#endif

/* Avoid a compiler warning when cross compiling for windows */
#ifdef __STRICT_ANSI__
FILE *fdopen(int handle, const char *mode);
#endif

/* Private structure to hold file pointers and useful info. */
struct ang_file
{
	FILE *fh;
	char *fname;
	file_mode mode;
};



/** Utility functions **/

/*
 * Delete file 'fname'.
 */
bool file_delete(const char *fname)
{
	char buf[1024];

	/* Get the system-specific paths */
	path_parse(buf, sizeof(buf), fname);

	return (remove(buf) == 0);
}

/*
 * Move file 'fname' to 'newname'.
 */
bool file_move(const char *fname, const char *newname)
{
	char buf[1024];
	char aux[1024];

	/* Get the system-specific paths */
	path_parse(buf, sizeof(buf), fname);
	path_parse(aux, sizeof(aux), newname);

	return (rename(buf, aux) == 0);
}


/*
 * Decide whether a file exists or not.
 */

#if defined(HAVE_STAT)

bool file_exists(const char *fname)
{
	struct stat st;
	return (stat(fname, &st) == 0);
}

#elif defined(WINDOWS)

bool file_exists(const char *fname)
{
	char path[MAX_PATH];
	DWORD attrib;

	/* API says we mustn't pass anything larger than MAX_PATH */
	my_strcpy(path, s, sizeof(path));

	attrib = GetFileAttributes(path);
	if (attrib == INVALID_FILE_NAME) return FALSE;
	if (attrib & FILE_ATTRIBUTE_DIRECTORY) return FALSE;

	return TRUE;
}

#else

bool file_exists(const char *fname)
{
	ang_file *f = file_open(fname, MODE_READ, 0);

	if (f) file_close(f);
	return (f ? TRUE : FALSE);
}

#endif

/*
 * Return TRUE if first is newer than second, FALSE otherwise.
 */
bool file_newer(const char *first, const char *second)
{
#ifdef HAVE_STAT
	struct stat stat1, stat2;

	/* If the first doesn't exist, the first is not newer. */
	if (stat(first, &stat1) != 0) return FALSE;

	/* If the second doesn't exist, the first is always newer. */
	if (stat(second, &stat2) != 0) return TRUE;

	/* Compare modification times. */
	return stat1.st_mtime > stat2.st_mtime ? TRUE : FALSE;
#else /* HAVE_STAT */
	return FALSE;
#endif /* !HAVE_STAT */
}




/** File-handle functions **/

void (*file_open_hook)(const char *path, file_type ftype);

/*
 * Open file 'fname', in mode 'mode', with filetype 'ftype'.
 * Returns file handle or NULL.
 */
ang_file *file_open(const char *fname, file_mode mode, file_type ftype)
{
	ang_file *f = ZNEW(ang_file);
	char buf[1024];

	(void)ftype;

	/* Get the system-specific path */
	path_parse(buf, sizeof(buf), fname);

	switch (mode) {
		case MODE_WRITE: {
			/* Grab permissions */
			safe_setuid_grab();

			if (ftype == FTYPE_SAVE) {
				int fd;

				/* File type is "SAVE" */
				FILE_TYPE(FILE_TYPE_SAVE);

				/* open only if the file does not exist */
				/* Create the file, fail if exists, write-only, binary */
#if defined(MACINTOSH)
				fd = open(buf, O_CREAT | O_EXCL | O_WRONLY | O_BINARY);
#else
				fd = open(buf, O_CREAT | O_EXCL | O_WRONLY | O_BINARY, S_IRUSR | S_IWUSR);
#endif
				if (fd < 0) {
					/* there was some error */
					f->fh = NULL;
				} else {
					f->fh = fdopen(fd, "wb");
				}
			} else
			if ((ftype == FTYPE_TEXT) || (ftype == FTYPE_HTML)) {
				/* File type is "TEXT" */
				FILE_TYPE(FILE_TYPE_TEXT);
				f->fh = fopen(buf, "w");
			} else
			{
				/* File type is "DATA" */
				FILE_TYPE(FILE_TYPE_DATA);
				f->fh = fopen(buf, "wb");
			}

			/* Drop permissions */
			safe_setuid_drop();

			break;
		}

		case MODE_READ: {
			if ((ftype == FTYPE_TEXT) || (ftype == FTYPE_HTML)) {
				f->fh = fopen(buf, "r");
			} else
			if (ftype == FTYPE_SAVE) {
				safe_setuid_grab();
				f->fh = fopen(buf, "rb");
				safe_setuid_drop();
			} else
			{
				f->fh = fopen(buf, "rb");
			}
			break;
		}

		case MODE_APPEND: {
			safe_setuid_grab();
			f->fh = fopen(buf, "a+");
			safe_setuid_drop();
			break;
		}

		default: f->fh = fopen(buf, "__");
	}

	if (f->fh == NULL)
	{
		FREE(f);
		return NULL;
	}

	f->fname = (char*)string_make(buf);
	f->mode = mode;

	if (mode != MODE_READ) {
		if (file_open_hook) {
			file_open_hook(buf, ftype);
#if defined(MAC_MPW) || defined(MACH_O_CARBON)
		} else {
			/* Set file creator and type */
			if (fd >= 0) fsetfileinfo(buf, _fcreator, _ftype);
#endif
		}
	}

	return f;
}

#ifdef HAVE_MKSTEMP

ang_file *file_open_temp(char *buf, int max)
{
	int fd;
	ang_file *f = ZNEW(ang_file);

	/* Prepare the buffer for mkstemp */
	strncpy(buf, "/tmp/anXXXXXX", max);

	/* Secure creation of a temporary file */
	fd = mkstemp(buf);

	/* Check the file-descriptor */
	if (fd < 0) {
		FREE(f);
		return (NULL);
	}
	f->fh = fdopen(fd, "w");
	if (f->fh == NULL)
	{
		FREE(f);
		return NULL;
	}

	/* Return a file stream */
	return (f);
}

#else  /* HAVE_MKSTEMP */
ang_file *file_open_temp(char *buf, int max)
{
	/* Generate a temporary filename */
	if (path_temp(buf, max)) return (NULL);

	/* Open the file */
	return (file_open(buf, MODE_WRITE, FTYPE_TEXT));
}

#endif /* HAVE_MKSTEMP */

/*
 * Close file handle 'f'.
 */
bool file_close(ang_file *f)
{
	if (fclose(f->fh) != 0)
		return FALSE;

	FREE(f->fname);
	FREE(f);

	return TRUE;
}

int file_flush(ang_file *f)
{
	return fflush(f->fh);
}
bool file_eof(ang_file *f)
{
	return feof(f->fh);
}
int file_error(ang_file *f)
{
	return ferror(f->fh);
}
int file_descriptor(ang_file *f)
{
	return fileno(f->fh);
}

bool file_getpos(ang_file *f, u32b *retpos)
{
	if (retpos) {
		*retpos = ftell(f->fh);
		return TRUE;
	}
	return FALSE;
}


/** Locking functions **/

/*
 * Lock a file using POSIX locks, on platforms where this is supported.
 */
void file_lock(ang_file *f)
{
#if defined(HAVE_FCNTL_H) && defined(SET_UID)
	struct flock lock;
	lock.l_type = (f->mode == MODE_READ ? F_RDLCK : F_WRLCK);
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;
	lock.l_pid = 0;
	fcntl(fileno(f->fh), F_SETLKW, &lock);
#endif /* HAVE_FCNTL_H && SET_UID */
}

/*
 * Unlock a file locked using file_lock().
 */
void file_unlock(ang_file *f)
{
#if defined(HAVE_FCNTL_H) && defined(SET_UID)
	struct flock lock;
	lock.l_type = F_UNLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;
	lock.l_pid = 0;
	fcntl(fileno(f->fh), F_SETLK, &lock);
#endif /* HAVE_FCNTL_H && SET_UID */
}


/** Byte-based IO and functions **/

/*
 * Seek to location 'pos' in file 'f'.
 */
bool file_seek(ang_file *f, u32b pos)
{
	return (fseek(f->fh, pos, SEEK_SET) == 0);
}

/*
 * Read a single, 8-bit character from file 'f'.
 */
bool file_readc(ang_file *f, byte *b)
{
	int i = fgetc(f->fh);

	if (i == EOF)
		return FALSE;

	*b = (byte)i;
	return TRUE;
}

/*
 * Write a single, 8-bit character 'b' to file 'f'.
 */
bool file_writec(ang_file *f, byte b)
{
	return file_write(f, (const char *)&b, 1);
}

/*
 * Read 'n' bytes from file 'f' into array 'buf'.
 */
int file_read(ang_file *f, char *buf, size_t n)
{
	size_t read = fread(buf, 1, n, f->fh);

	if (read == 0 && ferror(f->fh))
		return -1;
	else
		return read;
}

/*
 * Append 'n' bytes of array 'buf' to file 'f'.
 */
bool file_write(ang_file *f, const char *buf, size_t n)
{
	return fwrite(buf, 1, n, f->fh) == n;
}

/** Line-based IO **/

/*
 * Read a line of text from file 'f' into buffer 'buf' of size 'n' bytes.
 *
 * Support both \r\n and \n as line endings, but not the outdated \r that used
 * to be used on Macs.  Replace non-printables with '?', and \ts with ' '.
 */
#define TAB_COLUMNS 4

errr file_getl(ang_file *f, char *buf, size_t len)
{
	bool seen_cr = FALSE;
	byte b;
	size_t i = 0;

	/* Leave a byte for the terminating 0 */
	size_t max_len = len - 1;

	while (i < max_len)
	{
		char c;

		if (!file_readc(f, &b))
		{
			buf[i] = '\0';
			return (i == 0) ? (-1) : i;
		}

		c = (char) b;

		if (c == '\r')
		{
			seen_cr = TRUE;
			continue;
		}

		if (seen_cr && c != '\n')
		{
			fseek(f->fh, -1, SEEK_CUR);
			buf[i] = '\0';
			return TRUE;
		}

		if (c == '\n')
		{
			buf[i] = '\0';
			return TRUE;
		}

		/* Expand tabs */
		if (c == '\t')
		{
			/* Next tab stop */
			size_t tabstop = ((i + TAB_COLUMNS) / TAB_COLUMNS) * TAB_COLUMNS;
			if (tabstop >= len) break;

			/* Convert to spaces */
			while (i < tabstop)
				buf[i++] = ' ';

			continue;
		}
		if (isprint(c)) {
			buf[i++] = c;
		}
	}

	buf[i] = '\0';
	return i;
}

errr file_getl_raw(ang_file *f, char *buf, size_t len)
{
	bool seen_cr = FALSE;
	byte b;
	size_t i = 0;

	/* Leave a byte for the terminating 0 */
	size_t max_len = len - 1;

	while (i < max_len)
	{
		char c;

		if (!file_readc(f, &b))
		{
			buf[i] = '\0';
			return (i == 0) ? (-1) : i;
		}

		c = (char) b;

		if (c == '\r')
		{
			seen_cr = TRUE;
			continue;
		}

		if (seen_cr && c != '\n')
		{
			fseek(f->fh, -1, SEEK_CUR);
			buf[i] = '\0';
			return TRUE;
		}

		if (c == '\n')
		{
			buf[i] = '\0';
			return TRUE;
		}

		/* Expand tabs */
		if (c == '\t')
		{
			/* Next tab stop */
			size_t tabstop = ((i + TAB_COLUMNS) / TAB_COLUMNS) * TAB_COLUMNS;
			if (tabstop >= len) break;

			/* Convert to spaces */
			while (i < tabstop)
				buf[i++] = ' ';

			continue;
		}

		buf[i++] = c;
	}

	buf[i] = '\0';
	return i;
}
/*
 * Append a line of text 'buf' to the end of file 'f', using system-dependent
 * line ending.
 */
bool file_put(ang_file *f, const char *buf)
{
	return file_write(f, buf, strlen(buf));
}

/*
 * The comp.lang.c FAQ recommends this pairing for varargs functions.
 * See <http://c-faq.com/varargs/handoff.html>
 */

/**
 * Append a formatted line of text to the end of file 'f'.
 *
 * file_putf() is the ellipsis version. Most file output will call this
 * version. It calls file_vputf() to do the real work. It returns TRUE
 * if the write was successful and FALSE otherwise.
 */
bool file_putf(ang_file *f, const char *fmt, ...)
{
	va_list vp;
	bool status;

	if (!f) return FALSE;

	va_start(vp, fmt);
	vfprintf(f->fh, fmt, vp);
	status = TRUE;/*file_vputf(f, fmt, vp);*/
	va_end(vp);

	return status;
}

/**
 * Append a formatted line of text to the end of file 'f'.
 *
 * file_vputf() is the va_list version. It returns TRUE if the write was
 * successful and FALSE otherwise.
 */
bool file_vputf(ang_file *f, const char *fmt, va_list vp)
{
	char buf[1024];

	if (!f) return FALSE;

	(void)vstrnfmt(buf, sizeof(buf), fmt, &vp);
	return file_put(f, buf);
}

/*
 * Format and translate a string, then print it out to file.
 */
bool x_file_putf(ang_file *f, const char *fmt, ...)
{
	va_list vp;

 	char buf[1024];

 	/* Begin the Varargs Stuff */
 	va_start(vp, fmt);

 	/* Format the args, save the length */
 	(void)vstrnfmt(buf, sizeof(buf), fmt, &vp);

 	/* End the Varargs Stuff */
 	va_end(vp);

 	return file_put(f, buf);
}


bool dir_exists(const char *path)
{
	#ifdef HAVE_STAT
	struct stat buf;
	if (stat(path, &buf) != 0)
		return FALSE;
	else if (buf.st_mode & S_IFDIR)
		return TRUE;
	else
		return FALSE;
	#else
	return TRUE;
	#endif
}

#ifdef HAVE_STAT
bool dir_create(const char *path, u16b permissions)
{
	const char *ptr;
	char buf[512];

	/* If the directory already exists then we're done */
	if (dir_exists(path)) return TRUE;

	if (permissions == 0) {
		permissions = 0755;
	}

	#ifdef WINDOWS
	/* If we're on windows, we need to skip past the "C:" part. */
	if (isalpha(path[0]) && path[1] == ':') path += 2;
	#endif

	/* Iterate through the path looking for path segements. At each step,
	 * create the path segment if it doesn't already exist. */
	for (ptr = path; *ptr; ptr++)
	{
		/*if (*ptr == PATH_SEP)*/
		if (strcmp(ptr, PATH_SEP) == 0) {
			/* Find the length of the parent path string */
			size_t len = (size_t)(ptr - path);

			/* Skip the initial slash */
			if (len == 0) continue;

			/* If this is a duplicate path separator, continue */
			/*if (*(ptr - 1) == PATH_SEP) continue;*/
			if (strcmp((ptr-1), PATH_SEP) == 0) continue;

			/* We can't handle really big filenames */
			if (len - 1 > 512) return FALSE;

			/* Create the parent path string, plus null-padding */
			my_strcpy(buf, path, len + 1);

			/* Skip if the parent exists */
			if (dir_exists(buf)) continue;

			/* The parent doesn't exist, so create it or fail */
			if (my_mkdir(buf, permissions) != 0) return FALSE;
		}
	}
	return my_mkdir(path, permissions) == 0 ? TRUE : FALSE;
}

#else /* HAVE_STAT */
bool dir_create(const char *path, u16b permissions)
{
	return my_mkdir(path, permissions) == 0 ? TRUE : FALSE;
}
#endif /* !HAVE_STAT */

/*** Directory scanning API ***/

/*
 * For information on what these are meant to do, please read the header file.
 */

#ifdef WINDOWS


/* System-specific struct */
struct ang_dir
{
	HANDLE h;
	char *first_file;
};

ang_dir *dir_open(const char *dirname)
{
	WIN32_FIND_DATA fd;
	HANDLE h;
   	ang_dir *dir;
	
	/* Try to open it */
	h = FindFirstFile(format("%s\\*", dirname), &fd);

	/* Abort */
	if (h == INVALID_HANDLE_VALUE)
		return NULL;

	/* Set up the handle */
	dir = ZNEW(ang_dir);
	dir->h = h;
	dir->first_file = (char*)string_make(fd.cFileName);

	/* Success */
	return dir;
}

bool dir_read(ang_dir *dir, char *fname, size_t len)
{
	WIN32_FIND_DATA fd;
	BOOL ok;

	/* Try the first file */
	if (dir->first_file)
	{
		/* Copy the string across, then free it */
		my_strcpy(fname, dir->first_file, len);
		FREE(dir->first_file);

		/* Wild success */
		return TRUE;
	}

	/* Try the next file */
	while (1)
	{
		ok = FindNextFile(dir->h, &fd);
		if (!ok) return FALSE;

		/* Skip directories */
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ||
		    strcmp(fd.cFileName, ".") == 0 ||
		    strcmp(fd.cFileName, "..") == 0)
			continue;

		/* Take this one */
		break;
	}

	/* Copy name */
	my_strcpy(fname, fd.cFileName, len);

	return TRUE;
}

void dir_close(ang_dir *dir)
{
	/* Close directory */
	if (dir->h)
		FindClose(dir->h);

	/* Free memory */
	FREE(dir->first_file);
	FREE(dir);
}

#else /* WINDOWS */

#ifdef HAVE_DIRENT_H

/* Define our ang_dir type */
struct ang_dir
{
	DIR *d;
	char *dirname;
};

ang_dir *dir_open(const char *dirname)
{
	ang_dir *dir;
	DIR *d;

	/* Try to open the directory */
	d = opendir(dirname);
	if (!d) return NULL;

	/* Allocate memory for the handle */
	dir = ZNEW(ang_dir);
	if (!dir) 
	{
		closedir(d);
		return NULL;
	}

	/* Set up the handle */
	dir->d = d;
	dir->dirname = string_make(dirname);

	/* Success */
	return dir;
}

bool dir_read(ang_dir *dir, char *fname, size_t len)
{
	struct dirent *entry;
	struct stat filedata;
	char path[1024];

	assert(dir != NULL);

	/* Try reading another entry */
	while (1)
	{
		entry = readdir(dir->d);
		if (!entry) return FALSE;

		path_build(path, sizeof path, dir->dirname, entry->d_name);
            
		/* Check to see if it exists */
		if (stat(path, &filedata) != 0)
			continue;

		/* Check to see if it's a directory */
		if (S_ISDIR(filedata.st_mode))
			continue;

		/* We've found something worth returning */
		break;
	}

	/* Copy the filename */
	my_strcpy(fname, entry->d_name, len);

	return TRUE;
}

void dir__close(ang_dir *dir)
{
	/* Close directory */
	if (dir->d)
		closedir(dir->d);

	/* Free memory */
	FREE(dir->dirname);
	FREE(dir);
}

#endif /* HAVE_DIRENT_H */
#endif /* WINDOWS */

