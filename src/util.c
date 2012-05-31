/* File: util.c */

/* Purpose: Angband utilities -BEN- */


#include "angband.h"


/*
 * Helper function to assert something inside an expression
 *
 * (Note that the normal assert macro can only be used
 *  as a statement - which prevents debugging via
 *  function wrappers.)
 */
bool assert_helper(cptr expr, cptr file, int line, bool result)
{
	if (!result)
	{
		signals_ignore_tstp();\
		ANG__assert_save;\
		ANG__assert_fmt("\n"
						"Assertion failed:%s\n"
						"in file %s\n"
						"on line %d\n\n", expr, file, line);
	}

	return (result);
}


/* This works by masking off the lowest order set bits one at a time */
int count_bits(u32b x)
{
	int n = 0;

	if (x)
	{
		do
		{
			n++;
		}
		while (0 != (x = x & (x - 1)));
	}

	return (n);
}


/*
 * Check a char for "vowel-hood"
 */
bool is_a_vowel(int ch)
{
	switch (ch)
	{
		case 'a':
		case 'e':
		case 'i':
		case 'o':
		case 'u':
		case 'A':
		case 'E':
		case 'I':
		case 'O':
		case 'U':
			return (TRUE);
	}

	return (FALSE);
}


/*
 * Hack -- special buffer to hold the action of the current keymap
 */
static char request_command_buffer[256];

extern cptr inkey_next;

/*
 * Request a command from the user.
 *
 * Sets p_ptr->cmd.cmd, p_ptr->cmd.dir, p_ptr->cmd.rep,
 * p_ptr->cmd.arg.  May modify p_ptr->cmd.new.
 *
 * Note that "caret" ("^") is treated specially, and is used to
 * allow manual input of control characters.  This can be used
 * on many machines to request repeated tunneling (Ctrl-H) and
 * on the Macintosh to request "Control-Caret".
 *
 * Note that "backslash" is treated specially, and is used to bypass any
 * keymap entry for the following character.  This is useful for macros.
 *
 * Note that this command is used both in the dungeon and in
 * stores, and must be careful to work in both situations.
 *
 * Note that "p_ptr->cmd.new" may not work any more.  XXX XXX XXX
 */
