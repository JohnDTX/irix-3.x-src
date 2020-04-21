/* attribute lookup and storage */

/* idb_atname maintains a binary tree of attribute names by adding new ones
 * if they don't already exist there.  The unique return value is the
 * pointer to the stashed name.  Names are stashed only once, so there is
 * a one-to-one between names and these returned pointers.
 */

#define library
#include "idb.h"

extern char	*idb_stash ();

Atnode		*atroot = NULL;

char *
idb_atname (name)
	char		*name;
{
	register Atnode	**p;
	register int	t;

	p = &atroot;
	while (*p) {
		t = strcmp (name, (*p)->name);
		if (t < 0) p = &(*p)->left;
		else if (t > 0) p = &(*p)->right;
		else return ((*p)->name);
	}
	*p = (Atnode *) idb_getmem (sizeof (Atnode), NULL);
	(*p)->name = idb_stash (name, NULL);
	(*p)->left = (*p)->right = NULL;
	return ((*p)->name);
}

Attr *
idb_newattr (name, set)
	char		*name;
	Memset		*set;
{
	Attr		*at;

	at = (Attr *) idb_getmem (sizeof (Attr), set);
	at->atname = idb_atname (name);
	at->argc = 0;
	at->argv = NULL;
	return (at);
}

Attr *
idb_getattr (name, rec)
	char		*name;
	Rec		*rec;
{
	Attr		*at;
	char		*atname;

	atname = idb_atname (name);
	for (at = rec->attr; at < rec->attr + rec->nattr; ++at) {
		if (at->atname == atname) return (at);
	}
	return (NULL);
}

void
idb_addlong (at, v, set)
	Attr		*at;
	long		v;
	Memset		*set;
{
	char		buff [16];

	sprintf (buff, "%ld", v);
	idb_addarg (at, buff, set);
}

void
idb_addint (at, v, set)
	Attr		*at;
	int		v;
	Memset		*set;
{
	idb_addlong (at, (long) v, set);
}

void
idb_addarg (at, s, set)
	Attr		*at;
	char		*s;
	Memset		*set;
{
	at->argv = (char **) idb_getmore (at->argv,
		(at->argc + 1) * sizeof (char **), set);
	at->argv [at->argc++] = idb_stash (s, set);
}

void
idb_addargs (at, argc, argv, set)
	Attr		*at;
	int		argc;
	char		*argv [];
	Memset		*set;
{
	int		i;

	at->argv = (char **) idb_getmore (at->argv,
		(at->argc + argc) * sizeof (char **), set);
	for (i = 0; i < argc; ++i) {
		at->argv [at->argc++] = argv [i];
	}
}

Attr *
idb_addattr (rec, name, argc, argv, set)
	Rec		*rec;
	char		*name;
	int		argc;
	char		*argv [];
	Memset		*set;
{
	char		*atname;
	Attr		*at;

	atname = idb_atname (name);
	for (at = rec->attr; at < rec->attr + rec->nattr; ++at) {
		if (at->atname == atname) break;
	}
	if (at >= rec->attr + rec->nattr) {
		rec->attr = (Attr *) idb_getmore (rec->attr,
			(rec->nattr + 1) * sizeof (Attr), set);
		at = rec->attr + rec->nattr++;
		at->atname = atname;
	}
	at->argc = argc;
	at->argv = argv;
	return (at);
}

void
idb_copyattr (rec, at, set)
	Rec		*rec;
	Attr		*at;
	Memset		*set;
{
	Attr		*n;
	int		i;

	idb_addattr (rec, at->atname, at->argc, at->argv, set);
}

void
idb_repattr (rec, at)
	Rec		*rec;
	Attr		*at;
{
	Attr		*p;
	char		*atname;

	atname = idb_atname (at->atname);
	for (p = rec->attr; p < rec->attr + rec->nattr; ++p) {
		if (p->atname == atname) {
			p->argc = at->argc;
			p->argv = at->argv;
		}
	}
}

void
idb_delattr (rec, name)
	Rec		*rec;
	char		*name;
{
	Attr		*p, *t;
	char		*atname;

	atname = idb_atname (name);
	for (p = t = rec->attr; p < rec->attr + rec->nattr; ++p) {
		if (p->atname != atname) {
			if (t != p) {
				t->atname = p->atname;
				t->argc = p->argc;
				t->argv = p->argv;
			}
			++t;
		}
	}
	rec->nattr = t - rec->attr;
}

idb_intat (rec, name)
	Rec		*rec;
	char		*name;
{
	Attr		*p;

	if ((p = idb_getattr (name, rec)) == NULL) return (-1);
	if (p->argc != 1) return (-1);
	return (atoi (p->argv [0]));
}
