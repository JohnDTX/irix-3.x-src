#include "inst.h"
#include "idb.h"

#define Granule		8

extern char *sbrk();

/* select a new source, load the spec(s) */

getspec (whence, source, sourcedir, sourcetype, spec, mset)
	char		*whence;
	char		*source;
	char		*sourcedir;
	int		*sourcetype;
	Spec		*spec;
	Memset		*mset;
{
	int		t, dir, n, size;
	char		*p, buff [Strsize];
	Memset		*tset;

	spec->prod = spec->prodend = NULL;
	switch (t = typeof (whence)) {
	case Is_dir:
		if ((dir = vopendir (whence)) < 0) {
			perror (whence);
			idb_freeset (mset);
			spec = NULL;
			break;
		}
		tset = idb_newset ();
		n = 0; size = 0;
		while ((p = vreaddir (dir, tset)) != NULL) {
			if (filematch (p, "*.*"))
				continue;
			sprintf (buff, "%s/%s", whence, p);
			if (n >= size) {
				size += Granule;
				spec->prod = (Prod *) idb_getmore (spec->prod,
					size * sizeof (Prod), mset);
			}
			if (openprod (buff, spec->prod + n, mset) >= 0)
				++n;
		}
#ifdef notdef
		spec->prod = (Prod *) idb_getmore (spec->prod,
			n * sizeof (Prod), mset);	/* == idb_getless () */
#endif
		spec->prodend = spec->prod + n;
		vclosedir (dir);
		idb_dispose (tset);
		if (n == 0) {
			fprintf (stderr, "%s: no product descriptors found\n",
				whence);
			idb_freeset (mset);
			spec = NULL;
		}
		break;
	case Is_tape:
	case Is_file:
		spec->prod = spec->prodend =
			(Prod *) idb_getmem (sizeof (Prod), mset);
		if (openprod (whence, spec->prodend++, mset) < 0) {
			fprintf (stderr,
			    "Can't read product descriptor from %s\n", whence);
			idb_freeset (mset);
			spec = NULL;
		}
		break;
	default:
		fprintf (stderr, "%s: invalid source\n", whence);
		idb_freeset (mset);
		spec = NULL;
	}
	if (spec == NULL) return (-1);
	if (source != NULL) abspath (whence, source);
	if (sourcedir != NULL) {
		strcpy (sourcedir, whence);
		if (t != Is_dir) {
			p = sourcedir + strlen (sourcedir);
			while (--p >= sourcedir && *p != '/') ;
			if (p < sourcedir) strcpy (sourcedir, ".");
			else if (p == sourcedir) *++p = '\0';
			else *p = '\0';
		}
	}
	if (sourcetype != NULL) *sourcetype = t;
	return (0);
}

/* read a new spec */

openprod (fname, pr, mset)
	char		*fname;
	Prod		*pr;
	Memset		*mset;
{
	int		f;

	vrewind (fname);
	if (vfilnum (fname, ProdFileno) < 0) {
		fprintf (stderr, "%s: can't position tape\n", fname);
		return (-1);
	}
	if ((f = vopen (fname, 0)) < 0) {
		perror (fname); return (-1);
	}
	if (readprod (f, pr, mset) < 0) {
		vclose (f);
		return (-1);
	}
	vclose (f);
	return (0);
}
