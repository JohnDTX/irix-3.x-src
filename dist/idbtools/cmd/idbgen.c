/* generate a raw installation database from a destination tree.
 *
 * The input consists of a list of file names on stdin (or a named file) for
 * which database lines should be generated.  These names are considered
 * to be relative to the specified root, following the usual rules (default
 * "/dest", overridden by environment "DEST", overridden by option "-r").
 *
 * The output includes the basic destination information (e.g. all defined
 * fields other than "srcpath"), the "links" attribute, the "dev" attribute,
 * and the "symval" attribute.  In addition, command line options can be
 * used to add arbitrary attributes.
 *
 * The output is unsorted because links cannot be resolved until all names
 * are discovered.
 *
 * The existing database and source tree are used to find as many srcpaths
 * as possible.  These are over-ridden and/or supplemented by "dstpath srcpath"
 * name pairs in the "-m" file.  Destination files whose source cannot be
 * found are left with source "unknown".  The input
 * names and the existing database are expected to be properly sorted on
 * dstpath.
 */

#include "idb.h"
#include <sys/types.h>
#include <sys/stat.h>
#ifndef BSD
#include <sys/sysmacros.h>
#endif

typedef struct Linkname {
	struct Linkname	*next;
	char		*name;
} Linkname;

typedef struct Link {
	int		n;
	Linkname	*first;
	struct stat	*st;
	Rec		*rec;
} Link;

Rec		*makerec ();

extern int	optind;
extern char	*optarg;

char		mapname [1024] = "";
char		mapdst [1024];
char		mapsrc [1024];
int		map = 0;
Link		link [2048];
int		nlinks = 0;
Rec		*irec;
FILE		*fidb;
FILE		*fmap;
FILE		*out;
Memset		*iset;

main (argc, argv)
	int		argc;
	char		*argv [];
{
	char		name [1024], *s;
	Memset		*rset;
	Linkname	*p, *l;
	int		t, i;
	struct stat	st;
	Rec		*r;
	Attr		*at;

	while ((t = getopt (argc, argv, "r:s:i:m:")) != EOF) {
		switch (t) {
		case 'r': strcpy (rbase, optarg); break;
		case 's': strcpy (sbase, optarg); break;
		case 'i': strcpy (idb, optarg); break;
		case 'm': strcpy (mapname, optarg); ++map; break;
		}
	}
	idb_setbase ();
	if ((fidb = fopen (idb, "r")) == NULL) {
		perror (idb); exit (1);
	}
/*
	if ((out = popen ("sort -u +4", "w")) == NULL) {
		fprintf (stderr, "idbgen: can't pipe to sort\n"); exit (1);
	}
*/
out=stdout;
	if (map) {
		if ((fmap = fopen (mapname, "r")) == NULL) {
			perror (mapname); exit (1);
		}
		nextpair ();
	}
	rset = idb_newset ();
	iset = idb_newset ();
	irec = idb_read (fidb, iset);
	while (fgets (name, sizeof (name), stdin) != NULL) {
		idb_freeset (rset);
		t = strlen (name);
		if (name [t - 1] == '\n') name [t - 1] = '\0';
		if (lstat (idb_rpath (name), &st) < 0) {
			perror (name); continue;
		} 
		r = makerec (name, &st, rset);
		if (st.st_nlink > 1 &&
		    (st.st_mode & S_IFMT) != S_IFDIR) {
			dolink (name, &st, r);
			rset = NULL;
		} else {
			idb_write (out, r);
		}
	}
	for (i = 0; i < nlinks; ++i) {
		r = link [i].rec;
		r->dstpath = link [i].first->name;
		at = idb_addattr (r, "links", 0, NULL, NULL);
		for (l = link [i].first; l != NULL; l = l->next) {
			if (strcmp (l->name, r->dstpath) != 0) {
				idb_addarg (at, l->name, NULL);
			}
		}
		idb_write (out, r);
		if (link [i].n != link [i].st->st_nlink) {
			fprintf (stderr, "*** unknown link(s) for %s\n",
				link [i].first->name);
		}
	}
	fclose (fidb);
	pclose (out);
	exit (0);
}

Rec *
makerec (name, st, rset)
	char		*name;
	struct stat	*st;
	Memset		*rset;
{
	Rec		*rec;
	int		t, u;
	
	rec = (Rec *) idb_getmem (sizeof (Rec), rset);
	rec->dstpath = idb_stash (name, rset);
	rec->type = st->st_mode & S_IFMT;
	rec->mode = st->st_mode & ~S_IFMT;
	rec->user = idb_stash (idb_uname (st->st_uid), rset);
	rec->group = idb_stash (idb_gname (st->st_gid), rset);
	if (rec->type != S_IFREG) rec->srcpath = "-";
	else {
		while (irec && (t = strcmp (irec->dstpath, name)) < 0) {
			irec = idb_read (fidb, iset);
		}
		while (map && (u = strcmp (mapdst, name)) < 0) nextpair ();
		if (map && u == 0) rec->srcpath = idb_stash (mapsrc, rset);
		else if (t == 0) rec->srcpath = idb_stash (irec->srcpath, rset);
		else rec->srcpath = "unknown";
	}
	rec->nattr = 0;
	rec->attr = NULL;
	if (rec->type == S_IFBLK || rec->type == S_IFCHR) {
		gendevat (rec, st->st_rdev, rset);
	}
	if (rec->type == S_IFLNK) {
		gensymat (rec, name, rset);
	}
	return (rec);
}

nextpair ()
{
	char		buff [1024];

	if (fgets (buff, sizeof (buff), fmap) == NULL) {
		fclose (fmap); map = 0; return;
	}
	sscanf (buff, "%s %s", mapdst, mapsrc);
}

dolink (name, st, r)
	char		*name;
	struct stat	*st;
	Rec		*r;
{
	int		i;
	Linkname	*n;

	for (i = 0; i < nlinks; ++i) {
		if (link [i].st->st_dev == st->st_dev &&
		    link [i].st->st_ino == st->st_ino) break;
	}
	if (i >= nlinks) {
		link [i].st = (struct stat *)
			idb_getmem (sizeof (struct stat), NULL);
		bcopy (st, link [i].st, sizeof (struct stat));
		link [i].first = NULL;
		link [i].n = 0;
		link [i].rec = r;
		++nlinks;
	} else {
		if (strcmp (link [i].rec->srcpath, "unknown") == 0 &&
		    strcmp (r->srcpath, "unknown") != 0) {
			link [i].rec->srcpath = r->srcpath;
		}
	}
	n = (Linkname *) idb_getmem (sizeof (struct Linkname), NULL);
	n->name = idb_stash (name, NULL);
	n->next = link [i].first;
	link [i].first = n;
	++link [i].n;
}

gendevat (rec, dev, set)
	Rec		*rec;
	int		dev;
	Memset		*set;
{
	Attr		*at;

	at = idb_addattr (rec, "maj", 0, NULL, set);
	idb_addlong (at, major (dev), set);
	at = idb_addattr (rec, "min", 0, NULL, set);
	idb_addlong (at, minor (dev), set);
}

gensymat (rec, name, set)
	Rec		*rec;
	char		*name;
	Memset		*set;
{
	Attr		*at;
	char		buff [1024];
	int		n;

	if ((n = readlink (idb_rpath (name), buff, sizeof (buff))) < 0) return;
	buff [n] = '\0';
	at = idb_addattr (rec, "symval", 0, NULL, set);
	idb_addarg (at, buff, set);
}
