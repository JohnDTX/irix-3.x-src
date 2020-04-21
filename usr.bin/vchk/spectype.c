/*
 * (C) 1983 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

/* @(#)spectype.c	1.6 */
#include "vchk.h"

/* Each line of the input file is either blank, a comment, or a specification.
 * A specification is for either a file or directory.
 * Specifications have three fields; the first (and only manditory field) is
 * the name.  The second is some combination of version number,
 * checksum, and/or length check and is the data check field.
 * The third field is a set of special instructions and comments.
 */

extern char *instr(), *fname(), *index();
char *getmode(), *pmatch(), *devspec();

char *GLOBCHRS = "*?[]<>";
int Pdfcol = 4, Psfcol = 6;
char *locfile;
extern int lnexflg;
extern int findit();
extern char *locferr;

spectype(lp)
	register char *lp;
{
	char	*p = lp;	/* P always points to the beg of the line */
	char	*tp, *ep, *sp=0, *dp=0;
	register char *op, *np, *cp;
	int	rv, i, typ;	/* type is the kind of line we are parsing */
	char schr, *dfmatch(), *sfmatch(), *needcl(), *makvcl();
	static char *dfq[] = { "version number", "checksum", "length", 0 };
	int fld;		/* type of prens used in spec inst field */
	char tbuf[LINESIZ];
	int nomake=0, nomatter=0, norepair=0;
	char *cmdflag=0;

	D(2,("spectype <%s>\n",lp));

	for (np=lp; *np; np++) {	/* find end of pathname */
		if (*np == '\\') {
			if (!*++np) E(("Pathnames[%s] must not end with an unescaped `\\' character\n",lp));
		} else if (*np == ' ') {
			break;
		} else if (*np == '/' && np[1] == '/')
			E(("Pathnames[%s] may not contain adjcent '/' chars\n",lp));
	}
	ep = np;
	if (*np == ' ') *np++ = '\0';	/* np points to rest of line */
	else if (np[-1] == ';') {
		cmdflag = np;
		*--np = '\0';
	}
	if ((typ = chkpath(p,ep-p)) == ERROR) {
	haderr: ;
		X(("(parse): %s\n",Err));
		typ = ERROR;
		goto fini;
	}

	op = tbuf + xcps(tbuf,p);	/* setup output buffer */
	if (eflag) {
		typ = IGNORE;
		goto done;
	}

	if (*np) dp = np;		/* dp points to start of data chk */

	np = cindex(np,LPRENS);		/* find special inst field */
	if (!np) np = "";

	if (*np) {			/* lop off special instructions */
		if (*np == LANGL)	/* <> means totally optional */
			nomatter = 1;
		else if (*np == LBRKT)	/* [] not opt, but cannot be remade */
			nomake = 1;
		else if (*np == LSQIG)	/* {} not opt, but might be modified */
			norepair = 1;
		fld = index(LPRENS,*np) - LPRENS;	/* sav type of instr */
		if ((ep = pmatch(np)) == 0) {
			sprintf(Err,"%s: missing closing `%c' in `%s'",lp,*np,np);
			goto haderr;
		}
		*ep++ = '\0';		/* convert matching pren to null */
		if (*ep) {
			if (*ep == ';') {
				cmdflag = ep+1;
				ep++;
			}
			if (*ep) {
				sprintf(Err,"garbage[%s] after special instructions for `%s'",ep,p);
				goto haderr;
			}
		}
		if (*--np == ' ') *np++ = '\0';	/* terminate data chk field */
		else np++;
		*np++ = '\0';		/* kill left pren */
		sp = np;		/* flag and ptr to beg of instr field */
	}

/* At this point dp points to the beginning of the data chk field or is
 * null and sp points to the special instructions or is null.
 * Data check fields are some combination of Version, Chksum, Length,
 * device specification, or contents (for directories) modes and ownership
 * in any combination or order.
 * They need not be capitalized and my be truncated to any prefix.
 * The space between the keyword and value is also optional.
 */
	if (dp && (np = pindex(dp,';')) && np[1] == '\0') {
		*np = '\0';
		cmdflag = np+1;
	}

	if (cmdflag || nomake || nomatter || norepair) {
		*op++ = ' ';
		if (nomatter) *op++ = '^';	/* don't complain if missing */
		else if (nomake) *op++ = '-';	/* don't attempt to rebuild */
		else if (norepair) *op++ = '+';	/* don't attempt to repair */
		if (cmdflag) *op++ = '!';	/* don't generate commands */
	}

	if (Pflag) {				/* Preprocess only */
		lastnl = 0;
		if ((i = strlen(p)) > (rv = (Pdfcol + 2) * TAB_WIDTH)) {
			fld = LINE_LEN - 2;
			while (i > rv) {	/* split long lines */
				np = p + fld;
				if (np >= p + i) np = p + i - 1;
				tp = (char *)0;
				while (np - p > rv) {
					if (*np == ',') break;
					if (*np == '/' && tp == 0) tp = np;
					np--;
				}
				if (*np == ',') {
					*np++ = '\0';
					printf("%s,\\\n\t",p);
					p = np;
				} else if (tp) {
					*tp++ = '\0';
					printf("%s/\\\n\t",p);
					p = tp;
				} else {
					fld = LINE_LEN - 2;
					if (i-fld < TAB_WIDTH) fld -= TAB_WIDTH;
					printf("%*s\\\n\t",fld,p);
					p += fld;
				}
				i = strlen(p);
				fld = LINE_LEN - (TAB_WIDTH + 2);
			}
			i += TAB_WIDTH;
		}

		printf("%s",p);
		rv = (i + TAB_WIDTH) / TAB_WIDTH;
		if (typ==ITEM && aflag && (tp = needcl(dp))) {
			initpath();
			lnexflg = 1;
			pexpand(p,findit);	/* find on local sys */
			lnexflg = 0;
			if (locferr) {
				X(("Can't locate %s: %s\n",p,locferr));
			} else dp = makvcl(tp,locfile);
		}
		if (dp) {
			if (rv > Pdfcol) putc(' ',stdout);
			else while (rv++ <= Pdfcol) putc('\t',stdout);
			printf("%s",dp);
			i = strlen(dp) + (Pdfcol * TAB_WIDTH);
			rv = (i + TAB_WIDTH) / TAB_WIDTH;
		}
		if (sp) {
			if (rv > Psfcol) putc(' ',stdout);
			else while (rv++ <= Psfcol) putc('\t',stdout);
			if (nomake) printf("%c%s%c",LBRKT,sp,RBRKT);
			else if (nomatter) printf("%c%s%c",LANGL,sp,RANGL);
			else if (norepair) printf("%c%s%c",LSQIG,sp,RSQIG);
			else printf("%c%s%c",LPREN,sp,RPREN);
		}
		if (cmdflag) {
			putc(';',stdout);
			if (*cmdflag) printf("%s",cmdflag);
		}
		putc('\n',stdout);
		return typ;
	}

	while (dp && *dp) {	/* data check field exists (not null len)*/
		if (*dp == ' ') dp++;
		np = dp;	/* pointer to rest of data check field */
		if (dp = devspec(np)) {		/* device spec kludge */
			if (!*dp)		/* had error in dev spec */
				goto haderr;
			*op++ = ' ';		/* output parsed dev spec */
			while (*dp) *op++ = *dp++;
			while (*np++);		/* skip over dev spec */
			if (*np == ' ') np++;
		}
		if (!*np) break;
		cp = np;			/* save start of this arg */
		while (*np && *np != ' ') np++;	/* find beg of next arg */
		dp = np;			/* set next dp to be there */
		if (*dp == ' ') *dp++ = '\0';	/* if there really is another */
		np = cp;			/* restore ptr to this arg */
		rv = 0;
		if (!(lp = dfmatch(np,typ,&rv))) {
			sprintf(Err,"bad data check entry [%s] for `%s'",np,p);
			goto haderr;
		}
		if (!*lp) {		/* value separated by space */
			if (rv == -1) continue;
			if (!dp) {	/* no next value --- value missing */
				sprintf(Err,"missing %s for `%s'", dfq[rv], p);
				goto haderr;
			}
			lp = dp;
			if (dp = index(lp,' ')) *dp++ = '\0';
		}
		*op++ = ' ';
		if (rv != -1)
			if (typ == DIRECTORY)
				*op++ = *cp;
			else
				*op++ = toupper(*cp);
		while (*lp) *op++ = *lp++;
	}

/* Decode special instructions.
 */
	while (sp && *sp) {	/* special inst field exists (not null len)*/
		if (*sp == ' ') sp++;
		cp = np = sp;	/* pointer to rest of spec inst field */
		while (*np && *np != ';') np++;
		sp = np;
		if (*sp == ';') *sp++ = '\0';
		np = cp;
		if (!(lp = sfmatch(np,&schr))) {	/* must be comment */
			if (sp && *sp) {
				X(("Warning: [%s] considered part of `%s' comment\n",sp,np));
				sp[-1] = ';';
			}
			*op++ = ' ';
			*op++ = '"';
			op += xcps(op,np);
			break;
		}
		if (schr == 'E') {
			sprintf(Err,"%s for `%s'", lp, p);
			goto haderr;
		}
		*op++ = ' ';
		if (schr) *op++ = schr;
		while (*lp && *lp != ' ')
			*op++ = *lp++;
	}
done:
	*op = '\0';
	p += xcps(p,tbuf);
	while (*--p == ' ') *p = '\0';
fini:
	return typ;
}

