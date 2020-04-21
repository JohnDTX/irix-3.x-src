/*
 * (C) 1983 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

/* @(#)chklink.c	1.5 */
#include "vchk.h"

/* Chklink (filename, inode, device, link count)
 * Is called when a link is found to keep track of the filename and
 * so later calls will resolve the link
 */
chklink (fn, i, d, knt)
	char *fn;
	short i, d, knt;
{
	struct find_list *lp, *lprev=0, *startlink();
	struct find_next *fp, *fprev=0, *mklink();
	char *prtlinks();
#ifdef DEBUG
	char *dp;
#endif DEBUG

	D(10,("chklink(%s, ino=%d, dev=x%x, knt=%d) ",fn,i,d,knt));

/* Find any existing links to this ino/dev pair
 * set lp to that link header, lprev to the previous link header.
 */
	for (lp=lhdr; lp; lp=lp->fl_nxt) {
		if (i) {
			if (lp->fl_ino == i && lp->fl_dev == d) break;
		}
		lprev = lp;
	}

	if (lp == 0) {		/* must build a new link header */
		lp = startlink(fn,i,d,knt);
		if (!lprev) lhdr = lp;
		else lprev->fl_nxt = lp;
		return 1;	/* first link to this file */
	}

/* We get here if this is the second through last link to the file.
 * The list is checked to make sure the same link is not recorded twice.
 */
	knt = 1;
	for (fp= &lp->fl_ent; fp; fp=fp->fn_next) {
		fprev = fp;
		if (strcmp(fn,fp->fn_name) == 0) {
			D(10,("already entered\n"));
			return knt;
		}
		knt++;
	}

	fprev->fn_next = mklink(fn);
	if (--lp->fl_knt <= 0) {
		if ((knt = lp->fl_knt) == 0) {
#ifdef DEBUG
			D(10,("resolved: "))
			dp = prtlinks(fn);
			D(10,("linked to [%s]\n",dp))
#endif DEBUG
			lp->fl_id = Curdir->t_idnum;
		}
		return 0;
	}
	D(10,("added\n"));
	return knt;
}

/* Add an additional link name
 */
struct find_next *
mklink (fn)
	char *fn;
{
	struct find_next *n;
	int i;

	i = sizeof(struct find_next) + strlen(fn);
	n = (struct find_next *) getmem(i,1,"add link name",fn);

	n->fn_next = 0;
	cps(n->fn_name,fn);
	return n;
}

struct find_list *
startlink (fn, i, d, k)
	char *fn;
	short i, d, k;
{
	struct find_list *n;
	int x;

	x = sizeof(struct find_list) + strlen(fn);
	n = (struct find_list *) getmem(x,1,"start link trace",fn);
	n->fl_ino = i;
	n->fl_dev = d;
	n->fl_knt = k - 1;
	n->fl_nxt = 0;
	n->fl_ent.fn_next = 0;
	cps(n->fl_ent.fn_name,fn);
	D(12,("\nstartlink: built <`%s',ino=%d,dev=%d,rem=%d> at x%x\n",n->fl_ent.fn_name,n->fl_ino,n->fl_dev,n->fl_knt));
	return n;
}

/* Format link names in standard list notation (ie, name, name, ...)
 * Compare with mklnames which generates csh {} notation.
 */
char *
prtlinks (fn)
	char *fn;
{
	register struct find_list *lp;
	register struct find_next *fp;
	extern char *chrbp;
	register char *bp;
	int foundit = 0;

	for (lp=lhdr; lp; lp=lp->fl_nxt) {
		bp = chrbp;
		for (fp= &lp->fl_ent; fp; fp=fp->fn_next) {
			if (strcmp(fn,fp->fn_name) != 0) {
				bp += cpuniq(0,bp,fn,fp->fn_name);
				*bp++ = ',';
				*bp++ = ' ';
			} else foundit++;
		}
		if (foundit != 0) {
			*--bp = '\0';	/* get rid of trailing `, ' */
			*--bp = '\0';
			return chrbp;
		}
	}
	return 0;
}

/* Build the link name part of a vchk entry for fn (the resolving link)
 * This is in csh {} notation.  id is the identification number for the
 * directory containing this link.  It is used to determine the best way
 * to represent the link names (ie. the most readable)
 */
char *
mklnames (fn, id)
	char *fn;
{
	register struct find_list *lp;
	register struct find_next *fp;
	extern char *chrbp;
	register char *bp;
	int foundit=0;
	char *instr(), *croppath();

	for (lp=lhdr; lp; lp=lp->fl_nxt) {
		if (id && lp->fl_id != id) continue;
		bp = chrbp;		/* special link name buffer */
		*bp = '\0';
		for (fp= &lp->fl_ent; fp; fp=fp->fn_next) {
			if (strcmp(fn,fp->fn_name) != 0)
				bp += cpuniq(chrbp,bp,fn,fp->fn_name);
			else foundit++;
		}
		if (foundit != 0) {
			*bp = '\0';
			bp = chrbp;
			if (*bp == LSQIG && !instr(bp,"$./"))
				return croppath(chrbp);
			return chrbp;
		}
	}
	return 0;
}

