/*	@(#)sh.exp.c	2.1	SCCS id keyword	*/
/* Copyright (c) 1979 Regents of the University of California */
static	char	*Sccsid = "@(#)$Header: /d2/3.7/src/bin/csh/RCS/sh.exp.c,v 1.1 89/03/27 14:53:20 root Exp $";
/*
 * $Log:	sh.exp.c,v $
 * Revision 1.1  89/03/27  14:53:20  root
 * Initial check-in for 3.7
 * 
 * Revision 1.7  87/11/17  17:08:14  donl
 * ifdefed-out an extraneous printf "*vp='%s'" for missing file names on
 * a -<letter> test.  - donl
 * 
 * Revision 1.7  87/11/17  17:06:09  donl
 * Removed extraneous printf with ifdefs: "**vp='%s'".  SCR 1865.  - donl
 * 
 * Revision 1.6  86/07/27  15:48:22  paulm
 * Check in a bug fix of Donovan's.
 * 
 * Revision 1.5  85/06/19  13:25:58  bob
 * Fixed the bug whereby 'if ( -o / ) echo root' failed because slash was
 * interpreted as the divide operator.
 * 
 * Revision 1.4  85/01/16  23:29:31  bob
 * Fixed so that right op of =~ and !~ isn't wildcard expanded.
 * 
 */
#include "sh.h"

/*
 * C shell
 */

#define	IGNORE	1	/* in ignore. it means to ignore value, just parse */
#define	NOGLOB	2	/* in ignore. it means not to globone */
#define	ADDOP	1
#define	MULOP	2
#define	EQOP	4
#define	RELOP	8
#define	RESTOP	16
#define	ANYOP	31

#define	EQEQ	1
#define	GTR	2
#define	LSS	4
#define	NOTEQ	6
#define	EQMATCH	7
#define	NOTEQMATCH 8

exp(vp)
	register char ***vp;
{

	return (exp0(vp, 0));
}

exp0(vp, ignore)
	register char ***vp;
	bool ignore;
{
	register int p1 = exp1(vp, ignore);
	
#ifdef EDEBUG
	etraci("exp0 p1", p1, vp);
#endif
	if (**vp && eq(**vp, "||")) {
		register int p2;

		(*vp)++;
		p2 = exp0(vp, (ignore&IGNORE) || p1);
#ifdef EDEBUG
		etraci("exp0 p2", p2, vp);
#endif
		return (p1 || p2);
	}
	return (p1);
}

exp1(vp, ignore)
	register char ***vp;
{
	register int p1 = exp2(vp, ignore);

#ifdef EDEBUG
	etraci("exp1 p1", p1, vp);
#endif
	if (**vp && eq(**vp, "&&")) {
		register int p2;

		(*vp)++;
		p2 = exp1(vp, (ignore&IGNORE) || !p1);
#ifdef EDEBUG
		etraci("exp1 p2", p2, vp);
#endif
		return (p1 && p2);
	}
	return (p1);
}

exp2(vp, ignore)
	register char ***vp;
	bool ignore;
{
	register int p1 = exp2a(vp, ignore);

#ifdef EDEBUG
	etraci("exp3 p1", p1, vp);
#endif
	if (**vp && eq(**vp, "|")) {
		register int p2;

		(*vp)++;
		p2 = exp2(vp, ignore);
#ifdef EDEBUG
		etraci("exp3 p2", p2, vp);
#endif
		return (p1 | p2);
	}
	return (p1);
}

exp2a(vp, ignore)
	register char ***vp;
	bool ignore;
{
	register int p1 = exp2b(vp, ignore);

#ifdef EDEBUG
	etraci("exp2a p1", p1, vp);
#endif
	if (**vp && eq(**vp, "^")) {
		register int p2;

		(*vp)++;
		p2 = exp2a(vp, ignore);
#ifdef EDEBUG
		etraci("exp2a p2", p2, vp);
#endif
		return (p1 ^ p2);
	}
	return (p1);
}

