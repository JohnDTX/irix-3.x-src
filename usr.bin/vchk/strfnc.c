/*
 * (C) 1983 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

/* @(#)strfnc.c	1.6 */
#include "vchk.h"

/*
 * Sum bytes in file mod 2^16
 */
unsigned
csum (fn)
	char *fn;
{
	unsigned sum;
	int rv;
	FILE *f;
	static char buf[BUFSIZ];

	D(1,("in csum(%s)\n",fn));

	if ((f = fopen(fn, "r")) == NULL) {
		X(("Can't open %s to perform chksum", fn));
		return (unsigned)(-1);
	}
#ifdef UniSoft
	setbuf(f,buf);
#endif
	rv = chksum(f,&sum);
	fclose(f);
	if (rv == -1) {
		X(("Checksum(%s) failed: %s\n",fn,SE));
		return (unsigned)(-1);
	}
	return sum;
}

chksum (fp, up)
	register FILE *fp;
	unsigned *up;
{
	extern int fsumlen;
	register unsigned sum;
	register c;

	rewind(fp);
	sum = 0;
	fsumlen = 0;
	while ((c = getc(fp)) != EOF) {
		if (sum&01)
			sum = (sum>>1) + 0x8000;
		else
			sum >>= 1;
		sum += c;
		sum &= 0xFFFF;
		fsumlen++;
	}
	if (ferror(fp)) {
		return -1;
	}
	*up = sum;
	return 0;
}

strpref (a, b)
	register char *a, *b;
{
	char *s = a;
	while (*a++ == *b++)
		if (!a[-1]) break;
	return (a - s) - 1;
}

char *
instr(m, s)
	char *m;
	register char *s;
{	register int len = strlen(s);
	register char *p;
	char *index();

	while (p = index(m,*s))
		if (!strncmp(p,s,len)) break;
		else m = ++p;
	return p;
}

cmpstr (a, b, c)
	register char *a, *b;
	char c;
{
	while (*a == *b) {
		if (*a == c) return 0;
		a++;
		b++;
	}
	if (*a == c || *b == c) {
		if (!*a || !*b) return 0;
		if (*a == c) return -1;	/* b is longer */
		return 1;
	}
	if (*a < *b) return -1;
	return 1;
}

struct stat *
isfile (fp)
	FILE *fp;
{
	static struct stat sb;

	if (fstat(fileno(fp),&sb) || (sb.st_mode & S_IFMT) != S_IFREG)
		return 0;
	return &sb;
}

samefile (a, b)
	char *a, *b;
{
	struct stat sa, sb;

	D(4,("in samefile(`%s', `%s')\n",a,b));

	if (!strcmp(a, b)) return 1;
	if (stat(a,&sa) || stat(b,&sb)) return 0;
	if (sa.st_ino == sb.st_ino && sb.st_dev == sa.st_dev) return 1;
	if (sa.st_size != sb.st_size /* || csum(a) != csum(b) */) return 0;
	return 1;
}

/* Simplify {x,y,..} expression
 */
char *
croppath (p)
	char *p;
{
	static char buf[PATHSIZ];
	int i, j, sn;
	char *rp, **vp, *pmatch(), **divs();

	*(rp = pmatch(p)) = '\0';
	if ((vp = divs(p+1,",")) == 0) {
		*rp = RSQIG;
		return p;
	}
	sn = strpref(vp[0],vp[1]);
	for (j=2; vp[j]; j++)
		if ((i = strpref(vp[j],vp[j-1])) < sn)
			sn = i;
	if (sn < 0) sn = 0;
	cps(buf,vp[0]);
	rp = buf + sn;
	*rp++ = LSQIG;
	for (j=0; vp[j]; j++) {
		rp += cps(rp,vp[j]+sn);
		*rp++ = ',';
	}
	rp[-1] = RSQIG;
	*rp = '\0';
	free(vp);
	return buf;
}

char ESCCHARS[] = "()[]{}<>\\*?$";
char *
escchar (c)
	char c;
{
	return index(ESCCHARS,c);
}
