#include "inst.h"
#include "idb.h"

extern char	*getstr ();
extern short	getshort ();
extern long	getlong ();

static int
cmpmk (m1, m2)
	Mark		*m1;
	Mark		*m2;
{
	int		c;

	if ((c = strcmp (m1->pname, m2->pname)) == 0 &&
	    (c = strcmp (m1->iname, m2->iname)) == 0 &&
	    (c = strcmp (m1->sname, m2->sname)) == 0 &&
	    (c = m2->lowvers - m1->lowvers) == 0) {
		c = m2->highvers - m1->highvers;
	}
	return (c);
}

static int
cmpss (s1, s2)
	Subsys		*s1;
	Subsys		*s2;
{
	int		c;

	if ((c = strcmp (s1->name, s2->name)) == 0)
		c = s2->instdate - s1->instdate;
	return (c);
}

static int
cmpim (i1, i2)
	Image		*i1;
	Image		*i2;
{
	int		c;

	if ((c = strcmp (i1->name, i2->name)) == 0)
		c = i2->version - i1->version;
	return (c);
}

static int
writemarks (f, mk, mkend)
	int		f;
	Mark		*mk;
	Mark		*mkend;
{
	int		n;

	if ((n = mkend - mk) < 0 || putshort (f, n) < 0)
		return (-1);
	qsort (mk, n, sizeof (*mk), cmpmk);
	while (mk < mkend) {
		if (putstr (f, mk->pname) < 0 ||
		    putstr (f, mk->iname) < 0 ||
		    putstr (f, mk->sname) < 0 ||
		    putlong (f, mk->lowvers) < 0 ||
		    putlong (f, mk->highvers) < 0) return (-1);
		++mk;
	}
	return (0);
}

static int
writeprereqs (f, subsys)
	int		f;
	Subsys		*subsys;
{
	Prereq		*pr, *preend;
	int		n;

	if (subsys == NULL) return (-1);
	if ((n = subsys->preend - subsys->prereq) < 0 ||
	    putshort (f, n) < 0) return (-1);
	for (pr = subsys->prereq; pr < subsys->preend; ++pr) {
		if (writemarks (f, pr->pq, pr->pqend) < 0) return (-1);
		++pr;
	}
	return (0);
}

static int
writesubsyss (f, image)
	int		f;
	Image		*image;
{
	int		n;
	Subsys		*ss;

	if (image == NULL) return (-1);
	for (n = 0, ss = image->subsys; ss < image->subend; ++ss) {
		if ((ss->flags & Deleted) == 0) ++n;
	}
	if (putshort (f, n) < 0) return (-1);
	qsort (image->subsys, image->subend - image->subsys, sizeof (*ss),
		cmpss);
	for (ss = image->subsys; ss < image->subend; ++ss) {
		if (ss->flags & Deleted) continue;
		if (putshort (f, ss->flags) < 0 ||
		    putstr (f, ss->name) < 0 ||
		    putstr (f, ss->id) < 0 ||
		    putstr (f, ss->exp) < 0 ||
		    putlong (f, ss->instdate) < 0 ||
		    writemarks (f, ss->rep, ss->repend) < 0 ||
		    writeprereqs (f, ss) < 0) return (-1);
	}
	return (0);
}

static int
writeimages (f, prod)
	int		f;
	Prod		*prod;
{
	int		n;
	Image		*im;

	if (prod == NULL) return (-1);
	for (n = 0, im = prod->image; im < prod->imgend; ++im) {
		if ((im->flags & Deleted) == 0) ++n;
	}
	if (putshort (f, n) < 0) return (-1);
	qsort (prod->image, prod->imgend - prod->image, sizeof (*im), cmpim);
	for (im = prod->image; im < prod->imgend; ++im) {
		if (im->flags & Deleted) continue;
		if (putshort (f, im->flags) < 0 ||
		    putstr (f, im->name) < 0 ||
		    putstr (f, im->id) < 0 ||
		    putshort (f, im->format) < 0 ||
		    putshort (f, im->order) < 0 ||
		    putlong (f, im->version) < 0 ||
		    putlong (f, im->length) < 0 ||
		    putlong (f, im->padsize) < 0 ||
		    writesubsyss (f, im) < 0) return (-1);
	}
	return (0);
}

writeprod (f, pr)
	int		f;
	Prod		*pr;
{
	if (pr->flags & Deleted) return;
	if (putshort (f, Magic_prod) < 0 ||
	    putshort (f, Format_prod) < 0) return (-1);
	if (putstr (f, pr->name) < 0 ||
	    putstr (f, pr->id) < 0 ||
	    putshort (f, pr->flags) < 0 ||
	    writeimages (f, pr) < 0) return (-1);
	return (0);
}
