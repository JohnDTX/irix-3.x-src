/* idbrmat [ -i idb ] [ -e selexp ] atname ...
 */

#include "idb.h"

#define		LPAR		'('
#define		RPAR		')'

#define		Dstpath		1
#define		Srcpath		2
#define		Type		3
#define		Mode		4
#define		User		5
#define		Group		6

extern char	*optarg;
extern int	optind;

char		*nextifield ();
Attr		*iattr ();

char		selexpfile [1024] = "";
char		*selexp = "!noship";
int		more = 1;

char		*rmat [1024];

main (argc, argv)
	int		argc;
	char		*argv [];
{
	char		*p, buff [1024];
	int		i, t, nfields;
	Node		*sel;
	Memset		*pset, *xset;
	FILE		*g;
	Rec		*rec;

	while ((t = getopt (argc, argv, "r:s:i:f:e:")) != EOF) {
		switch (t) {
		case 'r': strcpy (rbase, optarg); break;
		case 's': strcpy (sbase, optarg); break;
		case 'i': strcpy (idb, optarg); break;
		case 'f': strcpy (selexpfile, optarg); break;
		case 'e': strcpy (selexp, optarg); break;
		}
	}
	nfields = 0;
	while (optind < argc) {
		rmat [nfields++] = idb_stash (argv [optind++], NULL);
	}
	idb_setbase ();
	pset = idb_newset ();
	xset = idb_newset ();
	if ((g = fopen (idb, "r")) == NULL) {
		perror (idb); exit (1);
	}
	if (*selexpfile != '\0') {
		if ((sel = idb_parsef (selexpfile, pset, Bool)) == NULL) {
			perror (selexpfile); exit (1);
		}
	} else {
		if ((sel = idb_parses (selexp, pset, Bool)) == NULL) {
			fprintf (stderr, "Can't parse '%s'\n", selexp);
			exit (1);
		}
	}
	while ((rec = idb_read (g, xset)) != NULL) {
		if (idb_expr (rec, sel, xset)) {
			for (i = 0; i < nfields; ++i) {
				idb_delattr (rec, rmat [i]);
			}
		}
		idb_write (stdout, rec);
	}
	idb_freeset (pset);
	exit (0);
}
