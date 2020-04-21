# include "mfile1.h"	/* includes macdefs.h manifest.h */


/*	this file contains code which is dependent on the target machine */
/* 	V. Pratt Dec. 12 incorporated jks fix to fincode (insert sw) */

NODE *
cast( p, t ) register NODE *p; TWORD t; {
	/* cast node p to type t */

	p = buildtree( CAST, block( NAME, NIL, NIL, t, 0, (int)t ), p );
	p->in.left->in.op = FREE;
	p->in.op = FREE;
	return( p->in.right );
	}

NODE *
clocal(p) NODE *p; {

	/* this is called to do local transformations on
	   an expression tree preparitory to its being
	   written out in intermediate code.
	*/

	/* the major essential job is rewriting the
	   automatic variables and arguments in terms of
	   REG and OREG nodes */
	/* conversion ops which are not necessary are also clobbered here */
	/* in addition, any special features (such as rewriting
	   exclusive or) are easily handled here as well */

	register struct symtab *q;
	register NODE *r;
	register o;
	register m, ml;


	o = p->in.op;

	switch( o ){

	case NAME:
		if( p->tn.rval<0 || p->tn.rval==NONAME ) { /* already processed; ignore... */
			return(p);
			}
		q = &stab[p->tn.rval];
		switch( q->sclass ){

		case AUTO:
		case PARAM:
			/* fake up a structure reference */
			r = block( REG, NIL, NIL, PTR+STRTY, 0, 0 );
			r->tn.lval = 0;
			r->tn.rval = (q->sclass==AUTO?STKREG:ARGREG);
			p = stref( block( STREF, r, p, 0, 0, 0 ) );
			break;

		case ULABEL:
		case LABEL:
		case STATIC:
			if( q->slevel == 0 ) break;
			p->tn.lval = 0;
			p->tn.rval = -q->offset;
			break;

		case REGISTER:
			p->in.op = REG;
			p->tn.lval = 0;
			p->tn.rval = q->offset;
			break;

			}
		break;

/* GB bug#76 - insert compare to zero for logical op on float/double */
	case CBRANCH:
	case QUEST:
/*			if (optype(p->in.left->in.op) != LTYPE) break; */
			if (((m = p->in.left->in.type) != FLOAT) && (m != DOUBLE)) break;
			/* okay, left is leaf and a float/double */
			r = block(FCON,NIL,NIL,m,0,m);
			r->fpn.dval = 0.0;
/* GB scr#2411 - float f; if (f) .. : the comparison condition is inverted 
   later. Thus, we must initially compare that the target is NE 0.0
*/
/*			r = block(EQ,p->in.left,r,INT,0,INT);  */
			r = block(NE,p->in.left,r,INT,0,INT);
			p->in.left = r;
			break;
	case ANDAND:
	case OROR:
/* GB bug#76 - insert compare to zero for logical op on float/double */
			if (((m = p->in.right->in.type) == FLOAT) || (m == DOUBLE)) {
				/* okay, node is logical and right is a float/double */
				r = block(FCON,NIL,NIL,m,0,m);
				r->fpn.dval = 0.0;
/* GB scr#2411 - float f; if {||,&&} (f) .. : the comparison condition is inverted 
   later. Thus, we must initially compare that the target is NE 0.0
*/
/*				r = block(EQ,p->in.right,r,INT,0,INT);  */
				r = block(NE,p->in.right,r,INT,0,INT);
				p->in.right = r;
			}
	case NOT:
			if (((m = p->in.left->in.type) == FLOAT) || (m == DOUBLE)) {
				/* okay, node is logical and left is a float/double */
				r = block(FCON,NIL,NIL,m,0,m);
				r->fpn.dval = 0.0;
/* GB scr#2411 - float f; if {||,&&} (f) .. : the comparison condition is inverted 
   later. Thus, we must initially compare that the target is NE 0.0
*/
/*				r = block(EQ,p->in.left,r,INT,0,INT);  */
				r = block(NE,p->in.left,r,INT,0,INT);
				p->in.left = r;
			}
			break;
/* ** bug#76 */
	case LT:
	case LE:
	case GT:
	case GE:
		if( ISPTR( p->in.left->in.type ) || ISPTR( p->in.right->in.type ) ){
			p->in.op += (ULT-LT);
			}
		break;

	case PCONV:
		/* do pointer conversions for char and shorts */
		ml = p->in.left->in.type;
		if( ( ml==CHAR || ml==UCHAR || ml==SHORT || ml==USHORT ) && p->in.left->in.op != ICON ) {
		  p->in.op = SCONV;
		  break;
		}

		/* pointers all have the same representation; the type is inherited */
		p->in.left->in.type = p->in.type;
		p->in.left->fn.cdim = p->fn.cdim;
		p->in.left->fn.csiz = p->fn.csiz;
		p->in.op = FREE;
		return( p->in.left );

	case SCONV:
		m = (p->in.type == FLOAT || p->in.type == DOUBLE );
		ml = (p->in.left->in.type == FLOAT || p->in.left->in.type == DOUBLE );
		if( m != ml ) break;

		/* now, look for conversions downwards */

		m = p->in.type;
		ml = p->in.left->in.type;
		if( p->in.left->in.op == ICON ){ /* simulate the conversion here */
			CONSZ val;
			val = p->in.left->tn.lval;
			switch( m ){
			case CHAR:
				p->in.left->tn.lval = (char) val;
				break;
			case UCHAR:
				p->in.left->tn.lval = val & 0XFF;
				break;
			case USHORT:
				p->in.left->tn.lval = val & 0XFFFFL;
				break;
			case SHORT:
				p->in.left->tn.lval = (short)val;
				break;
				}
			p->in.left->in.type = m;
			}
	 	else if (p->in.left->in.op == FCON ) 
		     	{/* simulate conversion for float consts */
		     /* this fixes the 'wont init float arrays' bug (#16) */
		 	p->in.left->in.type = m; }
		else break;

		/* clobber conversion */
		p->in.op = FREE;
		return( p->in.left );  /* conversion gets clobbered */

	case PVCONV:
	case PMCONV:
		if( p->in.right->in.op != ICON ) cerror( "bad conversion", 0);
		p->in.op = FREE;
		return( buildtree( o==PMCONV?MUL:DIV, p->in.left, p->in.right ) );

		}

	return(p);
	}

