#include <sys/param.h>
#if R2300 || IP4
#include <sys/types.h>
#include <sys/sysmacros.h>
#else
#include <sys/fs.h>
#endif
#include <sys/stat.h>

#include <setjmp.h>
#include <ctype.h>
#define library
#include "idb.h"

#define newnode()	((Node *) idb_getmem (sizeof (Node), pset))
#define nextch()	((ch = getc (pfile)) == '\\' ? chescape () : ch)

extern char	*getenv ();

hidden Name	*findname ();

hidden Node	*andif ();
hidden Node	*assign ();
hidden Node	*band ();
hidden Node	*bor ();
hidden Node	*bxor ();
hidden Node	*cat ();
hidden Node	*comparison ();
hidden Node	*expression ();
hidden Node	*factor ();
hidden int	indexlist ();
hidden Node	*makebool ();
hidden Node	*makeint ();
hidden Node	*makestr ();
hidden Node	*orif ();
hidden Node	*range ();
hidden Node	*statement ();
hidden Node	*subst ();
hidden Node	*sum ();
hidden Node	*term ();
hidden Node	*value ();
hidden Node	*whatif ();
hidden Attr	*findattr ();

hidden int	xaccess ();
hidden int	xadd ();
hidden int	xand ();
hidden int	xandif ();
hidden int	xargc ();
hidden int	xassigni ();
hidden int	xassigns ();
hidden int	xatstr ();
hidden int	xattr ();
hidden int	xattrarg ();
hidden int	xbasename ();
hidden int	xblocks ();
hidden int	xbor ();
hidden int	xbreak ();
hidden int	xbxor ();
hidden int	xbytes ();
hidden int	xcat ();
hidden int	xcomma ();
hidden int	xcont ();
hidden int	xctob ();
hidden int	xctoi ();
hidden int	xdiv ();
hidden int	xdirname ();
hidden int	xequali ();
hidden int	xequals ();
hidden int	xfor ();
hidden int	xgequali ();
hidden int	xgequals ();
hidden int	xgreateri ();
hidden int	xgreaters ();
hidden int	xiconst ();
hidden int	xif ();
hidden int	xinvert ();
hidden int	xitoc ();
hidden int	xvari ();
hidden int	xlequali ();
hidden int	xlequals ();
hidden int	xlessi ();
hidden int	xlesss ();
hidden int	xlist ();
hidden int	xmatch ();
hidden int	xmatchat ();
hidden int	xmatchats ();
hidden int	xmodeof ();
hidden int	xmul ();
hidden int	xnequali ();
hidden int	xnequals ();
hidden int	xnop ();
hidden int	xnot ();
hidden int	xnotmatch ();
hidden int	xonearg ();
hidden int	xorif ();
hidden int	xprint ();
hidden int	xprintf ();
hidden int	xputrec ();
hidden int	xrange ();
hidden int	xreturn ();
hidden int	xrpath ();
hidden int	xsconst ();
hidden int	xspath ();
hidden int	xstmtlist ();
hidden int	xsub ();
hidden int	xsubst ();
hidden int	xtypeof ();
hidden int	xvars ();
hidden int	xwhatif ();
hidden int	xwhile ();

hidden void	arglist ();
hidden void	setvars ();
extern char	*idb_itoc ();
extern long	toblocks ();

hidden intfunc	cmpfunc [][2] = {
	{ xlessi,	xlesss,		},
	{ xlequali,	xlequals,	},
	{ xequali,	xequals,	},
	{ xgequali,	xgequals,	},
	{ xgreateri,	xgreaters,	},
	{ xnequali,	xnequals,	},
	{ xmatch,	xmatch,		},
	{ xnotmatch,	xnotmatch,	},
};

hidden Name	*locals		= NULL;
hidden Name	*globals	= NULL;
hidden Name	builtins [] = {
	{ "mode",	Var,	Int,	0	},
	{ "type",	Var,	String,	0	},
	{ "owner",	Var,	String,	0	},
	{ "group",	Var,	String,	0	},
	{ "srcpath",	Var,	String,	0	},
	{ "dstpath",	Var,	String,	0	},
	{ "nattr",	Var,	Int,	0	},
	{ "rbase",	Var,	String,	0	},
	{ "sbase",	Var,	String,	0	},
	{ "idb",	Var,	String,	0	},
	{ "if",		If,	Int,	0	},
	{ "while",	While,	Int,	0	},
	{ "for",	For,	Int,	0	},
	{ "continue",	Cont,	Int,	0	},
	{ "break",	Break,	Int,	0	},
	{ "return",	Return,	Int,	0	},
	{ "else",	Else,	Int,	0	},
	{ "print",	Func,	Int,	xprint	},
	{ "printf",	Func,	Int,	xprintf	},
	{ "putrec",	Func,	Int,	xputrec },
	{ "access",	Func,	Int,	xaccess	},
	{ "typeof",	Func,	Int,	xtypeof	},
	{ "modeof",	Func,	Int,	xmodeof },
	{ "rpath",	Func,	String,	xrpath	},
	{ "spath",	Func,	String,	xspath	},
	{ "bytes",	Func,	Int,	xbytes	},
	{ "blocks",	Func,	Int,	xblocks	},
	{ "argc",	Func,	Int,	xargc	},
	{ "atstr",	Func,	String,	xatstr	},
	{ "matchat",	Func,	Int, 	xmatchat },
	{ "matchats",	Func,	String,	xmatchats },
	{ "basename",	Func,	String,	xbasename },
	{ "dirname",	Func,	String,	xdirname },
	{ "i",		Var,	Int,	0	},
	{ "j",		Var,	Int,	0	},
};

#define Nbuiltins	(sizeof (builtins) / sizeof (*builtins))

hidden int	builtinvals [Nbuiltins];

hidden int	nlocals		= 0;
hidden int	nglobals	= 0;
hidden int	nbuiltin	= Nbuiltins;

hidden int	*localbase = NULL;
hidden int	*globalbase = NULL;
hidden int	*builtinbase = builtinvals;

hidden int	**localstack;
hidden int	**localstackp;

hidden FILE	*pfile;
hidden Memset	*pset;
hidden Memset	*xset;
hidden jmp_buf	pbotch;
hidden jmp_buf	xbotch;
hidden int	*breakjmp;
hidden int	*contjmp;
hidden Rec	*xrec;

hidden int	sym;
hidden int	symint;
hidden char	symval [1024];
hidden Name	*symname;
hidden int	**symbase;
hidden int	symdisp;
hidden int	ch;

