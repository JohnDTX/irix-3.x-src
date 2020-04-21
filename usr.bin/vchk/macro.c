/*
 * (C) 1983 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

/* @(#)macro.c	1.6 */
#include "vchk.h"

extern int errflag;			/* had error during eval */

extern struct macro *first_mac;
struct macro *ismac();

setmacro (m, v)
	char *m, *v;
{
	register struct macro *p, *l;
	struct macro *pm;
	int dif, len;
	static *nullarg[] = { 0 };
	char *cp, **ap, **divs(), *pindex();
	extern char *REMKMAC, *Takecmd;

	if (strcmp(m,"ARGS") == 0) {
		cp = v + strlen(v);
		*cp++ = ' ';
		*cp++ = '\0';
		if ((ap = divs(v," ")) == 0) {
			args(1, 0, nullarg);
			return;
		}
		for (len=0; ap[len]; len++)
			;
		args(1,len,ap);
		free(ap);
		return;
	}

	if (p = ismac(m)) unset(p);

	if (cp = pindex(v,'#')) {
		if (cp > v && cp[-1] == ' ') cp--;
		*cp = '\0';
	}

	len = sizeof (struct macro) + strlen(m) + strlen(v) + 2;
	p = (struct macro *) getmem(len,1,"macro",m);

	D(4,("setmacro(%s to `%s') uses %d bytes\n",m,v,len));

	p->m_val = p->m_name + cps(p->m_name,m) + 1;
	cps(p->m_val,v);
	if (strcmp(m,REMKMAC) == 0) Takecmd = p->m_val;
	pm = (struct macro *)0;
	for (l=first_mac; l != (struct macro *)0; l=l->m_next) {
		if ((dif = strcmp(l->m_name,m)) > 0)
			break;
		pm = l;
	}
	if (first_mac == (struct macro *)0 || l == first_mac) {
		p->m_prev = (struct macro *)0;
		if (p->m_next = first_mac)
			l->m_prev = p;
		first_mac = p;
	} else {
		p->m_next = pm->m_next;
		pm->m_next = p;
		p->m_prev = pm;
		if (p->m_next) p->m_next->m_prev = p;
	}
#ifdef DEBUG
	if (Dflag >= 19) prtmacs();
#endif
}

unset (p)
	struct macro *p;
{
	extern char *REMKMAC, *Takecmd;

	if (strcmp(REMKMAC,p->m_name) == 0)
		Takecmd = 0;
	if (p->m_prev) p->m_prev->m_next = p->m_next;
	else first_mac = p->m_next;
	if (p->m_next) p->m_next->m_prev = p->m_prev;
	free(p);
}

prtmacs ()
{
	register struct macro *m;

	fprintf(stderr,"-- Currently defined macros:\n");
	for (m=first_mac; m; m=m->m_next)
		fprintf(stderr,"%s=%s\n",m->m_name,m->m_val);
	fprintf(stderr,"--\n");
}

struct macro *
ismac (n)
	char *n;
{
	register struct macro *m = first_mac;
	int v;

	while (m) {
		if ((v = strcmp(n,m->m_name)) == 0) return m;
		if (v < 0) break;
		m = m->m_next;
	}

	return (struct macro *) 0;
}

char *
explink (cp)
	char *cp;
{
	static char buf[LINESIZ];
	char *sp = cp;
	char *dp, *bp = buf, *pwd();
	int err = 0;

	D(9,("in explink(%s): x%x",cp,Curdir));
	while (*cp && (*cp == ' ' || *cp == '\t'))
		cp++;
	while (*cp) {
		if (*cp == '$') {
			if (*++cp == '.') {
				cp++;
				dp = pwd();
#ifdef BUGCHECK
				if (dp == 0)
					E(("Logic: no directory\n"));
#endif BUGCHECK
				while (*dp && bp < buf+sizeof(buf)-1)
					*bp++ = *dp++;
				if (*dp) err = 1;
			} else {
				if (bp < buf+sizeof(buf)-1) *bp++ = '$';
				else err = 1;
			}
		}
		if (*cp && (bp < buf+sizeof(buf)-1)) *bp++ = *cp++;
		else err = 1;
		if (err) {
			X(("Overflow expanding %s\n",sp));
			return "";
		}
	}

	*bp = '\0';
	D(9,("to `%s'\n",buf));
	return buf;
}

