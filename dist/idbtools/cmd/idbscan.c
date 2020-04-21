/* idbscan [ -r root ] [ -s src ] [ -i idb ] [ -f selexpfile | selexp ]
 *
 * Scan the database and execute "expr" on each record.  If invoked as "inq",
 * print the dstpath of records for which "expr" returns non-zero.
 */

#include "idb.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef BSD
#include <sys/sysmacros.h>
#endif

extern char	*optarg;
extern int	optind;

char		selexpfile [1024] = "";
char		*selectexp = "putrec()";

main (argc, argv)
	int		argc;
	char		*argv [];
{
	FILE		*f, *g;
	Memset		*xset, *pset;
	Node		*sel;
	int		t, i, inq, all;
	Rec		*rec;
	char		*p;
	Attr		*at;

	if (inq = strcmp (argv [0] + strlen (argv [0]) - 3, "inq") == 0)
		selectexp = "1";
	all = 0;
	pset = idb_newset ();
	xset = idb_newset ();
	while ((t = getopt (argc, argv, "r:s:i:f:a")) != EOF) {
		switch (t) {
		case 'r': strcpy (rbase, optarg); break;
		case 's': strcpy (sbase, optarg); break;
		case 'i': strcpy (idb, optarg); break;
		case 'f': strcpy (selexpfile, optarg); break;
		case 'a': all = 1;
		}
	}
	idb_setbase ();
	if ((g = fopen (idb, "r")) == NULL) {
		perror (idb); exit (1);
	}
	if (*selexpfile != '\0') {
		if ((sel = idb_parsef (selexpfile, pset, Bool)) == NULL) {
			perror (selexpfile); exit (1);
		}
	} else {
		if (optind < argc) {
			selectexp = NULL;
			while (optind < argc) {
				selectexp = idb_cat (selectexp, argv [optind],
					pset);
				++optind;
			}
		}
		if ((sel = idb_parses (selectexp, pset, Bool)) == NULL) {
			fprintf (stderr, "Can't parse expression %s\n",
				selectexp);
			exit (1);
		}
	}
	while (rec = idb_read (g, xset)) {
		t = idb_expr (rec, sel, xset);
		if (inq && t) {
			if (all) idb_write (stdout, rec);
			else printf ("%s\n", rec->dstpath);
		}
		idb_freeset (xset);
	}
	idb_freeset (pset);
	exit (0);
}
