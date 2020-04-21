/*	@(#)sh.parse.c	2.1	SCCS id keyword	*/
/* $Source: /d2/3.7/src/bin/csh/RCS/sh.parse.c,v $ */
static	char	*Sccsid = "@(#)$Revision: 1.1 $";
/* $Date: 89/03/27 14:53:30 $ */
/* Copyright (c) 1980 Regents of the University of California */
#include "sh.h"

/*
 * C shell
 */

/*
 * Perform aliasing on the word list lex
 * Do a (very rudimentary) parse to separate into commands.
 * If word 0 of a command has an alias, do it.
 * Repeat a maximum of 20 times.
 */
alias(lex)
	register struct wordent *lex;
{
	int aleft = 21;
	jmp_buf osetexit;

	getexit(osetexit);
	setexit();
#ifdef ALDEBUG
	prlex(lex);
	printf("\n");
#endif
	if (haderr) {
		resexit(osetexit);
		reset();
	}
	if (--aleft == 0)
		error("Alias loop");
	asyntax(lex->next, lex);
	resexit(osetexit);
}

asyntax(p1, p2)
	register struct wordent *p1, *p2;
{

	while (p1 != p2)
		if (any(p1->word[0], ";&\n"))
			p1 = p1->next;
		else {
			asyn0(p1, p2);
			return;
		}
}

asyn0(p1, p2)
	struct wordent *p1;
	register struct wordent *p2;
{
	register struct wordent *p;
	register int l = 0;

	for (p = p1; p != p2; p = p->next)
		switch (p->word[0]) {

		case '(':
			l++;
			continue;

		case ')':
			l--;
			if (l < 0)
				error("Too many )'s");
			continue;

		case '>':
			if (p->next != p2 && eq(p->next->word, "&"))
				p = p->next;
			continue;

		case '&':
		case '|':
		case ';':
		case '\n':
			if (l != 0)
				continue;
			asyn3(p1, p);
			asyntax(p->next, p2);
			return;
		}
	if (l == 0)
		asyn3(p1, p2);
}

asyn3(p1, p2)
	struct wordent *p1;
	register struct wordent *p2;
{
	register struct varent *ap;
	struct wordent alout;
	register bool redid;

	if (p1 == p2)
		return;
	if (p1->word[0] == '(') {
		for (p2 = p2->prev; p2->word[0] != ')'; p2 = p2->prev)
			if (p2 == p1)
				return;
		if (p2 == p1->next)
			return;
		asyn0(p1->next, p2);
		return;
	}
	ap = adrof1(p1->word, &aliases);
	if (ap == 0)
		return;
	alhistp = p1->prev;
	alhistt = p2;
	alvec = ap->vec;
#ifdef ALDEBUG
	printf("applying: %s --> ", p1->word);
	blkpr(ap->vec);
	printf("\n");
	printf("to: ");
	{ struct wordent *wp; for (wp = alhistp; wp != alhistt; wp = wp->next)
		printf("%s ", wp->word);
	  printf("\n");
	}
#endif
	redid = lex(&alout);
	alhistp = alhistt = 0;
	alvec = 0;
	if (err) {
		freelex(&alout);
		error(err);
	}
	if (p1->word[0] && eq(p1->word, alout.next->word)) {
		char *cp = alout.next->word;

/* rmt:07/84
 * compiler bug: "\200" -> "\0"
 *		alout.next->word = strspl("\200", cp);
 */
		alout.next->word = calloc(1, strlen(cp) + 2);
		alout.next->word[0] = 128;
		strcpy(alout.next->word+1,cp);
		xfree(cp);
	}
	p1 = freenod(p1, redid ? p2 : p1->next);
	if (alout.next != &alout) {
		p1->next->prev = alout.prev->prev;
		alout.prev->prev->next = p1->next;
		alout.next->prev = p1;
		p1->next = alout.next;
		xfree(alout.prev->word);
		xfree(alout.prev);
	}
	reset();		/* throw! */
}

struct wordent *
freenod(p1, p2)
	register struct wordent *p1, *p2;
{
	register struct wordent *retp = p1->prev;

	while (p1 != p2) {
		xfree(p1->word);
		p1 = p1->next;
		xfree(p1->prev);
	}
	retp->next = p2;
	p2->prev = retp;
	return (retp);
}

#define	PHERE	1
#define	PIN	2
#define	POUT	4
#define	PDIAG	8

/*
 * syntax
 *	empty
 *	syn0
 */
struct command *
syntax(p1, p2, flags)
	register struct wordent *p1, *p2;
	int flags;
{

	while (p1 != p2)
		if (any(p1->word[0], ";&\n"))
			p1 = p1->next;
		else
			return (syn0(p1, p2, flags));
	return (0);
}

/*
 * syn0
 *	syn1
 *	syn1 & syntax
 */