char mac_buf[LINESIZ];
char *mac_ptr;

addmbuf (c)
	char c;
{
	if (mac_ptr >= mac_buf+sizeof(mac_buf)-1) {
		sprintf(Err,"macro[%s] expansion yeilds excessively long line",
			mac_buf);
		errflag = 1;
		return 0;
	}
	*mac_ptr++ = c;
	return 1;
}

eval (cp, flg)
	char *cp;
{
	char nbuf[50], *np, *rp;
	int spcflag=0, retflag = 0, mustredo, first_col;
	char *sp = cp;
	struct macro *mp;
	extern int mustdef;

	mustdef = 0;
	if (!*cp) return 0;
	D(10,("In eval `%s'\n",cp));
	mac_ptr = mac_buf;
	first_col = 1;
	while (*cp) {
		spcflag = 0;
		switch (*cp) {
		when '#':
			if (cp[1] != '#') goto fini;
			if (!addmbuf('#')) return 0;
			cp++;
			if (!addmbuf('#')) return 0;
		when '\\':
			if (/* flg &&*/ !addmbuf('\\'))
				return 0;
			if (!*++cp) {
				sprintf(Err,"trailing backslash");
				errflag = 1;
				return 0;
			}
		default:
			if (*cp == '=') mustdef = 1;
			if (!addmbuf(*cp))
				return 0;
		when '$':
			if (first_col) {
				if (cp[1] == '\0') {
					prtmacs();
					*cp = '\0';
					return 0;
				}
			}
			rp = (char *)0;
			if (*++cp == LPREN) {
				if ((rp = pmatch(cp)) == 0) {
					sprintf(Err,"missing matching `%c' for macro invocation",*cp);
					errflag = 1;
					return 0;
				}
				*rp = '\0';
				cp++;
			}
			np = nbuf;
			if (!isalnum(*cp)) spcflag = 1;
			while (*cp) {
				if (np >= nbuf+sizeof(nbuf)) {
					*--np = '\0';
					sprintf(Err,"macro name [%s...] too long",nbuf);
					errflag = 1;
					return 0;
				}
				*np++ = *cp++;
				if (!rp) break;	/* stop if no ()'s */
			}
			*np = '\0';
			if (np == nbuf || (rp && *cp)) {
				sprintf(Err,"invalid macro invokation[%s]",
					cp - strlen(nbuf));
				errflag = 1;
				return 0;
			}
			if (rp) cp = rp;
			else cp--;
			if (mp = ismac(nbuf)) {		/* can expand */
				D(12,("got mac[%s] = `%s'\n",nbuf,mp->m_val));
				retflag++;
				np = mp->m_val;
				mustredo = 0;
				while (*np) {
					if (*np == '$') mustredo = 1;
					if (!addmbuf(*np++))
						return 0;
				}
				if (mustredo == 0) retflag--;
			} else {			/* undefined just now */
				if (spcflag || flg) {	/* can wait */
					if (!addmbuf('$')) return 0;
					if (nbuf[1] == '\0') {
						if (!addmbuf(nbuf[0])) return 0;
					} else {
						if (!addmbuf(LPREN)) return 0;
						for (np=nbuf; *np; np++)
							if (!addmbuf(*np))
								return 0;
						if (!addmbuf(RPREN)) return 0;
					}
					goto safe;
				}
				sprintf(Err,"macro `%s' undefined",nbuf);
				errflag = 1;
				return 0;
			}
		}
	safe:
		cp++;
		first_col = 0;
	}
fini:
	addmbuf(0);
	sp += cps(sp,mac_buf);
	while (*--sp == ' ') *sp = '\0';
	if (mac_buf[0] == '.' && mac_buf[1] != '/') mustdef = 1;
	D(13,("eval %sdone; returning `%s'\n",retflag?"not ":"",mac_buf));
	return retflag;
}
