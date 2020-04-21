/*
 * (C) 1983 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

/* @(#)ctrl.c	1.6 */
#include "vchk.h"

char *ctrlcmds[] = {
	"ifdef",		/* classic ifdef */
	"ifndef",		/* if not defined */
	"define",		/* define word */
	"undef",		/* undefine a word */
	"else",			/* else for ifdef, ifndef, ifset, ifnset */
	"endif",		/* endif for ifdef, ifndef, ifset, ifnset */
	"include", 		/* include file or standard output */
	"exit",			/* exit vchk prematurely */
	"chdir",		/* change directory */
	"unset",		/* remove macros */
	"ifset",		/* test for definition of macro */
	"ifnset",		/* test for undefined macro */
	"setup",		/* define action */
	"end",			/* end of setup definition */
	0
};

#define IFDEF	1
#define IFNDEF	2
#define DEFINE	3
#define UNDEF	4
#define ELSE	5
#define ENDIF	6
#define INCLUDE	7
#define EXIT	8
#define CHDIR	9
#define UNSET	10
#define IFSET	11
#define IFNSET	12
#define SETUP	13
#define END	14

#define CSTKSIZ 10			/* depth of ifdef nesting */
#define NDW 60				/* max number of defined words */
#define DLEN 12				/* max sig len of defined word */

char ctrlstk[CSTKSIZ];			/* to indicate current half of ifdef */
int idlev;				/* current ifdef nesting level */
char *dexp;				/* for error messages */
int sflg;				/* flag for defexp (isdef or ismac) */

int skipping;				/* whether we are ifdef'd out or not */
					/* if so, level we def'd out at */
char deflist[NDW][DLEN];

/* Called with each control line after the leading `.' has been stripped.
 */
doctrl (s)
	char *s;
{
	char **cc, *p, *t, *index();
	struct macro *mp, *ismac();
	int rflg = 1;

	D(1,("doctrl(%s): level %d, skip=%d\n",s,idlev,skipping));
	if (p = index(s,' '))		/* lop off control command */
		*p++ = '\0';

	if (!*s) return prtdefs();	/* . alone dumps def buffer */
		
	if (p) {			/* remove possibly escaped comment */
		if (t = index(p,'#')) {
			if (t[-1] == ' ') t --;
			*t = '\0';
		}
	}
	for (cc=ctrlcmds; *cc; cc++)
		if (!strcmp(*cc,s))
			break;
	if (!*cc)
		E(("invalid control command[%s]\n",s));
	switch (cc - ctrlcmds + 1) {
	when IFNDEF:
		rflg = 0;
	case IFDEF:
		sflg = 0;		/* macro or define type expression */
		idlev++;
		if (idlev >= CSTKSIZ)
			E(("ifdef's nested too deeply[%d max]\n",CSTKSIZ));
		ctrlstk[idlev] = 1;
		if (!skipping) {
			dexp = p;
			if (rflg == !defexp(&p)) skipping = idlev;
		}
	when IFNSET:
		rflg = 0;
	case IFSET:
		sflg = 1;
		idlev++;
		if (idlev >= CSTKSIZ)
			E(("ifset's nested too deeply[%d max]\n",CSTKSIZ));
		ctrlstk[idlev] = 1;
		if (!skipping) {
			dexp = p;
			if (rflg == !defexp(&p)) skipping = idlev;
		}
	when  DEFINE:
		if (!skipping) setdef(p);
	when  UNDEF:
		if (!skipping) undef(p);
	when  ELSE:
		if (idlev == 0 || ctrlstk[idlev] != 1)
			E(("unmatched `.else' control line\n"));
		ctrlstk[idlev] = 0;		/* indicate we saw .else */
		if (skipping) {
			if (skipping == idlev) skipping = 0;
		} else skipping = idlev;
	when  ENDIF:
		if (idlev == 0)
			E(("unmatched `.endif' control line\n"));
		if (skipping == idlev) skipping = 0;
		idlev--;
	when  INCLUDE:
		if (!skipping) {
			if (*p == '!') pushfile(F_COMMAND,++p);
			else pushfile(F_FILENAME,p);
		}
	when  EXIT:
		E(("Exiting: %s\n",p));
	when  CHDIR:
		if (chdir(p))
			E(("Cannot chdir to `%s'\n",p));
	when  UNSET:
		do {
			if (t = index(p,' '))
				*t++ = '\0';
			if (mp = ismac(p)) unset(mp);
		} while (p = t);
/****
	when  SETUP:
		t = index(p,' ');
		if (!t) E(("Invalid .setup directive\n"));
	when  END:
****/
	}
	D(1,("leaving doctrl(%s): level %d, skip=%d\n",s,idlev,skipping));
}

defexp (s)
	char **s;
{
	int left, right;
	char op;
	char *p;
	int val;

	D(4,("defexp(%s) =",*s));
	val = left = defterm(s);
	if ((op = **s) != 0 && !index(RPRENS,op)) {
		(*s)++;
		right = defexp(s);
		if (op == '&') {
			val = left && right;
			goto fini;
		}
		if (op == '|') {
			val = left || right;
			goto fini;
		}
		E(("control line expression `%s' is illegal\n",dexp));
	}
fini:
	D(4,(" = %s\n",val?"true":"false"));
	return val;
}

defterm (s)
	char **s;
{
	char op;
	char buf[30];
	char *cp, *p, *pmatch();
	int val;

	D(4,("defterm(%s) =",*s));
	while ((op = **s) == ' ')
		(*s)++;

	if (index(LPRENS,op)) {
		p = pmatch(*s);
		(*s)++;
		val = defexp(s);
		*s = p+1;
		goto fini;
	}
	if (op == '!') {
		(*s)++;
		val = defterm(s);
		val = !val;
		goto fini;
	}
	cp = buf;
	for (p= *s; isalnum(*p) || *p == '_'; p++)
		*cp++ = *p;
	*cp++ = '\0';
	while (*p == ' ') p++;
	*s = p;
	val = isdef(buf);
fini:
	D(4,(" %s\n",val?"true":"false"));
	return val;
}

setdef (s)		/* set list of words */
	char *s;
{
	char *p, *index();

	D(4,("setdef(%s)\n",s));
	while (s) {
		if (p = index(s,' ')) *p++ = '\0';
		if (!isdef(s)) define(s);
		s = p;
	}
}

isdef (s)		/* is a word defined */
	char *s;
{
	int i;
	struct macro *mp;

	D(4,("isdef %s(%s) =",sflg?"macro":"define",s));

	if (sflg) {
		if (mp = ismac(s)) {
			D(4,("true(%s)\n",mp->m_val));
			return -1;
		}
	} else
		for (i=0; i<NDW; i++)
			if (strncmp(deflist[i],s,DLEN) == 0) {
				D(4,("true(%d)\n",i+1));
				return i+1;
			}
	D(4,("false\n"));
	return 0;
}

define (s)
	char *s;
{
	int i;

	for (i=0; i<NDW; i++)
		if (deflist[i][0] == 0) {
			strncpy(deflist[i],s,DLEN);
			return;
		}
	E(("Maximum of %d defined words (use .undef)\n",NDW));
}

undef (s)
	char *s;
{
	int i;

	if (i = isdef(s)) {
		deflist[--i][0] = '\0';
		return 1;
	}
	return 0;
}

prtdefs()
{
	int i;
	int flag = 0;

	fprintf(stderr,"---- control words currently defined ----\n");
	for (i=0; i<NDW; i++)
		if (*deflist[i]) {
			flag++;
			fprintf(stderr,"\t%s",deflist[i]);
		}
	if (flag) fprintf(stderr,"\n");
}