void request_command(int shopping)
{
	int i;

	char cmd;

	int mode;

	cptr act;


	/* Roguelike */
	if (rogue_like_commands)
	{
		mode = KEYMAP_MODE_ROGUE;
	}

	/* Original */
	else
	{
		mode = KEYMAP_MODE_ORIG;
	}


	/* No command yet */
	p_ptr->cmd.cmd = 0;

	/* No "argument" yet */
	p_ptr->cmd.arg = 0;

	/* No "direction" yet */
	p_ptr->cmd.dir = 0;


	/* Get command */
	while (1)
	{
		/* Hack -- auto-commands */
		if (p_ptr->cmd.new)
		{
			/* Flush messages */
			message_flush();

			/* Use auto-command */
			cmd = (char)p_ptr->cmd.new;

			/* Forget it */
			p_ptr->cmd.new = 0;
		}

		/* Get a keypress in "command" mode */
		else
		{
			/* Hack -- no flush needed */
			msg_flag = FALSE;

			/* Reset the skip_more flag */
			p_ptr->state.skip_more = FALSE;

			/* Activate "command mode" */
			p_ptr->cmd.inkey_flag = TRUE;

			/* Get a command */
			cmd = inkey_m();
		}

		/* Clear top line */
		clear_msg();


		/* Command Count */
		if (cmd == '0')
		{
			int old_arg = p_ptr->cmd.arg;

			/* Reset */
			p_ptr->cmd.arg = 0;

			/* Begin the input */
			prtf(0, 0, "Count: ");

			/* Get a command count */
			while (1)
			{
				/* Get a new keypress */
				cmd = inkey();

				/* Simple editing (delete or backspace) */
				if ((cmd == 0x7F) || (cmd == KTRL('H')))
				{
					/* Delete a digit */
					p_ptr->cmd.arg = p_ptr->cmd.arg / 10;

					/* Show current count */
					prtf(0, 0, "Count: %d", p_ptr->cmd.arg);
				}

				/* Actual numeric data */
				else if (cmd >= '0' && cmd <= '9')
				{
					/* Stop count at 9999 */
					if (p_ptr->cmd.arg >= 1000)
					{
						/* Warn */
						bell("Invalid repeat count!");

						/* Limit */
						p_ptr->cmd.arg = 9999;
					}

					/* Increase count */
					else
					{
						/* Incorporate that digit */
						p_ptr->cmd.arg = p_ptr->cmd.arg * 10 + D2I(cmd);
					}

					/* Show current count */
					prtf(0, 0, "Count: %d", p_ptr->cmd.arg);
				}

				/* Exit on "unusable" input */
				else
				{
					break;
				}
			}

			/* Hack -- Handle "zero" */
			if (p_ptr->cmd.arg == 0)
			{
				/* Default to 99 */
				p_ptr->cmd.arg = 99;

				/* Show current count */
				prtf(0, 0, "Count: %d", p_ptr->cmd.arg);
			}

			/* Hack -- Handle "old_arg" */
			if (old_arg != 0)
			{
				/* Restore old_arg */
				p_ptr->cmd.arg = old_arg;

				/* Show current count */
				prtf(0, 0, "Count: %d", p_ptr->cmd.arg);
			}

			/* Hack -- white-space means "enter command now" */
			if ((cmd == ' ') || (cmd == '\n') || (cmd == '\r'))
			{
				/* Get a real command */
				if (!get_com("Command: ", &cmd))
				{
					/* Clear count */
					p_ptr->cmd.arg = 0;

					/* Continue */
					continue;
				}
			}
		}


		/* Allow "keymaps" to be bypassed */
		if (cmd == '\\')
		{
			/* Get a real command */
			(void)get_com("Command: ", &cmd);

			/* Hack -- bypass keymaps */
			if (!inkey_next) inkey_next = "";
		}


		/* Allow "control chars" to be entered */
		if (cmd == '^')
		{
			/* Get a new command and controlify it */
			if (get_com("Control: ", &cmd)) cmd = KTRL(cmd);
		}


		/* Look up applicable keymap */
		act = keymap_act[mode][(byte)(cmd)];

		/* Apply keymap if not inside a keymap already */
		if (act && !inkey_next)
		{
			/* Install the keymap (limited buffer size) */
			(void)strnfmt(request_command_buffer, 256, "%s", act);

			/* Start using the buffer */
			inkey_next = request_command_buffer;

			/* Continue */
			continue;
		}


		/* Paranoia */
		if (!cmd) continue;


		/* Use command */
		p_ptr->cmd.cmd = cmd;

		/* Done */
		break;
	}

	/* Hack -- Auto-repeat certain commands */
	if (p_ptr->cmd.arg <= 0)
	{
		/* Hack -- auto repeat certain commands */
		if (strchr("TBDoc+", p_ptr->cmd.cmd))
		{
			/* Repeat 99 times */
			p_ptr->cmd.arg = 99;
		}
	}

	/* Shopping */
	if (shopping == 1)
	{
		/* Convert */
		switch (p_ptr->cmd.cmd)
		{
				/* Command "p" -> "purchase" (get) */
			case 'p': p_ptr->cmd.cmd = 'g';
				break;

				/* Command "m" -> "purchase" (get) */
			case 'm': p_ptr->cmd.cmd = 'g';

				break;

				/* Command "s" -> "sell" (drop) */
			case 's': p_ptr->cmd.cmd = 'd';
				break;
		}
	}

	/* Hack -- Scan equipment */
	for (i = 0; i < EQUIP_MAX; i++)
	{
		cptr s;

		object_type *o_ptr = &p_ptr->equipment[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* No inscription */
		if (!o_ptr->inscription) continue;

		/* Obtain the inscription */
		s = quark_str(o_ptr->inscription);

		/* Find a '^' */
		s = strchr(s, '^');

		/* Process preventions */
		while (s)
		{
			/* Check the "restriction" character */
			if ((s[1] == p_ptr->cmd.cmd) || (s[1] == '*'))
			{
				/* Hack -- Verify command */
				if (!get_check("Are you sure? "))
				{
					/* Hack -- Use space */
					p_ptr->cmd.cmd = ' ';
				}
			}

			/* Find another '^' */
			s = strchr(s + 1, '^');
		}
	}


	/* Hack -- erase the message line. */
	clear_msg();
}


#define REPEAT_MAX		20

/* Number of chars saved */
static int repeat__cnt = 0;

/* Current index */
static int repeat__idx = 0;

/* Saved "stuff" */
static int repeat__key[REPEAT_MAX];


void repeat_push(int what)
{
	/* Too many keys */
	if (repeat__cnt == REPEAT_MAX) return;

	/* Push the "stuff" */
	repeat__key[repeat__cnt++] = what;

	/* Prevents us from pulling keys */
	++repeat__idx;
}

void repeat_clear(void)
{
	/* Start over from the failed pull */
	if (repeat__idx)
	{
		/* Decrease the number of characters */
		--repeat__idx;
	}

	/* Set the counter */
	repeat__cnt = repeat__idx;
}


bool repeat_pull(int *what)
{
	/* All out of keys */
	if (repeat__idx == repeat__cnt) return (FALSE);

	/* Grab the next key, advance */
	*what = repeat__key[repeat__idx++];

	/* Success */
	return (TRUE);
}

void repeat_check(void)
{
	int what;

	/* Ignore some commands */
	if (p_ptr->cmd.cmd == ESCAPE) return;
	if (p_ptr->cmd.cmd == ' ') return;
	if (p_ptr->cmd.cmd == '\r') return;
	if (p_ptr->cmd.cmd == '\n') return;

	/* Repeat Last Command */
	if (p_ptr->cmd.cmd == 'n')
	{
		/* Reset */
		repeat__idx = 0;

		/* Get the command */
		if (repeat_pull(&what))
		{
			/* Save the command */
			p_ptr->cmd.cmd = what;
		}
	}

	/* Start saving new command */
	else
	{
		/* Reset */
		repeat__cnt = 0;
		repeat__idx = 0;

		what = p_ptr->cmd.cmd;

		/* Save this command */
		repeat_push(what);
	}
}
