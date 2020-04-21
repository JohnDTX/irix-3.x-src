/*
 *  Generate cpio images from an idb and a compiled source tree.  The cpio
 *  file is basically a sequence of headers, with each followed by data if
 *  it is a regular file or a symbolic link.  The header is primarily a copy
 *  of the stat values.  Links are resolved as the image is read in, and
 *  are recognized by device/inode numbers.  Since we are working from
 *  "unlinked" copies of the binaries sitting in the source tree, we must
 *  manufacture device/inode pairs that the reading cpio can use.  Also,
 *  since the other links are not necessarily known at the first encounter of
 *  one of the linked name, the entire idb must be scanned before anything
 *  can be output.
 *
 *  The scheme is to scan the idb, loading up all required information
 *  in a form that will be easy to use on the second pass.  One of the
 *  options is to sort the regular files on size.  In such a case, all
 *  zero-length entries (i.e. directories, devices, etc.) will appear
 *  first, followed by the files in decreasing size order (including
 *  symbolic links).  This is an attempt to reduce fragmentation.
 *
 *  For those who have just come from the cpio code, or who make reference
 *  to it while looking here, there is some dynamic byte-swapping stuff
 *  littering up the cpio code.  It would seem that if you know what machine
 *  the program is going to run on at the time it is compiled, you can build
 *  that stuff in at compile time, rather than check again everytime you
 *  access a number.  That's what we do here.
 *
 *  Of course there will be a problem if the cpio format changes in some
 *  way, and significant changes to "cpio -o" must be tracked here.
 *
 *  Here is the interface:
 *
 *	-s <sbase>	The base of the source directory; srcpaths are
 *			relative to this value.  Default is the envariable
 *			$sbase.
 *
 *	-i <idb>	The pathname of the idb.  Default is sbase/idb,
 *			over-ridden by the envariable $idb.
 *		
 *	-v		Make lots of noise while creating the image.
 *
 *	-q		Be quiet about errors such as missing files.
 *
 *	-e <ename>	The name of the image; default is "foo".
 *
 *	-S		Sort on data file size, largest first.
 *
 *	-j		Read dstpath names from stdin, to take part in a
 *			join with dstpath in the idb to select records.
 *
 *	An argument after the options is taken as an idb expression used to
 *	select the appropriate records from the idb.  
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <a.out.h>
#include <idb.h>
#include <errno.h>

#ifdef vax
#define swapshort
#endif

#ifdef swapshort
#	define part1(l)	((short) ((l >> 16) & 0177777))
#	define part0(l)	((short) (l & 0177777))
#	define mklong(v)	(((long) v[1] << 16) | (long) v[0] & 0177777)
#else
#	define part0(l)	((short) ((l >> 16) & 0177777))
#	define part1(l)	((short) (l & 0177777))
#	define mklong(v)	(((long) v[0] << 16) | (long) v[1] & 0177777)
#endif

#define MAGIC	070707

#define eflush()	(_e_=errno,fflush(stdout),errno=_e_)
int _e_;

typedef struct header {			/* Cpio header format */
	short	h_magic,
		h_dev;
	ushort	h_ino,
		h_mode,
		h_uid,
		h_gid;
	short	h_nlink,
		h_rdev,
		h_mtime[2],
		h_namesize,
		h_filesize[2];
	char	h_name[256];
} Hdr;

#define HDRSIZE		(sizeof (Hdr) - 256)

typedef struct Listelement {
	Hdr		*h;		/* Cpio header for this file */
	char		*srcpath;	/* path of binary in source tree */
	int		blocks;		/* size in blocks */
	short		flags;		/* various flags */
} Listelement;

typedef struct Link {			/* linked name: */
	struct Link	*next;		/* next linked name */
	char		*dstpath;	/* the destination (installed) path */
	char		*srcpath;	/* the source path */
	short		flags;		/* various flags */
	dev_t		dev;		/* the fake device number */
	ino_t		ino;		/* the fake inode number */
} Link;

/* flag bits for Listelements and Links: */

#define Strip		0x0001		/* should be stripped */
#define Trailer		0x0002		/* is the TRAILER record */
#define Shared		0x0004		/* file is shared "+" */
#define After		0x0008		/* file is after "@" */
#define Config		0x0010		/* file is config "-" */
#define Defined		0x0020		/* linked name has been defined */
#define Symlink		0x0040		/* is a symbolic link */

/* state of join: */

#define Active		1
#define Endoffile	2

Listelement	*getlinks ();
Link		*findlink ();
Link		*addlink ();
int		cmpsize ();
int		cmpname ();

