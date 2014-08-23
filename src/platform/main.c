/* File: main.c */

/*
 * Copyright (c) 1997 Ben Harrison, and others
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 */

#include "angband.h"
#include "grafmode.h"
#include "pickfile.h"


/*
 * Some machines have a "main()" function in their "main-xxx.c" file,
 * all the others use this file for their "main()" function.
 */


#if !defined(MACINTOSH) && !defined(WINDOWS) && !defined(ACORN) && !defined(MACH_O_CARBON)


/*
 * List of available modules in the order they are tried.
 */
static const module_type modules[] =
{
#ifdef USE_GTK
	INIT_MODULE(gtk),
#endif /* USE_GTK */

#ifdef USE_XAW
	INIT_MODULE(xaw),
#endif /* USE_XAW */

#ifdef USE_X11
	INIT_MODULE(x11),
#endif /* USE_X11 */

#ifdef USE_XPJ
	INIT_MODULE(xpj),
#endif /* USE_XPJ */

#ifdef USE_TNB
	INIT_MODULE(tnb),
#endif /* USE_TNB */

#ifdef USE_GCU
	INIT_MODULE(gcu),
#endif /* USE_GCU */

#ifdef USE_CAP
	INIT_MODULE(cap),
#endif /* USE_CAP */

#ifdef USE_DOS
	INIT_MODULE(dos),
#endif /* USE_DOS */

#ifdef USE_IBM
	INIT_MODULE(ibm),
#endif /* USE_IBM */

#ifdef USE_EMX
	INIT_MODULE(emx),
#endif /* USE_EMX */

#ifdef USE_SLA
	INIT_MODULE(sla),
#endif /* USE_SLA */

#ifdef USE_LSL
	INIT_MODULE(lsl),
#endif /* USE_LSL */

#ifdef USE_AMI
	INIT_MODULE(ami),
#endif /* USE_AMI */

#ifdef USE_VME
	INIT_MODULE(vme),
#endif /* USE_VME */

#ifdef USE_VCS
	INIT_MODULE(vcs)
#endif /* USE_VCS */
};

static int init_sound_dummy(int argc, char *argv[], unsigned char *new_game) {
	/* prevent compiler warning*/
	(void) argc;(void) argv;(void) new_game;
	return 0;
}
static void close_sound_dummy(void) {
	return;
}

/*
 * List of sound modules in the order they should be tried.
 */
static const struct module_type sound_modules[] =
{
#ifdef SOUND_SDL
	{ "sdl", {"SDL_mixer sound module", NULL}, init_sound_sdl, close_sound_sdl },
#endif /* SOUND_SDL */

	{ "none", {"No sound", NULL}, init_sound_dummy, close_sound_dummy },
};


/*
 * A hook for "quit()".
 *
 * Close down, then fall back into "quit()".
 */
static void quit_hook(cptr s)
{
	int j;

	/* Show the error message */
	if (s) plog(s);

	/* Scan windows */
	for (j = ANGBAND_TERM_MAX - 1; j >= 0; j--) {
		/* Unused */
		if (!angband_term[j]) continue;

		/* Nuke it */
		term_nuke(angband_term[j]);
	}
	sound_modules[0].close();
	if (ANGBAND_SYS) {
		for (j = 0; j < (int)NUM_ELEMENTS(modules); j++) {
			if (streq(ANGBAND_SYS, modules[j].name)) {
				/* Try to use port */
				modules[j].close();
				break;
			}
		}
	}
	close_graphics_modes();
}



/*
 * Set the stack size (for the Amiga)
 */
#ifdef AMIGA
#include <dos.h>
__near long __stack = 32768L;
#endif /* AMIGA */


/*
 * Set the stack size and overlay buffer (see main-286.c")
 */
#ifdef USE_286
#include <dos.h>
extern unsigned _stklen = 32768U;
extern unsigned _ovrbuffer = 0x1500;
#endif /* USE_286 */


/*
 * Initialize and verify the file paths, and the score file.
 *
 * Use the ANGBAND_PATH environment var if possible, else use
 * DEFAULT_PATH, and in either case, branch off appropriately.
 *
 * First, we'll look for the ANGBAND_PATH environment variable,
 * and then look for the files in there.  If that doesn't work,
 * we'll try the DEFAULT_PATH constant.  So be sure that one of
 * these two things works...
 *
 * We must ensure that the path ends with "PATH_SEP" if needed,
 * since the "init_file_paths()" function will simply append the
 * relevant "sub-directory names" to the given path.
 *
 * Note that the "path" must be "Angband:" for the Amiga, and it
 * is ignored for "VM/ESA", so I just combined the two.
 *
 * Make sure that the path doesn't overflow the buffer.  We have
 * to leave enough space for the path separator, directory, and
 * filenames.
 */
