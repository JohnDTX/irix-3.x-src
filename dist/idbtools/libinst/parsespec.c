/* Convert textual product specification files into binary form */

#include "inst.h"
#include "idb.h"
#include <stdio.h>
#include <ctype.h>

#define	S_dot		1
#define S_error		2
#define S_exp		3
#define S_id		4
#define S_image		5
#define S_prereq	6
#define S_product	7
#define S_subsys	8
#define S_version	9
#define S_main		10
#define S_default	11
#define S_padsize	12
#define S_add		13
#define S_lpar		14
#define S_rpar		15
#define S_replaces	16
#define S_order		17

#define Scan		1
#define Stay		2

static void	getprod ();
static void	getimage ();
static void	getsubsys ();
static void	getprereq ();
static void	nextsym ();
static char	*strval ();
static long	longval ();
static char	*token ();
static int	getnb ();
static void	online ();

static FILE	*sfile;
static int	specline;
static int	sym;

int		padsize = Padsize;

Spec *
parsespec (name, bset)
	char		*name;
	Memset		*bset;
{
	Spec		*spec;
	int		n;

	specline = 1;
	if ((sfile = fopen (name, "r")) == NULL) {
		perror (name); exit (1);
	}
	nextsym ();
	spec = (Spec *) idb_getmem (sizeof (Spec), bset);
	spec->prod = NULL;
	n = 0;
	while (sym == S_product) {
		spec->prod = (Prod *) idb_getmore (spec->prod,
			(n + 1) * sizeof (Prod), bset);
		getprod (spec->prod + n++, bset);
	}
	spec->prodend = spec->prod + n;
	if (sym != EOF) online ("expected product spec");
	fclose (sfile);
	return (spec);
}

static void
getprod (prod, bset)
	Prod		*prod;
	Memset		*bset;
{
	int		n;

	prod->name = strval (bset, Scan);
	prod->id = "";
	prod->flags = 0;
	prod->image = NULL;
	n = 0;
	while (sym != EOF) {
		switch (sym) {
		case S_image:
			prod->image = (Image *) idb_getmore (prod->image,
				(n + 1) * sizeof (Image), bset);
			getimage (prod, prod->image + n++, bset);
			break;
		case S_id:
			prod->id = strval (bset, Scan);
			break;
		case S_dot:
			prod->imgend = prod->image + n;
			nextsym ();
			return;
		default:
			online ("unrecognized product component");
		}
	}
	online ("unexpected end of file");
}

static void
getimage (pr, im, bset)
	Prod		*pr;
	Image		*im;
	Memset		*bset;
{
	char		buff [Strsize];
	int		n;

	im->flags = 0;
	im->name = strval (bset, Scan);
	im->id = "";
	im->format = Format_image;
	im->order = 0;
	im->version = 0;
	im->length = 0;
	im->padsize = padsize;
	im->subsys = NULL;
	im->order = 0;
	n = 0;
	while (sym != EOF) {
		switch (sym) {
		case S_dot:
			im->subend = im->subsys + n;
			nextsym ();
			return;
		case S_version:
			im->version = longval (Scan);
			break;
		case S_add:
			im->flags |= Add;
			nextsym ();
			break;
		case S_id:
			im->id = strval (bset, Scan);
			break;
		case S_padsize:
			im->padsize = longval (Scan);
			break;
		case S_order:
			im->order = longval (Scan);
			break;
		case S_subsys:
			im->subsys = (Subsys *) idb_getmore (im->subsys,
				(n + 1) * sizeof (*im->subsys), bset);
			getsubsys (pr, im, im->subsys + n++, bset);
			break;
		default:
			online ("unrecognized image component");
		}
	}
	online ("unexpected end of file");
}

static void
getsubsys (pr, im, ss, bset)
	Prod		*pr;
	Image		*im;
	Subsys		*ss;
	Memset		*bset;
{
	Mark		*rep;
	int		nprereq, nrep;
	char		buff [Strsize];

	ss->flags = 0;
	ss->name = strval (bset, Scan);
	ss->id = "";
	ss->exp = "";
	ss->prereq = NULL;
	ss->rep = NULL;
	nprereq = nrep = 0;
	while (sym != EOF) {
		switch (sym) {
		case S_id:
			ss->id = strval (bset, Scan);
			break;
		case S_default:
			ss->flags |= Default;
			nextsym ();
			break;
		case S_main:
			ss->flags |= Main;
			nextsym ();
			break;
		case S_exp:
			ss->exp = strval (bset, Scan);
			break;
		case S_prereq:
			ss->prereq = (Prereq *) idb_getmore (ss->prereq,
				(nprereq + 1) * sizeof (Prereq), bset);
			getprereq (ss->prereq + nprereq++, bset);
			break;
		case S_replaces:
			ss->rep = (Mark *) idb_getmore (ss->rep,
				(nrep + 1) * sizeof (Mark), bset);
			rep = ss->rep + nrep++;
			rep->pname = strval (bset, Stay);
			if (strcmp (rep->pname, "self") == 0) {
				rep->pname = pr->name;
				rep->iname = im->name;
				rep->sname = ss->name;
				rep->lowvers = 0;
				rep->highvers = im->version - 1;
				nextsym ();
			} else {
				rep->iname = strval (bset, Stay);
				rep->sname = strval (bset, Stay);
				rep->lowvers = longval (Stay);
				rep->highvers = longval (Scan);
			}
			break;
		case S_dot:
			ss->preend = ss->prereq + nprereq;
			ss->repend = ss->rep + nrep;
			nextsym ();
			return;
		default:
			online ("unrecognized subsystem component");
		}
	}
	online ("unexpected end of file");
}

