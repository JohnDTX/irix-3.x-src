/*
 * (C) 1983 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

/* @(#)path.c	1.6 */
#include "vchk.h"

extern int lnexflg;
/* This code takes care of {} and () pathname expansions.
 * chkentry is called with complex pathnames, it clears some special
 * buffers breaks off the check fields (the version number etc) and calls
 * pexpand with the pathname and the name of a procedure to call with
 * each of the pathnames that result from the expansion while that function
 * returns -1.
 */

initpath()
{
	extkey = 0;
	brktlev = brktbuf;
	*brktlev = 0;
	chrbp = chrbuf;
	nxtent = &entab[1];
	nxtkey = &keytab[1];
}

addpath (p, b)
	char *p;
	short *b;
{
	short *n, getent();
	int knt = 1;

	D(9,("addpath(%s,%d) ",p,*b));

	while (*b) {
		knt++;
		n = &entab[*b].e_nxt;
		if (strcmp(chrbuf+entab[*b].e_str,p) == 0) {
			D(9,("already there\n"));
			return 0;
		}
		D(11,("skip `%s' to ",chrbuf+entab[*b].e_str));
		D(10,("%d ",*n));
		b = n;
	}
        *b = getent(p);
	D(10,("added %s entry ",nth(knt)));
	D(9,("at entab[%d]\n",*b));
	return 1;
}

short
getent (p)
	char *p;
{
	struct ent *ep;

	if ((ep = nxtent++) >= &entab[ENTSIZ]) {
		X(("max#[ENTSIZ=%d] pathname expansions exceeded\n",ENTSIZ));
		nxtent--;
		return 0;
	}

	if (chrbp-chrbuf > sizeof(chrbuf) - LINESIZ) {	/* need blk at end */
		X(("max#[MAXCHRS=%d] characters in path expansions exceeded\n",MAXCHRS));
		return 0;
	}

	ep->e_str = chrbp - chrbuf;
	ep->e_nxt = 0;
	chrbp += cps(chrbp,p);
	*chrbp++ = '\0';
	return ep - entab;
}

/* n is one of those funny pathnames with ()'s, and {}'s.
 * f is a function of two arguments.
 * We call it[f] for each expansion of the path (left to right)
 * until it returns 0.
 * Passed to it[f] are the filename and a pointer to a unique `key'
 * structure for each expansion of {}'s only.  ie it is the same for
 * calls resulting from the expansion of ()'s
 * The global variable lnexflg diables this distinction.
 * Note that () and {} are expanded left to right.
 * When none are detected in a given pass then pexpand falls through
 * to the function call[f].
 * Backslash delays the interpretation of both () and {}'s and can
 * be used to alter the order of interpretation because one layer
 * or backslashes is removed each pass.
 */
pexpand (n, f)
	char *n;
	int (*f)();
{
	char *tp, *lp, *rp, *cp, *bp, *xp;
	char buf[PATHSIZ];
	int nlev, i;
	char *fixpath(), *cindex(), *pmatch();
	int likebrkt, commaflag = 0;
	short getkey();

	D(12,("Pexpand[%s]\n",n));;

	if (lp = cindex(n,LPRENS)) {		/* eliminate set of prens */
		if (!(rp=pmatch(lp)) || (brktlev>=brktbuf+sizeof(brktbuf)-1)) {
			X(("pathname `%s' too complicated\n",n));
			return -1;
		}
		*++brktlev = (*(xp = lp) == LSQIG) ?'\1' :'\0';
		if (!lnexflg) {
			if (*brktlev == '\0' && brktlev[-1] == '\1') {
				if (getkey(n) == 0)
					return -1;
			}
		}
		*lp++ = '\0';			/* replace pren with null */
		bp = buf + xcps(buf,n);		/* copy prefix to buf */
		*rp++ = '\0';			/* replace right pren */
		tp = bp;			/* save ptr to varying area */
		while (*lp) {
			if (*lp == ',') {	/* end of this option */
				xcps(bp,rp);	/* copy right part */
				i = pexpand(buf,f);
				if ((lnexflg || *brktlev == 0) && i != -1)
					goto fini;
				commaflag = 1;	/* had comma so not optional */
				bp = tp;	/* prep for next alt */
				lp++;		/* skip over comma */
				continue;
			}
			if (index(LPRENS,*lp)) {
				if (!(cp = pmatch(lp)))
					return -1;
				while (lp < cp)
					*bp++ = *lp++;
			}
			*bp++ = *lp++;
		}
		xcps(bp,rp);			/* append what followed pren */

		if ((i = pexpand(buf,f)) == -1) {
			if (!commaflag && bp != tp) {	/* try without prens */
				xcps(tp,rp);
				i = pexpand(buf,f);
			}
		}
	fini:
		xp[0] = *brktlev ?LSQIG :LPREN;
		rp[-1] = *brktlev ?RSQIG :RPREN;
		brktlev--;
		return i;
	}

	if (!lnexflg) {
		if (*brktlev && getkey(n) == 0)
			return -1;
	}

	return f(fixpath(n),extkey);
}

	/* Like cps but filters eliminates backslash's */