char *
dfmatch (p, typ, rv)
	char *p;
	int *rv;
{
	static char *dtl[] = { "version", "checksum", "length", 0 };
	char **t, *cp;
	int pl, tl;
	struct plist *pwp, *pwlookup();
	static char buf[256];
	int saverrno;
	D(10,("dfmatch(`%s', %s)",p,typename(typ)));

	if (typ == DIRECTORY) {
		if (isdigit(*p)) cp = getmode(p);
		else if (*p == '~' && p[1] == '\0') cp = p;
		else if (pwp = pwlookup(p)) cp = pwp->name;
		else cp = 0;
		*rv = -1;
		if (cp) {
			if (isdigit(*cp)) {
				saverrno = errno;
				sscanf(cp,"%o",&pl);
				errno = saverrno;
			}
			buf[0] = '.';
			xcps(buf+1,cp);
			cp = buf;
		}
		D(10,(" returning `%s'\n",cp));
		return cp;
	}

	for (cp=p; *cp && isalpha(*cp); cp++)
		if (isupper(*cp)) *cp = tolower(*cp);

	if ((pl = cp - p) != 0) {
		for (t=dtl; *t; t++) {
			tl = strlen(*t);
			if (tl > pl) tl = pl;
			*rv = t - dtl;
			if (!strncmp(p,*t,tl)) {
				D(10,(" returning %s\n",p+tl));
				return p + tl;
			}
		}
	}
	D(10,(" failed\n"));
	return 0;
}

