#include <stdio.h>
#include "inst.h"
#include "idb.h"

#define		Maxpat		512

#ifdef R2300
#define		SAbsize		16384
#define		MRbsize		4096
#define		PRbsize		4096
#define		IMbsize		4096
#else
#define		SAbsize		16384
#define		MRbsize		327680
#define		PRbsize		16384
#define		IMbsize		327680
#endif

typedef long	Cmp;

char		*malloc ();

Spec		rspec;
Spec		*spec = &rspec;

char		source [Strsize];
char		sname [Strsize];
char		sdir [Strsize];
int		stype;

char		dest [Strsize];
int		dtype;

int		retension = 0;
int		silent = 0;
int		verbose = 0;
int		nopt = 0;
int		compare = 0;

int		npat = 1;
char		*pat [Maxpat] = { "*", };

main (argc, argv)
	int		argc;
	char		*argv [];
{
	extern int	optind;
	extern char	*optarg;
	int		c;

	while ((c = getopt (argc, argv, "snvcr")) != EOF) {
		switch (c) {
		case 's': ++silent; break;
		case 'n': ++nopt; break;
		case 'v': ++verbose; break;
		case 'c': ++compare; break;
		case 'r': ++retension; break;
		default: usage (); exit (1);
		}
	}
	if (argc - optind < 2) usage ();
	strcpy (source, argv [optind++]);
	strcpy (dest, argv [optind++]);
	if (optind < argc) {
		npat = 0;
		while (optind < argc) {
			if (npat >= Maxpat) {
				fprintf (stderr, "Ignoring some selectors\n");
				break;
			}
			pat [npat++] = argv [optind++];
		}
	}
	distcp ();
}

usage()
{
	fprintf (stderr, "usage: distcp from to [ products ]\n");
	exit (1);
}

distcp ()
{
	Prod		*pr;

	if (getspec (source, sname, sdir, &stype, spec, NULL) < 0) {
		fprintf (stderr, "%s: can't read product specifications.\n");
		exit (1);
	}
	if ((dtype = typeof (dest)) < 0) {
		perror (dest);
		fprintf (stderr, "%s: invalid destination\n", dest);
		exit (1);
	}
	if (dtype == Is_file) {
		fprintf (stderr, "%s: destination may not be a file.\n", dest);
		exit (1);
	} else if (dtype == Is_tape) {
		if (retension) {
			if (verbose) printf ("Retensioning.\n");
			vretension (dest);
		} else {
			if (verbose) printf ("Rewinding.\n");
			vrewind (dest);
		}
	}
	if (nopt) {
		if (dtype == Is_tape) {
			fprintf (stderr,
			    "warning: -n results in unusable tapes\n");
		}
	} else {
		if (verbose) printf ("sa (Standalone Tools)\n");
		copy ("sa", "", 0, SAbsize);
		if (verbose) printf ("mr (Miniroot)\n");
		copy ("mr", "", 1, MRbsize);
	}
	if (stype == Is_dir) {
		for (pr = spec->prod; pr < spec->prodend; ++pr) {
			if (want (pr)) {
				copyprod (pr);
			}
		}
	} else if (stype == Is_file || stype == Is_tape) {
		if (want (spec->prod)) {
			copyprod (spec->prod);
		}
	}
	if (dtype == Is_tape) {
		if (verbose) printf ("Rewinding.\n");
		vrewind (dest);
	}
	if (compare && !silent) {
		fprintf (stderr, "Compare succeeds.\n");
	}
}

copyprod (pr)
	Prod		*pr;
{
	Image		*im;

	if (verbose) printf ("%s (%s)\n", pr->name, pr->id);
	copy (pr->name, "", ProdFileno, PRbsize);
	for (im = pr->image; im < pr->imgend; ++im) {
		if (verbose)
			printf ("%s.%s (%s)\n", pr->name, im->name, im->id);
		copy (pr->name, im->name, im - pr->image, IMbsize);
	}
}

copy (base, suffix, filnum, bsize)
	char		*base;
	char		*suffix;
	int		filnum;
	int		bsize;
{
	char		spath [Strsize], dpath [Strsize], *buff, *cmpbuff;
	int		n, s, d;
	register Cmp	*p1, *p2;
	register int	cmpn;

	if ((buff = malloc (bsize)) == NULL) {
		fprintf (stderr, "Can't malloc buffer (%d)\n", bsize);
		exit (1);
	}
	if (compare) {
		if ((cmpbuff = malloc (bsize)) == NULL) {
			fprintf (stderr, "Can't malloc compare buffer (%d)\n",
				bsize);
			exit (1);
		}
	}
	makepath (spath, sname, sdir, stype, base, suffix);
	makepath (dpath, dest, dest, dtype, base, suffix);
	vfilnum (spath, filnum);
	if ((s = vopen (spath, 0)) < 0) {
		perror (spath);
		exit (1);
	}
	vdirect (s);
	if ((d = vopen (dpath, compare ? 0 : 1)) < 0) {
		perror (dpath);
		vclose (s);
		exit (1);
	}
	vdirect (d);
	while ((n = vread (s, buff, bsize)) > 0) {
		while (n < bsize) buff [n++] = '\0';
		if (compare) {
			if ((cmpn = vread (d, cmpbuff, n)) < 0) {
				perror (dest);
				break;
			} else if (cmpn < n) {
				fprintf (stderr, "end of file on %s\n", dpath);
			}
			if (bcmp (buff, cmpbuff, cmpn)) {
				if (!silent) fprintf (stderr,
					"%s: compare fails.\n", dpath);
				exit (1);
			}
		} else {
			if (vwrite (d, buff, n) != n) {
				perror (dest);
				break;
			}
		}
	}
	if (n < 0) {
		perror (spath);
	}
	vclose (d);
	vclose (s);
	free (buff);
}

makepath (path, name, dir, type, base, suffix)
	char		*path;
	char		*name;
	char		*dir;
	int		type;
	char		*base;
	char		*suffix;
{
	switch (type) {
	case Is_file:
	case Is_dir:
		strcpy (path, dir);
		if (*path != '\0') {
			strcat (path, "/");
		}
		strcat (path, base);
		if (*suffix != '\0') {
			strcat (path, ".");
			strcat (path, suffix);
		}
		break;
	case Is_tape:
		strcpy (path, name);
		break;
	default:
		fprintf (stderr, "unknown type for %s\n", name);
		exit (1);
	}
}

want (pr)
	Prod		*pr;
{
	int		i;

	for (i = 0; i < npat; ++i) {
		if (filematch (pr->name, pat [i])) return (1);
	}
	return (0);
}
