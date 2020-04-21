#include "idb.h"

extern int	optind;
extern char	*optarg;

main (argc, argv)
	int		argc;
	char		*argv [];
{
	Memset		*rset;
	int		i, t, nlinks;
	Rec		*r;
	Attr		*at;
	FILE		*fidb, *out;
	char		*links [1024];

	while ((t = getopt (argc, argv, "r:s:i:")) != EOF) {
		switch (t) {
		case 'r': strcpy (rbase, optarg); break;
		case 's': strcpy (sbase, optarg); break;
		case 'i': strcpy (idb, optarg); break;
		}
	}
	idb_setbase ();
	if ((fidb = fopen (idb, "r")) == NULL) {
		perror (idb); exit (1);
	}
	if ((out = popen ("sort -u +4", "w")) == NULL) {
		fprintf (stderr, "idbdelink: can't pipe through sort.\n");
		out = stdout;
	}
	rset = idb_newset ();
	while ((r = idb_read (fidb, rset)) != NULL) {
		nlinks = 0;
		if ((at = idb_getattr ("links", r)) != NULL) {
			nlinks = at->argc;
			for (i = 0; i < at->argc; ++i) {
				links [i] = at->argv [i];
			}
			idb_delattr (r, "links");
		}
		idb_write (out, r);
		for (i = 0; i < nlinks; ++i) {
			r->dstpath = links [i];
			idb_write (out, r);
		}
		idb_freeset (rset);
	}
	if (out != stdout) pclose (out);
	fclose (fidb);
	exit (0);
}
