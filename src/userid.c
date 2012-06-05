/* File: userid.c */

/* Purpose: Angband utilities -BEN- */


#include "angband.h"


#ifdef SET_UID

#ifdef PRIVATE_USER_PATH

/*
 * Create an ".angband/" directory in the users home directory.
 *
 * ToDo: Add error handling.
 * ToDo: Only create the directories when actually writing files.
 */
void create_user_dirs(void)
{
	char dirpath[1024];
	char subdirpath[1024];


	/* Get an absolute path from the filename */
	path_parse(dirpath, 1024, PRIVATE_USER_PATH);

	/* Create the ~/.angband/ directory */
	mkdir(dirpath, 0700);

	/* Build the path to the variant-specific sub-directory */
	path_make(subdirpath, dirpath, VERSION_NAME);

	/* Create the directory */
	mkdir(subdirpath, 0700);

#ifdef USE_PRIVATE_PATHS

	/* Build the path to the scores sub-directory */
	path_build(dirpath, sizeof(dirpath), subdirpath, "scores");

	/* Create the directory */
	mkdir(dirpath, 0700);

	/* Build the path to the savefile sub-directory */
	path_build(dirpath, sizeof(dirpath), subdirpath, "bone");

	/* Create the directory */
	mkdir(dirpath, 0700);

	/* Build the path to the savefile sub-directory */
	path_build(dirpath, sizeof(dirpath), subdirpath, "data");

	/* Create the directory */
	mkdir(dirpath, 0700);

	/* Build the path to the savefile sub-directory */
	path_build(dirpath, sizeof(dirpath), subdirpath, "save");

	/* Create the directory */
	mkdir(dirpath, 0700);

#endif /* USE_PRIVATE_PATHS */

}

#endif /* PRIVATE_USER_PATH */


/*
 * Hack -- drop permissions
 */
void safe_setuid_drop(void)
{

#ifdef SAFE_SETUID

#ifdef HAVE_SETEGID

	if (setegid(getgid()) != 0)
	{
		quit("setegid(): cannot set permissions correctly!");
	}

#else  /* HAVE_SETEGID */

#ifdef SAFE_SETUID_POSIX

	if (setgid(getgid()) != 0)
	{
		quit("setgid(): cannot set permissions correctly!");
	}

#else  /* SAFE_SETUID_POSIX */

	if (setregid(getegid(), getgid()) != 0)
	{
		quit("setregid(): cannot set permissions correctly!");
	}

#endif /* SAFE_SETUID_POSIX */

#endif /* HAVE_SETEGID */

#endif /* SAFE_SETUID */

}


/*
 * Hack -- grab permissions
 */
void safe_setuid_grab(void)
{

#ifdef SAFE_SETUID

#ifdef HAVE_SETEGID

	if (setegid(player_egid) != 0)
	{
		quit("setegid(): cannot set permissions correctly!");
	}

#else  /* HAVE_SETEGID */

#ifdef SAFE_SETUID_POSIX

	if (setgid(player_egid) != 0)
	{
		quit("setgid(): cannot set permissions correctly!");
	}

#else  /* SAFE_SETUID_POSIX */

	if (setregid(getegid(), getgid()) != 0)
	{
		quit("setregid(): cannot set permissions correctly!");
	}

#endif /* SAFE_SETUID_POSIX */

#endif /* HAVE_SETEGID */

#endif /* SAFE_SETUID */

}


/*
 * Initialise things for multiuser machines
 * Pay special attention to permisions.
 */
void init_setuid(void)
{
	/* Default permissions on files */
	(void)umask(022);

	/* Get the user id (?) */
	player_uid = getuid();

#ifdef VMS
	/* Mega-Hack -- Factor group id */
	player_uid += (getgid() * 1000);
#endif /* VMS */

#ifdef SAFE_SETUID

#if defined(HAVE_SETEGID) || defined(SAFE_SETUID_POSIX)

	/* Save some info for later */
	player_euid = geteuid();
	player_egid = getegid();

#endif /* defined(HAVE_SETEGID) || defined(SAFE_SETUID_POSIX) */

	/* XXX XXX XXX */
#if 0

	/* Redundant setting necessary in case root is running the game */
	/* If not root or game not setuid the following two calls do nothing */

	if (setgid(getegid()) != 0)
	{
		quit("setgid(): cannot set permissions correctly!");
	}

	if (setuid(geteuid()) != 0)
	{
		quit("setuid(): cannot set permissions correctly!");
	}

#endif /* 0 */

#endif /* SAFE_SETUID */

	/* Drop permissions */
	safe_setuid_drop();

	/* Get the "user name" as a default player name */
	user_name(player_name, player_uid);

#ifdef PRIVATE_USER_PATH

	/* Create a directory for the users files. */
	create_user_dirs();

#endif /* PRIVATE_USER_PATH */
}


#else /* SET_UID */

void safe_setuid_drop(void)
{

}

void safe_setuid_grab(void)
{

}

void init_setuid(void)
{

}

#endif /* SET_UID */



#ifdef SET_UID

# ifndef HAS_USLEEP

/*
 * For those systems that don't have "usleep()" but need it.
 *
 * Fake "usleep()" function grabbed from the inl netrek server -cba
 */
int usleep(huge usecs)
{
	struct timeval Timer;

	int nfds = 0;

#ifdef FD_SET
	fd_set *no_fds = NULL;
#else
	int *no_fds = NULL;
#endif


	/* Was: int readfds, writefds, exceptfds; */
	/* Was: readfds = writefds = exceptfds = 0; */


	/* Paranoia -- No excessive sleeping */
	if (usecs > 4000000L) core("Illegal usleep() call");


	/* Wait for it */
	Timer.tv_sec = (usecs / 1000000L);
	Timer.tv_usec = (usecs % 1000000L);

	/* Wait for it */
	if (select(nfds, no_fds, no_fds, no_fds, &Timer) < 0)
	{
		/* Hack -- ignore interrupts */
		if (errno != EINTR) return -1;
	}

	/* Success */
	return 0;
}

# endif /* !HAS_USLEEP */

/*
 * Hack -- External functions
 */
#ifndef	_PWD_H
# ifndef HAVE_GETPWUID
extern struct passwd *getpwuid();
# endif
# ifndef HAVE_GETPWNAM
extern struct passwd *getpwnam();
# endif
#endif /* _PWD_H */

/*
 * Find a default user name from the system.
 */
void user_name(char *buf, int id)
{
	struct passwd *pw;

	/* Look up the user name */
	if ((pw = getpwuid(id)))
	{
		/* Get the first 15 characters of the user name */
		(void)strncpy(buf, pw->pw_name, 16);
		buf[15] = '\0';

#ifdef CAPITALIZE_USER_NAME
		/* Hack -- capitalize the user name */
		if (islower(buf[0])) buf[0] = toupper(buf[0]);
#endif /* CAPITALIZE_USER_NAME */

		return;
	}

	/* Oops.  Hack -- default to "PLAYER" */
	strcpy(buf, "PLAYER");
}

#endif /* SET_UID */