static void init_stuff(void)
{
	char path[1024];
	
	int len = 0;

#if defined(AMIGA) || defined(VM)

	/* Hack -- prepare "path" */
	strnfmt(path, 511, "Angband:");

#else  /* AMIGA / VM */

	cptr tail = NULL;
	
	path[0] = 0;

#ifndef FIXED_PATHS

	/* Get the environment variable */
	tail = getenv("ANGBAND_PATH");

#endif /* FIXED_PATHS */

	/* Use the angband_path, or a default */
	strnfcat(path, 511, &len, "%s", tail ? tail : DEFAULT_PATH);

	/* Hack -- Add a path separator (only if needed) */
	if (!suffix(path, PATH_SEP)) strnfcat(path, 511, &len, PATH_SEP);

#endif /* AMIGA / VM */

#ifdef HAVE_CONFIG_H
	{
		/*
		 * XXX XXX Hack - fall back to './lib/' if DEFAULT_PATH doesn't work.
		 * Autoconf sets the default installation directory to be in
		 * /usr/local/share/games/zangband/lib/ however, for development, that sucks.
		 */
		
		char buf[1024];
		
		/* Look for "news" file - see init2.c */
		path_make(buf, path, "file/news.txt");
		
		/* Failure */
		if (!file_exists(path)) {
			/* plog_fmt("Cannot access the '%s' file!", buf); */
		
			/* Reset to be "./lib/" */
			strcpy(path, "./lib/");	
		}
	}
#endif /* HAVE_CONFIG_H */

	/* Initialize */
#ifdef PRIVATE_USER_PATH
	init_file_paths(path,path,PRIVATE_USER_PATH, NULL);
#else /* PRIVATE_USER_PATH */
	init_file_paths(path,path,path,path);
#endif /* PRIVATE_USER_PATH */

	/* make sure that the user directories exist */
	create_user_dirs();
}



/*
 * Handle a "-d<what>=<path>" option
 *
 * The "<what>" can be any string starting with the same letter as the
 * name of a subdirectory of the "lib" folder (i.e. "i" or "info").
 *
 * The "<path>" can be any legal path for the given system, and should
 * not end in any special path separator (i.e. "/tmp" or "~/.ang-info").
 */
static void change_path(cptr info)
{
	cptr s;

	/* Find equal sign */
	s = strchr(info, '=');

	/* Verify equal sign */
	if (!s) quit_fmt("Try '-d<what>=<path>' not '-d%s'", info);

	/* Analyze */
	switch (tolower(info[0]))
	{
#ifndef FIXED_PATHS
		case 'a':
		{
			string_free(ANGBAND_DIR_APEX);
			ANGBAND_DIR_APEX = string_make(s + 1);
			break;
		}

		case 'f':
		{
			string_free(ANGBAND_DIR_FILE);
			ANGBAND_DIR_FILE = string_make(s + 1);
			break;
		}

		case 'h':
		{
			string_free(ANGBAND_DIR_HELP);
			ANGBAND_DIR_HELP = string_make(s + 1);
			break;
		}

		case 'i':
		{
			string_free(ANGBAND_DIR_INFO);
			ANGBAND_DIR_INFO = string_make(s + 1);
			break;
		}

		case 'x':
		{
			string_free(ANGBAND_DIR_XTRA);
			ANGBAND_DIR_XTRA = string_make(s + 1);
			break;
		}

		case 'g':
		{
			string_free(ANGBAND_DIR_XTRA_GRAF);
			ANGBAND_DIR_XTRA_GRAF = string_make(s + 1);
			break;
		}

		case 'n':
		{
			string_free(ANGBAND_DIR_XTRA_FONT);
			ANGBAND_DIR_XTRA_FONT = string_make(s + 1);
			break;
		}

		case 'o':
		{
			string_free(ANGBAND_DIR_XTRA_SOUND);
			ANGBAND_DIR_XTRA_SOUND = string_make(s + 1);
			break;
		}

		case 'b':
		{
			string_free(ANGBAND_DIR_BONE);
			ANGBAND_DIR_BONE = string_make(s + 1);
			break;
		}

		case 'd':
		{
			string_free(ANGBAND_DIR_DATA);
			ANGBAND_DIR_DATA = string_make(s + 1);
			break;
		}

		case 'e':
		{
			string_free(ANGBAND_DIR_EDIT);
			ANGBAND_DIR_EDIT = string_make(s + 1);
			break;
		}

		case 's':
		{
			string_free(ANGBAND_DIR_SAVE);
			ANGBAND_DIR_SAVE = string_make(s + 1);
			break;
		}

		case 'z':
		{
			string_free(ANGBAND_DIR_SCRIPT);
			ANGBAND_DIR_SCRIPT = string_make(s + 1);
			break;
		}

#endif /* FIXED_PATHS */

		case 'u':
		{
			string_free(ANGBAND_DIR_USER);
			ANGBAND_DIR_USER = string_make(s + 1);
			break;
		}

		default:
		{
			quit_fmt("Bad semantics in '-d%s'", info);
		}
	}
}

