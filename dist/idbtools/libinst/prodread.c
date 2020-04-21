#include "inst.h"
#include "idb.h"

extern char	*getstr ();
extern short	getshort ();
extern long	getlong ();

static int	format;

static int
readmarks (f, mkp, mkendp, mset)
	int		f;
	Mark		**mkp;
	Mark		**mkendp;
	Memset		*mset;
{
	int		n, i;
	Mark		*mk;

	if ((n = getshort (f)) < 0) return (-1);
	if (n == 0) {
		*mkp = *mkendp = NULL;
		return (0);
	}
	*mkp = (Mark *) idb_getmem (n * sizeof (Mark), mset);
	*mkendp = *mkp + n;
	for (mk = *mkp; mk < *mkendp; ++mk) {
		if ((mk->pname = getstr (f, mset)) == NULL ||
		    (mk->iname = getstr (f, mset)) == NULL ||
		    (mk->sname = getstr (f, mset)) == NULL ||
		    (mk->lowvers = getlong (f)) < 0 ||
		    (mk->highvers = getlong (f)) < 0) return (-1);
	}
	return (n);
}

static int
readoldprereqs (f, ss, mset)
	int		f;
	Subsys		*ss;
	Memset		*mset;
{
	Mark		*mk;
	int		n, i;

	if ((n = getshort (f)) < 0) return (-1);
	ss->prereq = (Prereq *) idb_getmem (sizeof (Prereq), mset);
	ss->preend = ss->prereq + 1;
	ss->prereq->pq = (Mark *) idb_getmem (n * sizeof (Mark), mset);
	ss->prereq->pqend = ss->prereq->pq + n;
	for (mk = ss->prereq->pq; mk < ss->prereq->pqend; ++mk) {
		if ((mk->pname = getstr (f, mset)) == NULL ||
		    (mk->iname = getstr (f, mset)) == NULL ||
		    (mk->sname = getstr (f, mset)) == NULL ||
		    (mk->lowvers = getlong (f)) < 0) return (-1);
		mk->highvers = maxint (long);
	}
}

static int
readprereqs (f, ss, mset)
	int		f;
	Subsys		*ss;
	Memset		*mset;
{
	int		n;
	Prereq		*pr;

	if ((n = getshort (f)) < 0) return (-1);
	if (n == 0) {
		ss->prereq = ss->preend = NULL;
		return (0);
	}
	ss->prereq = (Prereq *) idb_getmem (n * sizeof (Prereq), mset);
	ss->preend = ss->prereq + n;
	for (pr = ss->prereq; pr < ss->preend; ++pr) {
		if (readmarks (f, &pr->pq, &pr->pqend, mset) < 0) return (-1);
	}
	return (n);
}

static int
readsubsyss (f, pr, im, mset)
	int		f;
	Prod		*pr;
	Image		*im;
	Memset		*mset;
{
	int		n;
	Subsys		*ss;

	if ((n = getshort (f)) < 0) return (-1);
	if (n == 0) {
		im->subsys = im->subend = NULL;
		return (0);
	}
	im->subsys = (Subsys *) idb_getmem (n * sizeof (Subsys), mset);
	im->subend = im->subsys + n;
	for (ss = im->subsys; ss < im->subend; ++ss) {
		if ((ss->flags = getshort (f)) < 0) return (-1);
		if ((ss->name = getstr (f, mset)) == NULL) return (-1);
		if ((ss->id = getstr (f, mset)) == NULL) return (-1);
		if ((ss->exp = getstr (f, mset)) == NULL) return (-1);
		if (format >= 2) {
			if ((ss->instdate = getlong (f)) < 0) return (-1);
			if (readmarks (f, &ss->rep, &ss->repend, mset) < 0 ||
			    readprereqs (f, ss, mset) < 0) return (-1);
		} else {
			ss->instdate = im->length;
			ss->rep = (Mark *) idb_getmem (sizeof (Mark), mset);
			ss->repend = ss->rep + 1;
			ss->rep->pname = pr->name;
			ss->rep->iname = im->name;
			ss->rep->sname = ss->name;
			ss->rep->lowvers = 0;
			ss->rep->highvers = im->version - 1;
			if (readoldprereqs (f, ss, mset) < 0) return (-1);
		}
	}
	return (n);
}

static int
readimages (f, pr, mset)
	int		f;
	Prod		*pr;
	Memset		*mset;
{
	int		n;
	Image		*im;

	if ((n = getshort (f)) < 0) return (-1);
	if (n == 0) {
		pr->image = pr->imgend = NULL;
		return (0);
	}
	pr->image = (Image *) idb_getmem (n * sizeof (Image), mset);
	pr->imgend = pr->image + n;
	for (im = pr->image; im < pr->imgend; ++im) {
		if ((im->flags = getshort (f)) < 0) return (-1);
		if ((im->name = getstr (f, mset)) == NULL) return (-1);
		if ((im->id = getstr (f, mset)) == NULL) return (-1);
		if ((im->format = getshort (f)) < 0) return (-1);
		if (format >= 3) {
			if ((im->order = getshort (f)) < 0) return (-1);
		} else {
			if (im->flags & Main) im->order = 1;
			else im->order = 0;
		}
		if ((im->version = getlong (f)) < 0) return (-1);
		if ((im->length = getlong (f)) < 0) return (-1);
		if ((im->padsize = getlong (f)) < 0) return (-1);
		if (readsubsyss (f, pr, im, mset) < 0) return (-1);
	}
	return (n);
}

int
readprod (f, pr, mset)
	int		f;
	Prod		*pr;
	Memset		*mset;
{
	int		i;

	if ((i = getshort (f)) < 0) return (-1);
	if (i != Magic_prod) {
		return (-1);
	}
	if ((format = getshort (f)) < 0) return (-1);
	if (format > Format_prod) {
		fprintf (stderr, "Unrecognized product structure\n");
		return (-1);
	}
	if ((pr->name = getstr (f, mset)) == NULL) return (-1);
	if ((pr->id = getstr (f, mset)) == NULL) return (-1);
	if ((pr->flags = getshort (f)) < 0) return (-1);
	if (readimages (f, pr, mset) < 0) return (-1);
	return (0);
}
