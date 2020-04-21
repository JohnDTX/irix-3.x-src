/*
 * (C) 1983 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

/* @(#)entry.c	1.6 */
#include "vchk.h"

int lnexflg;			/* flag to tell Pexpand not to gen any keys */
int lnsearch;			/* flag that we started link search (Chkit)*/
int errflag;			/* had reportable error in some parse phase */
int mustdef;			/* whether getline detected an = in the line */
int musteval;			/* whether getline detected a $ in the line */
char *rest;			/* line past filename */

char *
getspec ()
{
	char *f, *vp, *cp, *getline();

	do {
		if ((cp = getline()) == (char *)0)
			return (char *)0;
		if (!*cp) {
			if (Pflag && lastnl == 0) {
				putc('\n',stdout);
				lastnl = 1;
			}
			continue;
		}
		Curtype = ERROR;
		if (musteval && mustdef) while (eval(cp,1)) ;
		preprocess(cp);
	} while (notspec(cp));

	return cp;
}

notspec (s)
	char *s;
{
	char *p = s;
	int rv = 0;

	if (errflag) {
		X(("(macro expansion): %s\n",Err));
		rv = 1;
		errflag = 0;
	}
	if (skipping || Iflag) return 1;

	if (s != 0 && *s != '\0') {
		if (*p++ == '#') {
			if (*p++ == '#') {
				if (Pflag) {
					rv = strlen(p);
					rv = (LINE_LEN + rv)/2;
					printf("##%*s\n", rv, p);
				} else M(("\t%s\n",p));
			} else if (Pflag) printf("#%s\n",p-1);
			rv = 1;
			lastnl = 0;
		} else {
			if (musteval)
				while (eval(s,0))
					;
			if (!errflag) {
				if (mustdef) preprocess(s);
				Curtype = spectype(s);
			}
		}
	} else
		rv = 1;
	if (errflag) {
		X(("(preprocessing): %s\n",Err));
		rv = 1;
		errflag = 0;
	}
	return rv;
}

preprocess (s)
	char *s;
{
	char *vp;

	if (*s == '.' && s[1] != '/') {		/* control line */
		*s++ = '\0';
		eval(s,0);
		doctrl(s);
		return;
	}

	if (skipping) {
		*s = '\0';
		return;
	}

	if (!mustdef) return;

	for (vp=s; *vp; vp++)
		if (*vp == '=') {
			if (vp[-1] == ' ') vp[-1] = '\0';
			*vp++ = '\0';
			if (*vp == ' ') *vp++ = '\0';
			setmacro(s,vp);
			*s = '\0';
			return;
		}
	errflag = 1;
	sprintf(Err,"Invalid macro definition `%s'",s);
	return;
}

/* Check one input line.  Macros have been expanded.
 * We must expand (), {}, parse specifications and do checking.
 */
chkentry (cp)
	char *cp;
{
	char *f = cp;
	int dsrch(), doit();
	short i, v;
	char *rep, *getstr();

	if (*cp == '#' || !*cp) return;

	D(3,("IN chkentry <%s, %s>\n",cp,typename(Curtype)));

	while (*cp && *cp != ' ')
		cp++;
	if (*cp) *cp++ = '\0';	/* delimit pathname */

	if (*cp) rest = cp;	/* break off description */
	else rest = 0;

	/* At this point `f' points to the complex pathname and
	 * the global `rest' points at the rest of the line.
	 */

	Curdir = first_node;	/* start search at top of tree */
	initpath();
	pexpand(f,doit);	/* call doit with each path f gens */
	if (Curtype == IGNORE) return;

	D(5,("chkentry: back from pexpand (%d paths)\n",nxtkey-keytab-1));
	for (i=1; nxtkey>&keytab[i]; i++) {
		rep = getstr(reptab[i],0);
		if (!keytab[i].k_chk) {	/* all were missing, so try first */
			if (cp = getstr(keytab[i].k_mis,1)) {	/* get best */
				cp = chrbp + cps(chrbp,cp);
				dsrch(chrbp);
				if (Curtype == ITEM) {
					if (rest) {
						*cp++ = ' ';
						cps(cp,rest);
					}
					sibuf.st_ino = 0;
					lnexflg = 1;
					chkit(chrbp);
					lnexflg = 0;
				}
			}
		}
		D(6,("chkentry: looking for dups or alts for path %d: `%s'\n",i,rep));
		if (v = keytab[i].k_dup)
			putlist(v,"duplicates",rep);
		if (v = keytab[i].k_alt)
			putlist(v,"other versions",rep);
	}
	D(5,("Leaving chkentry <%s>\n",f));
}

/* Called via pexpand with cp pointing to the final, fully expanded pathname.
 * It's job is to check for it's existance.
 */
doit (cp, kp)
	char *cp;
	struct key *kp;
{
	char *p, *fn;
	short v;
	char *getstr();
	short getkey();
	char buf[LINESIZ];
	static int rrmsg;

	if (!kp) {
		if (getkey(cp) == 0) return -1;
		kp = extkey;
	}

#ifdef BUGCHECK
	if (kp == (struct key *)0)
		E(("Logic error: didn't get key struct\n"));
#endif BUGCHECK

	D(3,("doit <%s, %d, %s>\n",cp,kp-keytab,typename(Curtype)));

	if (stat(realname(cp),&sibuf)) {
		addpath(cp,&kp->k_mis);
		return -1;
	}

	lnsearch = 0;
	if (((v = sibuf.st_nlink) > 1) && (Curtype == ITEM)) {
		if ((v = chklink(cp,sibuf.st_ino,sibuf.st_dev,v)) <= 1)
			lnsearch = v ? 1 : -1;
	}

	if (dsrch(cp) == -1 && Curtype == ITEM)		/* establish path */
		return -1;
	
	if (Curtype == IGNORE)		/* -e option */
		return -1;

	if (Curtype == ITEM) {
		if (Curdir->t_ls) {
			if (eliminate(fname(cp),Curdir) == 0) {
				X(("%s has already been checked --- inspect tree file\n",cp));
				if (rrmsg++ == 0)
					M(("\tconsequently spurious errors may result\n"));
				return -1;
			}
		}
		if (v = kp->k_chk) {	/* checked one that existed already */
			fn = getstr(v,1);
			if (samefile(cp,fn)) {/* is exactly the same */
				addpath(cp,&kp->k_chk);
				return -1;
			}
			if (!mflag) {
				addpath(cp,&kp->k_alt);
				return -1;
			}
		}
		p = buf + cps(buf,cp);		/* save one we have checked */
		if (rest) {			/* add arguments for Chkit */
			*p++ = ' ';
			cps(p,rest);
		}
	}

	v = -1;
	if (addpath(cp,&kp->k_chk) && Curtype == ITEM) {
		lnexflg = 1;			/* tell pexpand no new key */
		if (!mflag && !chkit(buf)) v = 0;
		lnexflg = 0;
	}
	return v;
}

char *
fixpath (cp)
	char *cp;
{
	register char *f = cp;
	int dir;
	static char buf[PATHSIZ];

	if (!f || (f[0] == '/' && f[1] == '\0')) return f;
	cps(buf,f);
	cp = f = buf;
	D(8,("In fixpath(%s) returning ",buf));
	while (*f) f++;
	if (f[-1] == '/') {
		dir = 1;
		*--f = '\0';
	} else dir = 0;
	minpath(cp);
	f = cp;
	while (*f) f++;
	if (dir) {
		*f++ = '/';
		*f = '\0';
	}
	D(8,("`%s'\n",buf));
	return buf;
}