static void
getprereq (pr, bset)
	Prereq		*pr;
	Memset		*bset;
{
	Mark		*mk;

	nextsym ();
	if (sym != S_lpar) {
		online ("expected left paren");
	}
	mk = pr->pq = (Mark *) idb_getmem (0, bset);
	nextsym ();
	while (sym != S_rpar) {
		if (*(mk->pname = strval (bset, Stay)) == '\0' ||
		    *(mk->iname = strval (bset, Stay)) == '\0' ||
		    *(mk->sname = strval (bset, Stay)) == '\0') {
			online ("unexpected eof");
		}
		mk->lowvers = longval (Scan);
		mk->highvers = maxint (long);
		++mk;
	}
	pr->pqend = mk;
}

static void
nextsym ()
{
	char		*p;

	if (*(p = token ()) == '\0') sym = EOF;
	else if (strcmp (p, ".") == 0) sym = S_dot;
	else if (strcmp (p, "(") == 0) sym = S_lpar;
	else if (strcmp (p, ")") == 0) sym = S_rpar;
	else if (strcmp (p, "exp") == 0) sym = S_exp;
	else if (strcmp (p, "id") == 0) sym = S_id;
	else if (strcmp (p, "image") == 0) sym = S_image;
	else if (strcmp (p, "prereq") == 0) sym = S_prereq;
	else if (strcmp (p, "replaces") == 0) sym = S_replaces;
	else if (strcmp (p, "product") == 0) sym = S_product;
	else if (strcmp (p, "subsys") == 0) sym = S_subsys;
	else if (strcmp (p, "version") == 0) sym = S_version;
	else if (strcmp (p, "main") == 0) sym = S_main;
	else if (strcmp (p, "default") == 0) sym = S_default;
	else if (strcmp (p, "padsize") == 0) sym = S_padsize;
	else if (strcmp (p, "order") == 0) sym = S_order;
	else if (strcmp (p, "add") == 0) sym = S_add;
	else sym = S_error;
}

static char *
token ()
{
	int		c, quote;
	char		*p;
	static char	buff [Strsize];

	*(p = buff) = '\0';
	c = getnb ();
	if (c == EOF) return ("");
	if (c == '"' || c == '\'') { quote = c; c = getc (sfile); }
	else quote = 0;
	if (c == '(' || c == ')') {
		*p++ = c; *p = '\0';
		return (buff);
	}
	while (c != EOF && (quote ? c != quote : !isspace (c))) {
		if (c == '\\') {
			if ((c = getc (sfile)) == EOF) {
				online ("unexpected end of file");
			}
		}
		if (p < buff + sizeof (buff) - 1) *p++ = c;
		c = getc (sfile);
	}
	if (!quote && c != EOF) ungetc (c, sfile);
	*p++ = '\0';
	if (quote == 0 || quote == '"') envsub (buff);
	return (buff);
}

static int
getnb ()
{
	int		c;

	while ((c = getc (sfile)) != EOF) {
		if (c == '\n') {
			++specline;
		} else if (c == '#') {
			while ((c = getc (sfile)) != EOF && c != '\n') ;
			if (c == '\n') ++specline;
		} else if (c != ' ' && c != '\t') {
			break;
		}
	}
	return (c);
}

static char *
strval (mset, move)
	Memset		*mset;
	int		move;
{
	char		*p;

	p = idb_stash (token (), mset);
	if (move == Scan) nextsym ();
	return (p);
}

static long
longval (move)
	int		move;
{
	long		l;

	l = atol (token ());
	if (move == Scan) nextsym ();
	return (l);
}

static void
online (msg)
	char		*msg;
{
	fprintf (stderr, "%s on line %d\n", msg, specline);
	exit (1);
}
