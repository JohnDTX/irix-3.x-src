char _Version_[] = "(C) Copyright 1983 UniSoft Corp. Version V.1.0";
char _Origin_[] = "System V";

/*	@(#)sno1.c	1.2	*/
#include "sno.h"
#define INCR 200

/*
 *   Snobol III
 */


int 	incomp;
int     freesize;
struct  node *lookf;
struct  node *looks;
struct  node *lookend;
struct  node *lookstart;
struct  node *lookdef;
struct  node *lookret;
struct  node *lookfret;
int     cfail;
int     rfail;
struct  node *freelist, *freespace;
struct  node *namelist;
int     lc;
struct  node *schar;
FILE	*fin;
int	xargc;
char	**xargv;

char *malloc();

struct node *
init (s, t)
	char *s;
{
	register struct node *a, *b;

	a = strst1 (s);
	b = look (a);
	delete (a);
	b->typ = t;
	return (b);
}

main (argc, argv)
	char *argv[];
{
	register struct node *a, *b, *c;
	static char stdbuf[BUFSIZ];

	setbuf (stdout, stdbuf);
	ncinit (argc, argv);
	lookf = init ("f", 0);
	looks = init ("s", 0);
	lookend = init ("end", 0);
	lookstart = init ("start", 0);
	lookdef = init ("define", 0);
	lookret = init ("return", 0);
	lookfret = init ("freturn", 0);
	init ("syspit", 3);
	init ("syspot", 4);
	incomp = 1;
	a = c = compile();
	while (lookend->typ != 2) {
		a->p1 = b = compile();
		a = b;
	}
	cfail = 1;
	a->p1 = 0;
	if (lookstart->typ == 2)
		c = lookstart->p2;
	incomp = 0;
	while (c=execute (c));
}

struct node *
syspit()
{
	register struct node *b, *c, *d;
	int a;
	char nextchar();

	a = nextchar();
	if (a == '\n')
		return (0);
	if((a == '*') && incomp){
		while(nextchar() != '\n') ;
		return 0;
	}
	b = c = salloc();
	while (a != '\n') {
		c->p1 = d = salloc();
		c = d;
		c->ch = a;
		if (a == '\0') {
			rfail = 1;
			break;
		}
		a = nextchar();
	}
	b->p2 = c;
	if (rfail) {
		delete (b);
		b = 0;
	}
	return (b);
}

syspot (string)
	struct node *string;
{
	register struct node *a, *b, *s;

	s = string;
	if (s!=0) {
		a = s;
		b = s->p2;
		while (a != b) {
			a = a->p1;
			putchar (a->ch);
		}
	}
	putchar ('\n');
}

struct node *
strst1 (s)
	char s[];
{
	int c;
	register struct node *e, *f, *d;

	d = f = salloc();
	while ((c = *s++)!='\0') {
		(e=salloc())->ch = c;
		f->p1 = e;
		f = e;
	}
	d->p2 = e;
	return (d);
}

class (c)
{
	switch (c) {
		case ')':  return (1);
		case '(':  return (2);
		case '\t':
		case ' ': return (3);
		case '+':  return (4);
		case '-':  return (5);
		case '*': return (6);
		case '/':  return (7);
		case '$':  return (8);
		case '"':
		case '\'': return (9);
		case '=':  return (10);
		case ',':  return (11);
	}
	return (0);
}

struct node *
salloc()
{
	register struct node *f;
	register char *i;

	if (freelist==0) {
		if (--freesize < 0) {
			if ((i=malloc (INCR * sizeof (struct node))) == NULL) {
				puts ("Out of free space");
				exit (1);
			}
			freesize = INCR - 1;
			freespace = (struct node *) i;
		}
		return (freespace++);
	}
	f = freelist;
	freelist = freelist->p1;
	return (f);
}

sfree (pointer)
	struct node *pointer;
{
	pointer->p1 = freelist;
	freelist = pointer;
}

int
nfree()
{
	register int i;
	register struct node *a;

	i = freesize;
	a = freelist;
	while (a) {
		a = a->p1;
		i++;
	}
	return (i);
}

struct node *
look (string)
	struct node *string;
{
	register struct node *i, *j, *k;

	k = 0;
	i = namelist;
	while (i) {
		j = i->p1;
		if (equal (j->p1, string) == 0)
			return (j);
		i = (k=i)->p2;
	}
	i = salloc();
	i->p2 = 0;
	if (k)
		k->p2 = i;
	else
		namelist = i;
	j = salloc();
	i->p1 = j;
	j->p1 = copy (string);
	j->p2 = 0;
	j->typ = 0;
	return (j);
}

struct node *
copy (string)
	struct node *string;
{
	register struct node *j, *l, *m;
	struct node *i, *k;

	if (string == 0)
		return (0);
	i = l = salloc();
	j = string;
	k = string->p2;
	while (j != k) {
		m = salloc();
		m->ch = (j=j->p1)->ch;
		l->p1 = m;
		l = m;
	}
	i->p2 = l;
	return (i);
}

int
equal (string1, string2)
	struct node *string1, *string2;
{
	register struct node *i, *j, *k;
	struct node *l;
	int n, m;