hidden char	*hitstart;
hidden char	*hitend;

hidden void
synerr (s)
	char		*s;
{
	char		buff [1024];

	strcpy (buff, s); strcat (buff, "\n");
	write (1, buff, strlen (buff));
	longjmp (pbotch, 1);
}

Node *
idb_parse (f, set, type)	/* parse an open file */
	FILE		*f;
	Memset		*set;
	int		type;
{
	Node		*n;

	pfile = f; pset = set;
	scan ();
	localstack = localstack = NULL;
	if (setjmp (pbotch)) n = NULL;
	else {
		n = statement ();
		if (sym != Semi && sym != Eof) synerr ("extra characters");
		if (n->type != type) {
			if (n->type == Bool) {
				n = makebool (n);
			} else if (n->type == Int) {
				n = makeint (n);
			} else if (n->type == String) {
				n = makestr (n);
			}
		}
	}
	return (n);
}

Node *
idb_parsef (fname, set, type)	/* parse a named file */
	char		*fname;
	Memset		*set;
	int		type;
{
	FILE		*f;
	Node		*n;

	if ((f = fopen (fname, "r")) == NULL) return (NULL);
	n = idb_parse (f, set, type);
	fclose (f);
	return (n);
}

Node *
idb_parses (str, set, type)	/* parse a string */
	char		*str;
	Memset		*set;
	int		type;
{
	FILE		*f;
	Node		*n;

	if ((f = sopen (str, "r")) == NULL) return (NULL);
	n = idb_parse (f, set, type);
	fclose (f);
	return (n);
}

/* following is the recursive descent parser.  The output is a tree, where
 * each node contains a type (Int or String) based on the result of its
 * operations, a pointer to a function that "runs" the node, and pointers
 * to two operand nodes (which may be indirect when more than two operands
 * are needed.)
 */

hidden Node *
statement()
{
	Node		*n, *np;

	n = newnode ();
	if (sym == Semi) {
		n->func = xnop;
		n->type = Int;
	} else if (sym == Lbrace) {
		scan ();
		np = n;
		np->func = xstmtlist;
		np->n1 = statement ();
		np->type = np->n1->type;
		while (sym != Rbrace) {
			np->n2 = newnode ();
			np = np->n2;
			np->func = xstmtlist;
			np->n1 = statement ();
			np->type = np->n1->type;
		}
		np->n2 = NULL;
		scan ();
	} else if (sym == If) {
		scan ();
		if (sym != Lpar) synerr ("expected left paren");
		scan ();
		n->func = xif;
		n->n1 = makebool (expression ());
		if (sym != Rpar) synerr ("expected right paren");
		scan ();
		n->n2 = newnode ();
		n->n2->n1 = statement ();
		if (sym == Else) {
			scan ();
			n->n2->n2 = statement ();
			if (n->n2->n1->type != n->n2->n2->type) {
				if (n->n2->n1->type != String) {
					n->n2->n1 = makestr (n->n2->n1);
				} else {
					n->n2->n2 = makestr (n->n2->n1);
				}
			}
		} else {
			n->n2->n2 = newnode ();
			if (n->n2->n1->type == Int) {
				n->n2->n2->func = xiconst;
				n->n2->n2->type = Int;
				n->n2->n2->n1 = (Node *) 0;
			} else {
				n->n2->n2->func = xsconst;
				n->n2->n2->type = String;
				n->n2->n2->n1 = (Node *) "";
			}
		}
		n->type = n->n2->type = n->n2->n1->type;
	} else if (sym == While) {
		scan ();
		if (sym != Lpar) synerr ("expected left paren");
		scan ();
		n->func = xwhile;
		n->n1 = makebool (expression ());
		if (sym != Rpar) synerr ("expected right paren");
		scan ();
		n->n2 = statement ();
		n->type = n->n2->type;
	} else if (sym == For) {
		scan ();
		if (sym != Lpar) synerr ("expected left paren");
		scan ();
		n->func = xfor;
		n->n1 = newnode ();
		n->n2 = newnode ();
		if (sym == Semi) {
			n->n1->n1->func = xnop;
			n->n1->n1->type = Int;
		} else {
			n->n1->n1 = expression ();
			if (sym != Semi) synerr ("expected semicolon");
		}
		scan ();
		if (sym == Semi) {
			n->n1->n2->func = xiconst;
			n->n1->n2->type = Int;
			n->n1->n2->n1 = (Node *) 1;
		} else {
			n->n1->n2 = makebool (expression ());
			if (sym != Semi) synerr ("expected semicolon");
		}
		scan ();
		if (sym == Rpar) {
			n->n2->n1->func = xnop;
			n->n2->n1->type = Int;
		} else {
			n->n2->n1 = expression ();
		}
		if (sym != Rpar) synerr ("expected right paren");
		scan ();
		n->n2->n2 = statement ();
		n->type = n->n2->n2->type;
	} else if (sym == Break) {
		n->func = xbreak;
		n->type = Int;
		scan ();
		if (sym != Semi) synerr ("expected semicolon");
	} else if (sym == Cont) {
		n->func = xcont;
		n->type = Int;
		scan ();
		if (sym != Semi) synerr ("expected semicolon");
	} else if (sym == Return) {
		n->func = xreturn;
		scan ();
		if (sym == Semi) {
			n->type = Int;
			n->n1 = NULL;
		} else {
			n->n1 = expression ();
			n->type = n->n1->type;
			if (sym != Semi) synerr ("expected semicolon");
		}
		scan ();
	} else {
		n = expression ();
		if (sym != Semi && sym != Eof) synerr ("expected semicolon");
		scan ();
	}
	return (n);
}

hidden Node *
expression ()
{
	Node		*n, *n1;

	n = assign ();
	if (sym == Comma) {
		n1 = n;
		n = newnode ();
		n->func = xcomma;
		n->n1 = n1;
		scan ();
		n->n2 = expression ();
		n->type = n->n2->type;
	}
	return (n);
}

hidden Node *
assign ()
{
	Node		*n, *n1;

	n = whatif ();
	if (sym == Assign) {
		if (n->func != xvars && n->func != xvari) {
			synerr ("illegal lvalue");
		}
		scan ();
		n1 = n;
		n = newnode ();
		n->n2 = assign ();
		if (n1->type != n->n2->type) {
			if (n1->type == Int) n->n2 = makeint (n->n2);
			else n->n2 = makestr (n->n2);
		}
		n->func = n1->type == Int ? xassigni : xassigns;
		n->type = n1->type;
		n->n1 = n1;
	}
	return (n);
}