/*
 * The default message to print when we get bad input.
 */
static void game_usage(void)
{
	int i, j;

	/* Dump usage information */
	puts("Usage: angband [options] [-- subopts]");
	puts("  -n       Start a new character");
	puts("  -f       Request fiddle (verbose) mode");
	puts("  -w       Request wizard mode");
	puts("  -v       Request sound mode");
	puts("  -g#      Request graphics mode #n");
	puts("  -t       Request bigtile mode");
	puts("  -o       Request original keyset (default)");
	puts("  -r       Request rogue-like keyset");
	puts("  -M       Request monochrome mode");
	puts("  -s<num>  Show <num> high scores (default 10)");
	puts("  -u<who>  Use your <who> savefile");
#ifdef FIXED_PATHS
	puts("  -du=<dir>  Define user dir path");
#else /* FIXED_PATHS */
	puts("  -d<def>  Define a 'lib' dir sub-path");
#endif /* FIXED_PATHS */
#if 0
	/* Print the name and help for each available module */
	puts("  -s<sys>  Use sound module <sys>, where <sys> can be:");
	for (i = 0; i < (int)N_ELEMENTS(sound_modules); i++) {
		if (i) {
			printf("     %s   %s\n", sound_modules[i].name,
			       sound_modules[i].help);
		} else {
			/* The first line is special */
			printf("     %s   %s(default)\n", sound_modules[i].name,
			       sound_modules[i].help);
		}
	}
#endif
#if 0
	puts("  -m<sys>  Use module <sys>, where <sys> can be:");
	/* Print the name and help for each available module */
	for (i = 0; i < (int)N_ELEMENTS(modules); i++) {
		if (i) {
			printf("     %s   %s\n",
			       modules[i].name, modules[i].help);
		} else {
			/* The first line is special */
			printf("     %s   %s (default)\n",
			       modules[i].name, modules[i].help);
		}
	}
#endif

	for (i = 0; i < (int)NUM_ELEMENTS(modules); i++)
	{
		/* Spacer */
		puts("");

		for (j = 0; modules[i].help[j]; j++)
		{
			if (j)
			{
				/* Display seperator */
				if (j == 1)
				{
					puts("  --       Sub options");
				}

				puts(format("  -- %s", modules[i].help[j]));
			}
			else
			{
				/* The first line is special */
				puts(format("  -m%s    %s",
							modules[i].name, modules[i].help[j]));
			}
		}
	}

	/* Actually abort the process */
	quit(NULL);
}


/*
 * Simple "main" function for multiple platforms.
 *
 * Note the special "--" option which terminates the processing of
 * standard options.  All non-standard options (if any) are passed
 * directly to the "init_xxx()" function.
 */