char *
getmode(p)
	char *p;
{
	int mh, mu, mg, mo, mode;
	register char *l;
	static char buf[10];
	int saverrno;

	if (*p == ' ') p++;
	for (l=p; *l; l++)
		if (!index("01234567",*l)) return 0;

	saverrno = errno;
	sscanf(p,"%o",&mode);
	errno = saverrno;
	mh = (mode >> 9) & 7;
	mu = (mode >> 6) & 7;
	mg = (mode >> 3) & 7;
	mo = mode & 7;
	if ((mh && !(mu & 1)) ||	/* not execable */
	    mo != (mo & mu) ||	mo != (mo & mg) || mg != (mg & mu))
		X(("Warning: mode[%o] specified looks funny\n",mode));
	sprintf(buf,"%o",mode);
	return buf;
}

char *
sfmatch (cp, rc)
	char *cp, *rc;
{
	char *tp = (char *)0;		/* check for recognized fields */
	char *sp, tbuf[256];
	static char buf[256];
	struct plist *pwp, *pwlookup();
	int val, i;
	int saverrno;

	sp = tbuf;
	for (i=0; i<sizeof(tbuf) && *cp; i++)
		*sp++ = *cp++;
	*sp = '\0';
	cp = tbuf;
	if (isupper(*cp)) *cp = tolower(*cp);
	if (tp=instr(cp,"chown")) {		/* non-standard owner*/
		tp += 5;
		if (*tp == ' ') tp++;
		if (*tp == '~' && tp[1] == '\0') {	/* any owner flag */
			*rc = '\0';
			return tp;
		}
		if (isdigit(*tp) || !(pwp = pwlookup(tp))) {
			sprintf(buf,"invalid account name [%s]",tp);
			*rc = 'E';
			return buf;
		}
		*rc = '\0';
		return pwp->name;
	}


	if (tp=instr(cp,"chmod")) {	/* non-standard mode */
		*rc = '\0';
		if (!(sp = getmode(tp+5))) {
			*rc = 'E';
			sprintf(buf,"invalid mode [%s]",tp+5);
			return buf;
		}
		/*
		saverrno = errno;
		sscanf(sp,"%o",&val);
		errno = saverrno;
		*/
		return sp;
	}

	if (tp = instr(cp,"link to")) {	/* linked to other */
		tp += 7;
		if (*tp == ' ') tp++;
		sp = tp;			/* save start of file name */
		if (tp = cindex(tp, GLOBCHRS)) {
			sprintf(buf,"Warning: link name `%s'",cp);
			cp = buf + strlen(buf);
			*tp = '\0';
			sprintf(cp," truncated to `%s'\n",sp);
			*rc = 'E';
			return buf;
		}
		*rc = '>';
		return sp;
	}

	if (instr(cp,"nOT FOR DIST") || instr(cp,"not for dist")) {
		*rc = '*';
		return "";
	}

	return 0;
}