hidden Node *
whatif ()
{
	Node		*n, *n1, *n2;

	n = andif ();
	if (sym == Question) {
		n1 = makeint (n);
		n = newnode ();
		n->func = xwhatif;
		n->n1 = n1;
		scan ();
		n->n2 = newnode ();
		n->n2->type = Error;
		n->n2->func = NULL;
		n1 = whatif ();
		if (sym != Colon) synerr ("expected colon");
		scan ();
		n2 = whatif ();
		if (n1->type != n2->type) {
			if (n1->type == Int) n1 = makestr (n1);
			else n2 = makestr (n2);
		}
		n->n2->n1 = n1;
		n->n2->n2 = n2;
		n->type = n->n2->n1->type;
	}
	return (n);
}

hidden Node *
andif ()
{
	Node		*n, *n1;

	n = orif ();
	if (sym == Andif) {
		n1 = makeint (n);
		n = newnode ();
		n->func = xandif;
		n->type = Int;
		n->n1 = n1;
		scan ();
		n->n2 = makeint (andif ());
	}
	return (n);
}

hidden Node *
orif ()
{
	Node		*n, *n1;

	n = comparison ();
	if (sym == Orif) {
		n1 = makeint (n);
		n = newnode ();
		n->func = xorif;
		n->type = Int;
		n->n1 = n1;
		scan ();
		n->n2 = makeint (orif ());
	}
	return (n);
}

hidden Node *
comparison ()
{
	Node		*n, *n1, *n2;
	int		operator;

	n = subst ();
	if (sym == Comp) {
		operator = symint;
		n1 = n;
		n = newnode ();
		scan ();
		n2 = comparison ();
		if (operator == Match || operator == Notmatch) {
			if (n1->type == Int) n1 = makestr (n1);
			if (n2->type == Int) n2 = makestr (n2);
		} else if (n1->type != n2->type) {
			if (n1->type == Int) n1 = makestr (n1);
			else n2 = makestr (n2);
		}
		n->func = cmpfunc [operator][n1->type];
		n->type = Int;
		n->n1 = n1;
		n->n2 = n2;
	}
	return (n);
}

hidden Node *
subst ()
{
	Node		*n, *n1;

	n = cat ();
	if (sym == Subst) {
		n1 = makestr (n);
		n = newnode ();
		n->func = xsubst;
		n->type = String;
		n->n1 = n1;
		scan ();
		n->n2 = makestr (subst ());
	}
	return (n);
}

hidden Node *
cat ()
{
	Node		*n, *n1;

	n = bor ();
	if (sym == Cat) {
		n1 = makestr (n);
		n = newnode ();
		n->func = xcat;
		n->type = String;
		n->n1 = n1;
		scan ();
		n->n2 = makestr (cat ());
	}
	return (n);
}

hidden Node *
bor ()
{
	Node		*n, *n1;

	n = bxor ();
	if (sym == Or) {
		n1 = makeint (n);
		n = newnode ();
		n->func = xbor;
		n->type = Int;
		n->n1 = n1;
		scan ();
		n->n2 = makeint (bor ());
	}
	return (n);
}

hidden Node *
bxor ()
{
	Node		*n, *n1;

	n = band ();
	if (sym == Xor) {
		n1 = makeint (n);
		n = newnode ();
		n->func = xbxor;
		n->type = Int;
		n->n1 = n1;
		scan ();
		n->n2 = makeint (bxor ());
	}
	return (n);
}

hidden Node *
band ()
{
	Node		*n, *n1;

	n = sum ();
	if (sym == And) {
		n1 = makeint (n);
		n = newnode ();
		n->func = xand;
		n->type = Int;
		n->n1 = n1;
		scan ();
		n->n2 = makeint (band ());
	}
	return (n);
}

hidden Node *
sum ()
{
	Node		*n, *n1;

	n = term ();
	if (sym == Plus || sym == Minus) {
		n1 = makeint (n);
		n = newnode ();
		n->func = sym == Plus ? xadd : xsub;
		n->type = Int;
		n->n1 = n1;
		scan ();
		n->n2 = makeint (sum ());
	}
	return (n);
}

hidden Node *
term ()
{
	Node		*n, *n1;

	n = factor ();
	if (sym == Times || sym == Over) {
		n1 = makeint (n);
		n = newnode ();
		n->func = sym == Times ? xmul : xdiv;
		n->type = Int;
		n->n1 = n1;
		scan ();
		n->n2 = makeint (term ());
	}
	return (n);
}

hidden Node *
factor ()
{
	Node		*n;

	if (sym == Invert || sym == Not) {
		n = newnode ();
		n->func = sym == Invert ? xinvert : xnot;
		n->type = Int;
		scan ();
		n->n1 = makeint (value ());
	} else n = value ();
	return (n);
}

hidden Node *
value ()
{
	Node		*n;

	if (sym == Lpar) {
		scan ();
		n = expression ();
		if (sym != Rpar) synerr ("expected right paren");
		scan ();
	} else if (sym == Func) {
		n = newnode ();
		n->func = symname->val;
		n->type = symname->type;
		scan ();
		arglist (n);
	} else if (sym == Var) {
		n = newnode ();
		n->func = (symname->type == Int ? xvari : xvars);
		n->type = symname->type;
		n->n1 = (Node *) symbase;
		n->n2 = (Node *) symdisp;
		scan ();
	} else if (sym == Attribute) {
		n = newnode ();
		n->n1 = (Node *) idb_atname (symval);
		scan ();
		if (sym == Lbrack) {
			scan ();
			n->func = xattrarg;
			n->type = String;
			n->n2 = expression ();
			if (sym != Rbrack) synerr ("expected right bracket");
			scan ();
		} else {
			n->func = xattr;
			n->type = Int;
		}
	} else if (sym == Sconst) {
		n = newnode ();
		n->func = xsconst;
		n->type = String;
		n->n1 = (Node *) idb_stash (symval, pset);
		scan ();
	} else if (sym == Iconst) {
		n = newnode ();
		n->func = xiconst;
		n->type = Int;
		n->n1 = (Node *) symint;
		scan ();
	} else synerr ("expected primary value");
	return (n);
}

