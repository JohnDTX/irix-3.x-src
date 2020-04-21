#include "idb.h"
#include <sys/types.h>
#include <sys/stat.h>

char		*rpath ();
char		*spath ();

char		cwd [1024];
char		selexpfile [1024];
char		bootfile [1024] = "vmunix";
char		*selexpstr = "miniroot";
int		dest = 0;
int		blocks = 6000;
int		inodes;
int		levels;
int		nodelink;
char		*options = "i:r:s:f:b:dB:I:D";
char		*optarg;
int		optind;

main (argc, argv)
	int		argc;
	char		*argv [];
{
	FILE		*db;
	Node		*sel;
	Memset		*pset, *rset, *xset;
	Rec		*rec;
	int		c, rooted;
	char		buff [1024];

	while ((c = getopt (argc, argv, options)) != EOF) {
		switch (c) {
		case 'i': strcpy (idb, optarg); break;
		case 'r': strcpy (rbase, optarg); break;
		case 's': strcpy (sbase, optarg); break;
		case 'f': strcpy (selexpfile, optarg); break;
		case 'd': dest = 1; break;
		case 'b': strcpy (bootfile, optarg); break;
		case 'B': blocks = atoi (optarg); break;
		case 'I': inodes = atoi (optarg); break;
		case 'n': nodelink = 1; break;
		}
	}
	if (inodes == 0) inodes = blocks / 5;
	idb_setbase (rbase, sbase, idb);
	rset = idb_newset ();
	sprintf (buff, "idbdelink -r%s -s%s -i%s", rbase, sbase, idb);
	if (nodelink) {
		db = stdin;
	} else {
		if ((db = popen (buff, "r")) == NULL) {
			fprintf (stderr, "idbproto: can't read through idbdelink\n");
			exit (1);
		}
	} 
	printf ("%s\n%d %d\n", bootfile, blocks, inodes);
	pset = idb_newset ();
	xset = idb_newset ();
	if (optind < argc) {
		selexpstr = argv [optind];
	}
	if (selexpfile [0]) sel = idb_parsef (selexpfile, pset, Bool);
	else sel = idb_parses (selexpstr, pset, Bool);
	rooted = 0;
	while (idb_freeset (rset), (rec = idb_read (db, rset)) != NULL) {
		idb_freeset (xset);
		if (!idb_expr (rec, sel, xset)) continue;
		if (!rooted) {
			if (rec->type != S_IFDIR) {
				fprintf (stderr, "missing root dir spec\n");
				exit (1);
			}
			putspec (rec); putchar ('\n');
			rooted = 1;
		} else {
			putentry (rec);
		}
	}
	pop (levels);
	printf ("$\n");
	exit (0);
}

putspec (rec)
	Rec		*rec;
{
	switch (rec->type) {
	case S_IFREG: putchar ('-'); break;
	case S_IFBLK: putchar ('b'); break;
	case S_IFCHR: putchar ('c'); break;
	case S_IFDIR: putchar ('d'); break;
	case S_IFLNK: putchar ('l'); break;
	default: fprintf (stderr, "can't create type %c (%s)\n", rec->type,
			rec->dstpath);
	}
	if (rec->mode & S_ISUID) putchar ('u'); else putchar ('-');
	if (rec->mode & S_ISGID) putchar ('g'); else putchar ('-');
	printf ("%03o %d %d", rec->mode & 0777, idb_uid (rec->user),
		idb_gid (rec->group));
}

putentry (rec)
	Rec		*rec;
{
	int		i, n;
	char		*s, *t, *nexts;
	Attr		*at;

	n = 0;
	for (nexts = s = rec->dstpath, t = cwd; *s == *t; ++s, ++t) {
		if (*s == '\0') break;
		if (*s == '/') { ++n; nexts = s + 1; }
	}
	if (*s == '/' && *t == '\0') { ++n; nexts = s + 1; }
	pop (levels - n);
	for (i = 0; i < levels; ++i) putchar ('\t');
	for (s = nexts; *s; ++s) {
		if (*s == '/') {
			fprintf (stderr, "no parent directory spec for %s\n",
				rec->dstpath);
		}
		putchar (*s);
	}
	putchar ('\t');
	if (s - nexts < 8) putchar ('\t');
	putspec (rec);
	switch (rec->type) {
	case S_IFBLK:
	case S_IFCHR:
		if ((at = idb_getattr ("maj", rec)) != NULL &&
		    at->argc == 1) {
			printf (" %s", at->argv [0]);
		} else {
			printf (" ?maj");
			fprintf (stderr, "don't know major for %s\n",
				rec->dstpath);
		}
		if ((at = idb_getattr ("min", rec)) != NULL &&
		    at->argc == 1) {
			printf (" %s", at->argv [0]);
		} else {
			printf (" ?min");
			fprintf (stderr, "don't know minor for %s\n",
				rec->dstpath);
		}
		break;
	case S_IFDIR:
		if (*cwd) strcat (cwd, "/");
		strcat (cwd, nexts);
		++levels;
		break;
	case S_IFLNK:
		if ((at = idb_getattr ("symval", rec)) != NULL &&
		    at->argc == 1) {
			printf (" %s", at->argv [0]);
		} else {
			printf (" nosymval");
			fprintf (stderr, "don't know symval for %s\n",
				rec->dstpath);
		}
		break;
	case S_IFREG:
		printf (" %s", dest ?
			rpath (rec->dstpath) : spath (rec->srcpath));
	}
	putchar ('\n');
}

pop (n)
	int		n;
{
	char		*p;
	int		i;

	if (n <= 0) return;
	for (p = cwd + strlen (cwd) - 1; p >= cwd; --p) {
		if (*p == '/' || p == cwd) {
			for (i = 0; i < levels; ++i) putchar ('\t');
			putchar ('$'); putchar ('\n');
			--levels;
			if (--n <= 0) break;
		}
	}
	*p = '\0';
}

char *
rpath (name)
	char		*name;
{
	static char	buff [1024];

	strcpy (buff, rbase);
	if (buff [0] && buff [strlen (buff) - 1] != '/') strcat (buff, "/");
	strcat (buff, name);
	return (buff);
}

char *
spath (name)
	char		*name;
{
	static char	buff [1024];

	strcpy (buff, sbase);
	if (buff [0] && buff [strlen (buff) - 1] != '/') strcat (buff, "/");
	strcat (buff, name);
	return (buff);
}