cpuniq (flg, rcv, cmp, snd)
	char *flg, *rcv, *cmp, *snd;
{
	char *cp, *a, *b, *s, *p, *t;
	char *instr(), *index(), *fname();
	char buf[PATHSIZ];
	extern char TOPDIR;
	int l;

if (flg)
D(12,("MKLINK (`%s', %d, `%s', `%s'): ",flg,rcv-flg,cmp,snd));
	a = cmp;
	b = fname(cmp);		/* filename (last segment of pathname) */
	s = snd;
	t = buf;

	while (*s++ == *a++)
		if (a >= b) {		/* matched all the way to filename */
D(12,("LOCAL s=`%s', a=`%s'\n",s,a));
		nearby:
			if (!flg)			/* not csh{} format */
				return cps(rcv,s);	/* append link name */
			l = strlen(s);			/* l is bytes needed */
			if (cp = instr(flg,"$.")) {	/* had local link */
				l++;			/* need , too */
				if (cp[3] != LSQIG) {	/* only one other */
D(12,("adding second local link"));
					l++;		/* have to add {}'s */
					cp += 3;	/* start of prev link */
					cps(cp+l,cp);	/* make room (insert)*/
					l++;
					*cp++ = LSQIG;	/* add left squigly */
					*(cp += cps(cp,s)) = ',';
					while (*++cp)	/* now insert rsquig */
						if (*cp == RSQIG || *cp == ',')
							break;
					if (*cp) cps(cp+1,cp);
					else cp[1] = '\0';
					*cp = RSQIG;
				} else {
D(12,("adding additional local link"));
					cp = pmatch(cp+3);
					if (!cp) E(("LOGIC -- cuniq"));
					cps(cp+l,cp);
					*cp++ = ',';
					cp[cps(cp,s)] = RSQIG;
				}
			} else {	/* this is the first local link */
				l += 3;		/* have to add $./ */
				if (*flg) {	/* had link previously */
D(12,("adding first local link"));
					l += 1;	/* , to separate link names */
					if (*flg != LSQIG) {	/* just 1 prev*/
D(12,(" (second)"));
						l += 2;		/* for {}'s */
						cp = flg + cps(flg+1,flg) + 1;
						*flg = LSQIG;
						cp += cps(cp,",$./");
						cp += cps(cp,s);
					} else {
D(12,(" (additional)"));
						cp = pmatch(flg);
						if (!cp) E(("LOGIC -- cpuniq"));
						cp += cps(cp,",$./");
						cp += cps(cp,s);
					}
					*cp++ = RSQIG;
				} else {
D(12,(" (initial)"));
					cp = flg + cps(flg,"$./");
					cp += cps(cp,s);
				}
				*cp = '\0';
			}
D(12,(" `%s'\n",flg));
			rcv[l] = '\0';
			return l;
		}

		/* We get here if the link is not in the same directory */
	while (*--s != '/')
		if (s <= snd) break;
	if (*s == '/') s++;
	while (*--a != '/')
		if (a <= cmp) break;
	if (*a == '/') a++;
D(12,("non local (%s -- %s)\n",a,s));

	while (a < b && (p = index(a,'/'))) {	/* add ../ for each dir left */
		a = p + 1;
		*t++ = '.';
		*t++ = '.';
		*t++ = '/';
	}
	t += cps(t,s);				/* plus remainder */
	
	if (*snd != '/') l = 3;
	else l = 0;

	if (t - buf < (l += strlen(snd))) {	/* shorter to use .. */
		s = buf;
		goto nearby;
	}

	if (flg) {			/* prepare output buf if build format */
		if (*flg) {		/* buf has link names in it already */
			l += 1;		/* for , */
			if (*flg == LSQIG) {	/* more than 1 name in buf */
D(12,(" additional\n"));
				rcv = pmatch(flg);
				if (!rcv) E(("LOGIC - cuniq(pmatch)"));
			} else {
D(12,(" second\n"));
				l += 2;		/* space for {}'s */
				rcv = flg + cps(flg+1,flg) + 1;
				*flg = LSQIG;	/* inserted left squigly */
			}
			*rcv++ = ',';	/* separator */
		} else {
D(12,(" first\n"));
			flg = 0;	/* degenerates to simplest case */
		}
	}
		/* at this point if flg is true we must add a right squigly
		 * when finished constructing the link path in rcv.
		 */

	if (*snd != '/') {
		*rcv++ = '$';
		*rcv++ = TOPDIR;
		*rcv++ = '/';
	}
	cp = rcv + cps(rcv,snd);

	if (flg) {
		*cp++ = RSQIG;
		*cp = '\0';
	}
	return l;
}

unresolved ()
{
	register struct find_list *lp;
	register struct find_next *fp;
	char buf[LINESIZ];
	register char *bp;
	int foundit = 0;

	for (lp=lhdr; lp; lp=lp->fl_nxt) {
		bp = buf;
		if (lp->fl_knt <= 0) continue;
		for (fp= &lp->fl_ent; fp; fp=fp->fn_next) {
			bp += cps(bp,fp->fn_name);
			*bp++ = ' ';
		}
		*--bp = '\0';
		if (foundit++ == 0)
			fprintf(stderr,"Unresolved links outstanding:\n");
		fprintf(stderr,"%d left:  %s\n",lp->fl_knt,buf);
	}
	return 0;
}