int main(int argc, char *argv[])
{
	int i;

	bool done = FALSE;

	bool new_game = FALSE;
	bool game_in_progress = FALSE;

	int show_score = 0;

	cptr mstr = NULL;

	bool args = TRUE;


	/* Save the "program name" XXX XXX XXX */
	argv0 = argv[0];


#ifdef USE_286
	/* Attempt to use XMS (or EMS) memory for swap space */
	if (_OvrInitExt(0L, 0L))
	{
		_OvrInitEms(0, 0, 64);
	}
#endif /* USE_286 */

	/* Get the file paths */
	init_stuff();
	
	/* Catch nasty signals */
	signals_init();

	/* Drop permissions and initialize multiuser stuff */
	init_setuid();

	/* Process the command line arguments */
	for (i = 1; args && (i < argc); i++)
	{
		/* Require proper options */
		if (argv[i][0] != '-') game_usage();

		/* Analyze option */
		switch (argv[i][1])
		{
			case 'N':
			case 'n':
			{
				new_game = TRUE;
				game_in_progress = TRUE;
				break;
			}

			case 'F':
			case 'f':
			{
				arg_fiddle = TRUE;
				break;
			}

			case 'W':
			case 'w':
			{
				arg_wizard = TRUE;
				break;
			}

			case 'V':
			case 'v':
			{
				arg_sound = TRUE;
				break;
			}

			case 'G':
			case 'g':
			{
				arg_graphics = 1;
				if (argv[i][2]) arg_graphics = atoi(&(argv[i][2]));
				break;
			}
			
			case 'T':
			case 't':
			{
				arg_bigtile = TRUE;
				break;
			}

			case 'R':
			case 'r':
			{
				arg_force_roguelike = TRUE;
				break;
			}

			case 'O':
			case 'o':
			{
				arg_force_original = TRUE;
				break;
			}

			case 'S':
			case 's':
			{
				show_score = atoi(&(argv[i][2]));
				if (show_score <= 0) show_score = 10;
				break;
			}

			case 'u':
			case 'U':
			{
				if (!argv[i][2]) game_usage();

				/* Get the savefile name */
				strncpy(player_name, &argv[i][2], 32);

				/* Make sure it's terminated */
				player_name[31] = '\0';
				game_in_progress = TRUE;
				break;
			}

			case 'm':
			{
				if (!argv[i][2]) game_usage();
				mstr = &argv[i][2];
				break;
			}

			case 'M':
			{
				arg_monochrome = TRUE;
				break;
			}

			case 'd':
			case 'D':
			{
				change_path(&argv[i][2]);
				break;
			}

			case '-':
			{
				argv[i] = argv[0];
				argc = argc - i;
				argv = argv + i;
				args = FALSE;
				break;
			}

			default:
			{
				/* Default usage-help */
				game_usage();
			}
		}
	}

	/* Hack -- Forget standard args */
	if (args)
	{
		argc = 1;
		argv[1] = NULL;
	}


	/* Process the player name */
	process_player_name(TRUE);

	/* load the possible graphics modes */
	if (!init_graphics_modes("graphics.txt")) {
		plog("Graphics list load failed");
	}

	/* Install "quit" hook */
	quit_aux = quit_hook;

	for (i = 0; i < (int)NUM_ELEMENTS(modules); i++)
	{
		if (!mstr || (streq(mstr, modules[i].name)))
		{
			/* Try to use port */
			if (0 == modules[i].init(argc, argv, (unsigned char *)&new_game))
			{
				/* Set port name */
				ANGBAND_SYS = modules[i].name;
				done = TRUE;
				break;
			}
		}
	}

	/* Make sure we have a display! */
	if (!done) quit("Unable to prepare any 'display module'!");

	/* Try the modules in the order specified by sound_modules[] */
	/*for (i = 0; i < (int)N_ELEMENTS(sound_modules); i++)
		if (!soundstr || streq(soundstr, sound_modules[i].name))
			if (0 == sound_modules[i].init(argc, argv, NULL)) {
				ANGBAND_SOUND = sound_modules[i].name;
				break;
			}*/
	/* initialize the first sound module (until there is a global variable
	 * to store a pointer to the name, or a pointer to the sound module)*/
	sound_modules[0].init(argc, argv, (unsigned char *)&new_game);

	/* Gtk and Tk initialise earlier */
	if (!(streq(ANGBAND_SYS, "gtk") || streq(ANGBAND_SYS, "tnb")))
	{
		/* Initialize */
		init_angband();
	}

	/* Hack -- If requested, display scores and quit */
	if (show_score > 0) display_scores(0, show_score);

	/* This section is to play repeated games without exiting first was
	 * modified from Sil */
	while (1) {
#ifdef USE_SAVER
		if (screensaver) {
			/* Start the screensaver */
			start_screensaver();
		}
#endif /* USE_SAVER */


		/* Let the player choose a savefile or start a new game */
		if (!game_in_progress) {
			char key;

			/* Process Events until "new" or "open" is selected */
			while (!game_in_progress) {

				/* process any windows messages */
				Term_flush();

				/* remove any previous buttons */
				button_kill_all();

				/* Show the initial screen again */
				display_introduction();

				/* Prompt the user */
				if (savefile[0] != 0) {
					if (p_ptr->state.is_dead) {
						prtf(14, 22, "[Choose $U'(N)ew game'$Yn$V, $U'(O)pen saved'$Yo$V, or $U'e(X)it'$Yx$V]");
						prtf(14, 23, " [Or choose to $U'load (L)ast'$Yl$V or $U'return to (G)raveyard'$Yg$V]");
					} else {
						prtf(14, 22, "[Choose $U'(N)ew game'$Yn$V, $U'(O)pen saved'$Yo$V, or $U'e(X)it'$Yx$V]");
						prtf(14, 23, " [Or choose to $U'load (L)ast'$Yl$V]");
					}
				} else {
					prtf(14, 23, "[Choose $U'(N)ew game'$Yn$V, $U'(O)pen saved'$Yo$V, or $U'e(X)it'$Yx$V]");
				}

				/* Flush it */
				Term_fresh();

				/* see if there is a key press on the queue */
				key = inkey();

				if ((key == 'n') || (key == 'N')) {
					/* New game */
					savefile[0] = 0;
					game_in_progress = TRUE;
					new_game = TRUE;
				} else
				if ((key == 'o') || (key == 'O')) {
					/* Load a save game */
					char file[96];

					/* pick a save file to load */
#ifdef SAVEFILE_USE_UID
					char plr_uid[32];
					strnfmt(plr_uid, 32, "%u.", player_uid);
					if (file_pick(file, 96, "Pick save file", ANGBAND_DIR_SAVE,
					              NULL,NULL,NULL,plr_uid) >= 0)
#else
					if (file_pick(file, 96, "Pick save file", ANGBAND_DIR_SAVE,
					              NULL,NULL,NULL,NULL) >= 0)
#endif
					{
						strnfmt(savefile, 1024, "%s%s%s",ANGBAND_DIR_SAVE, PATH_SEP, file);
						game_in_progress = TRUE;
						new_game = FALSE;
					}
				} else
				if ((key == 'l') || (key == 'L')) {
					if (savefile[0] != 0) {
						if (file_exists(savefile)) {
							game_in_progress = TRUE;
							new_game = FALSE;
						}
					} else
					/* load the last saved file in the save dir, by date/time */
					/* for now open a username based file, if it exists */
					if (player_name[0] != 0) {
						char buf[1024];
#ifdef SAVEFILE_USE_UID
						strnfmt(buf, 1024, "%s%s%d.%s",ANGBAND_DIR_SAVE, PATH_SEP, player_uid, player_name);
#else
						strnfmt(buf, 1024, "%s%s%s",ANGBAND_DIR_SAVE, PATH_SEP, player_name));
#endif
						if (file_exists(buf)) {
							my_strcpy(savefile, buf, 1024);
							game_in_progress = TRUE;
							new_game = FALSE;
						}
					}
				} else
				if ((key == 'g') || (key == 'G')) {
					if ((savefile[0] != 0) && (p_ptr->state.is_dead)) {
						/* show the graveyard */
						tomb_menu(FALSE);

						button_kill_all();
					}
				} else
				/*if ((key == 't') || (key == 'T')) {
					*//* Start the tutorial */ /*
					game_in_progress = TRUE;
					new_game = FALSE;
				} else*/
				if ((key == 's') || (key == 'S')) {
					if (!angband_term[0]->resize_hook) {
						angband_term[0]->resize_hook = resize_map;
					}
					/* Start the tutorial */
					top_twenty();

				} else
				if ((key == ESCAPE) || (key == 'x') || (key == 'X')) {
					/* Free resources */
					cleanup_angband();
					/* Quit */
					quit(NULL);
					return (0);
				}
			}
		}

		button_kill_all();

		/*
		 * Play a game -- "new_game" is set by "new", "open" or the open document
		 * event handler as appropriate
		 */
		play_game(new_game);

		/* game no longer in progress */
		game_in_progress = FALSE;
		new_game = FALSE;

/* abort the program until repeated play works */
cleanup_angband();
quit("Repeated play is not working yet");
return 0;
		/* Free the messages */
		messages_free();

		/* Free the "quarks" */
		//quarks_free();

		//cleanup_angband();

		/* Hack - redraw everything + recalc bigtile regions */
		angband_term[0]->resize_hook();
    
		// reset some globals that start at 0
		character_loaded = FALSE;

		// rerun the first initialization routine
		//init_stuff();
		
		// do some more between-games initialization
		//re_init_some_things();
		/* Initialize the "quark" package */
		//(void)quarks_init();

		/* Initialize the "message" package */
		(void)messages_init();

		//init_angband();

		/* reinitialize arrays that use quarks */
		//note("[Initializing arrays... (wilderness)]");
		//if (init_w_info()) quit("Cannot initialize wilderness");
	}

	/* Free resources */
	cleanup_angband();

	/* Quit */
	quit(NULL);

	/* Exit */
	return (0);
}

#endif /* !defined(MACINTOSH) && !defined(WINDOWS) && !defined(ACORN) */