Link		*linkbase	= NULL;
Listelement	*list		= NULL;
int		nlist		= 0;
char		jname [1024];
long		date;
dev_t		ldev		= 0;		/* false dev for links */
ino_t		lino		= 0;		/* false ino for links */
long		bsize		= 1024L * 250L;	/* blocking size */
int		join		= 0;
int		Verbose		= 0;		/* verbose on input side */
int		verbose		= 0;		/* verbose on output side */
int		nostrip		= 0;		/* don't do any stripping */
int		update		= 0;		/* add dstpath suffixes */
int		quiet		= 0;		/* don't complain */
int		sortsize	= 0;		/* sort by size */
char		distdir [1024]	= "dist";	/* name of distribution dir */
char		ename [1024]	= "foo";	/* short name of entry */

main (argc, argv)
	int		argc;
	char		*argv [];
{
	extern int	optind;
	extern char	*optarg;
	int		c, t;
	FILE		*in;
	Memset		*rset, *xset;
	Node		*expr;
	Rec		*r;

	time (&date);
	while ((c = getopt (argc, argv, "s:i:uvqje:Snb:d:V")) != EOF) {
		switch (c) {
		case 's': strcpy (rbase, optarg); break;
		case 'i': strcpy (idb, optarg); break;
		case 'u': ++update; break;
		case 'v': ++verbose; break;
		case 'q': ++quiet; break;
		case 'j': join = Active; break;
		case 'e': strcpy (ename, optarg); break;
		case 'S': ++sortsize; break;
		case 'n': ++nostrip; break;
		case 'b': bsize = atol (optarg); break;
		case 'd': strcpy (distdir, optarg); break;
		case 'V': ++Verbose; break;
		}
	}
	idb_setbase ();
	rset = idb_newset ();
	xset = idb_newset ();
	if (optind < argc) {
		expr = idb_parses (argv [optind], NULL, Bool);
	} else {
		expr = idb_parses ("1", NULL, Bool);
	}
	if (expr == NULL) exit (1);
	if ((in = fopen (idb, "r")) == NULL) { perror (idb); exit (1); }
	nextjname ();
	while ((r = idb_read (in, rset)) != NULL) {
		while (join == Active && (t = strcmp (r->dstpath, jname)) > 0) {
			if (!quiet) {
				eflush ();
				fprintf (stderr, "%s: not in idb\n", jname);
			}
			nextjname ();
		}
		if (join == Endoffile) break;
		if (join == Active) {
			if (t > 0) continue;
			nextjname ();
		}
		idb_freeset (xset);
		if (idb_expr (r, expr, xset) == 0) continue;
		addrecord (r);
	}
	fclose (in);
	while (join == Active) {
		eflush ();
		fprintf (stderr, "%s: not in idb\n", jname);
		nextjname ();
	}
	qsort (list, nlist, sizeof (*list), sortsize ? cmpsize : cmpname);
	addtrailer ();
	exit (generate () != 0);
}

/* cmpsize - non-regular files first, by name, then regular files by size
 * major, then by dev/ino for linked files, then linked files first,
 * then by name.
 */

cmpsize (p1, p2)
	Listelement	*p1;
	Listelement	*p2;
{
	int		t1, t2, c;
	long		s1, s2;

	t1 = p1->h->h_mode & S_IFMT;
	t2 = p2->h->h_mode & S_IFMT;
	if (t1 == S_IFREG) {
		if (t2 == S_IFREG) {
			c = p2->blocks / 4 - p1->blocks / 4; /* reverse */
			if (c != 0) return (c);
			if (p1->h->h_nlink > 1 && p2->h->h_nlink > 1) {
				c = p1->h->h_dev - p2->h->h_dev;
				if (c != 0) return (c);
				return (p1->h->h_ino - p2->h->h_ino);
			}
			if (p1->h->h_nlink > 1) return (1);
			if (p2->h->h_nlink > 1) return (-1);
			return (strcmp (p1->h->h_name, p2->h->h_name));
		} else {
			return (1);
		}
	} else {
		if (t2 == S_IFREG) {
			return (-1);
		} else {
			return (strcmp (p1->h->h_name, p2->h->h_name));
		}
	}
}

cmpname (p1, p2)
	Listelement	*p1;
	Listelement	*p2;
{
	return (strcmp (p1->h->h_name, p2->h->h_name));
}

nextjname ()
{
	if (join == 0 || join == Endoffile) return;
	if (scanf ("%s", jname) != 1) join = Endoffile;
}