andable( p ) NODE *p; {
	return(1);  /* all names can have & taken on them */
	}

cendarg(){ /* at the end of the arguments of a ftn, set the automatic offset */
	autooff = AUTOINIT;
	}

cisreg( t ) TWORD t; { /* is an automatic variable of type t OK for a register variable */
	switch (t) {
	  case INT:
	  case UNSIGNED:
	  case SHORT:
	  case USHORT:
	  case CHAR:
#ifdef DOUBLES32BITS
	/* GB bug #70 */
	  case FLOAT:
#endif
	  case UCHAR:	return(1);

	  default:	if ( ISPTR(t) ) return(1);
			return(0);
	}
}

NODE *
offcon( off, t, d, s ) OFFSZ off; TWORD t; {

	/* return a node, for structure references, which is suitable for
	   being added to a pointer of type t, in order to be off bits offset
	   into a structure */

	register NODE *p;

	/* t, d, and s are the type, dimension offset, and sizeoffset */
	/* in general they  are necessary for offcon, but not on H'well */

	/* GB BUG FIX! bcon must be passed the correct value! 
	p = bcon(0);
	*/
	p = bcon(off/SZCHAR);
	p->tn.lval = off/SZCHAR;
	return(p);

	}

static inwd	/* current bit offsed in word */;
static long word	/* word being built from fields */;

incode( p, sz ) register NODE *p; {

	/* generate initialization code for assigning a constant c
		to a field of width sz */
	/* we assume that the proper alignment has been obtained */
	/* inoff is updated to have the proper final value */
	/* we also assume sz  < SZINT */

	if((sz+inwd) > SZINT) cerror("incode: field > int");
	word |= (p->tn.lval & ((1 << sz) -1)) << (SZINT - sz - inwd);
	inwd += sz;
	inoff += sz;
	while (inwd >= 16) {
	  printf( "	.word	%ld\n", (word>>16)&0xFFFFL );
	  word <<= 16;
	  inwd -= 16;
	}
}

#define sw(x) ((x>>16)&0xFFFF | (x<<16) & 0xFFFF0000)

