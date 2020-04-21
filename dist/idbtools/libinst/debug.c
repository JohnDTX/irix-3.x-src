#ifdef DEBUGFUNCS

#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include "inst.h"
#include "hist.h"
#include "idb.h"

#define nullcheck(p)	(p==NULL ? \
			(fprintf(stderr,"%s is NULL.\n", "p"),1):0)

#define ncheck(st,en,mx) ((en-st) > mx ? \
			(fprintf(stderr,"high %s count %d\n","st",en-st),1):0)

dumphist (title, hist, withspec, withtree)
	char		*title;
	Hist		*hist;
	int		withspec;
	int		withtree;
{
	Handle		*hand;
	int		i;

	fflush (stdout);
	fprintf (stderr, "\n@ history dump, %s\n", title);
	if (nullcheck (hist)) return;
	fprintf (stderr, "hist->format %d\n", hist->format);
	fprintf (stderr, "hist->nhandle %d\n", hist->nhandle);
	if (!nullcheck (hist->handle)) {
		for (hand = hist->handle; hand < hist->handle + hist->nhandle; ++hand) {
			fprintf (stderr, "\t");
			if (!nullcheck (hand->name))
				fprintf (stderr, "%s, ", hand->name);
			if (!nullcheck (hand->pr) &&
			    !nullcheck (hand->pr->name))
				fprintf (stderr, "%s ", hand->pr->name);
			if (!nullcheck (hand->im) &&
			    !nullcheck (hand->im->name))
				fprintf (stderr, "%s ", hand->im->name);
			if (!nullcheck (hand->ss) &&
			    !nullcheck (hand->ss->name))
				fprintf (stderr, "%s", hand->ss->name);
			fprintf (stderr, "\n");
		}
	}
	fprintf (stderr, "hist->nattr %d\n", hist->nattr);
	if (!nullcheck (hist->attr)) {
		for (i = 0; i < hist->nattr; ++i) {
			if (!nullcheck (hist->attr [i])) {
				fprintf (stderr, "	%s\n", hist->attr [i]);
			}
		}
	}
	if (withspec) dumpspec ("", hist->spec);
	if (withtree) dumptree (title, hist);
}

dumpspec (title, spec)
	char		*title;
	Spec		*spec;
{
	Prod		*pr;

	fflush (stdout);
	fprintf (stderr, "\nspec dump, %s\n", title);
	if (nullcheck (spec) || nullcheck (spec->prod) ||
		nullcheck (spec->prodend)) return;
	if (ncheck (spec->prod, spec->prodend, 30)) return;
	for (pr = spec->prod; pr < spec->prodend; ++pr) {
		if (!nullcheck (pr->name))
			fprintf (stderr, "pr->name %s\n", pr->name);
		if (!nullcheck (pr->id))
			fprintf (stderr, "pr->id %s\n", pr->id);
		fprintf (stderr, "pr->flags %0x\n", pr->flags);
		dumpimages (pr);
	}
}

dumpimages (pr)
	Prod		*pr;
{
	Image		*im;

	if (nullcheck (pr)) return;
	if (nullcheck (pr->name)) return;
	if (nullcheck (pr->image) || nullcheck (pr->imgend)) return;
	if (ncheck (pr->image, pr->imgend, 10)) return;
	for (im = pr->image; im < pr->imgend; ++im) {
		if (!nullcheck (im->name))
			fprintf (stderr, "im->name %s.%s\n", pr->name, im->name);
		if (!nullcheck (im->id))
			fprintf (stderr, "im->id %s\n", im->id);
		fprintf (stderr, "im->flags %0x\n", im->flags);
		fprintf (stderr, "im->format %d, im->version %ld, im->length %ld, ",
			im->format, im->version, im->length);
		fprintf (stderr, "im->padsize %ld\n", im->padsize);
		dumpsubsyss (pr, im);
	}
}

dumpsubsyss (pr, im)
	Prod		*pr;
	Image		*im;
{
	Subsys		*ss;

	if (nullcheck (im)) return;
	if (nullcheck (im->name)) return;
	if (nullcheck (im->subsys) || nullcheck (im->subend)) return;
	if (ncheck (im->subsys, im->subend, 40)) return;
	for (ss = im->subsys; ss < im->subend; ++ss) {
		if (!nullcheck (ss->name))
			fprintf (stderr, "ss->name %s.%s.%s\n", pr->name,
				im->name, ss->name);
		if (!nullcheck (ss->id))
			fprintf (stderr, "ss->id '%s'\n", ss->id);
		if (!nullcheck (ss->exp))
			fprintf (stderr, "ss->exp '%s'\n", ss->exp);
		fprintf (stderr, "ss->flags %0x\n", ss->flags);
		fprintf (stderr, "ss->instdate %s", ctime (&ss->instdate));
		dumpreplacements (ss);
		dumpprereqs (ss);
	}
}