exp2b(vp, ignore)
	register char ***vp;
	bool ignore;
{
	register int p1 = exp2c(vp, ignore);

#ifdef EDEBUG
	etraci("exp2b p1", p1, vp);
#endif
	if (**vp && eq(**vp, "&")) {
		register int p2;

		(*vp)++;
		p2 = exp2b(vp, ignore);
#ifdef EDEBUG
		etraci("exp2b p2", p2, vp);
#endif
		return (p1 & p2);
	}
	return (p1);
}

exp2c(vp, ignore)
	register char ***vp;
	bool ignore;
{
	register char *p1 = exp3(vp, ignore);
	register char *p2;
	register int i;

#ifdef EDEBUG
	etracc("exp2c p1", p1, vp);
#endif
	if (i = isa(**vp, EQOP)) {
		(*vp)++;
		if (i == EQMATCH || i == NOTEQMATCH)
			ignore |= NOGLOB;
		p2 = exp3(vp, ignore);
#ifdef EDEBUG
		etracc("exp2c p2", p2, vp);
#endif
		if (!(ignore&IGNORE)) switch (i) {

		case EQEQ:
			i = eq(p1, p2);
			break;

		case EQMATCH:
			i = Gmatch(p1, p2);
			break;

		case NOTEQMATCH:
			i = !Gmatch(p1, p2);
			break;

		case NOTEQ:
			i = !eq(p1, p2);
			break;
		}
		xfree(p1), xfree(p2);
		return (i);
	}
	i = egetn(p1);
	xfree(p1);
	return (i);
}

char *
exp3(vp, ignore)
	register char ***vp;
	bool ignore;
{
	register char *p1, *p2;
	register int i;

	p1 = exp3a(vp, ignore);
#ifdef EDEBUG
	etracc("exp3 p1", p1, vp);
#endif
	if (i = isa(**vp, RELOP)) {
		(*vp)++;
		if (**vp && eq(**vp, "="))
			i |= 1, (*vp)++;
		p2 = exp3(vp, ignore);
#ifdef EDEBUG
		etracc("exp3 p2", p2, vp);
#endif
		if (!(ignore&IGNORE)) switch (i) {

		case GTR:
			i = egetn(p1) > egetn(p2);
			break;

		case GTR|1:
			i = egetn(p1) >= egetn(p2);
			break;

		case LSS:
			i = egetn(p1) < egetn(p2);
			break;

		case LSS|1:
			i = egetn(p1) <= egetn(p2);
			break;
		}
		xfree(p1), xfree(p2);
		return (putn(i));
	}
	return (p1);
}

char *
exp3a(vp, ignore)
	register char ***vp;
	bool ignore;
{
	register char *p1, *p2, *op;
	register int i;

	p1 = exp4(vp, ignore);
#ifdef EDEBUG
	etracc("exp3a p1", p1, vp);
#endif
	op = **vp;
	if (op && any(op[0], "<>") && op[0] == op[1]) {
		(*vp)++;
		p2 = exp3a(vp, ignore);
#ifdef EDEBUG
		etracc("exp3a p2", p2, vp);
#endif
		if (op[0] == '<')
			i = egetn(p1) << egetn(p2);
		else
			i = egetn(p1) >> egetn(p2);
		xfree(p1), xfree(p2);
		return (putn(i));
	}
	return (p1);
}

char *
exp4(vp, ignore)
	register char ***vp;
	bool ignore;
{
	register char *p1, *p2;
	register int i = 0;

	p1 = exp5(vp, ignore);
#ifdef EDEBUG
	etracc("exp4 p1", p1, vp);
#endif
	if (isa(**vp, ADDOP)) {
		register char *op = *(*vp)++;

		p2 = exp4(vp, ignore);
#ifdef EDEBUG
		etracc("exp4 p2", p2, vp);
#endif
		if (!(ignore&IGNORE)) switch (op[0]) {

		case '+':
			i = egetn(p1) + egetn(p2);
			break;

		case '-':
			i = egetn(p1) - egetn(p2);
			break;
		}
		xfree(p1), xfree(p2);
		return (putn(i));
	}
	return (p1);
}