addrecord (r)
	Rec		*r;
{
	Listelement	*p;

	if (Verbose) printf ("< %s\n", r->dstpath);
	if (nlist % 32 == 0) {
		list = (Listelement *) idb_getmore (list,
			(nlist + 32) * sizeof (Listelement), NULL);
	}
	p = list + nlist++;
	p->h = (Hdr *) idb_getmem (HDRSIZE + strlen (r->dstpath) + 2, NULL);
	p->h->h_magic = MAGIC;
	p->h->h_dev = 0;
	p->h->h_ino = 0;
	p->h->h_mode = r->type | r->mode;
	p->h->h_uid = idb_uid (r->user);
	p->h->h_gid = idb_gid (r->group);
	p->h->h_rdev = idb_intat (r, "maj") << 8 | idb_intat (r, "min");
	p->h->h_mtime [0] = part0 (date);
	p->h->h_mtime [1] = part1 (date);
	strcpy (p->h->h_name, r->dstpath);
	p->h->h_namesize = strlen (p->h->h_name) + 1;
	p->flags = 0;
	p->srcpath = idb_stash (r->srcpath, NULL);
	getsize (r, p);
	p = getlinks (r, p);
	if (idb_getattr ("after", r) != NULL) p->flags |= After;
	else if (idb_getattr ("config", r) != NULL) p->flags |= Config;
	else if (idb_getattr ("shared", r) != NULL) p->flags |= Shared;
}

addtrailer ()
{
	Listelement	*p;

	if (nlist % 32 == 0) {
		list = (Listelement *) idb_getmore (list,
			(nlist + 32) * sizeof (Listelement), NULL);
	}
	p = list + nlist++;
	p->h = (Hdr *) idb_getmem (sizeof (Hdr), NULL);
	bzero (p->h, sizeof (Hdr));
	p->h->h_magic = MAGIC;
	strcpy (p->h->h_name, "TRAILER!!!");
	p->h->h_namesize = strlen (p->h->h_name) + 1;
	p->srcpath = NULL;
	p->flags = Trailer;
}

Listelement *
getlinks (r, p)
	Rec		*r;
	Listelement	*p;
{
	Attr		*lat;
	Link		*lnbase, *ln;
	Listelement	*e;
	char		**nam;
	int		x;

	p->h->h_nlink = 1;
	if ((lat = idb_getattr ("links", r)) == NULL) return (p);
	lnbase = findlink (r->dstpath);
	for (nam = lat->argv; nam < lat->argv + lat->argc; ++nam) {
		if ((ln = findlink (*nam)) != NULL) {
			if (strcmp (r->srcpath, ln->srcpath) != 0) {
				eflush ();
				fprintf (stderr, "ambiguous links: %s %s\n",
					r->dstpath, ln->dstpath);
			}
			if (lnbase == NULL) {
				lnbase = ln;
			}
		} else {
			for (e = list; e < list + nlist; ++e) {
				if (strcmp (e->h->h_name, *nam) != 0) continue;
				e->h->h_nlink = 2;
				if (strcmp (e->srcpath, r->srcpath) != 0) {
					eflush ();
					fprintf (stderr,
						"ambiguous links: %s %s\n",
						e->h->h_name, r->dstpath);
				}
				break;
			}
			ln = addlink (*nam, r->srcpath);
		}
	}
	ln = addlink (r->dstpath, r->srcpath);
	if (lnbase == NULL) {
		lnbase = ln;
		lnbase->dev = ldev;
		lnbase->ino = lino++;
		if (lino <= 0) ++ldev, lino = 0;	/* wrap, bump dev */
	} else {
		ln->dev = lnbase->dev;
		ln->ino = lnbase->ino;
	}
	for (nam = lat->argv; nam < lat->argv + lat->argc; ++nam) {
		ln = findlink (*nam);
		ln->dev = lnbase->dev;
		ln->ino = lnbase->ino;
	}
	p->h->h_nlink = 2;
	return (p);
}