int
xcps (A,B)
	register char *A, *B;
{
	char c, *b = B;			/* hold copy of B in to compute len */

	if (!A || !B) return 0;		/* if either is null return no copy */
	do {
		if ((*A++ = *B) == '\\')
			if ((c = B[1]) && index("\\(){}",c)) A--;
	} while (*B++);
	return B - b - 1;	/* return length copied	*/
}

short
getkey (n)
	char *n;
{
	register struct key *k;

	D(4,("getkey(%s)\n",n));
	if ((k = nxtkey++) >= &keytab[NKEYS]) {
		X(("%s: max#[%d] {} expansions exceeded\n",n,NKEYS));
		nxtkey--;
		return 0;
	}
	k->k_mis = 0;
	k->k_chk = 0;
	k->k_alt = 0;
	k->k_dup = 0;
	reptab[k-keytab] = 0;
	addpath(n,&reptab[k-keytab]);
	extkey = k;
	return k - keytab;
}

char *
getstr (x, i)
	short x;
{
	static char buf[PATHSIZ];
	struct ent *p = 0;
	char *cp = buf, *np;
	int n = i;
	D(6,("getstr(ent=%d, inx=%d) ",x,i));

	*cp = 0;
	while (x > 0) {
		if (p) {
			if (i == 0) break; 
			if (i < 0) *cp++ = ' ';
		}

#ifdef BUGCHECK
		if (x >= ENTSIZ)
			E(("LOGIC: offset[%d] out of range[1-%d]\n",x,ENTSIZ));
#endif BUGCHECK

		p = &entab[x];

#ifdef BUGCHECK
		if (p >= nxtent)
			E(("LOGIC: offset[%d] past lim[%d]\n",x,nxtent-entab));
#endif BUGCHECK

		np = chrbuf + p->e_str;

#ifdef BUGCHECK
		if ((np = chrbuf + p->e_str) >= chrbp)
			E(("LOGIC: string offset[%d] past lim[%d]\n",np-chrbuf,chrbp-chrbuf));
#endif BUGCHECK
		x = p->e_nxt;
		D(7,("`%s' at %d ",chrbuf+p->e_str,x));
		if (--i <= 0)
			cp += cps(cp,chrbuf+p->e_str);
	}
	D(7,("returning `%s'",buf));
	D(6,("\n"));
	if (*buf) return buf;
	return 0;
}

putlist (lst, m, f)
	short lst;
	char *m, *f; 
{
	char len;
	struct ent *p = &entab[lst];
	char buf[LINESIZ];
	char chr, *sp, *cp;

	if (Sflag) return;

#ifdef BUGCHECK
	if (!p->e_str)
		E(("LOGIC ERR: putlist of empty list\n"));
#endif BUGCHECK

	sprintf(buf,"%s: %s exist:",f,m);
	len = strlen(buf);
	chr = ' ';

	do {
		sp = chrbuf + p->e_str;
		p = &entab[p->e_nxt];
		cp = buf+len;
		*cp++ = chr;
		chr = ' ';
		cp += cps(cp,sp);
		if (cp > buf+70) {
			chr = '\t';
			*cp++ = '\0';
			M(("%s\n",buf));
			len = 0;
		} else len = cp - buf;
	} while (p != &entab[0]);
	if (len > 0) M(("\n"));
}