hidden int
indexlist (n)		/* parse an attribute index list */
	Node		*n;
{
	while (sym != Rbrack && sym != Semi && sym != Eof) {
		n->n2 = newnode (); n = n->n2;
		n->func = xlist;
		n->type = String;
		n->n1 = range ();
	}
	scan ();
	n->n2 = NULL;
}

hidden Node *
range ()		/* parse a range in an attribute index list */
{
	Node		*n, *n1;

	n = newnode ();
	n->type = String;
	n->n1 = makeint (whatif ());
	if (sym == Dotdot) {
		scan ();
		n->func = xrange;
		n->n2 = whatif ();
	} else {
		n->func = xonearg;
	}
	return (n);
}

hidden void
arglist (n)		/* parse a function argument list */
	Node		*n;
{
	Node		*n0;
	int		argc;

	if (sym != Lpar) synerr ("expected argument list");
	scan ();
	argc = 0;
	n0 = n;
	while (sym != Rpar) {
		if (sym == Eof) synerr ("expected right paren");
		n0->n2 = newnode ();
		n0 = n0->n2;
		n0->n1 = assign ();
		if (sym == Comma) scan ();
		++argc;
	}
	scan ();
	n0->n2 = NULL;
	n->n1 = (Node *) argc;
}

hidden Node *
makebool (n)		/* convert node to boolean */
	Node		*n;
{
	Node		*n1;

	if (n->type == String) {
		n1 = n;
		n = newnode ();
		n->func = xctob;
		n->type = Int;
		n->n1 = n1;
	}
	return (n);
}

hidden Node *
makeint (n)		/* convert node to integer */
	Node		*n;
{
	Node		*n1;

	if (n->type != Int) {
		n1 = n;
		n = newnode ();
		n->func = xctoi;
		n->type = Int;
		n->n1 = n1;
	}
	return (n);
}

hidden Node *
makestr (n)		/* convert node to string */
	Node		*n;
{
	Node		*n1;

	if (n->type != String) {
		n1 = n;
		n = newnode ();
		n->func = xitoc;
		n->type = String;
		n->n1 = n1;
	}
	return (n);
}

hidden int
scan ()		/* get next token */
{
	int		base, pch, quote, neg;
	register char	*p;

	ch = nextch ();
	while (isspace (ch)) {
		if (ch == '\n') {
			pch = peekch (pfile);
			if (!isspace (pch)) return (sym = Semi);
		}
		nextch ();
	}
	if (ch == EOF) return (sym = Eof);
	pch = peekch (pfile);
	if ((neg = ch == '-') && isdigit (pch) || isdigit (ch)) {
		if (neg) nextch ();
		sym = Iconst;
		base = (ch == '0' ? 8 : 10);
		for (symint = 0; isdigit (ch); nextch ()) {
			symint = symint * base + ch - '0';
		}
		if (neg) symint = -symint;
		ungetc (ch, pfile);
	} else if (isalpha (ch) || ch == '_' || ch == '@') {
		if (quote = (ch == '@')) nextch ();
		for (p = symval; isalnum (ch) || isdigit (ch) || ch == '_' ||
		    ch == '.' || ch == '%'; nextch ()) {
			*p++ = ch;
		}
		ungetc (ch, pfile);
		*p = '\0';
		if (!quote && (symname = findname (symval)) != NULL) {
			sym = symname->sym;
		} else sym = Attribute;
	} else switch (ch) {
	case EOF:
		sym = Eof;
		break;
	case '"':
	case '\'':
		quote = ch;
		nextch ();
		sym = Sconst;
		for (p = symval; ch != quote; nextch ()) {
			if (ch == EOF) { sym = Error; break; }
			*p++ = ch;
		}
		*p = '\0';
		break;
	case '!':
		sym = Comp;
		if (pch == '=') nextch (), symint = Nequal;
		else if (pch == '~') nextch (), symint = Notmatch;
		else sym = Not;
		break;
	case '&':
		if (pch == '&') nextch (), sym = Andif;
		else sym = And;
		break;
	case '(':
		sym = Lpar;
		break;
	case ')':
		sym = Rpar;
		break;
	case '{':
		sym = Lbrace;
		break;
	case '}':
		sym = Rbrace;
		break;
	case '*':
		sym = Times;
		break;
	case '+':
		sym = Plus;
		break;
	case '-':
		sym = Minus;
		break;
	case '/':
		if (pch == '/') nextch (), sym = Subst;
		else sym = Over;
		break;
	case '<':
		sym = Comp;
		if (pch == '=') nextch (), symint = Lequal;
		else symint = Less;
		break;
	case '=': 
		if (pch == '=') sym = Comp, symint = Equal, nextch ();
		else if (pch == '~') sym = Comp, symint = Match, nextch ();
		else sym = Assign;
		break;
	case '>':
		sym = Comp;
		if (pch == '=') nextch (), symint = Gequal;
		else symint = Greater;
		break;
	case '[':
		sym = Lbrack;
		break;
	case ']':
		sym = Rbrack;
		break;
	case '^':
		sym = Xor;
		break;
	case '|':
		if (pch == '|') nextch (), sym = Orif;
		else sym = Or;
		break;
	case '~':
		sym = Invert;
		break;
	case '?':
		sym = Question;
		break;
	case '.':
		if (pch == '.') nextch (), sym = Dotdot;
		else sym = Dot;
		break;
	case ':':
		if (pch == ':') nextch (), sym = Cat;
		else sym = Colon;
		break;
	case ',':
		sym = Comma;
		break;
	case ';':
		sym = Semi;
		break;
	default:
		sym = Error;
		break;
	}
	return (sym);
}

hidden int
chescape ()
{
	int		n;

	nextch ();
	if (ch >= '0' && ch < '8') {
		n = ch - '0';
		nextch ();
		if (ch >= '0' && ch < '8') {
			n = n * 8 + ch - '0';
			nextch ();
			if (ch >= '0' && ch < '8') {
				n = n * 8 + ch - '0';
			nextch ();
			}
		}
		ch = n;
	} else switch (ch) {
	case 'b': ch = '\b'; break;
	case 'f': ch = '\f'; break;
	case 'n': ch = '\n'; break;
	case 'r': ch = '\r'; break;
	case 't': ch = '\t'; break;
	}
	return (esc (ch));
}

