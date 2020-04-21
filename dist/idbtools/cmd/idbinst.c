#include "idb.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef BSD
#include <sys/sysmacros.h>
#endif
#include <ctype.h>

#define VEND	(vline + strlen (vline))

#ifdef SVR0
#	define		doranlibs 	1
#else
#	define		doranlibs	0
#endif

extern char	*optarg;
extern int	optind;

#ifdef BSD
extern int	errno;
#endif

struct stat	st;
char		dest [1024];

int		verbose = 0;
int		nop = 0;
int		quiet = 0;
int		okremove = 0;
int		straight = 1;
int		more = 1;
int		list = 0;
char		iname [1024];
char		vline [1024];
char		selexpfile [1024] = "";

char		*selectexp = "!noship";

main (argc, argv)
	int		argc;
	char		*argv [];
{
	FILE		*f, *g;
	Memset		*xset, *pset;
	Node		*sel;
	int		t, i, uid, gid, exists, maj, min;
	Rec		*rec;
	char		line [1024], buff [1024], *p;
	Attr		*at;

	while ((t = getopt (argc, argv, "r:s:i:f:vdnqoul")) != EOF) {
		switch (t) {
		case 'r': strcpy (rbase, optarg); break;
		case 's': strcpy (sbase, optarg); break;
		case 'i': strcpy (idb, optarg); break;
		case 'f': strcpy (selexpfile, optarg); break;
		case 'v': verbose = 1; break;
		case 'n': nop = 1; break;
		case 'q': quiet = 1; break;
		case 'o': okremove = 1; break;
		case 'u': straight = 0; break;
		case 'l': list = 1; break;
		}
	}
	idb_setbase ();
	pset = idb_newset ();
	xset = idb_newset ();
	if ((g = fopen (idb, "r")) == NULL) {
		evomit (idb); exit (1);
	}
	if (*selexpfile != '\0') {
		if ((sel = idb_parsef (selexpfile, pset, Bool)) == NULL) {
			evomit (selexpfile); exit (1);
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
			vomit ("Can't parse expression %s\n", selectexp);
			exit (1);
		}
	}
	if (list) nextiname ();
	while (more && (rec = idb_read (g, xset)) != NULL) {
		if (idb_expr (rec, sel, xset) == 0) continue;
		strcpy (buff, rec->dstpath);
		if (!straight) {
			if (idb_getattr ("after", rec) != NULL) {
				strcat (buff, "@");
			} else if (idb_getattr ("config", rec) != NULL) {
				strcat (buff, "-");
			} else if (idb_getattr ("shared", rec) != NULL) {
				strcat (buff, "+");
			}
		}
		if (list) {
			while (more && (t = strcmp (iname, buff)) < 0) {
				nextiname ();
			}
			if (!more || t > 0) {
				continue;
			}
		}
		strcpy (dest, idb_rpath (buff));
		if (verbose) {
			sprintf (vline, "%s\t", rec->dstpath);
			for (i = strlen (rec->dstpath) + 8; i < 32; i += 8) {
				strcat (vline, "\t");
			}
		}
		if (exists = lstat (dest, &st) >= 0) {
			if ((st.st_mode & S_IFMT) != rec->type) {
				if ((st.st_mode & S_IFMT) == S_IFDIR) {
					fprintf (stderr,
						"%s exists as a directory\n",
						rec->dstpath);
				} else {
					if (verbose) strcat (vline, "rm ");
					if (!nop) {
						if (unlink (dest) < 0) {
							evomit (buff);
							continue;
						}
					}
				}
			}
		} 
		makelinks (rec);	/* early call */
		i = 0;
		switch (rec->type) {
		case S_IFDIR:
			if (exists) break;
			if (verbose) strcat (vline, "mkdir ");
			sprintf (buff, "mkdir %s\n", dest);
			if (!nop) system (buff);
			break;
		case S_IFBLK:
		case S_IFCHR:
			if ((at = idb_getattr ("maj", rec)) == NULL ||
			    at->argc != 1 || !isdigit (*at->argv [0])) {
				vomit ("%s: bad maj spec\n", rec->dstpath);
				continue;
			}
			maj = atoi (at->argv [0]);
			if ((at = idb_getattr ("min", rec)) == NULL ||
			    at->argc != 1 || !isdigit (*at->argv [0])) {
				vomit ("%s: bad min spec\n", rec->dstpath);
				continue;
			}
			min = atoi (at->argv [0]);
			i = makedev (maj, min);
			if (verbose) sprintf (VEND, "%d %d ", maj, min);
		case S_IFIFO:
			if (verbose) strcat (vline, "mknod ");
			if (!nop &&
			    mknod (dest, rec->type|rec->mode, i) < 0) {
				evomit (dest); continue;
			}
			break;
		case S_IFLNK:
			if ((at = idb_getattr ("symval", rec)) == NULL
			    || at->argc != 1) {
				vomit ("%s: bad symval\n", rec->dstpath);
				continue;
			}
			if (verbose) sprintf (VEND, "symlink %s ",
				at->argv [0]);
			if (!nop) symlink (at->argv [0], dest);
			break;
		default:
			if (verbose) sprintf (VEND, "<- %s ",
				rec->srcpath);
			if (!nop) copyfile (rec);
			if (doranlibs && islib (dest) &&
			    idb_getattr ("noranlib", rec) == NULL) {
				if (verbose) strcat (vline, "ranlib ");
				sprintf (buff, "ranlib %s", dest);
				if (!nop) system (buff);
			}
			break;
		}
		if (!nop && lstat (dest, &st) < 0) {
			evomit (rec->dstpath); continue;
		}
		makelinks (rec);		/* late call */
		if (rec->type == S_IFLNK) {
			if (verbose) printf ("%s\n", vline);
			continue;
		}
		uid = idb_uid (rec->user);
		gid = idb_gid (rec->group);
		if (st.st_uid != uid || st.st_gid != gid) {
			if (verbose) sprintf (VEND, "[%s %s] ", rec->user,
				rec->group);
			if (!nop && chown (dest, uid, gid) < 0)
				evomit (dest);
		}
		if (st.st_mode != (rec->type | rec->mode)) {
			if (verbose) sprintf (VEND, "%04o ", rec->mode);
			if (!nop && chmod (dest, rec->type | rec->mode) < 0)
				evomit (dest);
		}
		if (verbose) printf ("%s\n", vline);
	}
	idb_freeset (pset);
	exit (0);
}