char *
exp5(vp, ignore)
	register char ***vp;
	bool ignore;
{
	register char *p1, *p2;
	register int i = 0;

	p1 = exp6(vp, ignore);
#ifdef EDEBUG
	etracc("exp5 p1", p1, vp);
#endif
	if (isa(**vp, MULOP)) {
		register char *op = *(*vp)++;

		p2 = exp5(vp, ignore);
#ifdef EDEBUG
		etracc("exp5 p2", p2, vp);
#endif
		if (!(ignore&IGNORE)) switch (op[0]) {

		case '*':
			i = egetn(p1) * egetn(p2);
			break;

		case '/':
			i = egetn(p2);
			if (i == 0)
				error("Divide by 0");
			i = egetn(p1) / i;
			break;

		case '%':
			i = egetn(p2);
			if (i == 0)
				error("Mod by 0");
			i = egetn(p1) % i;
			break;
		}
		xfree(p1), xfree(p2);
		return (putn(i));
	}
	return (p1);
}

char *
exp6(vp, ignore)
	register char ***vp;
{
	int ccode, i;
	register char *cp, *dp, *ep;

	if (eq(**vp, "!")) {
		(*vp)++;
		cp = exp6(vp, ignore);
#ifdef EDEBUG
		etracc("exp6 ! cp", cp, vp);
#endif
		i = egetn(cp);
		xfree(cp);
		return (putn(!i));
	}
	if (eq(**vp, "~")) {
		(*vp)++;
		cp = exp6(vp, ignore);
#ifdef EDEBUG
		etracc("exp6 ~ cp", cp, vp);
#endif
		i = egetn(cp);
		xfree(cp);
		return (putn(~i));
	}
	if (eq(**vp, "(")) {
		(*vp)++;
		ccode = exp0(vp, ignore);
#ifdef EDEBUG
		etraci("exp6 () ccode", ccode, vp);
#endif
		if (*vp == 0 || **vp == 0 || ***vp != ')')
			bferr("Expression syntax");
		(*vp)++;
		return (putn(ccode));
	}
	if (eq(**vp, "{")) {
		int pid;
		register char **v;

		(*vp)++;
		v = *vp;
		for (;;) {
			if (!**vp)
				bferr("Missing }");
			if (eq(*(*vp)++, "}"))
				break;
		}
		if (ignore&IGNORE)
			return ("");
		pid = fork();
		if (pid < 0)
			Perror("fork");
		if (pid == 0) {
			if (setintr)
				signal(SIGINT, SIG_DFL);
			*--(*vp) = 0;
			evalav(v);
			exitstat();
		}
		cadd(pid, "{}");
		pwait(pid);
#ifdef EDEBUG
		etraci("exp6 {} status", egetn(value("status")), vp);
#endif
		return (putn(egetn(value("status")) == 0));
	}
	if (isa(**vp, ANYOP))
		return ("");
	cp = *(*vp)++;
	if (*cp == '-' && any(cp[1], "erwxfdzocbpugkst")) {
		struct stat stb;

				/* special-case for file "/" (not divide) */
		if (isa(**vp, ANYOP) && ***vp != '/') {
#ifdef notdef
			printf("**vp='%s'\n",**vp);
#endif
			bferr("Missing file name");
		}
		dp = *(*vp)++;
		if (ignore)
			return ("");
		ep = globone(dp);
		switch (cp[1]) {

		case 'r':
			i = !access(ep, 4);
			break;

		case 'w':
			i = !access(ep, 2);
			break;

		case 'x':
			i = !access(ep, 1);
			break;

		case 't':
			i = isatty(egetn(ep));
			break;

		default:
			if (stat(ep, &stb)) {
				xfree(ep);
				return ("0");
			}
			switch (cp[1]) {

			case 'f':
				i = (stb.st_mode & S_IFMT) == S_IFREG;
				break;

			case 'd':
				i = (stb.st_mode & S_IFMT) == S_IFDIR;
				break;

			case 'c':
				i = (stb.st_mode & S_IFMT) == S_IFCHR;
				break;

			case 'b':
				i = (stb.st_mode & S_IFMT) == S_IFBLK;
				break;

			case 'p':
				i = (stb.st_mode & S_IFMT) == S_IFIFO;
				break;

			case 'u':
				i = (stb.st_mode & S_ISUID);
				break;

			case 'g':
				i = (stb.st_mode & S_ISGID);
				break;

			case 'k':
				i = (stb.st_mode & S_ISVTX);
				break;

			case 's':
				i = stb.st_size != 0;
				break;

			case 'z':
				i = stb.st_size == 0;
				break;

			case 'e':
				i = 1;
				break;

			case 'o':
				i = stb.st_uid == uid;
				break;
			}
		}
#ifdef EDEBUG
		etraci("exp6 -? i", i, vp);
#endif
		xfree(ep);
		return (putn(i));
	}
#ifdef EDEBUG
	etracc("exp6 default", cp, vp);
#endif
	return (ignore&NOGLOB ? savestr(strip(cp)) : globone(cp));
}

