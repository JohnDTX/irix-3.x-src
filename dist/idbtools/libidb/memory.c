/* miscellaneous malloc-interface routines
 *
 * A "memset" is a collection of pointers that can be logically grouped and
 * freed together; they are in the same scope.  A memset is represented by
 * a structure containing a list of pointers and the size of the set.  The
 * routines that allocate memory maintain memsets by adding the pointers when
 * new memory is allocated.  The "idb_newset" function creates a new empty
 * structure; "idb_freeset" frees all of the pointers in it (but not the structure
 * itself, which is reusable).  The "idb_addset" function is basically internal,
 * but may be called from elsewhere with a pointer to put in the memset.
 *
 * If, for some reason, idb_addset can't get more memory to expand the list of
 * pointers when necessary, it quietly ignores the request.  This lets the
 * rest of the program keep running, but perhaps compounds the problem of
 * lack of memory.  The "memseterr" variable is set, and idb_freeset will return
 * a non-zero value the next time it is called, if this occurs.
 *
 * The basic interface consists of idb_getmem and idb_getmore, which use malloc
 * and realloc to get memory and check for errors.  idb_getmore does the
 * polite thing when passed a NULL pointer; i.e. allocates new memory anyway.
 *
 * The "idb_stash" function gets memory for a string and copies the string into it;
 * "idb_cat" tacks two strings together, returning a pointer to the result.
 *
 * The idb_getmem, idb_getmore, idb_stash, and idb_cat functions will bypass the
 * memset code if the supplied memset structure pointer is NULL.  This is
 * effectively "global" allocation.
 */

#define library
#include "idb.h"

extern char	*malloc ();
extern char	*realloc ();

int		memseterr = 0;


Memset *
idb_newset ()			/* create a new empty Memset */
{
	Memset		*set;

	if ((set = (Memset *) malloc (sizeof (Memset))) == NULL) {
		fprintf (stderr, "out of memory in newset.\n");
		exit (1);
	}
	set->list = NULL;
	set->allocated = 0;
	set->size = 0;
	return (set);
}

idb_free (p)
	char		*p;
{
	free (p);
}

int
idb_freeset (set)			/* free all memory in a set */
	Memset		*set;
{
	int		i;

	if (set == NULL) return (memseterr);
	for (i = 0; i < set->size; ++i) free (set->list [i]);
	set->size = 0;
	return (memseterr);
}

int
idb_dispose (set)
	Memset		*set;
{
	int		r;

	if (set == NULL) return (-1);
	r = 0;
	if (set->size > 0) r = idb_freeset (set);
	if (set->list != NULL) idb_free (set->list);
	idb_free (set);
	return (r);
}

char *
idb_addset (p, set)			/* add a pointer to a set */
	char		*p;
	Memset		*set;
{
	if (set == NULL) return (p);
	if (set->allocated == 0) {
		set->list = (char **) malloc (Memsetinc * sizeof (char *));
		if (set->list == NULL) {
			++memseterr;
			return (p);
		}
		set->allocated = Memsetinc;
	}
	if (set->size >= set->allocated) {
		set->allocated += Memsetinc;
		set->list = (char **) realloc (set->list,
			set->allocated * sizeof (char *));
		if (set->list == NULL) {
			set->allocated = 0;
			++memseterr;
			return (p);
		}
	}
	return (set->list [set->size++] = p);
}

char *
idb_repset (oldp, newp, set)	/* replace a pointer in a set */
	register char	*oldp;
	char		*newp;
	register Memset	*set;
{
	register int	i;

	if (set == NULL) return (newp);
	for (i = 0; i < set->size; ++i) if (set->list [i] == oldp) break;
	if (i >= set->size) idb_addset (newp, set);
	else set->list [i] = newp;
	return (newp);
}

char *
idb_getmem (n, set)			/* get memory and add to a memset */
	int		n;
	Memset		*set;
{
	char		*p;

	if ((p = malloc (n)) == NULL) {
		fprintf (stderr, "out of memory in getmem\n");
		exit (1);
	}
	return (idb_addset (p, set));
}

char *
idb_getmore (p, n, set)		/* get more memory and add to a memset */
	char		*p;
	int		n;
	Memset		*set;
{
	int		i;
	char		*newp;

	if (p == NULL) return (idb_getmem (n, set));
	if ((newp = realloc (p, n)) == NULL) {
		fprintf (stderr, "out of memory in getmore\n");
		exit (1);
	}
	return (idb_repset (p, newp, set));
}

char *
idb_stash (s, set)			/* idb_stash a string in memory */
	char		*s;
	Memset		*set;
{
	char		*p;

	if (s == NULL) s = "";
	p = idb_getmem (strlen (s) + 1, set);
	strcpy (p, s);
	return (p);
}

char *
idb_cat (s, t, set)		/* idb_stash a concatenation */
	char		*s;
	char		*t;
	Memset		*set;
{
	int		i;

	if (s == NULL) { s = idb_getmem (1, set); *s = '\0'; }
	if (t == NULL || *t == '\0') return (idb_stash (s, set));
	s = idb_getmore (s, (i = strlen (s)) + strlen (t) + 1, set);
	strcpy (s + i, t);
	return (s);
}
