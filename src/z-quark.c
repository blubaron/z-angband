/* File: quark.c */

/* Purpose: Angband utilities -BEN- */


#include "angband.h"
#include "z-quark.h"

/*
 * The "quark" package
 *
 * This package is used to reduce the memory usage of object inscriptions.
 *
 * We use dynamic string allocation because otherwise it is necessary to
 * pre-guess the amount of quark activity.  We limit the total number of
 * quarks, but this is much easier to "expand" as needed.  XXX XXX XXX
 *
 * Two objects with the same inscription will have the same "quark" index.
 *
 * Some code uses "zero" to indicate the non-existance of a quark.
 *
 * Note that "quark zero" is NULL and should never be "dereferenced".
 *
 * ToDo: Automatically resize the array if necessary.
 */

/*
 * The number of quarks
 */
static s16b quark__num;

/*
 * The pointers to the quarks [QUARK_MAX]
 */
static cptr *quark__str;

/*
 * Refcount for Quarks
 */
static u16b *quark__ref;


/*
 * Add a new "quark" to the set of quarks.
 */
s16b quark_add(cptr str)
{
	int i;
	int posn = 0;

	/* Look for an existing quark */
	for (i = 1; i < quark__num; i++)
	{
		/* Test refcount */
		if (!quark__ref[i]) continue;

		/* Check for equality */
		if (streq(quark__str[i], str))
		{
			/* Increase refcount */
			quark__ref[i]++;

			return (i);
		}
	}

	/* Look for an empty quark */
	for (i = 1; i < quark__num; i++)
	{
		if (!quark__ref[i])
		{
			posn = i;
			break;
		}
	}

	/* Did we fail to find room? */
	if (!posn)
	{
		/* Paranoia -- Require room */
		if (quark__num == QUARK_MAX)
		{
			/* Paranoia - no room? */
			return (0);
		}
		else
		{
			/* Use new quark */
			posn = quark__num;

			/* New maximal quark */
			quark__num++;
		}
	}

	/* Add a new quark */
	quark__str[posn] = string_make(str);

	/* One use of this quark */
	quark__ref[posn] = 1;

	/* Return the index */
	return (posn);
}

/*
 * Like quark_add(), but take a format string.
 */
s16b quark_fmt(cptr str, ...)
{
	va_list vp;

	char buf[1024];

	/* Begin the Varargs Stuff */
	va_start(vp, str);

	/* Format the args, save the length */
	(void)vstrnfmt(buf, 1024, str, &vp);

	/* End the Varargs Stuff */
	va_end(vp);

	/* Quark stuff */
	return (quark_add(buf));
}


/*
 * Remove the quark
 */
void quark_remove(s16b *i)
{
	/* Only need to remove real quarks */
	if (!(*i)) return;

	/* Verify */
	if ((*i < 0) || (*i >= quark__num)) return;

	/* Decrease refcount */
	quark__ref[*i]--;

	/* Deallocate? */
	if (!quark__ref[*i])
	{
		string_free(quark__str[*i]);
		quark__str[*i] = NULL;
	}

	/* No longer have a quark here */
	*i = 0;
}

/*
 * Duplicate a quark
 */
void quark_dup(s16b i)
{
	/* Verify */
	if ((i < 0) || (i >= quark__num)) return;

	/* Paranoia */
	if (!quark__ref[i]) return;

	/* Increase refcount */
	quark__ref[i]++;
}


/*
 * This function looks up a quark
 */
cptr quark_str(s16b i)
{
	cptr q;

	/* Verify */
	if ((i < 0) || (i >= quark__num)) return (NULL);

	/* Get the quark */
	q = quark__str[i];

	/* Return the quark */
	return (q);
}


/*
 * Initialize the "quark" package
 */
errr quarks_init(void)
{
	/* Quark variables */
	C_MAKE(quark__str, QUARK_MAX, cptr);
	C_MAKE(quark__ref, QUARK_MAX, u16b);

	quark__num = 1;

	/* Success */
	return (0);
}


/*
 * Free the entire "quark" package
 */
errr quarks_free(void)
{
	int i;

	/* Free the "quarks" */
	for (i = 1; i < quark__num; i++)
	{
		/* Paranoia - only try to free existing quarks */
		if (quark__str[i])
		{
			string_free(quark__str[i]);
		}
	}

	/* Free the list of "quarks" */
	FREE((void *)quark__str);
	FREE((void *)quark__ref);

	/* Success */
	return (0);
}