char *
devspec (p)
	char *p;
{
	static char buf[20];
	register char *b = buf;
	char *s = p;
	int val;

	*b = *p++;
	if (isupper(*b)) *b = tolower(*b);
	if (*b == 'b' || *b == 'c') {
		if (*p++ != ' ') return 0;
		if ((val = devtoi(&p,s)) == -2) return "";
		b++;
		*b++ = ':';
		if (val == -1) *b++ = 'x';
		else {
			sprintf(b,"%d",val);
			while (isdigit(*b)) b++;
		}
		if (*p++ != ' ') return 0;
		*b++ = ':';
		if ((val = devtoi(&p,s)) == -2) return "";
		if (val == -1) *b++ = 'x';
		else	{
			sprintf(b,"%d",val);
			while (isdigit(*b)) b++;
		}
		p[-1] = '\0';
		*b = '\0';
		return buf;
	}
	return 0;
}

devtoi(ap, s)
	char **ap;
	char *s;
{
	int i, base, val;
	register char *p = *ap;

	if (isdigit(*p)) {
		base = 10;
		if (*p == '0') base = 8;
		val = *p++ - '0';
		for (i=0; i<4 && isdigit(*p); i++) {
			if (base == 8 && !index("01234567",*p)) break;
			val = (val * base) + (*p++ - '0');
		}
		if (val > 255 || isdigit(*p)) {
			sprintf(Err,"invalid device spec [%s]",s);
			return -2;
		}
		*ap = p;
		return val;
	}
	if (*p++ == 'x') {
		*ap = p;
		return -1;
	}
	sprintf(Err,"Unrecognized device specification[%s]",s);
	return -2;
}

