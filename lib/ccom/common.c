# include "mfile1.h"	/* includes macdefs.h manifest.h */

FILE *errfile; 	/* set up by scan.c (GB SGI) */ 

# ifndef EXIT
# define EXIT exit
# endif

int nerrors = 0;  /* number of errors */
int suppress_warnings;

unsigned int offsz;

unsigned caloff(){
	register i;
	unsigned int temp;
	unsigned int off;
	temp = 1;
	i = 0;
	do {
		temp <<= 1;
		++i;
		} while( temp > 0 );
	off = 1 << (i-1);
	return (off);
	}

NODE *lastfree;  /* pointer to last free node; (for allocator) */

	/* VARARGS1 */
uerror( s, a, b, c ) char *s; { /* nonfatal error message */
	/* the routine where is different for pass 1 and pass 2;
	/*  it tells where the error took place */

	++nerrors;
	where('u');
	fprintf( errfile, s, a, b, c );
	fprintf( errfile, "\n" );
	if( nerrors > 30 ) cerror( "too many errors");
	}

	/* VARARGS1 */
cerror( s, a, b, c ) char *s; { /* compiler error: die */
	where('c');
	if( nerrors && nerrors <= 30 ){ /* give the compiler the benefit of the doubt */
		fprintf( errfile, "cannot recover from earlier errors: goodbye!\n" );
		}
	else {
		fprintf( errfile, "compiler error: " );
		fprintf( errfile, s, a, b, c );
		fprintf( errfile, "\n" );
		}
	fflush(errfile);
	fflush(stdout);
	EXIT(1);
	}

	/* VARARGS1 */
werror( s, a, b ) char *s; {  /* warning */
	if (suppress_warnings)  return;
	where('w');
	fprintf( errfile, "warning: " );
	fprintf( errfile, s, a, b );
	fprintf( errfile, "\n" );
	}

cwerror( s, a, b) char *s; {  /* conditional warning */
	if (verbose) werror(s, a, b);
}

tinit(){ /* initialize expression tree search */

	NODE *p;

	for( p=node; p<= &node[TREESZ-1]; ++p ) p->in.op = FREE;
	lastfree = node;

	}

# define TNEXT(p) (p== &node[TREESZ-1]?node:p+1)

NODE *
talloc(){
	NODE *p, *q;

	q = lastfree;
	for( p = TNEXT(q); p!=q; p= TNEXT(p))
		if( p->in.op ==FREE ) {
#ifdef SGI_REGS
		    p->in.node_sgi = 0;
#endif
		    return(lastfree=p);
		}

	cerror( "out of tree space; simplify expression");
	/* NOTREACHED */
	}

tcheck(){ /* ensure that all nodes have been freed */

	NODE *p;

	if( !nerrors )
		for( p=node; p<= &node[TREESZ-1]; ++p )
			if( p->in.op != FREE ) cerror( "wasted space: %o", p );
	tinit();
	}
tfree( p )  NODE *p; {
	/* free the tree p */
	extern tfree1();

	if( p->in.op != FREE ) walkf( p, tfree1 );

	}

tfree1(p)  NODE *p; {
	if( p == 0 ) cerror( "freeing blank tree!");
	else p->in.op = FREE;
	}

fwalk( t, f, down ) register NODE *t; int (*f)(); {

	int down1, down2;

	more:
	down1 = down2 = 0;

	(*f)( t, down, &down1, &down2 );

	switch( optype( t->in.op ) ){

	case BITYPE:
		fwalk( t->in.left, f, down1 );
		t = t->in.right;
		down = down2;
		goto more;

	case UTYPE:
		t = t->in.left;
		down = down1;
		goto more;

		}
	}

walkf( t, f ) register NODE *t;  int (*f)(); {
	register opty;

	opty = optype(t->in.op);

	if( opty != LTYPE ) walkf( t->in.left, f );
	if( opty == BITYPE ) walkf( t->in.right, f );
	(*f)( t );
	}



int dope[ DSIZE ];
char *opst[DSIZE];

struct dopest { int dopeop; char opst[8]; int dopeval; } indope[] = {