getsize (r, p)
	Rec		*r;
	Listelement	*p;
{
	int		f;
	struct stat	st;
	struct exec	head;
	Attr		*sat;

	if (r->type != S_IFREG && r->type != S_IFLNK) {
		p->h->h_filesize [0] = 0;
		p->h->h_filesize [1] = 0;
		p->blocks = 0;
		return;
	}
	if (r->type == S_IFLNK) {
		if ((sat = idb_getattr ("symval", r)) == NULL ||
		    sat->argc != 1) {
			p->srcpath = idb_stash ("undefined", NULL);
			eflush ();
			fprintf (stderr, "%s: symval undefined\n", r->dstpath);
		} else {
			p->srcpath = idb_stash (sat->argv [0], NULL);
		}
		p->flags |= Symlink;
		p->h->h_filesize [0] = part0 (strlen (p->srcpath));
		p->h->h_filesize [1] = part1 (strlen (p->srcpath));
		p->blocks = 1;
		return;
	}
	if (stat (idb_spath (r->srcpath), &st) < 0) {
		eflush ();
		perror (idb_spath (r->srcpath));
		fprintf (stderr, "Can't find srcpath object.\n");
		p->h->h_filesize [0] = 0;
		p->h->h_filesize [1] = 0;
		p->blocks = 0;
		return;
	}
	p->blocks = toblocks (st.st_size);
	p->h->h_filesize [0] = part0 (st.st_size);
	p->h->h_filesize [1] = part1 (st.st_size);
	if ((r->mode & 0111) == 0) return;
	f = -1;
	if (idb_getattr ("nostrip", r) == NULL &&
	    (f = open (idb_spath (r->srcpath), 0)) >= 0 &&
	    read (f, &head, sizeof (head)) == sizeof (head) &&
	    !N_BADMAG (head) &&
	    (head.a_syms != 0 || head.a_trsize != 0 || head.a_drsize != 0)) {
		p->h->h_filesize [0] = part0 ((long) sizeof (head) +
			head.a_text + head.a_data);
		p->h->h_filesize [1] = part1 ((long) sizeof (head) +
			head.a_text + head.a_data);
		p->flags |= Strip;
	}
	if (f != -1) close (f);
}

Link *
findlink (dstpath)
	char		*dstpath;
{
	Link		*ln;

	for (ln = linkbase; ln != NULL; ln = ln->next) {
		if (strcmp (ln->dstpath, dstpath) == 0) break;
	}
	return (ln);
}

Link *
addlink (dstpath, srcpath)
	char		*dstpath;
	char		*srcpath;
{
	Link		*ln;

	for (ln = linkbase; ln != NULL; ln = ln->next) {
		if (strcmp (ln->dstpath, dstpath) == 0) return (ln);
	}
	ln = (Link *) idb_getmem (sizeof (Link), NULL);
	bzero (ln, sizeof (ln));
	ln->dstpath = idb_stash (dstpath, NULL);
	ln->srcpath = idb_stash (srcpath, NULL);
	ln->next = linkbase; linkbase = ln;
	return (ln);
}

static Link	*scanlink;
static dev_t	scandev;
static ino_t	scanino;
static int	devino;

setlink (dstpath)
	char		*dstpath;
{
	Link		*ln;

	if (dstpath != NULL) {
		if ((ln = findlink (dstpath)) == NULL) {
			devino = 0;
		} else {
			devino = 1;
			scandev = ln->dev;
			scanino = ln->ino;
		}
	} else {
		devino = 0;
	}
	scanlink = linkbase;
}

Link *
nextlink ()
{
	Link		*ln;

	while (scanlink != NULL) {
		ln = scanlink; scanlink = scanlink->next;
		if (!devino ||
		    ln->dev == scandev && ln->ino == scanino)
			return (ln);
	}
	return (NULL);
}

