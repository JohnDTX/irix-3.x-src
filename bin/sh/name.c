/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 */

/*	@(#)name.c	1.2	*/
/*	3.0 SID #	1.2	*/
static	char	*Sccsid = "@(#)$Header: /d2/3.7/src/bin/sh/RCS/name.c,v 1.1 89/03/27 14:55:46 root Exp $";
/*
 * $Log:	name.c,v $
 * Revision 1.1  89/03/27  14:55:46  root
 * Initial check-in for 3.7
 * 
 * Revision 1.2  85/04/18  12:01:56  bob
 * Fixed SCR #717 regarding the "read" command without parameters caused the
 * shell to core dump.
 * 
 */

#include	"defs.h"

PROC BOOL	chkid();



NAMNOD	ps2nod	= {	(NAMPTR) NIL,	(NAMPTR) NIL,	ps2name},
	cdpnod = {	(NAMPTR) NIL,	(NAMPTR) NIL,	cdpname},
	fngnod	= {	&cdpnod,	(NAMPTR) NIL,	fngname},
	pathnod = {	(NAMPTR) NIL,	(NAMPTR) NIL,	pathname},
	ifsnod	= {	(NAMPTR) NIL,	(NAMPTR) NIL,	ifsname},
	ps1nod	= {	&pathnod,	&ps2nod,	ps1name},
	homenod = {	&fngnod,	&ifsnod,	homename},
	mailnod = {	&homenod,	&ps1nod,	mailname};

NAMPTR		namep = &mailnod;


/* ========	variable and string handling	======== */

syslook(w,syswds)
	STRING		w;
	SYSTAB		syswds;
{
	REG CHAR	first;
	REG STRING	s;
	REG SYSPTR	syscan;

	syscan=syswds; first = *w;
	IF *w == 0 THEN return(0); FI

	WHILE s=syscan->sysnam
	DO  IF first == *s
		ANDF eq(w,s)
	    THEN return(syscan->sysval);
	    FI
	    syscan++;
	OD
	return(0);
}

setlist(arg,xp)
	REG ARGPTR	arg;
	INT		xp;
{
	WHILE arg
	DO REG STRING	s=mactrim(arg->argval);
	   setname(s, xp);
	   arg=arg->argnxt;
	   IF flags&execpr
	   THEN prs(s);
		IF arg THEN blank(); ELSE newline(); FI
	   FI
	OD
}


VOID	setname(argi, xp) /* does parameter assignments */
	STRING		argi;
	INT		xp;
{
	INT	rsflag=1;	/* local restricted flag */
	STRING sim;
	REG STRING	argscan=argi;
	REG NAMPTR	n;

	IF letter(*argscan)
	THEN	WHILE alphanum(*argscan) DO argscan++ OD
		IF *argscan=='='
		THEN	*argscan = 0; /* make name a cohesive string */

#ifndef RES	/*	restricted stuff excluded from research */
		IF eq(argi, "SHELL") ANDF !(flags&rshflg)
		THEN 	*argscan++;
			IF any('r', sim=(STRING) simple(argscan))
			THEN	rsflag=0;	/* restricted shell */
			FI
			*argscan--;
		FI
		IF eq(argi,pathname) ANDF (flags&rshflg)   /* cannot set PATH */
		THEN failed(argi,restricted);
		ELIF eq(argi, "SHELL") ANDF (flags&rshflg)
		THEN	failed(argi, restricted);
		ELSE
#endif
			n=lookup(argi);
			*argscan++ = '=';
			attrib(n, xp);
			IF xp&N_ENVNAM
			THEN	n->namenv = n->namval = argscan;
			ELSE	assign(n, argscan);
			FI
			return(rsflag);
#ifndef RES	/*	end of restricted IF	*/
		FI
#endif
		FI
	FI
	failed(argi,notid);
}

replace(a, v)
	REG STRING	*a;
	STRING		v;
{
	free(*a); *a=make(v);
}

dfault(n,v)
	NAMPTR		n;
	STRING		v;
{
	IF n->namval==0
	THEN	assign(n,v)
	FI
}

assign(n,v)
	NAMPTR		n;
	STRING		v;
{
	IF n->namflg&N_RDONLY
	THEN	failed(n->namid,wtfailed);
	ELSE	replace(&n->namval,v);
	FI
}