hidden Name *
findname (name)
	char		*name;
{
	if ((symdisp = lookupname (name, builtins, nbuiltin)) >= 0) {
		symbase = &builtinbase;
		return (builtins + symdisp);
	}
	if ((symdisp = lookupname (name, locals, nlocals)) >= 0) {
		symbase = &localbase;
		return (locals + symdisp);
	}
	if ((symdisp = lookupname (name, globals, nglobals)) >= 0) {
		symbase = &globalbase;
		return (globals + symdisp);
	}
	return (NULL);
}

hidden int
lookupname (name, table, size)
	char		*name;
	Name		*table;
	register int	size;
{
	register Name	*pp;
	register char	*s, *p;

	for (pp = table; size-- > 0; ++pp) {
		p = pp->name;
		s = name;
		while (*p && *p == *s) ++p, ++s;
		if (*p == '\0' && *s == '\0') return (pp - table);
	}
	return (-1);
}

hidden int
addglobal (name)
	char		*name;
{
	globals = (Name *) idb_getmore (globals,
		(nglobals + 1) * sizeof (*globals), xset);
	globals [nglobals].name = idb_stash (name, xset);
	return (nglobals++);
}

hidden int
addlocal (name)
	char		*name;
{
	locals = (Name *) idb_getmore (locals,
		(nlocals + 1) * sizeof (*locals), xset);
	locals [nlocals].name = idb_stash (name, xset);
	return (nlocals++);
}

Rec *
idb_select (f, n, memset)	/* get next selected record */
	FILE		*f;
	Node		*n;
{
	Rec		*rec;
	int		t;

	while ((rec = idb_read (f, memset)) != NULL) {
		setvars (rec);
		t = idb_expr (rec, n, memset);
		if (n->type == String) {
			t = (char *)t != NULL && *(char *)t != '\0';
		}
		if (t) return (rec);
	}
	return (NULL);
}

int
idb_expr (rec, n, set)		/* execute an expression on a record */
	Rec		*rec;
	Node		*n;
	Memset		*set;
{
	int		t;
	jmp_buf		breakbuf, contbuf;

	xrec = rec; xset = set;
	setvars (rec);
	if (setjmp (xbotch) || setjmp (breakbuf) || setjmp (contbuf)) t = 0;
	else {
		breakjmp = breakbuf;
		contjmp = contbuf;
		t = n->func (n);
	}
	return (t);
}

hidden void
setvars (rec)
	Rec		*rec;
{
	static char	ftypebuff [2] = "x";

	ftypebuff [0] = idb_typec (rec->type);

	if (findname ("mode") != NULL)
		(*symbase) [symdisp] = rec->mode;
	if (findname ("type") != NULL)
		(*symbase) [symdisp] = (int) ftypebuff;
	if (findname ("owner") != NULL)
		(*symbase) [symdisp] = (int) rec->user;
	if (findname ("group") != NULL)
		(*symbase) [symdisp] = (int) rec->group;
	if (findname ("srcpath") != NULL)
		(*symbase) [symdisp] = (int) rec->srcpath;
	if (findname ("dstpath") != NULL)
		(*symbase) [symdisp] = (int) rec->dstpath;
	if (findname ("nattr") != NULL)
		(*symbase) [symdisp] = rec->nattr;
}

/* idb_setbase () -- set base pathnames
 *
 * Null strings in rpath, spath, and idb are replaced by the appropriate
 * defaults (first the builtin values, then over-ridden by the environment).
 * The values are stashed away for access in user expressions, for use
 * by the rpath() and spath() builtin functions, and the idb_rpath() and
 * idb_spath() library functions.
 */

void
idb_setbase ()
{
	char		*p, passwd [1024], group [1024];

	if (*rbase == '\0') {
		strcpy (rbase, RBASE);
		if ((p = getenv ("rbase")) != NULL) strcpy (rbase, p);
	}
	if (*sbase == '\0') {
		strcpy (sbase, SBASE);
		if ((p = getenv ("sbase")) != NULL) strcpy (sbase, p);
	}
	if (*idb == '\0') {
		sprintf (idb, "%s/idb", sbase);
		if ((p = getenv ("idb")) != NULL) strcpy (idb, p);
	}
	if (findname ("rbase") != NULL) 
		(*symbase) [symdisp] = (int) idb_stash (rbase, NULL);
	if (findname ("sbase") != NULL)
		(*symbase) [symdisp] = (int) idb_stash (sbase, NULL);
	if (findname ("idb") != NULL)
		(*symbase) [symdisp] = (int) idb_stash (idb, NULL);
	sprintf (passwd, "%s/etc/passwd", sbase);
	sprintf (group, "%s/etc/group", sbase);
	if (idb_passwd (passwd, group) < 0) {
		sprintf (passwd, "%s/etc/passwd", rbase);
		sprintf (group, "%s/etc/group", rbase);
		if (idb_passwd (passwd, group) < 0) {
			if (idb_passwd ("/etc/passwd", "/etc/group") < 0) {
				fprintf (stderr, "no /etc/passwd or group!");
			}
		}
	}
}

/* The following routines execute nodes in the tree.  Each one (with certain
 * exceptions) is called with it's own node as an argument, and returns the
 * result cast into an integer.
 */

hidden int
xadd (n)
	Node		*n;
{
	return (n->n1->func (n->n1) + n->n2->func (n->n2));
}

hidden int
xand (n)
	Node		*n;
{
	return (n->n1->func (n->n1) & n->n2->func (n->n2));
}

hidden int
xandif (n)
	Node		*n;
{
	return (n->n1->func (n->n1) && n->n2->func (n->n2));
}

hidden int
xargc (n)
	Node		*n;
{
	Attr		*at;

	if ((at = findattr (n->n1, xrec)) == NULL) {
		return (-1);
	} else {
		return (at->argc);
	}
}

hidden int
xassigni (n)
	Node		*n;
{
	int		t;

	t = n->n2->func (n->n2);
	(*(int **) n->n1->n1) [(int) n->n1->n2] = t;
	return (t);
}

hidden int
xassigns (n)
	Node		*n;
{
	int		t;

	t = n->n2->func (n->n2);
	(*(char ***) n->n1->n1) [(int) n->n1->n2] = (char *) t;
	return (t);
}

hidden int
xattr (n)
	Node		*n;
{
	Attr		*at;
	
	return (findattr (n->n1, xrec) != NULL);
}

hidden int
xattrarg (n)
	Node		*n;
{
	int		x;
	char		*p;
	Attr		*at;

	if ((at = findattr (n->n1, xrec)) == NULL ||
	    (x = n->n2->func (n->n2)) < 0 || x >= at->argc) {
		p = "";
	} else {
		p = at->argv [x];
	}
	return ((int) idb_stash (p, xset));
}

