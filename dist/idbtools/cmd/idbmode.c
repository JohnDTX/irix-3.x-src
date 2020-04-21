/* idbmode [ -r rbase ] [ -s sbase ] [ -i idb ] [ -f selexpfile | selexp ]
 *
 * Scan the database and execute "expr" on each record.  If the expression
 * evaluates true, make sure that the dstpath under rbase has the proper
 * mode, owner, and group.
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
	int		t, i, vopt;
	Rec		*rec;
	char		*p;
	Attr		*at;
	struct stat	st;

	pset = idb_newset ();
	xset = idb_newset ();
	vopt = 0;
	while ((t = getopt (argc, argv, "r:s:i:f:v")) != EOF) {
		switch (t) {
		case 'r': strcpy (rbase, optarg); break;
		case 's': strcpy (sbase, optarg); break;
		case 'i': strcpy (idb, optarg); break;
		case 'f': strcpy (selexpfile, optarg); break;
		case 'v': ++vopt; break;
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
		if (!t) continue;
		if (lstat (idb_rpath (rec->dstpath), &st) < 0) {
			perror (rec->dstpath); continue;
		}
		if (rec->type != (st.st_mode & S_IFMT)) {
			fprintf (stderr, "%s: type mismatch\n", rec->dstpath);
			continue;
		}
		if (vopt) printf ("%s ", rec->dstpath);
		if ((st.st_mode & 04777) != rec->mode) {
			if (chmod (idb_rpath (rec->dstpath), rec->mode) < 0) {
				perror (rec->dstpath);
			}
			if (vopt) printf (" %04d", rec->mode);
		}
		if (st.st_uid != idb_uid (rec->user) ||
		    st.st_gid != idb_gid (rec->group)) {
			if (chown (idb_rpath (rec->dstpath),
			    idb_uid (rec->user), idb_gid (rec->group)) < 0) {
				perror (rec->dstpath);
			}
			if (vopt) printf (" %s %s", rec->user, rec->group);
		}
		if (vopt) printf ("\n");
		idb_freeset (xset);
	}
	idb_freeset (pset);
	exit (0);
}