/* check validity of pathname
 * returns type.
 */
chkpath(p, len)
	char *p;
{
	char *f, *dir = 0;
	static int sawmsg;
	int type;

	D(11,("chkpath <%s>\n",p));
	f = p + len;
	if (*--f == '*' && f[-1] != '\\') {
		if (*--f != '/') {
			sprintf(Err,"`*' (in path `%s') may only be used to\n\tindicate all files in a directory",p);
			return ERROR;
		}
		type = CONTENTS;
		dir = f;
		*dir = '\0';
	} else if (*f == '/') {
		type = DIRECTORY;
		dir = f;
		*dir = '\0';
	} else
		type = ITEM;

	if (cindex(p,GLOBCHRS)) {
		if (!sawmsg++)
			sprintf(Err,"Use of any of `%s' in pathnames (as in `%s')\n\tis unimplemented with the exception of `.../*'\nuse `\\' to escape glob characters", GLOBCHRS, p);
		else sprintf(Err,"Pathname `%s' contains glob characters",p);
		return ERROR;
	}
	
	if (chkpren(p)) {
		sprintf(Err,"Pathname[%s] contains unbalanced parenthesis",p);
		return ERROR;
	}

	if (dir)
		*dir = '/';
	
	return type;
}

chkpren (p)
	char *p;
{
	char *left, *right, *cindex();

	D(12,("chkpren ",p));

	left = p;
	right = p;
	while (left && *left) {
		D(12,("`%s' ",left));
		if (left = cindex(left,LPRENS)) {
			if (right = pmatch(left)) left = ++right;
			else return 1;
		} else if (right = cindex(right,RPRENS)) return 1;
	}
	D(12,("is ok\n"));
	return 0;
}

/* Given a pointer to a data specification determine if it has a checksum
 * and length and if not return the version number or null pointer.
 * If so, or if not applicable return 0.
 */
char *
needcl (p)
	char *p;
{
	static char needtab[LINESIZ];
	char **ap, **l, **divs();
	char *vp = "";
	int CF=0, L=0, V=0;

	if (xcps(needtab,p) == 0)
		return vp;
	ap = divs(needtab," ");
	for (l=ap; *l; l++) {
		switch (**l) {
		case 'C': CF = 1;
			  if (l[1] && isdigit(*l[1])) l++;
			  break;
		case 'L': L = 1;
			  if (l[1] && isdigit(*l[1])) l++;
			  break;
		case 'V': V = 1;
			  vp = *l;
			  if (!index(vp,'.') && l[1]) {
				vp = l[1] - 2;		/* convert Ver... to V*/
				*vp = 'V';
				l++;
			  }
			  break;
		case 'c':
		case 'b':
			  if (l[1] && l[2])
				return 0;
			  break;
		}
	}
	free(ap);
	if (CF != 1 || L != 1)
		return vp;
	return 0;
}

/* Find a pathname (as the take program would) using the system prefixes
 * gotten while looking up the OEM code in the /etc/takelist file.
 * Note that this function is only used when the P and a options are given.
 */
extern char *file, ***Pline;
findit (p)
	char *p;
{
	char ***lp, ***getoem();
	char *fn, *anyof();

	file = p;
	if (fn = anyof(Pline)) {
		if (*fn) {
			locferr = 0;
			locfile = fn;
		} else locferr = fn + 1;
		return 0;
	}
	return 1;
}

int fsumlen;			/* length of file just sum'd */
/* Get the data check info for a given filename
 */
char *
makvcl (vp, fn)
	char *vp, *fn;
{
	char *p, *tp, *getdesc();

	p = getdesc(fn);
	if (*p == 'V') {
		tp = index(p,' ');
		if (!strcmp(++tp,vp))
			X(("Warning: Version #[%s] in original tree invalid (should be `%s')\n",vp,tp));
	}
	return p;
}