generate ()
{
	int		g, strip, putsize, errs;
	struct exec	*head;
	char		buff [4096];
	long		size;
	Listelement	*p;
	unsigned	n, nr;
	Link		*ln, *tln;
	FILE		*nsizes, *listpipe;
	char		*prevname;

	mkdir (distdir, 0755);
	sprintf (buff, "%s/dist", distdir);
	mkdir (buff, 0755);
	sprintf (buff, "%s/entry.%s", distdir, ename);
	if (bopen (buff) < 0) {
		eflush ();
		perror (buff);
		fprintf (stderr, "Can't open entry for output.\n");
		exit (1);
	}
	sprintf (buff, "sort -o list.%s", ename);
	if ((listpipe = popen (buff, "w")) == NULL) {
		eflush ();
		perror (buff);
		fprintf (stderr, "Can't open pipe to sort list\n");
		exit (1);
	}
	sprintf (buff, "nsizes.%s", ename);
	if ((nsizes = fopen (buff, "w")) == NULL) {
		eflush ();
		perror (buff);
		fprintf (stderr, "Can't open nsizes for append.\n");
		exit (1);
	}
	++ldev, lino = 0;
	prevname = "";
	for (p = list; p < list + nlist; ++p) {
		putsize = update;
		if (p->h->h_nlink <= 1) {
			p->h->h_ino = lino++;
			p->h->h_dev = ldev;
			if (lino <= 0) ++ldev, lino = 0;
		} else {
			if (strcmp (prevname, p->h->h_name) == 0) 
				continue;
			if ((ln = findlink (p->h->h_name)) != NULL) {
				p->h->h_ino = ln->ino;
				p->h->h_dev = ln->dev;
				setlink (p->h->h_name);
				while ((tln = nextlink ()) != NULL) {
					if (tln->flags & Defined) {
						putsize = 0;
						break;
					}
				}
				ln->flags |= Defined;
			} else {
				eflush ();
				fprintf (stderr, "%s: undefined links\n",
					p->h->h_name);
				p->h->h_ino = lino++;
				p->h->h_dev = ldev;
				if (lino <= 0) ++ldev, lino = 0;
			}
		}
		prevname = p->h->h_name;
		if (!update) {
			;
		} else if (p->flags & After) {
			strcat (p->h->h_name, "@"); ++p->h->h_namesize;
		} else if (p->flags & Config) {
			strcat (p->h->h_name, "-"); ++p->h->h_namesize;
		} else if (p->flags & Shared) {
			strcat (p->h->h_name, "+"); ++p->h->h_namesize;
		}
		size = mklong (p->h->h_filesize);
		if (putsize && (p->h->h_mode & S_IFMT) == S_IFREG && 
		    (p->flags & Trailer) == 0) {
			fprintf (nsizes, "%5ld\t%s\n",
				p->blocks, p->h->h_name);
		}
		if (size == 0L) {
			bwrite (p->h, (long) HDRSIZE + p->h->h_namesize);
			if ((p->flags & Trailer) == 0) {
				fprintf (listpipe, "%s\n", p->h->h_name);
				if (verbose) printf ("%s\n", p->h->h_name);
			}
			continue;
		}
		if (p->flags & Symlink) {
			bwrite (p->h, (long) HDRSIZE + p->h->h_namesize);
			bwrite (p->srcpath, (long) size);
			fprintf (listpipe, "%s\n", p->h->h_name);
			if (verbose) printf ("%s\n", p->h->h_name);
			continue;
		}
		if ((g = open (idb_spath (p->srcpath), 0)) < 0) {
			eflush ();
			perror (idb_spath (p->srcpath));
			fprintf (stderr, "Can't open srcpath object.\n");
			continue;
		}
		bwrite (p->h, (long) HDRSIZE + p->h->h_namesize);
		if (nostrip) strip = 0;
		else strip = p->flags & Strip;
		while (size > 0) {
			if ((n = sizeof (buff)) > size) n = size;
			if ((nr = read (g, buff, n)) != n) {
				eflush ();
				perror (idb_spath (p->srcpath));
				fprintf (stderr,
					"Can't read from srcpath object\n");
				break;
			}
			size -= n;
			if (strip && n > sizeof (struct exec)) {
				head = (struct exec *) buff;
				head->a_syms = 0;
				head->a_trsize = 0;
				head->a_drsize = 0;
				strip = 0;
			}
			bwrite (buff, (long) n);
		}
		close (g);
		fprintf (listpipe, "%s\n", p->h->h_name);
		if (verbose) printf ("%s\n", p->h->h_name);
	}
	bclose ();
	pclose (listpipe);
	fclose (nsizes);
	setlink (NULL);
	errs = 0;
	while ((ln = nextlink ()) != NULL) {
		if ((ln->flags & Defined) == 0) {
			eflush ();
			fprintf (stderr, "%s: undefined link\n", ln->dstpath);
			++errs;
		}
	}
	return (errs);
}

static char	bname [1024];
static char	*bbuf = NULL;
static long	bcnt;
static int	bfile;

bopen (name)
	char		*name;
{
	if ((bfile = creat (name, 0644)) < 0) {
		perror (name); exit (-1);
	}
	if (bbuf == NULL) bbuf = idb_getmem (bsize, NULL);
	strcpy (bname, name);
	bcnt = 0;
}

bwrite (buff, len)
	char		*buff;
	long		len;
{
	long		n;

	if (len & 1) ++len;
	while (len) {
		n = bsize - bcnt;
		if (n > len) n = len;
		bcopy (buff, bbuf + bcnt, n);
		bcnt += n;
		buff += n;
		len -= n;
		if (bcnt >= bsize) {
			if (write (bfile, bbuf, bsize) != bsize) {
				perror (bname);
				exit (1);
			}
			bcnt = 0;
		}
	}
}

bclose ()
{
	if (bcnt) {
		if (bsize - bcnt) {
			bzero (bbuf + bcnt, bsize - bcnt);
		}
		if (write (bfile, bbuf, bsize) != bsize) {
			perror (bname);
			exit (1);
		}
	}
	close (bfile);
	bfile = -1;
}