	if (string1==0) {
		if (string2==0)
			return (0);
		return (-1);
	}
	if (string2==0)
		return (1);
	i = string1;
	j = string1->p2;
	k = string2;
	l = string2->p2;
	for (;;) {
		m = (i=i->p1)->ch;
		n = (k=k->p1)->ch;
		if (m>n)
			return (1);
		if (m<n)
			return (-1);
		if (i==j) {
			if (k==l)
				return (0);
			return (-1);
		}
		if (k==l)
			return (1);
	}
}

int
strbin (string)
	struct node *string;
{
	int n, m, sign;
	register struct node *p, *q, *s;

	s = string;
	n = 0;
	if (s==0)
		return (0);
	p = s->p1;
	q = s->p2;
	sign = 1;
	if (class (p->ch)==5) { /* minus */
		sign = -1;
		if (p==q)
			return (0);
		p = p->p1;
	}
loop:
	m = p->ch - '0';
	if (m>9 || m<0)
		writes ("bad integer string");
	n = n * 10 + m;
	if (p==q)
		return (n*sign);
	p = p->p1;
	goto loop;
}

struct node *
binstr (binary)
{
	int n, sign;
	register struct node *m, *p, *q;

	n = binary;
	p = salloc();
	q = salloc();
	sign = 1;
	if (binary<0) {
		sign = -1;
		n = -binary;
	}
	p->p2 = q;
loop:
	q->ch = n%10+'0';
	n = n / 10;
	if (n==0) {
		if (sign<0) {
			m = salloc();
			m->p1 = q;
			q = m;
			q->ch = '-';
		}
		p->p1 = q;
		return (p);
	}
	m = salloc();
	m->p1 = q;
	q = m;
	goto loop;
}

struct node *
add (string1, string2)
	register struct node *string1, *string2;
{
	return (binstr (strbin (string1) + strbin (string2)));
}

struct node *
sub (string1, string2)
	register struct node *string1, *string2;
{
	return (binstr (strbin (string1) - strbin (string2)));
}

struct node *
mult (string1, string2)
	register struct node *string1, *string2;
{
	return (binstr (strbin (string1) * strbin (string2)));
}

struct node *
div (string1, string2)
	register struct node *string1, *string2;
{
	return (binstr (strbin (string1) / strbin (string2)));
}

struct node *
cat (string1, string2) 
	struct node *string1, *string2;
{
	register struct node *a, *b;

	if (string1==0)
		return (copy (string2));
	if (string2==0)
		return (copy (string1));
	a = copy (string1);
	b = copy (string2);
	a->p2->p1 = b->p1;
	a->p2 = b->p2;
	sfree (b);
	return (a);
}

struct node *
dcat (a,b)
	struct node *a, *b;
{
	register struct node *c;

	c = cat (a,b);
	delete (a);
	delete (b);
	return (c);
}

delete (string)
	struct node *string;
{
	register struct node *a, *b, *c;

	if (string==0)
		return;
	a = string;
	b = string->p2;
	while (a != b) {
		c = a->p1;
		sfree (a);
		a = c;
	}
	sfree (a);
}

sysput (string)
	struct node *string;
{
	syspot (string);
	delete (string);
}

dump()
{
	dump1 (namelist);
}

dump1 (base)
	struct node *base;
{
	register struct node *b, *c, *e;
	struct node *d;

	while (base) {
		b = base->p1;
		c = binstr (b->typ);
		d = strst1 ("  ");
		e = dcat (c, d);
		sysput (cat (e, b->p1));
		delete (e);
		if (b->typ==1) {
			c = strst1 ("   ");
			sysput (cat (c, b->p2));
			delete (c);
		}
		base = base->p2;
	}
}

writes (s)
	char *s;
{
	sysput (dcat (binstr (lc),dcat (strst1 ("\t"),strst1 (s))));
	fflush (stdout);
	if (cfail) {
		dump();
		fflush (stdout);
		exit (1);
	}
	while (sgetc());
	while (compile());
	fflush (stdout);
	exit (1);
}

struct node *
sgetc()
{
	register struct node *a;
	static struct node *line;
	static linflg;

	while (line==0) {
		line = syspit();
		if (rfail) {
			cfail++;
			writes ("eof on input");
		}
		lc++;
	}
	if (linflg) {
		line = 0;
		linflg = 0;
		return (0);
	}
	a = line->p1;
	if (a==line->p2) {
		sfree (line);
		linflg++;
	} else
		line->p1 = a->p1;
	return (a);
}

ncinit (argc, argv)
	int argc;
	char *argv[];
{
	xargc = argc - 1;
	xargv = argv + 1;
	ncswitch();
}

ncswitch()
{
	if (fin && fin != stdin)
		fclose (fin);
	if (xargc > 0) {
		fin = fopen (*xargv, "r");
		if (fin == NULL) {
			fputs ("Cannot open ", stdout);
			fputs (*xargv, stdout);
			putchar ('\n');
			exit (1);
		}
		xargv++;
		xargc--;
	} else
		fin = stdin;
}

char
nextchar()
{
	register int a;

	a = getc (fin);
	if (a == EOF) {
		while (a == EOF && fin != stdin) {
			ncswitch();
			a = getc (fin);
		}
		if (a == EOF)
			a = 0;
	}
	return a;
}