struct command *
syn0(p1, p2, flags)
	struct wordent *p1, *p2;
	int flags;
{
	register struct wordent *p;
	register struct command *t, *t1;
	int l;

	l = 0;
	for (p = p1; p != p2; p = p->next)
		switch (p->word[0]) {

		case '(':
			l++;
			continue;

		case ')':
			l--;
			if (l < 0)
				seterr("Too many )'s");
			continue;

		case '|':
			if (p->word[1] == '|')
				continue;
			/* fall into ... */

		case '>':
			if (p->next != p2 && eq(p->next->word, "&"))
				p = p->next;
			continue;

		case '&':
			if (l != 0)
				break;
			if (p->word[1] == '&')
				continue;
			t1 = syn1(p1, p, flags);
			if (t1->t_dtyp == TLST) {
				t = (struct command *) calloc(1, sizeof (*t));
				t->t_dtyp = TPAR;
				t->t_dflg = FAND|FPRS|FINT;
				t->t_dspr = t1;
				t1 = t;
			} else
				t1->t_dflg |= FAND|FPRS|FINT;
			t = (struct command *) calloc(1, sizeof (*t));
			t->t_dtyp = TLST;
			t->t_dflg = 0;
			t->t_dcar = t1;
			t->t_dcdr = syntax(p, p2, flags);
			return(t);
		}
	if (l == 0)
		return (syn1(p1, p2, flags));
	seterr("Too many ('s");
	return (0);
}

/*
 * syn1
 *	syn1a
 *	syn1a ; syntax
 */
struct command *
syn1(p1, p2, flags)
	struct wordent *p1, *p2;
	int flags;
{
	register struct wordent *p;
	register struct command *t;
	int l;

	l = 0;
	for (p = p1; p != p2; p = p->next)
		switch (p->word[0]) {

		case '(':
			l++;
			continue;

		case ')':
			l--;
			continue;

		case ';':
		case '\n':
			if (l != 0)
				break;
			t = (struct command *) calloc(1, sizeof (*t));
			t->t_dtyp = TLST;
			t->t_dcar = syn1a(p1, p, flags);
			t->t_dcdr = syntax(p->next, p2, flags);
			if (t->t_dcdr == 0)
				t->t_dcdr = t->t_dcar, t->t_dcar = 0;
			return (t);
		}
	return (syn1a(p1, p2, flags));
}

/*
 * syn1a
 *	syn1b
 *	syn1b || syn1a
 */
struct command *
syn1a(p1, p2, flags)
	struct wordent *p1, *p2;
	int flags;
{
	register struct wordent *p;
	register struct command *t;
	register int l = 0;

	for (p = p1; p != p2; p = p->next)
		switch (p->word[0]) {

		case '(':
			l++;
			continue;

		case ')':
			l--;
			continue;

		case '|':
			if (p->word[1] != '|')
				continue;
			if (l == 0) {
				t = (struct command *) calloc(1, sizeof (*t));
				t->t_dtyp = TOR;
				t->t_dcar = syn1b(p1, p, flags);
				t->t_dcdr = syn1a(p->next, p2, flags);
				t->t_dflg = 0;
				return (t);
			}
			continue;
		}
	return (syn1b(p1, p2, flags));
}

/*
 * syn1b
 *	syn2
 *	syn2 && syn1b
 */
struct command *
syn1b(p1, p2, flags)
	struct wordent *p1, *p2;
	int flags;
{
	register struct wordent *p;
	register struct command *t;
	register int l = 0;

	l = 0;
	for (p = p1; p != p2; p = p->next)
		switch (p->word[0]) {

		case '(':
			l++;
			continue;

		case ')':
			l--;
			continue;

		case '&':
			if (p->word[1] == '&' && l == 0) {
				t = (struct command *) calloc(1, sizeof (*t));
				t->t_dtyp = TAND;
				t->t_dcar = syn2(p1, p, flags);
				t->t_dcdr = syn1b(p->next, p2, flags);
				t->t_dflg = 0;
				return (t);
			}
			continue;
		}
	return (syn2(p1, p2, flags));
}

/*
 * syn2
 *	syn3
 *	syn3 | syn2
 *	syn3 |& syn2
 */
struct command *
syn2(p1, p2, flags)
	struct wordent *p1, *p2;
	int flags;
{
	register struct wordent *p, *pn;
	register struct command *t;
	register int l = 0;
	int f;

	for (p = p1; p != p2; p = p->next)
		switch (p->word[0]) {

		case '(':
			l++;
			continue;

		case ')':
			l--;
			continue;

		case '|':
			if (l != 0)
				continue;
			t = (struct command *) calloc(1, sizeof (*t));
			f = flags | POUT;
			pn = p->next;
			if (pn != p2 && pn->word[0] == '&') {
				f |= PDIAG;
				t->t_dflg |= FDIAG;
			}
			t->t_dtyp = TFIL;
			t->t_dcar = syn3(p1, p, f);
			if (pn != p2 && pn->word[0] == '&')
				p = pn;
			t->t_dcdr = syn2(p->next, p2, flags | PIN);
			return (t);
		}
	return (syn3(p1, p2, flags));
}

