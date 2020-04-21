#include "inst.h"
#include "hist.h"
#include "idb.h"
#include <sys/types.h>
#include <sys/stat.h>

typedef struct Fluff {
	short		hx;
	long		lowvers;
	long		highvers;
} Fluff;

static Hist	*hist;
static Memset	*fset;
static Fluff	*fltab;
static int	nfluff;
static int	flsize;
static int	quiet;
static int	configat;

/* fluffinit -- initialize a pass through the fluff routines */

fluffinit (h)
	Hist		*h;
{
	fset = idb_newset ();
	fltab = NULL;
	flsize = 0;
	nfluff = 0;
	hist = h;
	for (configat = 0; configat < hist->nattr; ++configat) {
		if (strcmp (hist->attr [configat], "config") == 0) break;
	}
}

/* fluffadd -- identify a product.image.subsystem to be removed */

fluffadd (pr, im, ss)
	Prod		*pr;
	Image		*im;
	Subsys		*ss;
{
	int		n, hx;
	Mark		*p;

	n = 1 + (ss->repend - ss->rep);
	if (nfluff + n > flsize) {
		flsize = nfluff + n;
		fltab = (Fluff *) idb_getmore (fltab,
			flsize * sizeof (Fluff), fset);
	}
	hx = findhandle (hist, pr->name, im->name, ss->name);
	if (hx >= 0) {
		fltab [nfluff].hx = hx;
		fltab [nfluff].lowvers = 0;
		fltab [nfluff++].highvers = maxint (long);
	}
	for (p = ss->rep; p < ss->repend; ++p) {
		hx = findhandle (hist, p->pname, p->iname, p->sname);
		if (hx >= 0) {
			fltab [nfluff].hx = hx;
			fltab [nfluff].lowvers = p->lowvers;
			fltab [nfluff++].highvers = p->highvers;
		}
	}
}

/* fluff -- initiate the removal of identified products.images.subsystems */

fluff (hush)
	int		hush;
{
	int		i;
	Handle		*p;

	quiet = hush;
	fluffnode (hist->root);
	for (i = 0; i < nfluff; ++i) {
		p = hist->handle + fltab [i].hx;
		histdelss (hist, p->pr, p->im, p->ss);
	}
	idb_dispose (fset);
}

/* fluffnode -- remove fluff from the children of a node */

static
fluffnode (n)
	H_node		*n;
{
	H_node		*ch;
	int		i;
	char		name [Strsize];
	Fluff		*fp;

	for (ch = n->child; ch < n->chend; ++ch) {
		if (ch->type == 0) continue;
		if (ch->type == S_IFDIR) fluffnode (ch);
		for (fp = fltab; fp < fltab + nfluff; ++fp) {
			if (ch->hx < 0 || ch->hx >= hist->nhandle ||
			    hist->handle [ch->hx].pr == NULL ||
			    hist->handle [ch->hx].im == NULL ||
			    hist->handle [ch->hx].ss == NULL) {
#ifdef notdef
				fprintf (stderr, "Internal error; fluff reference to bad handle\n");
#endif
				continue;
			}
			if (ch->hx == fp->hx &&
			    hist->handle [ch->hx].im->version >= fp->lowvers &&
			    hist->handle [ch->hx].im->version <= fp->highvers)
				break;
		}
		if (fp >= fltab + nfluff) continue;
		for (i = 0; i < ch->nattr; ++i) {
			if (ch->attr [i] == configat) break;
		}
		if (i < ch->nattr) continue;
		strcpy (name, idb_rpath (histpath (ch)));
		if (ch->type == S_IFDIR) {
			if (rmdir (name) < 0) {
				if (!quiet) perror (name);
			} else {
				if (!quiet) printf ("rmdir %s\n", name);
				ch->type = 0;
			}
		} else {
			if (unlink (name) < 0) {
				if (!quiet) perror (name);
			} else {
				if (!quiet) printf ("rm %s\n", name);
				ch->type = 0;
			}
		}
	}
}