hidden int
xbor (n)
	Node		*n;
{
	return (n->n1->func (n->n1) | n->n2->func (n->n2));
}

hidden int
xbreak (n)
	Node		*n;
{
	/* longjmp (breakjmp, 1); */
}

hidden int
xbxor (n)
	Node		*n;
{
	return (n->n1->func (n->n1) ^ n->n2->func (n->n2));
}

hidden int
xcat (n)
	Node		*n;
{
	char		*p1, *p2;

	p1 = (char *) n->n1->func (n->n1);
	p2 = (char *) n->n2->func (n->n2);
	p1 = idb_cat (p1, p2, xset);
	return ((int) p1);
}

hidden int
xctob (n)
	Node		*n;
{
	char		*p;

	p = (char *) n->n1->func (n->n1);
	return (p != NULL && *p != '\0');
}

hidden int
xctoi (n)
	Node		*n;
{
	char		*p;

	p = (char *) n->n1->func (n->n1);
	return (atoi (p));
}

hidden int
xcomma (n)
	Node		*n;
{
	int		r;

	return (n->n1->func (n->n1), n->n2->func (n->n2));
}

hidden int
xcont (n)
	Node		*n;
{
	/* longjmp (contjmp, 1); */
}

hidden int
xdiv (n)
	Node		*n;
{
	return (n->n1->func (n->n1) / n->n2->func (n->n2));
}

hidden int
xequali (n)
	Node		*n;
{
	return (n->n1->func (n->n1) == n->n2->func (n->n2));
}

hidden int
xequals (n)
	Node		*n;
{
	return (strcmp ((char *) n->n1->func (n->n1),
		(char *) n->n2->func (n->n2)) == 0);
}

hidden int
xfor (n)
	Node		*n;
{
	int		t;
	int		*prevbreak, *prevcont;
	jmp_buf		breakbuf, contbuf;

	t = 0;
	if (setjmp (breakbuf)) goto broken;
	if (setjmp (contbuf)) goto cont;
	prevbreak = breakjmp, breakjmp = breakbuf;
	prevcont = contjmp, contjmp = contbuf;
	for (n->n1->n1->func (n->n1->n1); n->n1->n2->func (n->n1->n2);
	    n->n2->n1->func (n->n2->n1)) {
		t = n->n2->n2->func (n->n2->n2);
cont:
		;
	}
broken:
	breakjmp = prevbreak;
	contjmp = prevcont;
	return (t);
}

hidden int
xgequali (n)
	Node		*n;
{
	return (n->n1->func (n->n1) >= n->n2->func (n->n2));
}

hidden int
xgequals (n)
	Node		*n;
{
	return (strcmp ((char *) n->n1->func (n->n1),
		(char *) n->n2->func (n->n2)) >= 0);
}

hidden int
xgreateri (n)
	Node		*n;
{
	return (n->n1->func (n->n1) > n->n2->func (n->n2));
}

hidden int
xgreaters (n)
	Node		*n;
{
	return (strcmp ((char *) n->n1->func (n->n1),
		(char *) n->n2->func (n->n2)) >= 0);
}

hidden int
xiconst (n)
	Node		*n;
{
	return ((int)n->n1);
}

hidden int
xif (n)
	Node		*n;
{
	int		t;

	if (n->n1->func (n->n1)) t = n->n2->n1->func (n->n2->n1);
	else t = n->n2->n2->func (n->n2->n2);
	return (t);
}

hidden int
xinvert (n)
	Node		*n;
{
	return (~n->n1->func (n->n1));
}

hidden int
xitoc (n)
	Node		*n;
{
	int		i;

	return ((int) idb_stash (idb_itoc (n->n1->func (n->n1), 0, 10, '0'),
		xset));
}

hidden int
xlequali (n)
	Node		*n;
{
	return (n->n1->func (n->n1) <= n->n2->func (n->n2));
}

hidden int
xlequals (n)
	Node		*n;
{
	return (strcmp ((char *) n->n1->func (n->n1),
		(char *) n->n2->func (n->n2)) <= 0);
}

hidden int
xlessi (n)
	Node		*n;
{
	return (n->n1->func (n->n1) < n->n2->func (n->n2));
}

hidden int
xlesss (n)
	Node		*n;
{
	return (strcmp ((char *) n->n1->func (n->n1),
		(char *) n->n2->func (n->n2)) <= 0);
}

hidden int
xlist (n, p, attr)
	Node		*n;
	char		*p;
	Attr		*attr;
{
	Node		*n1;

	while (n != NULL) {
		p = (char *) n->n1->func (n->n1, p, attr);
		n = n->n2;
	}
	return ((int) p);
}

hidden int
xmatch (n)
	Node		*n;
{
	return (match ((char *) n->n1->func (n->n1),
		(char *) n->n2->func (n->n2), Int, xset));
}

hidden int
xmul (n)
	Node		*n;
{
	return (n->n1->func (n->n1) * n->n2->func (n->n2));
}

hidden int
xnequali (n)
	Node		*n;
{
	return (n->n1->func (n->n1) != n->n2->func (n->n2));
}

hidden int
xnequals (n)
	Node		*n;
{
	return (strcmp ((char *) n->n1->func (n->n1),
		(char *) n->n2->func (n->n2)) != 0);
}

hidden int
xnop (n)
	Node		*n;
{
	return (0);
}

hidden int
xnot (n)
	Node		*n;
{
	return (!n->n1->func (n->n1));
}

hidden int
xnotmatch (n)
	Node		*n;
{
	return (!match ((char *) n->n1->func (n->n1),
		(char *) n->n2->func (n->n2), Int, xset));
}

hidden int
xonearg (n, p, attr)
	Node		*n;
	char		*p;
	Attr		*attr;
{
	int		i;

	i = n->n1->func (n->n1) - 1;
	if (i < 0 || i >= attr->argc) return ((int) p);
	p = idb_cat (p, attr->argv [i], xset);
	p = idb_cat (p, " ", xset);
	return ((int) p);
}

hidden int
xorif (n)
	Node		*n;
{
	return (n->n1->func (n->n1) || n->n2->func (n->n2));
}