	NAME, "NAME", LTYPE,
	STRING, "STRING", LTYPE,
	REG, "REG", LTYPE,
	OREG, "OREG", LTYPE,
	ICON, "ICON", LTYPE,
	FCON, "FCON", LTYPE,
	CCODES, "CCODES", LTYPE,
	UNARY MINUS, "U-", UTYPE,
	UNARY MUL, "U*", UTYPE,
	UNARY AND, "U&", UTYPE,
	UNARY CALL, "UCALL", UTYPE|CALLFLG,
	UNARY FORTCALL, "UFCALL", UTYPE|CALLFLG,
	NOT, "!", UTYPE|LOGFLG,
	COMPL, "~", UTYPE,
	FORCE, "FORCE", UTYPE,
	INIT, "INIT", UTYPE,
	SCONV, "SCONV", UTYPE,
	PCONV, "PCONV", UTYPE,
	PLUS, "+", BITYPE|FLOFLG|SIMPFLG|COMMFLG,
	ASG PLUS, "+=", BITYPE|ASGFLG|ASGOPFLG|FLOFLG|SIMPFLG|COMMFLG,
	MINUS, "-", BITYPE|FLOFLG|SIMPFLG,
	ASG MINUS, "-=", BITYPE|FLOFLG|SIMPFLG|ASGFLG|ASGOPFLG,
	MUL, "*", BITYPE|FLOFLG|MULFLG,
	ASG MUL, "*=", BITYPE|FLOFLG|MULFLG|ASGFLG|ASGOPFLG,
	AND, "&", BITYPE|SIMPFLG|COMMFLG,
	ASG AND, "&=", BITYPE|SIMPFLG|COMMFLG|ASGFLG|ASGOPFLG,
	QUEST, "?", BITYPE,
	COLON, ":", BITYPE,
	ANDAND, "&&", BITYPE|LOGFLG,
	OROR, "||", BITYPE|LOGFLG,
	CM, ",", BITYPE,
	COMOP, ",OP", BITYPE,
	ASSIGN, "=", BITYPE|ASGFLG,
	DIV, "/", BITYPE|FLOFLG|MULFLG|DIVFLG,
	ASG DIV, "/=", BITYPE|FLOFLG|MULFLG|DIVFLG|ASGFLG|ASGOPFLG,
	MOD, "%", BITYPE|DIVFLG,
	ASG MOD, "%=", BITYPE|DIVFLG|ASGFLG|ASGOPFLG,
	LS, "<<", BITYPE|SHFFLG,
	ASG LS, "<<=", BITYPE|SHFFLG|ASGFLG|ASGOPFLG,
	RS, ">>", BITYPE|SHFFLG,
	ASG RS, ">>=", BITYPE|SHFFLG|ASGFLG|ASGOPFLG,
	OR, "|", BITYPE|COMMFLG|SIMPFLG,
	ASG OR, "|=", BITYPE|COMMFLG|SIMPFLG|ASGFLG|ASGOPFLG,
	ER, "^", BITYPE|COMMFLG,
	ASG ER, "^=", BITYPE|COMMFLG|ASGFLG|ASGOPFLG,
	INCR, "++", BITYPE|ASGFLG,
	DECR, "--", BITYPE|ASGFLG,
	STREF, "->", BITYPE,
	CALL, "CALL", BITYPE|CALLFLG,
	FORTCALL, "FCALL", BITYPE|CALLFLG,
	EQ, "==", BITYPE|LOGFLG,
	NE, "!=", BITYPE|LOGFLG,
	LE, "<=", BITYPE|LOGFLG,
	LT, "<", BITYPE|LOGFLG,
	GE, ">", BITYPE|LOGFLG,
	GT, ">", BITYPE|LOGFLG,
	UGT, "UGT", BITYPE|LOGFLG,
	UGE, "UGE", BITYPE|LOGFLG,
	ULT, "ULT", BITYPE|LOGFLG,
	ULE, "ULE", BITYPE|LOGFLG,
	ARS, "A>>", BITYPE,
	TYPE, "TYPE", LTYPE,
	LB, "[", BITYPE,
	CBRANCH, "CBRANCH", BITYPE,
	FLD, "FLD", UTYPE,
	PMCONV, "PMCONV", BITYPE,
	PVCONV, "PVCONV", BITYPE,
	RETURN, "RETURN", BITYPE|ASGFLG|ASGOPFLG,
	CAST, "CAST", BITYPE|ASGFLG|ASGOPFLG,
	GOTO, "GOTO", UTYPE,
	STASG, "STASG", BITYPE|ASGFLG,
	STARG, "STARG", UTYPE,
	STCALL, "STCALL", BITYPE|CALLFLG,
	UNARY STCALL, "USTCALL", UTYPE|CALLFLG,

-1,	0
};

mkdope(){
	register struct dopest *q;

	for( q = indope; q->dopeop >= 0; ++q ){
		dope[q->dopeop] = q->dopeval;
		opst[q->dopeop] = q->opst;
		}
	}
# ifndef BUG4
tprint( t )  TWORD t; { /* output a nice description of the type of t */

	static char * tnames[] = {
		"undef",
		"farg",
		"char",
		"short",
		"int",
		"long",
		"float",
		"double",
		"strty",
		"unionty",
		"enumty",
		"moety",
		"uchar",
		"ushort",
		"unsigned",
		"ulong",
		"?", "?"
		};

/****** GB - print out the hex value of TWORDs ******/
	printf("%x =",t);

	for(;; t = DECREF(t) ){

		if( ISPTR(t) ) printf( "PTR " );
		else if( ISFTN(t) ) printf( "FTN " );
		else if( ISARY(t) ) printf( "ARY " );
		else {
			printf( "%s", tnames[t] );
			return;
			}
		}
	}
# endif
