/* write a record
 *
 * Given a record structure, write the ascii version on the specified file.
 */

#define library
#include "idb.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>

#define tabstop(n)	(((n) + 8) / 8 * 8)
#define LPAR		'('
#define RPAR		')'

static int	cmpattr ();
static int	cmparg ();

void
idb_write (f, rec)
	FILE		*f;
	Rec		*rec;
{
	int		i, j, argc, c;
	Attr		*at;
	Memset		*mset;
	char		*p, **argv;

	fprintf (f, "%c %04o %s %s %s %s", idb_typec (rec->type), rec->mode,
		rec->user, rec->group, rec->dstpath, rec->srcpath);
	if (rec->nattr == 0) { putc ('\n', f); return; }
	mset = idb_newset ();
	if (rec->nattr > 1) {
		qsort (rec->attr, rec->nattr, sizeof (Attr), cmpattr);
	}
	for (at = rec->attr; at < rec->attr + rec->nattr; ++at) {
		if (at->argc <= 0) {
			fprintf (f, " %s", at->atname);
			continue;
		}
		if (at->argc > 1) {
			qsort (at->argv, at->argc, sizeof (*at->argv), cmparg);
		}
		fprintf (f, " %s%c", at->atname, LPAR);
		for (j = 0; j < at->argc; ++j) {
			for (p = at->argv [j]; *p; ++p) {
				if (isspace (*p) || *p == RPAR || *p == '\\') {
					putc ('\\', f);
				}
				putc (*p, f);
			}
			putc (j < at->argc - 1 ? ' ' : RPAR, f);
		}
	}
	idb_freeset (mset);
	putc ('\n', f);
}

void
idb_pack (f, rec)
	FILE		*f;
	Rec		*rec;
{
	Attr		*at;

	fprintf (f, "%c %04o %s %s %s %s", idb_typec (rec->type), rec->mode,
		rec->user, rec->group, rec->dstpath, rec->srcpath);
	for (at = rec->attr; at < rec->attr + rec->nattr; ++at) {
		fprintf (f, " %s", at->atname);
		if (at->argc) putargs (at, f);
	}
	putc ('\n', f);
}

static
putargs (at, f)
	Attr		*at;
	FILE		*f;
{
	int		i;

	putc (LPAR, f);
	for (i = 0; i < at->argc; ++i) {
		fprintf (f, "%s", at->argv [i]);
		if (i < at->argc - 1) putc (' ', f);
	}
	putc (RPAR, f);
}

static int
cmpattr (a1, a2)
	Attr		*a1;
	Attr		*a2;
{
	return (strcmp (a1->atname, a2->atname));
}

static int
cmparg (a1, a2)
	char		**a1;
	char		**a2;
{
	return (strcmp (*a1, *a2));
}