nextiname ()
{
	if (!more || fgets (iname, sizeof (iname), stdin) == NULL) {
		more = 0;
		return;
	}
	iname [strlen (iname) - 1] = '\0';
}

islib (name)
	register char	*name;
{
	register char	*p;
	int		len;

	p = name;
	while (*p) if (*p++ == '/') name = p;
	return ((len = strlen (name)) >= 6 &&
	    strncmp (name, "lib", 3) == 0 &&
	    strncmp (name + len - 2, ".a", 2) == 0);
}

/* makelinks - create all links named for this record.  Consideration is made
 * for existing files and links; the assumption (currently) is that the known
 * name with the most number of links should be the base file to which all
 * others are linked.  Because an inode other than than the one for
 * rec->dstpath might be used, an attempt is made run through this algorithm
 * before the file is installed.  Another attempt is made afterwards, which
 * will have no affect if all links are already in place, to cover the case
 * where there was no inode to work from before the file was installed.  See
 * the main loop.
 */

makelinks (rec)
	Rec		*rec;
{
	Memset		*mset;
	Attr		*lat;
	struct stat	st;
	int		i, j, hit;
	char		base [1024];
	struct linktable {
		char		*name;
		short		nlinks;
		time_t		mtime;
		dev_t		dev;
		long		ino;
	} *tab;

	if ((lat = idb_getattr ("links", rec)) == NULL || lat->argc <= 0)
		return;
	mset = idb_newset ();
	tab = (struct linktable *)
		idb_getmem ((lat->argc + 1) * sizeof (struct linktable), mset);
	for (i = 0; i < lat->argc + 1; ++i) {
		if (i >= lat->argc) tab [i].name = rec->dstpath;
		else tab [i].name = lat->argv [i];
		if (stat (idb_rpath (tab [i].name), &st) < 0 ||
		    (st.st_mode & S_IFMT) == S_IFDIR) {
			tab [i].nlinks = 0;
			tab [i].mtime = 0;
			tab [i].dev = 0;
			tab [i].ino = 0;
		} else {
			tab [i].nlinks = st.st_nlink;
			tab [i].mtime = st.st_mtime;
			tab [i].dev = st.st_dev;
			tab [i].ino = st.st_ino;
		}
	}
	/* search for the inode with the greatest number of links, and use */
	/* it.  The search could be modified to look for the inode with the */
	/* greatest mtime, for example, as the situation warrents in the */
	/* future, depending on which inode is to get precedence.  */
	hit = lat->argc;	/* default to the dstpath */
	for (i = 0; i < lat->argc; ++i) {
		if (tab [i].nlinks > tab [hit].nlinks) hit = i;
	}
	/* If none found, give up for now */
	if (tab [hit].nlinks == 0) {
		idb_dispose (mset);
		return;
	}
	/* make links to the inode we found */
	strcpy (base, idb_rpath (tab [hit].name));
	for (i = 0; i < lat->argc + 1; ++i) {
		/* skip base and its links */
		if (i == hit ||
		    tab [i].dev == tab [hit].dev &&
		    tab [i].ino == tab [hit].ino)
			continue;
		/* discount all known links */
		for (j = 0; j < lat->argc + 1; ++j) {
			if (tab [i].dev == tab [j].dev &&
			    tab [i].ino == tab [j].ino)
				--tab [i].nlinks;
		}
		/* remaining count indicates unknown links */
		if (tab [i].nlinks > 0) {
			vomit ("warning: links to %s may be lost\n",
				tab [i].name);
		}
		/* relink to base */
		unlink (idb_rpath (tab [i].name));
		if (link (base, idb_rpath (tab [i].name)) < 0) {
			makeway (idb_rpath (tab [i].name));
			if (link (base, idb_rpath (tab [i].name)) < 0) {
				evomit (idb_rpath (tab [i].name));
				fprintf (stderr, "%s: could not link to %s\n",
					tab [hit].name, tab [i].name);
			}
		}
	}
	idb_dispose (mset);
}