hidden int
xrange (n, p, attr)
	Node		*n;
	char		*p;
	Attr		*attr;
{
	char		*v;
	int		i, low, high, size, argc;

	low = n->n1->func (n->n1) - 1;		/* they use origin 1 */
	high = n->n2->func (n->n2) - 1;
	size = (p == NULL ? 0 : strlen (p)) + 1;
	if (low < 0) low = 0;
	if (high >= attr->argc) high = attr->argc - 1;
	for (i = low; i <= high; ++i) {
		p = idb_cat (p, attr->argv [i], xset);
		p = idb_cat (p, " ", xset);
	}
	return ((int) p);
}

hidden int
xreturn (n)
{
	return (0);
}

hidden int
xsconst (n)
	Node		*n;
{
	return ((int) idb_stash ((char *) n->n1, xset));
}

hidden int
xstmtlist (n)
	Node		*n;
{
	int		t;

	while (n != NULL) {
		t = n->n1->func (n->n1);
		n = n->n2;
	}
	return (t);
}

hidden int
xsub (n)
	Node		*n;
{
	return (n->n1->func (n->n1) - n->n2->func (n->n2));
}

hidden int
xsubst (n)
	Node		*n;
{
	return (match ((char *) n->n1->func (n->n1),
		(char *) n->n2->func (n->n2), String, xset));
}

hidden int
xvari (n)
	Node		*n;
{
	return ((*(int **)n->n1) [(int) n->n2]);
}

hidden int
xvars (n)
	Node		*n;
{
	return ((int) idb_stash ((*(char ***)n->n1) [(int) n->n2], xset));
}

hidden int
xwhatif (n)
	Node		*n;
{
	return (n->n1->func (n->n1) ?
		n->n2->n1->func (n->n2->n1) :
		n->n2->n2->func (n->n2->n2));
}

hidden int
xwhile (n)
	Node		*n;
{
	int		t, *prevbreak, *prevcont;
	jmp_buf		breakbuf, contbuf;

	t = 0;
	if (setjmp (breakbuf)) goto broken;
	if (setjmp (contbuf)) goto cont;
	prevbreak = breakjmp, breakjmp = breakbuf;
	prevcont = contjmp, contjmp = contbuf;
	while (n->n1->func (n->n1)) {
		t = n->n2->func (n->n2);
cont:
		;
	}
broken:
	breakjmp = prevbreak;
	contjmp = prevcont;
	return (t);
}

/* user-accessable builtin functions */

hidden int
xprint (n)
	register Node		*n;
{
	int		t;

	for (n = n->n2; n != NULL; n = n->n2) {
		t = n->n1->func (n->n1);
		if (n->n1->type == String) {
			printf ("%s", (char *) t);
		} else {
			printf ("%d", t);
		}
		putchar (n->n2 == NULL ? '\n' : ' ');
	}
}

hidden int
xmatchat (n)
	register Node		*n;
{
	int		t;
	Attr		*at;

	for (n = n->n2; n != NULL; n = n->n2) {
		t = n->n1->func (n->n1);
		if (n->n1->type != String) continue;
		for (at = xrec->attr; at < xrec->attr + xrec->nattr; ++at) {
			if (match (at->atname, t, Int, xset)) return (1);
		}
	}
	return (0);
}

hidden int
xmatchats (n)
	register Node	*n;
{
	int		t;
	Attr		*at;

	for (n = n->n2; n != NULL; n = n->n2) {
		t = n->n1->func (n->n1);
		if (n->n1->type != String) continue;
		for (at = xrec->attr; at < xrec->attr + xrec->nattr; ++at) {
			if (match (at->atname, t, Int, xset)) {
				return ((int) idb_stash (at->atname, xset));
			}
		}
	}
	return ((int) idb_stash ("", xset));
}

hidden int
xprintf (n)
	register Node		*n;
{
	register char	*fmt, *p;
	int		i, j, nch, w, type, pad;
	char		buff [10];

	if (n->n2 == NULL) return (0);
	n = n->n2;
	if (n->n1->type != String) fmt = "(Bad format type)";
	else fmt = (char *) n->n1->func (n->n1);
	nch = 0;
	while (*fmt) {
		if (*fmt != '%') {
			putchar (*fmt++); ++nch;
			continue;
		}
		if (n->n2 == NULL) p = 0, i = 0;
		else {
			n = n->n2;
			type = n->n1->type;
			i = n->n1->func (n->n1);
			p = (char *) i;
		}
		pad = ' ';
		if (fmt [1] == '0') pad = '0';
		w = 0;
		while (*++fmt >= '0' && *fmt <= '9') {
			w = w * 10 + *fmt - '0';
		}
		if (*fmt == 's') {
			if (type != String) p = "(error)";
			while (*p) { putchar (*p), ++p, --w, ++nch; }
			while (w > 0) { putchar (' '), --w, ++nch; }
		} else if (*fmt == 'd' || *fmt == 'o') {
			if (type != Int) i = 0;
			p = idb_itoc (i, w, *fmt == 'd' ? 10 : 8, pad);
			while (*p) {
				putchar (*p++); ++nch;
			}
		} else {
			putchar ('%'); putchar (*fmt);
			nch += 2;
		}
		++fmt;
	}
	return (nch);
}

hidden int
xputrec (n)
	Node		*n;
{
	idb_write (stdout, xrec);
	return (0);
}

hidden int
xaccess (n)
	register Node		*n;
{
	int		mode;
	char		*name;

	if ((int) n->n1 != 2 ||
	    n->n2->n1->type != String ||
	    n->n2->n2->n1->type != Int) return (-1);
	name = (char *) n->n2->n1->func (n->n2->n1);
	mode = n->n2->n2->n1->func (n->n2->n2->n1);
	return (access (name, mode));
}

hidden int
xbasename (n)
	register Node	*n;
{
	int		nargs, nlen, slen;
	char		*name, *suffix, buff [Buffsize];
	register char	*p;

	if ((nargs = (int) n->n1) < 1 || nargs > 2) return ((int) "");
	if (n->n2->n1->type != String) return ((int) "");
	if (nargs == 2) {
		if (n->n2->n2->n1->type != String) return (-1);
		suffix = (char *) n->n2->n2->n1->func (n->n2->n2->n1);
	} else {
		suffix = "";
	}
	name = (char *) n->n2->n1->func (n->n2->n1);
	for (p = name + strlen (name) - 1; p >= name; --p) {
		if (*p == '/') break;
	}
	strcpy (buff, p + 1);
	nlen = strlen (buff);
	slen = strlen (suffix);
	if (nlen > slen && strcmp (buff + nlen - slen, suffix) == 0) {
		buff [nlen - slen] = '\0';
	}
	return ((int) idb_stash (buff, xset));
}

