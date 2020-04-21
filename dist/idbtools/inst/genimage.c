/* generate an image, given a spec file and a compiled source tree.
 *
 * genimage [-r rbase] [-s sbase] [-i idb] specfile images...
 *
 * specfile is the textual representation of the product files related to the
 * images in question.  Image names are given as "<product>.<name>", and are
 * written into files with names of the same form.
 */

#include "idb.h"
#include "inst.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <filehdr.h>

Spec		*parsespec ();
long		time ();

char		*interest [] = {
			"config",
			"symval",
			"maj",
			"min",
			"links",
			"mach",
			"preop",
			"postop",
			"size",
			"sum"
		};
int		ninterest = sizeof (interest) / sizeof (*interest);

int		aopt;
int		dopt;
int		vopt;

char		obase [1024] = ".";

main (argc, argv)
	int		argc;
	char		*argv [];
{
	extern char	*optarg;
	extern int	optind;
	int		c;
	Spec		*spec;

	while ((c = getopt (argc, argv, "r:s:i:o:adv")) != EOF) {
		switch (c) {
		case 'r': strcpy (rbase, optarg); break;
		case 's': strcpy (sbase, optarg); break;
		case 'i': strcpy (idb, optarg); break;
		case 'o': strcpy (obase, optarg); break;
		case 'a': ++aopt; break;
		case 'd': ++dopt; break;
		case 'v': ++vopt; break;
		}
	}
	if (optind >= argc) usage ();
	idb_setbase ();
	if ((spec = parsespec (argv [optind], NULL)) == NULL) {
		fprintf (stderr, "%s: can't parse spec\n", argv [optind]);
		exit (1);
	}
	if (aopt) genall (spec);
	else for (++optind; optind < argc; ++optind) {
		genimages (spec, argv [optind]);
	}
	exit (0);
}

usage ()
{
	fprintf (stderr, "usage: genimage [idbopts] spec ");
	fprintf (stderr, "<prodname>.<imagename>...\n");
	exit (1);
}

genall (spec)
	Spec		*spec;
{
	Prod		*pr;
	Image		*im;

	for (pr = spec->prod; pr < spec->prodend; ++pr) {
		for (im = pr->image; im < pr->imgend; ++im) {
			genimage (spec, pr, im);
		}
	}
}

genimages (spec, name)
	Spec		*spec;
	char		*name;
{
	char		pname [Strsize], iname [Strsize];
	Prod		*pr;
	Image		*im;
	int		hits;

	split (name, pname, '.', iname);
	if (*pname == '\0') usage ();
	for (pr = spec->prod; pr < spec->prodend; ++pr) {
		if (strcmp (pr->name, pname) == 0) break;
	}
	if (pr >= spec->prodend) {
		fprintf (stderr, "%s: can't find product %s\n", name, pname);
		exit (1);
	}
	for (hits = 0, im = pr->image; im < pr->imgend; ++im) {
		if (*iname == '\0' || strcmp (im->name, iname) == 0) {
			++hits;
			genimage (spec, pr, im);
		}
	}
	if (hits == 0) {
		fprintf (stderr, "no image '%s' found\n", name);
	}
}