INT	readvar(names)
	STRING		*names;
{
	FILEBLK		fb;
	REG FILE	f = &fb;
	REG CHAR	c;
	REG INT		rc=0;
	NAMPTR		n=0;
	STKPTR		rel=(STKPTR) relstak();


	if (*names)
		n=lookup(*names++);

	push(f); initf(dup(0));

	IF lseek(0,0L,1)==-1
	THEN	f->fsiz=1;
	FI


	/*	strip leading IFS characters	*/
	WHILE	(any((c=nextc(0)), ifsnod.namval)) ANDF !(eolchar(c)) DONE
	LOOP
		IF (*names ANDF any(c, ifsnod.namval)) ORF eolchar(c)
		THEN	zerostak();
			if (n)
				assign(n,absstak(rel));
			setstak(rel);
			IF *names
			THEN	n=lookup(*names++);
			ELSE	n=0;
			FI
			IF eolchar(c)
			THEN	break;
			ELSE	/*	strip imbedded IFS characters 	*/
			 	WHILE (any((c=nextc(0)), ifsnod.namval)) ANDF
					!(eolchar(c)) DONE
			FI
		ELSE	pushstak(c);
			c=nextc(0);
			IF eolchar(c)
			THEN
				STKPTR top=staktop;
				while(any(*(--top), ifsnod.namval));;
				staktop = top+1;
			FI
		FI
	POOL
	WHILE n
	DO assign(n, nullstr);
	   IF *names THEN n=lookup(*names++); ELSE n=0; FI
	OD

	IF eof THEN rc=1 FI
	lseek(0, (long)(f->fnxt-f->fend), 1);
	pop();
	return(rc);
}

assnum(p, i)
	STRING		*p;
	INT		i;
{
	itos(i); replace(p,numbuf);
}

STRING	make(v)
	STRING		v;
{
	REG STRING	p;

	IF v
	THEN	movstr(v,p=alloc(length(v)));
		return(p);
	ELSE	return(0);
	FI
}


NAMPTR		lookup(nam)
	REG STRING	nam;
{
	REG NAMPTR	nscan=namep;
	REG NAMPTR	*prev;
	INT		LR;


	IF !chkid(nam)
	THEN	failed(nam,notid);
	FI
	WHILE nscan
	DO	IF (LR=cf(nam,nscan->namid))==0
		THEN	return(nscan);
		ELIF LR<0
		THEN	prev = &(nscan->namlft);

		ELSE	prev = &(nscan->namrgt);
		FI
		nscan = *prev;
	OD

	/* add name node */
	nscan=(NAMPTR) alloc(sizeof *nscan);
	nscan->namlft=nscan->namrgt=(NAMPTR) NIL;
	nscan->namid=make(nam);
	nscan->namval=0; nscan->namflg=N_DEFAULT; nscan->namenv=0;

	return(*prev = nscan);
}

LOCAL BOOL	chkid(nam)
	STRING		nam;
{
	REG CHAR *	cp=nam;


	IF !letter(*cp)
	THEN	return(FALSE);
	ELSE	WHILE *++cp
		DO IF !alphanum(*cp)
		   THEN	return(FALSE);
		   FI
		OD
	FI
	return(TRUE);
}

LOCAL VOID (*namfn)();
namscan(fn)
	VOID		(*fn)();
{
	namfn=fn;
	namwalk(namep);
}

LOCAL VOID	namwalk(np)
	REG NAMPTR	np;
{
	IF np
	THEN	namwalk(np->namlft);
		(*namfn)(np);
		namwalk(np->namrgt);
	FI
}

VOID	printnam(n)
	NAMPTR		n;
{
	REG STRING	s;

	sigchk();
	IF s=n->namval
	THEN	prs(n->namid);
		prc('='); prs(s);
		newline();
	FI
}

LOCAL STRING	staknam(n)
	REG NAMPTR	n;
{
	REG STRING	p;

 		/* 2 lines added to handle mem faults */
 	while (((int)staktop+length(n->namid)+length(n->namval)+1) > (int)brkend)
 		setbrk(brkincr);
	p=movstr(n->namid,staktop);
	p=movstr("=",p);
	p=movstr(n->namval,p);
	return(getstak(p+1-ADR(stakbot)));
}

VOID	exname(n)
	REG NAMPTR	n;
{
	IF n->namflg&N_EXPORT
	THEN	free(n->namenv);
		n->namenv = make(n->namval);
	ELSE	free(n->namval);
		n->namval = make(n->namenv);
	FI
}

VOID	printflg(n)
	REG NAMPTR		n;
{
	IF n->namflg&N_EXPORT
	THEN	prs(export); blank();
	FI
	IF n->namflg&N_RDONLY
	THEN	prs(readonly); blank();
	FI
	IF n->namflg&(N_EXPORT|N_RDONLY)
	THEN	prs(n->namid); newline();
	FI
}

VOID	getenv()
{
	INT	rsflag=1;	/* local restricted flag */
	REG STRING	*e=environ;

	WHILE *e
	DO	rsflag=setname(*e++, N_ENVNAM) & rsflag OD
	return(rsflag);
}

LOCAL INT	namec;

VOID	countnam(n)
	NAMPTR		n;
{
	namec++;
}

LOCAL STRING 	*argnam;

VOID	pushnam(n)
	NAMPTR		n;
{
	IF n->namval
	THEN	*argnam++ = staknam(n);
	FI
}

STRING	*setenv()
{
	REG STRING	*er;

	namec=0;
	namscan(countnam);
	argnam = er = (STRING *) getstak(namec*BYTESPERWORD+BYTESPERWORD);
 		/* 2 lines added to handle mem faults */
 	while (((int)argnam + namec*BYTESPERWORD+BYTESPERWORD) > (int)brkend)
 		setbrk(brkincr);
	namscan(pushnam);
	*argnam++ = 0;
	return(er);
}