dumpreplacements (ss)
	Subsys		*ss;
{
	if (nullcheck (ss)) return;
	nullcheck (ss->rep); nullcheck (ss->repend);
	if (ncheck (ss->rep, ss->repend, 30)) return;
	fprintf (stderr, "ss->rep (%d)\n", ss->repend - ss->rep);
	dumpmarks (ss->rep, ss->repend);
}

dumpprereqs (ss)
	Subsys		*ss;
{
	Prereq		*pq;

	if (nullcheck (ss)) return;
	nullcheck (ss->prereq);
	nullcheck (ss->preend);
	if (ncheck (ss->prereq, ss->preend, 30)) return;
	fprintf (stderr, "ss->prereq (%d)\n", ss->preend - ss->prereq);
	for (pq = ss->prereq; pq < ss->preend; ++pq) {
		fprintf (stderr, "	pq[%d] (%d): ", pq - ss->prereq,
			pq->pqend - pq->pq);
		dumpmarks (pq->pq, pq->pqend);
		if (pq < ss->preend - 1) fprintf (stderr, "\n");
	}
}

dumpmarks (mk, mkend)
	Mark		*mk;
	Mark		*mkend;
{
	nullcheck (mk);
	nullcheck (mkend);
	if (ncheck (mk, mkend, 30)) return;
	while (mk < mkend) {
		if (!nullcheck (mk->pname))
			fprintf (stderr, "	pname %s, ", mk->pname);
		if (!nullcheck (mk->iname))
			fprintf (stderr, "iname %s, ", mk->iname);
		if (!nullcheck (mk->sname))
			fprintf (stderr, "sname %s, ", mk->sname);
		fprintf (stderr, "lowvers %ld, highvers %ld\n", mk->lowvers,
			mk->highvers);
		++mk;
	}
}

dumptree (title, hist)
	char		*title;
	Hist		*hist;
{
	fprintf (stderr, "hist tree dump, %s\n", title);
	if (nullcheck (hist) || nullcheck (hist->root)) return;
	dumpnode ("", hist, hist->root);
}

dumpnode (parent, h, n)
	char		*parent;
	Hist		*h;
	H_node		*n;
{
	char		pathname [1024], *s;
	H_node		*ch;
	int		i;

	if (nullcheck (n)) return;
	switch (n->type) {
	case S_IFDIR: s = "d"; break;
	case S_IFREG: s = "f"; break;
	case S_IFLNK: s = "l"; break;
	case S_IFBLK: s = "b"; break;
	case S_IFCHR: s = "c"; break;
	default: s = "?"; break;
	}
	fprintf (stderr, "%s ", s);
	if (n->hx < 0 || n->hx > h->nhandle) {
		fprintf (stderr, "(bad n->hx) ");
	} else if (!nullcheck (h->handle) &&
	    !nullcheck (h->handle [n->hx].name)) {
		fprintf (stderr, "%s ", h->handle [n->hx].name);
	}
	fprintf (stderr, "chk %u ", n->chksum);
	strcpy (pathname, parent);
	strcat (pathname, "/");
	strcat (pathname, n->name);
	fprintf (stderr, "%s ", pathname);
	fprintf (stderr, "attr (%d) {", n->nattr);
	for (i = 0; i < n->nattr && i < 8; ++i) {
		if (!nullcheck (h->attr) &&
		    !nullcheck (h->attr [n->attr [i]])) {
			fprintf (stderr, "%s ", h->attr [n->attr [i]]);
		}
	}
	fprintf (stderr, "} ");
	if (!nullcheck (n->parent) && !nullcheck (n->parent->name))
		fprintf (stderr, "parent %s ", n->parent->name);
	fprintf (stderr, "and %d children\n", n->chend - n->child);
	for (ch = n->child; ch < n->chend; ++ch) {
		dumpnode (pathname, h, ch);
	}
}

#endif
