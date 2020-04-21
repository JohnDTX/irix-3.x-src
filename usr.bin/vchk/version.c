/*
 * (C) 1983 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

/* @(#)version.c	1.6 */
#include "vchk.h"

#if	defined(UniSoft)
# include <b.out.h>
#else
# include <a.out.h>
#endif

long fixlong();		/* ABCD -> DCBA */
long fixshort();	/* ABCD -> BADC (actually fixes a long)*/

int fixflg;

char *
chkvers (n, v)
	char *n, *v;
{
	char *cv, *vp, *instr(), *getvers();

	cv = getvers(n);
	if (cv && instr(cv,v)) return (char *)0;
	if (cv && (vp = instr(cv,"Version"))) return vp + 8;
	return "pre history";
}

char *
getvers(fn)
	char *fn;
{
	char buf[sizeof(struct bhdr)];
	static char iobuf[BUFSIZ];
	FILE *fp;
	char *vp, *findvers();

	fp = fopen(fn,"r");
	if (fp == NULL) return (char *)0;
	setbuf(fp,iobuf);

	rewind(fp);
	if (fread(buf, sizeof buf, 1, fp) != 1) {
		fclose(fp);
		return (char *) 0;
	}
	vp = findvers(buf,fp);
	fclose(fp);
	return vp;
}

long (*fix[])() = { fixlong, fixshort, fixlong, 0 };

char *
findvers(hdr,fp)
	struct bhdr *hdr;
	FILE *fp;
{
	static char buf[LINESIZ];
	register char *cp;
	register int c, cc;
	char *vp, *instr(), *index();
	int trap = 0, minlength=10, cnt;

	fixflg = 0;

	if (!ismagic(hdr->fmagic))	/* this sets up fixflg */
		return 0;

	if (fixflg) {
		for (cnt=0; cnt<fixflg; cnt++)
			hdr->tsize = (*fix[cnt])(hdr->tsize);
		for (cnt=0; cnt<fixflg; cnt++)
			hdr->dsize = (*fix[cnt])(hdr->dsize);
	}

	fseek(fp, (long) hdr->tsize, 1);
	cnt = hdr->dsize;
	cp = buf, cc = 0;
	for (; cnt != 0; cnt--) {
		c = getc(fp);
		if (c == '\n' || dirt(c) || cnt == 0) {
			if (cp > buf && cp[-1] == '\n')
				--cp;
			*cp++ = 0;
			if (cp > &buf[minlength]) {
				if (vp = instr(buf,"Version"))
					/* if (!bflag || isdigit(vp[8])) */
						return buf;
#ifdef UniSoft
				if (trap) {
					if (vp = index(buf,'.')) {
						while (isdigit(*--vp)) ;
						return vp+1;
					}
					if (--trap <= 0)
						return "";
				}
				if (instr(buf,"@(#)"))
					trap = 3;
#endif UniSoft
			}
			cp = buf, cc = 0;
		} else {
			if (cp < &buf[sizeof buf - 2])
				*cp++ = c;
			cc++;
		}
		if (ferror(fp) || feof(fp))
			break;
	}
	return "";
}

dirt(c)
	int c;
{

	switch (c) {

	case '\n':
	case '\f':
		return (0);

	case 0177:
		return (1);

	default:
		return (c > 0200 || c < ' ');
	}
}

ismagic(a)
	int a;
{
	int (*ff)();

retry:
	switch (a) {

	case IMAGIC:
	case NMAGIC:
	case FMAGIC:
	case OMAGIC:
#ifdef UniSoft
	case 0413:		/* kludge to allow finding vax pgm version #'s*/
#endif UniSoft
		return (1);
	}
	if ((ff = fix[fixflg++]) != 0) {
		a = ff(a);
		goto retry;
	}
	fixflg = 0;
	return (0);
}

fixlong (l)
	long l;
{
	long x;
	char *sp=(char *)&l, *rp=(char *)&x;

	rp[0] = sp[3];
	rp[1] = sp[2];
	rp[2] = sp[1];
	rp[3] = sp[0];
	return x;
}

fixshort (l)
	long l;
{
	long x;
	char *sp=(char *)&l, *rp=(char *)&x;

	rp[0] = sp[1];
	rp[1] = sp[0];
	rp[2] = sp[3];
	rp[3] = sp[2];
	return x;
}