genimage (spec, pr, im)
	Spec		*spec;
	Prod		*pr;
	Image		*im;
{
	char		tmpidb [Strsize], tmpobj [Strsize], *p;
	char		idattr [Strsize], imagename [Strsize], dest [Strsize];
	Subsys		*ss;
	Node		**exp;
	Rec		*rec;
	FILE		*f, *tmp;
	Memset		*pset, *rset, *xset;
	int		nexp, i, imf, missing;

	if ((f = fopen (idb, "r")) == NULL) {
		perror (idb); exit (1);
	}
	sprintf (tmpobj, "/tmp/obj", getpid ());
	sprintf (tmpidb, "/tmp/idb%d", getpid ());
	if ((tmp = fopen (tmpidb, "w")) == NULL) {
		perror (tmpidb); exit (1);
	}
	pset = idb_newset ();
	rset = idb_newset ();
	xset = idb_newset ();
	nexp = im->subend - im->subsys;
	exp = (Node **) idb_getmem (nexp * sizeof (Node *), pset);
	for (i = 0; i < nexp; ++i) {
		ss = im->subsys + i;
		exp [i] = idb_parses (ss->exp, pset, Bool);
		if (exp [i] == NULL) {
			fprintf (stderr, "%s.%s.%s: can't parse expression\n",
				pr->name, im->name, ss->name);
		}
	}
	sprintf (imagename, "%s/%s.%s", obase, pr->name, im->name);
	if ((imf = vopen (imagename, 1)) < 0) {
		unlink (tmpidb);
		perror (imagename);
		exit (1);
	}
	if (vopt) printf ("\n%s.%s\n\n", pr->name, im->name);
	missing = 0;
	while ((rec = idb_read (f, rset)) != NULL) {
		idb_freeset (xset);
		for (i = 0; i < nexp; ++i) {
			if (idb_expr (rec, exp [i], xset)) break;
		}
		if (i >= nexp) continue;
		p = dopt ? idb_rpath (rec->dstpath) : idb_spath (rec->srcpath);
		if (rec->type == S_IFREG && access (p, 0) < 0) {
			perror (p);
			++missing;
			continue;
		}
		fixattr (rec, pr, im, im->subsys + i, rset);
		idb_write (tmp, rec);
	}
	fclose (f);
	fclose (tmp);
	if (missing) fprintf (stderr, "\n%d files missing in %s.%s\n",
		missing, pr->name, im->name);
	catfile (imf, tmpidb, -1);
	vpad (imf, im->padsize);
	if ((f = fopen (tmpidb, "r")) == NULL) {
		perror (tmpidb); exit (1);
	}
	while ((rec = idb_read (f, rset)) != NULL) {
		idb_freeset (xset);
		if (vopt) printf ("%s\n", rec->dstpath);
		if (rec->type != S_IFREG) continue;
		if (dopt) strcpy (dest, idb_rpath (rec->dstpath));
		else strcpy (dest, idb_spath (rec->srcpath));
		putstr (imf, rec->dstpath);
		catfile (imf, dest, idb_intat (rec, "size"));
	}
	fclose (f);
	unlink (tmpidb);
	vpad (imf, im->padsize);
	vclose (imf);
}

fixattr (rec, pr, im, ss, set)
	Rec		*rec;
	Prod		*pr;
	Image		*im;
	Subsys		*ss;
	Memset		*set;
{
	int		i, nattr;
	Attr		*at, *attr;
	char		*p, buff [Strsize];
	long		size, sum;

	p = dopt ? idb_rpath (rec->dstpath) : idb_spath (rec->srcpath);
	if (rec->type == S_IFREG) {
		if (xchksum (p, &size, &sum, rec) < 0) {
			fprintf (stderr, "Can't compute checksum for %s\n",
				rec->dstpath);
		}
		idb_addlong (idb_addattr (rec, "size", 0, NULL, set),
			size, set);
		idb_addlong (idb_addattr (rec, "sum", 0, NULL, set),
			sum, set);
	}
	attr = rec->attr; nattr = rec->nattr;
	rec->attr = NULL; rec->nattr = 0;
	for (at = attr; at < attr + nattr; ++at) {
		p = at->atname;
		for (i = 0; i < ninterest; ++i) {
			if (strcmp (p, interest [i]) == 0) {
				idb_copyattr (rec, at, set);
				break;
			}
		}
	}
	sprintf (buff, "%s.%s.%s", pr->name, im->name, ss->name);
	idb_addattr (rec, buff, 0, NULL, set);
}

catfile (f, name, size)
	int		f;
	char		*name;
	int		size;
{
	int		fd, nr, strip;
	char		buff [8192];
	struct stat	st;
	FILHDR		*hdr;

	if ((fd = open (name, 0)) < 0) {
		perror (name); return;
	}
	if (size < 0) {
		size = maxint (int);
		strip = 0;
	} else {
		if (fstat (fd, &st) < 0) {
			perror (name); return;
		}
		strip = (st.st_mode & 0111) &&
		    (st.st_mode & S_IFMT) == S_IFREG &&
		    size < st.st_size;
	}
	while (size && (nr = read (fd, buff, sizeof (buff))) > 0) {
		if (strip) {
			hdr = (FILHDR *) buff;
			if (nr >= sizeof (FILHDR) &&
			    (hdr->f_magic == MIPSEBMAGIC ||
			    hdr->f_magic == MIPSELMAGIC ||
			    hdr->f_magic == MIPSEBUMAGIC ||
			    hdr->f_magic == MIPSELUMAGIC) &&
			    hdr->f_symptr == size) {
				hdr->f_symptr = 0;
				hdr->f_nsyms = 0;
			}
			strip = 0;
		}
		if (nr > size) nr = size;
		if (vwrite (f, buff, nr) != nr) {
			perror ("image"); exit (1);
		}
		size -= nr;
	}
	if (nr < 0) {
		perror (name);
	}
	close (fd);
}