char	*RELPAR =	"<>()";

/*
 * syn3
 *	( syn0 ) [ < in  ] [ > out ]
 *	word word* [ < in ] [ > out ]
 *	KEYWORD ( word* ) word* [ < in ] [ > out ]
 *
 *	KEYWORD = (@ exit foreach if set switch test while)
 */
struct command *
syn3(p1, p2, flags)
	struct wordent *p1, *p2;
	int flags;
{
	register struct wordent *p;
	struct wordent *lp, *rp;
	register struct command *t;
	register int l;
	char **av;
	int n, c;
	bool specp = 0;

	if (p1 != p2) {
		p = p1;
again:
		switch (srchx(p->word)) {

		case ZELSE:
			p = p->next;
			if (p != p2)
				goto again;
			break;

		case ZEXIT:
		case ZFOREACH:
		case ZIF:
		case ZLET:
		case ZSET:
		case ZSWITCH:
		case ZWHILE:
			specp = 1;
			break;
		}
	}
	n = 0;
	l = 0;
	for (p = p1; p != p2; p = p->next)
		switch (p->word[0]) {

		case '(':
			if (specp)
				n++;
			l++;
			continue;

		case ')':
			if (specp)
				n++;
			l--;
			continue;

		case '>':
		case '<':
			if (l != 0) {
				if (specp)
					n++;
				continue;
			}
			if (p->next == p2)
				continue;
			if (any(p->next->word[0], RELPAR))
				continue;
			n--;
			continue;

		default:
			if (!specp && l != 0)
				continue;
			n++;
			continue;
		}
	if (n < 0)
		n = 0;
	t = (struct command *) calloc(1, sizeof (*t));
	av = (char **) calloc(n + 1, sizeof (char **));
	t->t_dcom = av;
	n = 0;
	if (p2->word[0] == ')')
		t->t_dflg = FPAR;
	lp = 0;
	rp = 0;
	l = 0;
	for (p = p1; p != p2; p = p->next) {
		c = p->word[0];
		switch (c) {

		case '(':
			if (l == 0) {
				if (lp != 0 && !specp)
					seterr("Badly placed (");
				lp = p->next;
			}
			l++;
			goto savep;

		case ')':
			l--;
			if (l == 0)
				rp = p;
			goto savep;

		case '>':
			if (l != 0)
				goto savep;
			if (p->word[1] == '>')
				t->t_dflg |= FCAT;
			if (p->next != p2 && eq(p->next->word, "&")) {
				t->t_dflg |= FDIAG, p = p->next;
				if (flags & (POUT|PDIAG))
					goto badout;
			}
			if (p->next != p2 && eq(p->next->word, "!"))
				t->t_dflg |= FANY, p = p->next;
			if (p->next == p2) {
missfile:
				seterr("Missing name for redirect");
				continue;
			}
			p = p->next;
			if (any(p->word[0], RELPAR))
				goto missfile;
			if ((flags & POUT) && (flags & PDIAG) == 0 || t->t_drit)
badout:
				seterr("Ambiguous output redirect");
			else
				t->t_drit = savestr(p->word);
			continue;

		case '<':
			if (l != 0)
				goto savep;
			if (p->word[1] == '<')
				t->t_dflg |= FHERE;
			if (p->next == p2)
				goto missfile;
			p = p->next;
			if (any(p->word[0], RELPAR))
				goto missfile;
			if ((flags & PHERE) && (t->t_dflg & FHERE))
				seterr("Can't << within ()'s");
			else if ((flags & PIN) || t->t_dlef)
				seterr("Ambiguous input redirect");
			else
				t->t_dlef = savestr(p->word);
			continue;

savep:
			if (!specp)
				continue;
		default:
			if (l != 0 && !specp)
				continue;
			if (err == 0)
				av[n] = savestr(p->word);
			n++;
			continue;
		}
	}
	if (lp != 0 && !specp) {
		if (n != 0)
			seterr("Badly placed ()'s");
		t->t_dtyp = TPAR;
		t->t_dspr = syn0(lp, rp, PHERE);
	} else {
		if (n == 0)
			seterr("Invalid null command");
		t->t_dtyp = TCOM;
	}
	return (t);
}

freesyn(t)
	register struct command *t;
{
	register char **v;

	if (t == 0)
		return;
	switch (t->t_dtyp) {

	case TCOM:
		for (v = t->t_dcom; *v; v++)
			xfree(*v);
		xfree(t->t_dcom);
		goto lr;

	case TPAR:
		freesyn(t->t_dspr);
		/* fall into ... */

lr:
		xfree(t->t_dlef), xfree(t->t_drit);
		break;

	case TAND:
	case TOR:
	case TFIL:
	case TLST:
		freesyn(t->t_dcar), freesyn(t->t_dcdr);
		break;
	}
	xfree(t);
}
