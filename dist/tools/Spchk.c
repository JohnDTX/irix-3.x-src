/* spchk -- check space requirements before an update installation
 *
 * spchk [ -r root ] [ -n nsizes ] [ -vgqti ] [ names ... ]
 *
 * The -r specifies an alternate root directory for the update destination;
 * the default is /.  The "name" arguments are entry names to be processed.
 * Each entry "name" is transformed to "list.<name>", and a file read by that
 * name is expected to exist in /dist, containing the names of the files in
 * that tape entry, one per line.  The default entries are taken from the
 * "/dist/toc" file.  The -v option causes Spchk to be verbose about where
 * the space changes are.  The -g option causes Spchk to generate an "nsizes"
 * list for the given names.  A name of "-" causes the list of names to be
 * read from stdin.  The "-q" option causes errors about missing files to be
 * suppressed during list generation ("-g" option).  The "-n nsizes" option
 * specifies an alternate nsizes file, which by default is "dist/nsizes"
 * (under root).  The "t" option causes the program to do some "df" style
 * checking for available blocks, and silently return an exit status of 0
 * for success and 1 for failure, based on the available blocks being at
 * least 10% greater than the space required.
 */

#include 	<stdio.h>
#include	<sys/param.h>
#include 	<sys/stat.h>
#include	<sys/fs.h>
#include	<ustat.h>

#define		SECTORSHIFT	9
#define		SECTORSIZE	(1<<SECTORSHIFT)

#define		MAXFILES	16
#define		MAXLINKS	2048
#define		BUFFSIZE	256
#define		EMPTY		1
#define		OK		2
#define		DIRECT		10

#define		ABS(n)		((n) < 0 ? -(n) : (n))
#define		SGN(n)		((n) < 0 ? '-' : '+')

typedef struct {
	FILE		*fdesc;
	int		state;
	char		buff [BUFFSIZE];
} Ifile;

typedef struct {
	dev_t		dev;
	long		ino;
} Link;

char		*root		= "/";
char		*nsizes		= "dist/nsizes";
int		usize		= 0;
int		rsize		= 0;
int		utot		= 0;
int		rtot		= 0;
Ifile		ifile [MAXFILES];
int		nfiles		= 0;
Link		link [MAXLINKS];
int		nlinks		= 0;
int		verbose		= 0;
int		gen		= 0;
int		quiet		= 0;
int		unified		= 0;
int		testonly	= 0;
int		ignlinks	= 0;
struct stat	st;

main (argc, argv)
	int		argc;
	char		*argv [];
{
	FILE		*f;
	char		line [1024], *p, *r;
	int		a, c;

	for (a = 1; a < argc && *argv [a] == '-'; ++a) {
		if (*(p = argv [a] + 1) == '\0') break;
		while (*p) {
			switch (*p++) {
			case 'v':	verbose = 1;
					break;
			case 'g':	gen = 1;
					break;
			case 'q':	quiet = 1;
					break;
			case 'u':	unified = 1;
					break;
			case 'i':	ignlinks = 1;
					break;
			case 'n':	if (*(nsizes = p) == '\0') {
						if (a < argc - 1)
							nsizes = argv [++a];
						else usage ();
					}
					p = nsizes + strlen (nsizes);
					break;
			case 'r':
					if (*(root = p) == '\0') {
						if (a < argc - 1)
							root = argv [++a];
						else usage ();
					}
					p = root + strlen (root);
					break;
			case 't':	testonly = 1;
					break;
			default:	usage ();
			}
		}
	}
	if (chdir (root) == -1) { perror (root); exit (1); }
	/* gain some space... these are only used by standalone installs */
	unlink ("dist/ipfex");
	unlink ("dist/mdfex");
	unlink ("dist/sifex");
	/* fill in the default list if none given */
	if (a >= argc) {
		if ((f = fopen ("dist/toc", "r")) == NULL) {
			fprintf (stderr, "no dist/toc\n"); exit (1);
		}
		while (fgets (line, sizeof (line), f) != NULL) {
			zap (line);
			openup (line);
		}
	}
	else while (a < argc) openup (argv [a++]);
	if (gen) genlist ();
	else {
		if ((f = fopen (nsizes, "r")) == NULL) {
			perror (nsizes); exit (1);
		}
		while (fgets (line, sizeof (line), f) != NULL) {
			add (line);
		}
		fclose (f);
		if (testonly) {
			if (unified) {
				if ((rsize + usize) * 1.1 > avail (root))
					exit (1);
			} else {
				if (rsize * 1.1 > avail (root)) exit (1);
				strcpy (line, root);
				strcat (line, "/usr");
				if (usize * 1.1 <= avail (line)) exit (1);
			}
			exit (0);
		} else {
			if (unified) {
				printf ("%d\n", rsize + usize);
			} else {
				printf ("/	%s%d blocks\n",
					rsize < 0 ? "" : "+", rsize);
				printf ("/usr	%s%d blocks\n",
					usize < 0 ? "" : "+", usize);
			}
		}
	}
	exit (0);
}

usage ()
{
	fprintf (stderr, "usage: Spchk [ -uvgq ] [ -r root ] [ -n nsizes ] [ names ... ]\n");
	exit (1);
}