evalav(v)
	register char **v;
{
	struct wordent paraml;
	register struct wordent *hp = &paraml;
	struct command *t;
	register struct wordent *wdp = hp;
	
	child++;
	set("status", "0");
	hp->prev = hp->next = hp;
	hp->word = "";
	while (*v) {
		register struct wordent *new = (struct wordent *) calloc(1, sizeof *wdp);

		new->prev = wdp;
		new->next = hp;
		wdp->next = new;
		wdp = new;
		wdp->word = savestr(*v++);
	}
	hp->prev = wdp;
	alias(&paraml);
	t = syntax(paraml.next, &paraml, 0);
	if (err)
		error(err);
	execute(t,-1);
	freelex(&paraml), freesyn(t);
}

isa(cp, what)
	register char *cp;
	register int what;
{

	if (cp == 0)
		return ((what & RESTOP) != 0);
	if (cp[1] == 0) {
		if ((what & ADDOP) && any(cp[0], "+-"))
			return (1);
		if ((what & MULOP) && any(cp[0], "*/%"))
			return (1);
		if ((what & RESTOP) && any(cp[0], "()!~^"))
			return (1);
	}
	if ((what & RESTOP) && (any(cp[0], "|&") || eq(cp, "<<") || eq(cp, ">>")))
		return (1);
	if (what & EQOP) {
		if (eq(cp, "=="))
			return (EQEQ);
		if (eq(cp, "!="))
			return (NOTEQ);
		if (eq(cp, "=~"))
			return (EQMATCH);
		if (eq(cp, "!~"))
			return (NOTEQMATCH);
	}
	if (!(what & RELOP))
		return (0);
	if (*cp == '<')
		return (LSS);
	if (*cp == '>')
		return (GTR);
	return (0);
}

egetn(cp)
	register char *cp;
{

	if (*cp && *cp != '-' && !digit(*cp))
		bferr("Expression syntax");
	return (getn(cp));
}

/* Phew! */

#ifdef EDEBUG
etraci(str, i, vp)
	char *str;
	int i;
	char ***vp;
{

	printf("%s=%d\t", str, i);
	blkpr(*vp);
	printf("\n");
}

etracc(str, cp, vp)
	char *str, *cp;
	char ***vp;
{

	printf("%s=%s\t", str, cp);
	blkpr(*vp);
	printf("\n");
}
#endif