hidden int
xdirname (n)
	register Node	*n;
{
	char		buff [Buffsize], *p;

	if ((int) n->n1 != 1 || n->n2->n1->type != String) return ((int) "");
	strcpy (buff, (char *) n->n2->n1->func (n->n2->n1));
	for (p = buff + strlen (buff) - 1; p >= buff; --p) {
		if (*p == '/') break;
	}
	if (p > buff) {
		*p = '\0';
	} else if (p == buff) {
		p [1] = '\0';
	} else {
		strcpy (buff, ".");
	}
	return ((int) idb_stash (buff, xset));
}

hidden int
xtypeof (n)
	register Node	*n;
{
}

hidden int
xmodeof (n)
	register Node	*n;
{
}

hidden int
xrpath (n)
	register Node	*n;
{
	char		*p;

	if ((int) n->n1 == 1 && n->n2->n1->type == String &&
	    findname ("rbase") != NULL) {
		p = idb_stash ((char *) (*symbase) [symdisp], xset);
	} else {
		p = "";
	}
	if (*p) p = idb_cat (p, "/", xset);
	return ((int) idb_cat (p, (char *) n->n2->n1->func (n->n2->n1), xset));
}

hidden int
xspath (n)
	register Node	*n;
{
	char		*p;

	if ((int) n->n1 == 1 && n->n2->n1->type == String &&
	    findname ("sbase") != NULL) {
		p = idb_stash ((char *) (*symbase) [symdisp], xset);
	} else {
		p = "";
	}
	if (*p) p = idb_cat (p, "/", xset);
	return ((int) idb_cat (p, (char *) n->n2->n1->func (n->n2->n1), xset));
}

hidden int
xbytes (n)
	Node		*n;
{
	char		*p;
	struct stat	st;

	if ((int) n->n1 != 1 || n->n2->n1->type != String) return (0);
	p = (char *) n->n2->n1->func (n->n2->n1);
	if (stat (p, &st) < 0) return (0);
	return (st.st_size);
}

hidden int
xblocks (n)
	Node		*n;
{
	char		*p;
	struct stat	st;

	if ((int) n->n1 != 1 || n->n2->n1->type != String) return (0);
	p = (char *) n->n2->n1->func (n->n2->n1);
	if (stat (p, &st) < 0) return (0);
	return (toblocks (st.st_size));
}

/* xatstr (attrname) -- return string value of named attribute */

hidden int
xatstr (n)
	Node		*n;
{
	Attr		*at;
	char		*p;
	int		i;

	if ((int) n->n1 != 1 || n->n2->n1->type != Int ||
	    (at = findattr (n->n2->n1->func (n->n2->n1), xrec)) == NULL) {
		p = idb_stash ("", xset);
	} else {
		p = idb_stash (at->atname, xset);
		for (i = 0; i < at->argc; ++i) {
			if (i == 0) p = idb_cat (p, "(", xset);
			p = idb_cat (p, at->argv [i], xset);
			p = idb_cat (p, i == at->argc - 1 ? " " : ")", xset);
		}
	}
	return ((int) p);
}

/* various support routines */

char *
idb_itoc (i, width, base, pad)		/* itoc -- integer to characters */
	int		i;
	int		width;
	register int	base;
	int		pad;
{
	register char	*p, *s;
	static char	buff [20], rbuff [20];
	int		minus;

	p = buff;
	i = (minus = i < 0) ? -i : i;
	if (i == 0) *p++ = '0';
	else while (i > 0) {
		*p++ = i % base + '0';
		i /= base;
	}
	if (minus && base == 10) *p++ = '-';
	while (p - buff < width) *p++ = pad;
	s = rbuff;
	while (--p >= buff) *s++ = *p;
	*s = '\0';
	return (rbuff);
}

/* match -- pattern matching
 *
 * The string argument "s" is matched with the pattern "pat".  The return
 * value is an integer (non-zero is a match) or a stashed string, according
 * to the requested type.  The memset is not used unless a string is being
 * returned.
 */

hidden int
match (s, pat, type, set)
	register char	*s;
	register char	*pat;
	int		type;
	Memset		*set;
{
	char		*p, *hit;
	int		t;

	t = runmatch (s, pat);
	if (type == Int) return (t);
	if (!t) return (NULL);
	p = hit = idb_getmem (hitend - hitstart, set);
	while (hitstart < hitend) *p++ = unesc (*hitstart++);
	*p = '\0';
	return ((int) hit);
}

hidden int
runmatch (s, pat)
	register char	*s;
	register char	*pat;
{
	char		*starts;
	register int	n;

	if (*pat == '\0') return (*s == '\0');
	while (*pat == esc ('(') || *pat == esc (')')) {
		if (*pat == esc ('(')) hitstart = s;
		else hitend = s;
		pat += 2;
	}
	if (*pat == '*') {
		starts = s;
		while (*s) ++s;
		while (s >= starts) {
			if (runmatch (s, pat + 1)) return (1);
			--s;
		}
		return (0);
	}
	if (*pat == '%') {
		starts = s;
		while (*s && *s != '/') ++s;
		while (s >= starts) {
			if (runmatch (s, pat + 1)) return (1);
			--s;
		}
		return (0);
	}
	if (*s == '\0') return (0);
	if (n = onecharmatch (*s, pat)) return (runmatch (s + 1, pat + n));
	return (0);
}

hidden int
onecharmatch (ch, p)
	register int	ch;
	register char	*p;
{
	register int	c, m;	
	int		not;
	char		*initp;

	initp = p;
	c = *p++;
	if (c == '\\') {
		m = (ch == *p++);
	} else if (c == '[') {
		m = not = 0;
		if ((c = *p++) == '^') { ++not, c = *p++; }
		do {
			if (*p == '-') {
				++p;
				m = m || (ch >= c && ch <= *p);
				++p;
			} else {
				m = m || (ch == c);
			}
		} while ((c = *p++) != ']');
		if (not) m = !m;
	} else if (c == '?') {
		m = 1;
	} else m = (ch == c);
	if (!m) return (0);
	else return (p - initp);
}

/* look for an attribute */

hidden Attr *
findattr (name, rec)
	register char	*name;
	register Rec	*rec;
{
	register Attr	*at;

	for (at = rec->attr; at < rec->attr + rec->nattr; ++at) {
		if (strcmp (at->atname, name) == 0) return (at);
	}
	return (NULL);
}