add (line)
	register char	*line;
{
	register int	c;
	register char	*p;
	register long	n;
	int		usr, dup, i;
	long		osize;
	char		buff [64];

	while ((c = *line) == ' ' || c == '\t' || c == '\n') ++line;
	for (n = 0; c >= '0' && c <= '9'; c = *++line) {
		n = n * 10 + c - '0';
	}
	while ((c = *line) == ' ' || c == '\t' || c == '\n') ++line;
	usr = line [0] == 'u' && line [1] == 's' && line [2] == 'r'
		&& line [3] == '/';
	zap (line);
	if (!include (line))
		return;
	if (usr) usize += n;
	else rsize += n;
	osize = 0;
	dup = 0;
	if (nstat (line, &st) != -1) {
		if (st.st_nlink > 1 && (st.st_mode & S_IFMT) != S_IFDIR) {
			for (i = 0; i < nlinks && !dup; ++i) {
				if (st.st_dev == link [i].dev &&
				    st.st_ino == link [i].ino) dup = 1;
			}
			if (!dup && nlinks < MAXLINKS) {
				link [nlinks].dev = st.st_dev;
				link [nlinks].ino = st.st_ino;
				++nlinks;
			}
		}
		if (!dup) {
			osize = blocks (st.st_size);
			if (usr) usize -= osize;
			else rsize -= osize;
		}
	}
	if (verbose && !dup && n - osize) {
		printf ("%7ld %c %s\n", ABS (n-osize), SGN (n-osize), line);
	}
}

genlist ()
{
	Ifile	*e;
	FILE	*f;
	int	i, dup;
	char	line [1024];

	for (e = ifile; e < ifile + nfiles; ++e) {
		f = e->fdesc;
		while (fgets (line, sizeof (line), f) != NULL) {
			zap (line);
			if (nstat (line, &st) == -1) {
				if (!quiet) perror (line);
				continue;
			}
			dup = 0;
			if (!ignlinks && st.st_nlink > 1 &&
			    (st.st_mode & S_IFMT) != S_IFDIR) {
				for (i = 0; i < nlinks && !dup; ++i) {
					if (st.st_dev == link [i].dev &&
					    st.st_ino == link [i].ino) dup = 1;
				}
				if (!dup && nlinks < MAXLINKS) {
					link [nlinks].dev = st.st_dev;
					link [nlinks].ino = st.st_ino;
					++nlinks;
				}
			}
			if (!ignlinks && !dup) {
				printf ("%5ld\t%s\n", blocks (st.st_size), line);
			}
			if (ignlinks && !dup) {
				printf ("%7ld\t%s\n", st.st_size, line);
			}
		}
	}
}

long
blocks (n)
	long		n;
{
	register long	t, tot;

	t = tot = (n + BSIZE - 1) / BSIZE;
	t /= 2;
	if (t > DIRECT) {
		tot += ((t - DIRECT - 1) >> NSHIFT) + 1;
		if (t > DIRECT + NINDIR) {
			tot += ((t - DIRECT - NINDIR - 1) >> (NSHIFT * 2)) + 1;
			if (t > DIRECT + NINDIR + NINDIR * NINDIR) {
				tot++;
			}
		}
	}
	return (tot);
}

openup (name)
	char		*name;
{
	char		buff [1024];
	char		buff2 [1024];

	if (nfiles >= MAXFILES) {
		fprintf (stderr, "too many lists; ignoring %s\n", name);
		return;
	}
	if (strcmp (name, "-") == 0) {
		ifile [nfiles].fdesc = stdin;
	} else {
		strcpy (buff, "dist/list."); strcat (buff, name);
		if ((ifile [nfiles].fdesc = fopen (buff, "r")) == NULL) {
		    /* Look for dist/list.foo% */
		    strcpy(buff2, buff);
		    strcat (buff2, "%");
		    if ((ifile [nfiles].fdesc = fopen (buff2, "r")) == NULL) {
			perror (buff); perror (buff2); exit (1);
		    }
		}
	}
	ifile [nfiles].state = EMPTY;
	++nfiles;
}

include (name)
	char		*name;
{
	FILE		*f;
	register Ifile	*e;
	register char	*p;
	register int	c;

	for (e = ifile; e < ifile + nfiles; ++e) {
		do {
			if (e->state == EMPTY) {
				if ((fgets (e->buff, BUFFSIZE, e->fdesc)) ==
				    NULL) {
					e->state = EOF;
				} else e->state = OK;
				zap (e->buff);
			}
			if (e->state == EOF) break;
			c = strcmp (name, e->buff);
			if (c == 0) { e->state = EMPTY; return (1); }
			else if (c > 0) e->state = EMPTY;
		} while (e->state == EMPTY);
	}
	return (0);
}

zap(s)
	register char	*s;
{
	register int	c;

	while ((c = *s) && c != ' ' && c != '\t' && c != '\n') ++s;
	*s = '\0';
}

avail (name)
	char		*name;
{
	struct stat	st;
	struct ustat	ust;
	char		sb [SECTORSIZE];
	int		f, magic, fsbshift;

	nstat (name, &st);
	ustat (st.st_rdev, &ust);
	f = open (name, 0);
	lseek (f, SUPERBOFF, 0);
	read (f, sb, SECTORSIZE);
	close (f);
	if (((struct filsys *)sb)->s_magic == FsMAGIC
	    && ((struct filsys *)sb)->s_type == Fs1b) {
		fsbshift = SECTORSHIFT;
	} else if (((struct filsys *)sb)->s_magic == FsMAGIC
	    && ((struct filsys *)sb)->s_type == Fs2b) {
		fsbshift = SECTORSHIFT+1;
	} else if (((struct efs *)sb)->fs_magic == EFS_MAGIC) {
		fsbshift = BBSHIFT;
	} else {
		fprintf (stderr, "bad.\n");
		exit (1);
	}
	return ((ust.f_tfree >> (fsbshift - SECTORSHIFT)) << fsbshift >> 10);
}
