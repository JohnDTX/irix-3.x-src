/* setscan, nextscan - general product hierarchy scanning
 *
 * setscan (spec, pname, iname, sname)
 *
 *	Initiates scanning from the given spec, and pname, iname, and sname
 *	patterns.
 *
 * nextscan (&pr, &im, &ss)
 *
 *	Returns the next product, image, and/or subsystem structures that
 *	match the pname, iname, and sname patterns.  The return values can
 *	be restricted to the upper levels by giving NULL or zero-length
 *	patterns for iname and/or sname; in this case no image and/or
 *	subsystem pointers will be returned.  Also, if pr, im, or ss is
 *	NULL, no value will be returned.
 */

#include "inst.h"
#include <stdio.h>

static Spec		*spec;
static Prod		*pr;
static Image		*im;
static Subsys		*ss;
static char		prpat [Strsize];
static char		impat [Strsize];
static char		sspat [Strsize];
static int		ch;

setscan (pname, iname, sname, uspec)
	char		*pname;
	char		*iname;
	char		*sname;
	Spec		*uspec;
{
	if (pname == NULL) *prpat = '\0'; else strcpy (prpat, pname);
	if (iname == NULL) *impat = '\0'; else strcpy (impat, iname);
	if (sname == NULL) *sspat = '\0'; else strcpy (sspat, sname);
	if (uspec != NULL) {
		spec = uspec;
		pr = spec->prod;
		if (pr != NULL) im = pr->image;
		if (ss != NULL) ss = im->subsys;
		ch = Chpr | Chim | Chss;
	}
}

nextscan (prp, imp, ssp)
	Prod		**prp;
	Image		**imp;
	Subsys		**ssp;
{
	int		t;

	if (prp != NULL) *prp = NULL;
	if (imp != NULL) *imp = NULL;
	if (ssp != NULL) *ssp = NULL;
	if (spec == NULL || !*prpat) {
		return (0);
	}
	for ( ; pr < spec->prodend; ++pr, ch |= Chpr, im = NULL, ss = NULL) {
		if (!filematch (pr->name, prpat)) {
			continue;
		}
		if (prp != NULL) *prp = pr;
		if (!*impat) {
			++pr;
			t = ch; ch = Chpr;
			return (t);
		}
		if (im == NULL) im = pr->image;
		for ( ; im < pr->imgend; ++im, ch |= Chim, ss = NULL) {
			if (!filematch (im->name, impat)) {
				continue;
			}
			if (imp != NULL) *imp = im;
			if (!*sspat) {
				++im;
				t = ch; ch = Chim;
				return (t);
			}
			if (ss == NULL) ss = im->subsys;
			for ( ; ss < im->subend; ++ss, ch |= Chss) {
				if (!filematch (ss->name, sspat)) {
					continue;
				}
				if (ssp != NULL) *ssp = ss;
				++ss;
				t = ch; ch = Chss;
				return (t);
			}
		}
	}
	return (0);
}

locate (prpat, impat, sspat, spec, prp, imp, ssp, all)
	char		*prpat;
	char		*impat;
	char		*sspat;
	Spec		*spec;
	Prod		**prp;
	Image		**imp;
	Subsys		**ssp;
	int		all;
{
	Prod		*pr;
	Image		*im;
	Subsys		*ss;

	if (prp != NULL) *prp = NULL;
	if (imp != NULL) *imp = NULL;
	if (ssp != NULL) *ssp = NULL;
	if (!*prpat) {
		return (0);
	}
	if (spec == NULL) return (0);
	for (pr = spec->prod; pr < spec->prodend; ++pr) {
		if (!all && pr->flags & Deleted) continue;
		if (pr->name == NULL ||
		    !filematch (pr->name, prpat)) continue;
		if (prp != NULL) *prp = pr;
		if (!*impat) {
			++pr;
			return (1);
		}
		for (im = pr->image; im < pr->imgend; ++im) {
			if (!all && im->flags & Deleted) continue;
			if (im->name == NULL ||
			    !filematch (im->name, impat)) continue;
			if (imp != NULL) *imp = im;
			if (!*sspat) {
				++im;
				return (1);
			}
			for (ss = im->subsys; ss < im->subend; ++ss) {
				if (!all && ss->flags & Deleted) continue;
				if (ss->name == NULL ||
				    !filematch (ss->name, sspat)) continue;
				if (ssp != NULL) *ssp = ss;
				++ss;
				return (1);
			}
		}
	}
	return (0);
}