fincode( d, sz ) long float d; {
	/* output code to initialize space of size sz to the value d */
	/* the proper alignment has been obtained */
	/* inoff is updated to have the proper final value */
	/* on the target machine, write it out in hex(GB)! */

/*****	GB altered fincode to write out data in hex 8/12/83 ******/

	register long *mi = (int *)&d;

#ifdef IEEE
/***** 	A note here is appropriate on IEEE format.  SGI uses
	FULL IEEE format.  We have gone to some lengths to ensure
	that floating point using IEEE is standard and is as efficient
	as possible.  Thus, in our compiler, floats are 32-bits,
	even across calls, doubles are 32-bits, and 'long floats'
	are 64-bit floats in FULL IEEE format. Compiler output of
	our IEEE format is controlled by the IEEE definition. (IF
	it is NOT defined, the compiler will use standard Vax format,
	without any of the alterations above (i.e., floats are extended
	to doubles across calls, etc.).)
	
	The representation of the incoming floating point value
	in this routine has, of course, a drastic effect, as
	we want to dump the correct floating point value in full
	IEEE format in hex to the .s file.  There are three possibilities:
	
		1. The host is a VAX, or uses Vax floating point format.
		   this is true if HOST_DEC_FLOAT is defined.
		   
		2. The host also uses full IEEE format.  In this case, the
		   binary representation does not have to be parsed and
		   the host's double to float conversion routine can be
		   used. In this case HOST_FULL_IEEE is defined.
		   
		3. The host uses a non-standard IEEE format.  This is
		   currently unsupported, but the hooks are in.


	GB (SGI) 9/26/83.
***********/
#ifndef HOST_FULL_IEEE
	/* the binary floating point representation must be parsed. */
	register unsigned long exp;
#ifdef HOST_DEC_FLOAT
#ifdef ONVAX
	mi[0]=sw(mi[0]);
	mi[1]=sw(mi[1]);
#endif
	exp = (mi[0]&0x7F800000)>>23;
	if (exp<3) /* this loses for small double prec. values*/
	{	mi[0]=0;
		if (sz==SZDOUBLE) mi[1]=0;
	}
	else
	{	
		if (sz==SZDOUBLE)
		{	exp += 894;
			mi[1]=
			 ((mi[1]>>3)&0x1FFFFFFF)|((mi[0]<<29)&0xE0000000);
			mi[0]=
			 ((exp<<20)|((mi[0]>>3)&0x000FFFFF))
				|(mi[0]<0 ?0x80000000:0);
		}
		else
		{	exp -= 2;
			mi[0]=(exp<<23)|(mi[0]&0x807FFFFF);
		}
	}

#else HOST_DEC_FLOAT
#ifdef HOST_IEEE_EXTENDED
	/* the host is using an extended floating point format.
	   the parameters of the extended floating point format are:
	   
	   IEEE_SEXP	- number of bits in the single precision exp.
	   IEEE_DEXP	- number of bits in the double precision exp.
	   IEEE_J	- integer part of the format.
	   IEEE_SBIAS	- bias of the single precision exponent.
	   IEEE_DBIAS	- bias of the double precision exponent.

	If it is ever necessary to support this format, these defines can
	be used to take apart the floating point numbers.
	   */
	   
	  
#endif HOST_IEEE_EXTENDED
#endif /* else HOST_DEC_FLOAT*/
#endif /* ifndef HOST_FULL_IEEE */
if( sz==SZDOUBLE )
		printf( "	.long	0x%lx, 0x%lx\n", mi[0], mi[1] );
	else
#ifndef HOST_FULL_IEEE
	/* the conversion to single-precision was done above. */
		printf( "	.long	0x%lx\n", mi[0] );
#else
	/* convert the double (64-bit) floating point number to
	   single precision using native conversion
	*/
	{	float f;
		f = d;
		*mi = *(long *)&f;
		printf("	.long   0x%lx\n",mi[0]);
	}
#endif
	inoff += sz;
	}
#else IEEE

	if( sz==SZDOUBLE )
		printf( "	.long	0x%lx, 0x%lx\n", sw(mi[0]), sw(mi[1]) );
	else
		printf( "	.long	0x%lx\n", sw(mi[0]) );
	inoff += sz;
	}
#endif

cinit( p, sz ) NODE *p; {
	/* arrange for the initialization of p into a space of
	size sz */
	/* the proper alignment has been opbtained */
	/* inoff is updated to have the proper final value */
	ecode( p );
	inoff += sz;
	}

vfdzero( n ){ /* define n bits of zeros in a vfd */

	if( n <= 0 ) return;

	inwd += n;
	inoff += n;
	while (inwd >= 16) {
	  printf( "	.word	%ld\n", (word>>16)&0xFFFFL );
	  word <<= 16;
	  inwd -= 16;
	}
}


/* GB - FORTRAN */
char *
fortexname( p ) char *p; {
	/* make a name look like an external name in the local machine */

	static char text[NCHNAM+1];

	register i;
	register char *q=text;

	while( *p&&q<text+NCHNAM ){
		*q++ = *p++;
		}

	*q='\0';

	return( text );
	}

char *
exname( p ) char *p; {
	/* make a name look like an external name in the local machine */

	static char text[NCHNAM+1];

	register i;
	register char *q=text;

#ifdef NATIVE
	*q++='_';
#endif
	while( *p&&q<text+NCHNAM ){
		*q++ = *p++;
		}

	*q='\0';

	return( text );
	}

ctype( type ) TWORD type; { /* map types which are not defined on the local machine */
	switch( BTYPE(type) ){
	case LONG:
		MODTYPE(type,INT);
		break;
	case ULONG:
		MODTYPE(type,UNSIGNED);
		}
	return( type );
	}

noinit() { /* curid is a variable which is defined but
	is not initialized (and not a function );
	This routine returns the stroage class for an uninitialized declaration */

	return(EXTERN);

	}

commdec( id ){ /* make a common declaration for id, if reasonable */
	register struct symtab *q;
	OFFSZ off;

	q = &stab[id];
	printf( "	.comm	%s,", exname( q->sname ) );
	off = tsize( q->stype, q->dimoff, q->sizoff );
	printf( CONFMT, off/SZCHAR );
	printf( "\n" );
	}

isitlong( cb, ce ){ /* is lastcon to be long or short */
	/* cb is the first character of the representation, ce the last */

	if( ce == 'l' || ce == 'L' ||
		lastcon >= (1L << (SZINT-1) ) ) return (1);
	return(0);
	}


isitfloat( s ) char *s; {
#ifdef ONVAX
	long float atof();
	dcon = atof(s);
#else
	long float _latof();
	dcon = _latof(s);
#endif
	return( FCON );
	}

ecode( p ) NODE *p; {

	/* walk the tree and write out the nodes.. */

	if( nerrors ) return;
	p2tree( p );
	p2compile( p );
	}

char *
getftnnm()
{
	return(stab[curftn].sname);
}