/* copyfile - Copy the file at rec->srcpath into the file named by the global
 * "dest" value.  Just data - no mode, owner, group consideration.  This is
 * the basic "install".
 */

copyfile (rec)
	Rec		*rec;
{
	int		r, w, n, nw;
	char		buff [4096];

	if ((r = open (idb_spath (rec->srcpath), 0)) < 0) {
		evomit (idb_spath (rec->srcpath)); close (w); return;
	}
	w = creat (dest, rec->mode);
	if (w < 0 && errno == ENOENT) {
		makeway (dest);
		w = creat (dest, rec->mode);
	}
	if (w < 0) {
		evomit (rec->dstpath); return;
	}
	while ((n = read (r, buff, sizeof (buff))) > 0) {
		while (n) {
			if ((nw = write (w, buff, n)) < 0) {
				evomit (dest); close (r); close (w);
				return;
			}
			n -= nw;
		}
	}
	if (n < 0) evomit (rec->srcpath);
	close (r);
	close (w);
}

/* makeway - Scan the path name, making sure that all parent directories exist.
 * The tail is not touched.  Mode, owner, group are not considered.
 */

makeway (name)
	char		*name;
{
	char		*p, buff [1024];
	int		c;
	struct stat	st;

	for (p = name; *p; ++p) {
		if (p > name && *p == '/') {
			c = *p; *p = '\0';
			if (stat (name, &st) < 0) {
				sprintf (buff, "mkdir %s", name);
				if (system (buff)) return;
				else if (verbose) {
					sprintf (VEND, "(mkdir %s) ", name);
				}
			} else if ((st.st_mode & S_IFMT) != S_IFDIR) {
				break;
			}
			*p = c;
		}
	}
}

vomit (fmt, s)
	char		*fmt;
	char		*s;
{
	fflush (stdout);
	fprintf (stderr, fmt, s);
}

evomit (s)
	char		*s;
{
	int		t;

	t = errno;
	fflush (stdout);
	errno = t;
	perror (s);
}
