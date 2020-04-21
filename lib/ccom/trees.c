# include "mfile1.h"	/* includes macdefs.h manifest.h */

	    /* corrections when in violation of lint */

/*	some special actions, used in finding the type of nodes */
# define NCVT 01
# define PUN 02
# define TYPL 04
# define TYPR 010
# define TYMATCH 040
# define LVAL 0100
# define CVTO 0200
# define CVTL 0400
# define CVTR 01000
# define PTMATCH 02000
# define OTHER 04000
# define NCVTR 010000
# define RINT 020000
# define LINT 040000

/* node conventions:

	NAME:	rval>0 is stab index for external
		rval<0 is -inlabel number
		lval is offset in bits
	ICON:	lval has the value
		rval has the STAB index, or - label number,
			if a name whose address is in the constant
		rval = NONAME means no name
	REG:	rval is reg. identification cookie

	*/

int bdebug = 0;
int tdebug = 0;

extern ddebug;

screduce( p ) register NODE *p; {
	register NODE *t;
	int v;
	if( p->in.op == SCONV ){
		t = p->in.left;
		if( t->in.op == SCONV )
			screduce( t );
		if( t->in.op == ICON ) {
# ifndef BUG1
	if( ddebug ) printf( "screduce( %o ) %o\n", p, t );
# endif
			v = p->in.type;
			*p = *t;
			p->in.type = v;
			t->in.op = FREE;
			}
		}
	}

#ifdef NOTDEF
NODE *
sccheck( p ) register NODE *p; {
	while (p->in.op == SCONV)
		p = p->in.left;
	return( (p->in.op == ICON) ? p : 0 );
	}
#endif
/* GB - sccheck has to coerce the constants as it
   goes down or we are going to have problems later.
*/
NODE *
sccheck( p ) register NODE *p; {
	NODE *result;
	if (p->in.op == SCONV) {
		CONSZ val;
		if (result = sccheck(p->in.left)) {
			int btype = BTYPE(p->in.type);
			int isunsigned = ISUNSIGNED(btype);
			if (isunsigned ) btype = ENUNSIGN(btype);
			val = result->tn.lval;
			switch (btype) {
				case  CHAR: 
					if (isunsigned)
						val = (unsigned char)val;
					else
						val = (char)val;
					break;
				case  SHORT: 
					if (isunsigned)
						val = (unsigned short)val;
					else
						val = (short)val;
					break;
				case INT:
				case  LONG: 
					if (isunsigned)
						val = (unsigned long)val;
					else
						val = (long)val;
					break;
			}	
			result->tn.lval = val;
			return(result);
		}
		return(0);
	}
	return( (p->in.op == ICON) ? p : 0 );
}
/* return type of SCONV node */
sctype( p ) register NODE *p; {
	while (p->in.op == SCONV)
		p = p->in.left;
	return( p->in.op );
	}

NODE *
buildtree( o, l, r ) register NODE *l, *r; {
	register NODE *p, *q;
	register actions;
	register opty;
	register struct symtab *sp;
	register NODE *lr, *ll;
	int i;
	NODE *lc, *rc;
	extern int eprint();

	opty = optype(o);
# ifndef BUG1
	if( bdebug ) printf( "buildtree( %s, %o, %o ) %d %d %d\n",
		opst[o], l, r, opty, l?l->in.op:0, r?r->in.op:0 );
# endif

	lc = ( l ) ? sccheck( l ) : 0;
	rc = ( r ) ? sccheck( r ) : 0;

	/* check for constants */

	/* if( opty == UTYPE && l->in.op == ICON ){ */
	if( opty == UTYPE && lc ){

		switch( o ){

		case NOT:
			if( hflag ) werror( "constant argument to NOT" );
		case UNARY MINUS:
		case COMPL:
			if( conval( lc?lc:l, o, lc?lc:l, l, l ) ){
				screduce( l );
				return(l);
				}
			break;

			}
		}

	else if( o==UNARY MINUS && l->in.op==FCON ){
		l->fpn.dval = -l->fpn.dval;
		return(l);
		}

	/* else if( o==QUEST && l->in.op==ICON ) { */
	else if( o==QUEST && lc ) {
		screduce( l );
		l->in.op = FREE;
		r->in.op = FREE;
		if( l->tn.lval ){
			tfree( r->in.right );
			return( r->in.left );
			}
		else {
			tfree( r->in.left );
			return( r->in.right );
			}
		}

	/* else if( (o==ANDAND || o==OROR) && (l->in.op==ICON||r->in.op==ICON) ) goto ccwarn; */
	else if( (o==ANDAND || o==OROR) && (lc||rc) ) goto ccwarn;

	/* else if( opty == BITYPE && l->in.op == ICON && r->in.op == ICON ){ */
	else if( opty == BITYPE && lc && rc ){

		switch( o ){

		case ULT:
		case UGT:
		case ULE:
		case UGE:
		case LT:
		case GT:
		case LE:
		case GE:
		case EQ:
		case NE:
		case ANDAND:
		case OROR:
		case CBRANCH:

		ccwarn:
			if( hflag ) werror( "constant in conditional context" );

		case PLUS:
		case MINUS:
		case MUL:
		case DIV:
		case MOD:
		case AND:
		case OR:
		case ER:
		case LS:
		case RS:
			if( conval( lc?lc:l, o, rc?rc:r, l, r ) ) {
				screduce( l );
				screduce( r );
				r->in.op = FREE;
				return(l);
				}
			break;
			}
		}

	/* else if( opty == BITYPE && (l->in.op==FCON||l->in.op==ICON) &&
		(r->in.op==FCON||r->in.op==ICON) ){ */
	else if( opty == BITYPE && (lc||l->in.op==FCON) &&
		(rc||r->in.op==FCON)
#ifdef DOUBLES32BITS
		&& (rc||r->in.type == FLOAT) && (lc||l->in.type == FLOAT)
#endif
		){
		switch(o){
		case PLUS:
		case MINUS:
		case MUL:
		case DIV:
			if( lc ) screduce( l );
			if( rc ) screduce( r );
			if( l->in.op == ICON ){
				l->fpn.dval = l->tn.lval;
				}
			if( r->in.op == ICON ){
				r->fpn.dval = r->tn.lval;
				}
			l->in.op = FCON;
/*****  BUGREPORT (#30) GB SGI  default float type is FLOAT if 
	using IEEE format with DOUBLES32BITS *******/
#ifdef DOUBLES32BITS
			l->in.type = l->fn.csiz = FLOAT;
#else
			l->in.type = l->fn.csiz = DOUBLE;
#endif
			r->in.op = FREE;
			switch(o){
			case PLUS:
				l->fpn.dval += r->fpn.dval;
				return(l);
			case MINUS:
				l->fpn.dval -= r->fpn.dval;
				return(l);
			case MUL:
				l->fpn.dval *= r->fpn.dval;
				return(l);
			case DIV:
				if( r->fpn.dval == 0 ) uerror( "division by 0." );
				else l->fpn.dval /= r->fpn.dval;
				return(l);
				}
			}
		}

	/* its real; we must make a new node */

	p = block( o, l, r, INT, 0, INT );

	/* scr1775 */
	if (l) {
		if (((l->in.op == UNARY CALL)||(l->in.op == CALL)) &&
		     (l->in.type == UNDEF)){
			if (o != COMOP)
			uerror(
		"the (non-existant) value of a void function cannot be used");
		/*
			l->in.type = INT;
		*/
		}
	}
	if ((opty == BITYPE)&&(r)) {
		if (((r->in.op == UNARY CALL)||(r->in.op == CALL)) && 
		  (r->in.type == UNDEF)) {
		    if ((o != CAST) || (l->in.type != UNDEF))

			/* GB - if this is a COMOP, its not an error
			   unless we assign the result, which (sigh)
			   we dont know here.  The error is caught later.
			*/
			if (o != COMOP)

			uerror(
		"the (non-existant) value of a void function cannot be used");
		/*
			r->in.type = INT;
		*/
		}
	}

	actions = opact(p);
	if (tdebug) printf("...result is 0x%x\n",actions);

	if( actions&LVAL ){ /* check left descendent */
		if( notlval(p->in.left) ) {
			uerror( "illegal lhs of assignment operator" );
			}
		}

	if( actions & NCVTR ){
		p->in.left = pconvert( p->in.left );
		}
	else if( !(actions & NCVT ) ){
		switch( opty ){

		case BITYPE:
			p->in.right = pconvert( p->in.right );
		case UTYPE:
			p->in.left = pconvert( p->in.left );

			}
		}

	if( (actions&PUN) && (o!=CAST||cflag) ){
		chkpun(p);
		}

	if (actions & RINT) p->in.right = makety( p->in.right, INT, 0, INT );
	if (actions & LINT) p->in.left = makety( p->in.left, INT, 0, INT );

	if( actions & (TYPL|TYPR) ){

		q = (actions&TYPL) ? p->in.left : p->in.right;

		/* GB -- coerce ICONs here!
		*/
#ifdef SGI_FIELDS /* else the function nbytes isn't defined */
		if (p->in.op == ICON)
		{
			int nbytes();
			switch (nbytes(q->in.type))
			{
				case 1:	p->tn.lval &= 0xff;
					break;
				case 2: p->tn.lval &= 0xffff;
			}
		}
#endif
		p->in.type = q->in.type;
		p->fn.cdim = q->fn.cdim;
		p->fn.csiz = q->fn.csiz;
		}

	if( actions & CVTL ) p = convert( p, CVTL );
	if( actions & CVTR ) p = convert( p, CVTR );
	if( actions & TYMATCH ) p = tymatch(p);
	if( actions & PTMATCH ) p = ptmatch(p);

	if( actions & OTHER ){
		l = p->in.left;
		r = p->in.right;

		switch(o){

		case NAME:
			sp = &stab[idname];
			if( sp->stype == UNDEF ){
				uerror( "%.32s undefined", sp->sname );
				/* make p look reasonable */
				p->in.type = p->fn.cdim = p->fn.csiz = INT;
				p->tn.rval = idname;
				p->tn.lval = 0;
				defid( p, SNULL );
				break;
				}
			p->in.type = sp->stype;
			p->fn.cdim = sp->dimoff;
			p->fn.csiz = sp->sizoff;
			p->tn.lval = 0;
			p->tn.rval = idname;
			/* special case: MOETY is really an ICON... */
			if( p->in.type == MOETY ){
				p->tn.rval = NONAME;
				p->tn.lval = sp->offset;
				p->fn.cdim = 0;
				p->in.type = ENUMTY;
				p->in.op = ICON;
				}
			break;

		case ICON:
			p->fn.cdim = 0;
			p->fn.csiz = INT;
			break;

		case STRING:
			p->in.op = NAME;
			p->in.type = CHAR+ARY;
			p->tn.lval = 0;
			p->tn.rval = NOLAB;
			p->fn.cdim = curdim;
			p->fn.csiz = CHAR;
			break;

		case FCON:
			p->tn.lval = 0;
			p->tn.rval = 0;
#ifdef DOUBLES32BITS
		/* also fixed further for bug #94, scr0742 */
			p->in.type = FLOAT;
			p->fn.csiz = FLOAT;
#else
			p->in.type = DOUBLE;
			p->fn.csiz = DOUBLE;
#endif
			p->fn.cdim = 0;
			break;

		case STREF:
			/* p->x turned into *(p+offset) */
			/* rhs must be a name; check correctness */

			i = r->tn.rval;
			if( i<0 || ((sp= &stab[i])->sclass != MOS && sp->sclass != MOU && !(sp->sclass&FIELD)) ){
				uerror( "member of structure or union required" );
				}else
			/* if this name is non-unique, find right one */
			if( stab[i].sflags & SNONUNIQ &&
				(l->in.type==PTR+STRTY || l->in.type == PTR+UNIONTY) &&
				(l->fn.csiz +1) >= 0 ){
				/* nonunique name && structure defined */
				char * memnam, * tabnam;
				register k;
				int j;
				int memi;
				j=dimtab[l->fn.csiz+1];
				for( ; (memi=dimtab[j]) >= 0; ++j ){
					tabnam = stab[memi].sname;
					memnam = stab[i].sname;
# ifndef BUG1
					if( ddebug>1 ){
						printf("member ");
						printf(NAME_FMT, memnam);
						printf("==");
						printf(NAME_FMT, tabnam);
						printf("?\n");
						}
# endif
					if( stab[memi].sflags & SNONUNIQ ){
						for( k=0; k<NCHNAM; ++k ){
							if(*memnam++!=*tabnam)
								goto next;
							if(!*tabnam++) break;
							}
						r->tn.rval = i = memi;
						break;
						}
					next: continue;
					}
				if( memi < 0 )
					uerror("illegal member use: %.32s",
						stab[i].sname);
				}
			else {
				register j;
				if( l->in.type != PTR+STRTY && l->in.type != PTR+UNIONTY ){
					if( stab[i].sflags & SNONUNIQ ){
						uerror( "nonunique name demands struct/union or struct/union pointer" );
						}
					else werror( "struct/union or struct/union pointer required" );
					}
				else if( (j=l->fn.csiz+1)<0 ) cerror( "undefined structure or union" );
				else if( !chkstr( i, dimtab[j], DECREF(l->in.type) ) ){
					werror( "illegal member use: %.32s", stab[i].sname );
					}
				}

			p = stref( p );
			break;

		case UNARY MUL:
			if( l->in.op == UNARY AND ){
				p->in.op = l->in.op = FREE;
				p = l->in.left;
				}
			if( !ISPTR(l->in.type))uerror("illegal indirection");
			p->in.type = DECREF(l->in.type);
			p->fn.cdim = l->fn.cdim;
			p->fn.csiz = l->fn.csiz;
			break;

		case UNARY AND:
			switch( l->in.op ){

			case UNARY MUL:
				p->in.op = l->in.op = FREE;
				p = l->in.left;
			case NAME:
				p->in.type = INCREF( l->in.type );
				p->fn.cdim = l->fn.cdim;
				p->fn.csiz = l->fn.csiz;
				break;

			case COMOP:
				lr = buildtree( UNARY AND, l->in.right, NIL );
				p->in.op = l->in.op = FREE;
				p = buildtree( COMOP, l->in.left, lr );
				break;

			case QUEST:
				lr = buildtree( UNARY AND, l->in.right->in.right, NIL );
				ll = buildtree( UNARY AND, l->in.right->in.left, NIL );
				p->in.op = l->in.op = l->in.right->in.op = FREE;
				p = buildtree( QUEST, l->in.left, buildtree( COLON, ll, lr ) );
				break;

# ifdef ADDROREG
			case OREG:
				/* OREG was built in clocal()
				 * for an auto or formal parameter
				 * now its address is being taken
				 * local code must unwind it
				 * back to PLUS/MINUS REG ICON
				 * according to local conventions
				 */
				{
				extern NODE * addroreg();
				p->in.op = FREE;
				p = addroreg( l );
				}
				break;

# endif
			default:
				uerror( "unacceptable operand of &" );
				break;
				}
			break;

		case LS:
		case RS:
		case ASG LS:
		case ASG RS:
			if(tsize(p->in.right->in.type, p->in.right->fn.cdim, p->in.right->fn.csiz) > SZINT)
				p->in.right = makety(p->in.right, INT, 0, INT );
			break;

		case RETURN:
		case ASSIGN:
		case CAST:
			/* structure assignment */
			/* take the addresses of the two sides; then make an
			/* operator using STASG and
			/* the addresses of left and right */

			{
				register TWORD t;
				register d, s;

				if( l->fn.csiz != r->fn.csiz ) uerror( "assignment of different structures" );

				r = buildtree( UNARY AND, r, NIL );
				t = r->in.type;
				d = r->fn.cdim;
				s = r->fn.csiz;

				l = block( STASG, l, r, t, d, s );

				if( o == RETURN ){
					p->in.op = FREE;
					p = l;
					break;
					}

				p->in.op = UNARY MUL;
				p->in.left = l;
				p->in.right = NIL;
				break;
				}
		case COLON:
			/* structure colon */

			if( l->fn.csiz != r->fn.csiz ) uerror( "type clash in conditional" );
			break;

		case CALL:
			p->in.right = r = strargs( p->in.right );
		case UNARY CALL:
			if( !ISPTR(l->in.type)) uerror("illegal function");
			p->in.type = DECREF(l->in.type);
			if( !ISFTN(p->in.type)) uerror("illegal function");
			p->in.type = DECREF( p->in.type );
			p->fn.cdim = l->fn.cdim;
			p->fn.csiz = l->fn.csiz;
			if( l->in.op == UNARY AND && l->in.left->in.op == NAME &&
				l->in.left->tn.rval >= 0 && l->in.left->tn.rval != NONAME &&
				( (i=stab[l->in.left->tn.rval].sclass) == FORTRAN || i==UFORTRAN ) ){
				p->in.op += (FORTCALL-CALL);
				}
			if( p->in.type == STRTY || p->in.type == UNIONTY ){
				/* function returning structure */
				/*  make function really return ptr to str., with * */

				p->in.op += STCALL-CALL;
				p->in.type = INCREF( p->in.type );
				p = buildtree( UNARY MUL, p, NIL );

				}
			break;

		default:
			cerror( "other code %d", o );
			}

		}

	if( actions & CVTO ) p = oconvert(p);
	p = clocal(p);

# ifndef BUG1
	if( bdebug ) fwalk( p, eprint, 0 );
# endif

	return(p);

	}

NODE *
strargs( p ) register NODE *p;  { /* rewrite structure flavored arguments */

	if( p->in.op == CM ){
		p->in.left = strargs( p->in.left );
		p->in.right = strargs( p->in.right );
		return( p );
		}

	if( p->in.type == STRTY || p->in.type == UNIONTY ){
		p = block( STARG, p, NIL, p->in.type, p->fn.cdim, p->fn.csiz );
		p->in.left = buildtree( UNARY AND, p->in.left, NIL );
		p = clocal(p);
		}
	else if (p->in.type==CHAR || p->in.type==UCHAR || p->in.type==SHORT || p->in.type==USHORT)
		p = makety(p, INT, 0, INT);

	return( p );
	}

chkstr( i, j, type ) TWORD type; {
	/* is the MOS or MOU at stab[i] OK for strict reference by a ptr */
	/* i has been checked to contain a MOS or MOU */
	/* j is the index in dimtab of the members... */
	int k, kk;

	extern int ddebug;

# ifndef BUG1
	if( ddebug > 1 ) {printf( "chkstr( ");
			printf(NAME_FMT, stab[i].sname);
			printf("(%d), %d )\n", i, j );
		}
# endif
	if( (k = j) < 0 ) uerror( "undefined structure or union" );
	else {
		for( ; (kk = dimtab[k] ) >= 0; ++k ){
			if( kk >= SYMTSZ ){
				cerror( "gummy structure" );
				return(1);
				}
			if( kk == i ) return( 1 );
			switch( stab[kk].stype ){

			case STRTY:
			case UNIONTY:
				if( type == STRTY ) continue;  /* no recursive looking for strs */
				if( hflag && chkstr( i, dimtab[stab[kk].sizoff+1], stab[kk].stype ) ){
					if( stab[kk].sname[0] == '$' ) return(0);  /* $FAKE */
					werror(
					"illegal member use: perhaps %.32s.%.32s?",
					stab[kk].sname, stab[i].sname );
					return(1);
					}
				}
			}
		}
	return( 0 );
	}

conval( p, o, q, l, r ) register NODE *p, *q; NODE *l, *r; {
	/* apply the op o to the lval part of p; if binary, rhs is val */
	/* UNI GB p,q are the left,right primary nodes (constants resolved),
	   l,r are the left,right final SCONV nodes with the final type */
	int sz,isunsigned;
	int i, u;
	CONSZ val;

	val = q->tn.lval;
	u = ISUNSIGNED(l->in.type) || ISUNSIGNED(r->in.type);
	if( u && (o==LE||o==LT||o==GE||o==GT)) o += (UGE-GE);

	if( bdebug ) printf( 
	    "conval( %s, %o(final=%o), %o(final=%o) ) ,0x%x %s 0x%x\n",
		opst[o], p,l,q, r, p->tn.lval,opst[o],val);

	if( p->tn.rval != NONAME && q->tn.rval != NONAME ) return(0);
	if( q->tn.rval != NONAME && o!=PLUS ) return(0);
	if( p->tn.rval != NONAME && o!=PLUS && o!=MINUS ) return(0);

	switch( o ){

	case PLUS:
		p->tn.lval += val;
		if( p->tn.rval == NONAME ){
			p->tn.rval = q->tn.rval;
			p->in.type = q->in.type;
			}
		break;
	case MINUS:
		p->tn.lval -= val;
		break;
	case MUL:
		p->tn.lval *= val;
		break;
	case DIV:
		if( val == 0 ) uerror( "division by 0" );
		else if ( l->in.type == UNSIGNED )
			p->tn.lval = (unsigned)p->tn.lval / val;
		else
			p->tn.lval /= val;
		break;
	case MOD:
		if( val == 0 ) uerror( "division by 0" );
		else if ( l->in.type == UNSIGNED )
			p->tn.lval = (unsigned)p->tn.lval % val;
		else
			p->tn.lval %= val;
		break;
	case AND:
		p->tn.lval &= val;
		break;
	case OR:
		p->tn.lval |= val;
		break;
	case ER:
		p->tn.lval ^=  val;
		break;
	case LS:
		i = val;
		p->tn.lval = p->tn.lval << i;
		break;
	case RS:
		i = val;
		if ( l->in.type == UNSIGNED )
			p->tn.lval = (unsigned)p->tn.lval >> i;
		else
			p->tn.lval = p->tn.lval >> i;
		break;

	case UNARY MINUS:
		p->tn.lval = - p->tn.lval;
		break;
	case COMPL:
		p->tn.lval = ~p->tn.lval;
		break;
	case NOT:
		p->tn.lval = !p->tn.lval;
		break;
	case LT:
		p->tn.lval = p->tn.lval < val;
		break;
	case LE:
		p->tn.lval = p->tn.lval <= val;
		break;
	case GT:
		p->tn.lval = p->tn.lval > val;
		break;
	case GE:
		p->tn.lval = p->tn.lval >= val;
		break;
	case ULT:
		p->tn.lval = (p->tn.lval-val)<0;
		break;
	case ULE:
		p->tn.lval = (p->tn.lval-val)<=0;
		break;
	case UGE:
		p->tn.lval = (p->tn.lval-val)>=0;
		break;
	case UGT:
		p->tn.lval = (p->tn.lval-val)>0;
		break;
	case EQ:
		p->tn.lval = p->tn.lval == val;
		break;
	case NE:
		p->tn.lval = p->tn.lval != val;
		break;
	default:
		return(0);
		}
	/* GB (sgi) - oops.  Constants no longer have the default
	   type INT.  Re-compute the type according to the result
	   of the operation.
	*/
	isunsigned = ISUNSIGNED(p->in.type);
	sz = BTYPE(p->in.type);
	if (isunsigned) sz = DEUNSIGN(sz);
	if (sz < INT) {
		int resulttype;
		val = p->tn.lval;
		if (isunsigned) {
			if (val >= 0x10000)
				resulttype = INT;
			else if (val >= 0x100)
				resulttype = SHORT;
			else resulttype = CHAR;
		}
		else {
			if ((val > 0x7fff)||(val < (-32768)))
				resulttype = INT;
			else if ((val > 0x7f)||(val < (-128)))
				resulttype = SHORT;
			else
				resulttype = CHAR;
		}
		if (resulttype != CHAR) {
			if (isunsigned) {
				p->in.type &= ~BTMASK;
				p->in.type |= ENUNSIGN(resulttype);
			}
			else {
				p->in.type &= ~BTMASK;
				p->in.type |= (resulttype);
			}
		}
	}
	return(1);
	}

chkpun(p) register NODE *p; {

	/* checks p for the existance of a pun */

	/* this is called when the op of p is ASSIGN, RETURN, CAST, COLON, or relational */

	/* one case is when enumerations are used: this applies only to lint */
	/* in the other case, one operand is a pointer, the other integer type */
	/* we check that this integer is in fact a constant zero... */

	/* in the case of ASSIGN, any assignment of pointer to integer is illegal */
	/* this falls out, because the LHS is never 0 */

	register NODE *q;
	register t1, t2;
	register d1, d2;

	t1 = p->in.left->in.type;
	t2 = p->in.right->in.type;

	if( t1==ENUMTY || t2==ENUMTY ) { /* check for enumerations */
		if( logop( p->in.op ) && p->in.op != EQ && p->in.op != NE ) {
			uerror( "illegal comparison of enums" );
			return;
			}
		if( t1==ENUMTY && t2==ENUMTY && p->in.left->fn.csiz==p->in.right->fn.csiz ) return;
		if( (p->in.left->in.op == FLD) && (t2==ENUMTY)) {
			/* enumeration type bitfields can be assigned 
			   enumeration constants.  Dont give a 
			   warning message in this case.
		   	   GB (SGI) scr0784 10/15/85
			   The problem is that the enum type of the bitfield
			   has been lost by fldty() earlier.  We simply
			   assume that the field is of the correct enumeration 
			   type.
			*/
			return;
		}
		werror( "enumeration type clash, operator %s", opst[p->in.op] );
		return;
	}

	if( ISPTR(t1) || ISARY(t1) ) q = p->in.right;
	else q = p->in.left;

	if( !ISPTR(q->in.type) && !ISARY(q->in.type) ){
		if( q->in.op != ICON || q->tn.lval != 0 ){
			combo( "pointer/integer", p );
			}
		}
	else {
		d1 = p->in.left->fn.cdim;
		d2 = p->in.right->fn.cdim;
		for( ;; ){
			if( t1 == t2 ) {;
				if(p->in.left->fn.csiz!=p->in.right->fn.csiz) {
					combo( "structure pointer", p );
					}
				return;
				}
			if( ISARY(t1) || ISPTR(t1) ){
				if( !ISARY(t2) && !ISPTR(t2) ) break;
				if( ISARY(t1) && ISARY(t2) &&
					dimtab[d1] != dimtab[d2] ){
					combo( "array size", p );
					return;
					}
				if( ISARY(t1) ) ++d1;
				if( ISARY(t2) ) ++d2;
				}
			else break;
			t1 = DECREF(t1);
			t2 = DECREF(t2);
			}
		combo( "pointer", p );
		}

	}

combo( s, p ) char *s; register NODE *p; {
	char buf[100];
	sprintf( buf, "illegal %s combination, op %s", s, opst[p->tn.op] );
	werror( buf );
	}

NODE *
stref( p ) register NODE *p; {

	TWORD t;
	int d, s, dsc, align;
	OFFSZ off;
	register struct symtab *q;

	/* make p->x */
	/* this is also used to reference automatic variables */

	q = &stab[p->in.right->tn.rval];
	p->in.right->in.op = FREE;
	p->in.op = FREE;
	p = pconvert( p->in.left );

	/* make p look like ptr to x */

	if( !ISPTR(p->in.type)){
		p->in.type = PTR+UNIONTY;
		}

	t = INCREF( q->stype );
	d = q->dimoff;
	s = q->sizoff;

	p = makety( p, t, d, s );

	/* compute the offset to be added */

	off = q->offset;
	dsc = q->sclass;

	if( dsc & FIELD ) {  /* normalize offset */
		switch(q->stype) {

		case CHAR:
		case UCHAR:
			align = ALCHAR;
			s = CHAR;
			break;

		case SHORT:
		case USHORT:
			align = ALSHORT;
			s = SHORT;
			break;

		case INT:
		case UNSIGNED:
			align = ALINT;
			s = INT;
			break;

# ifdef LONGFIELDS
		case LONG:
		case ULONG:
			align = ALLONG;
			s = LONG;
			break;
# endif

		default:
			cerror( "undefined bit field type" );
			}
		off = (off/align)*align;
		}
	if( off != 0 ) {
	   if (tdebug) printf("normalizing offset:\n");
	   p = clocal( block( PLUS, p, makety(offcon( off, t, d, s ),INT,0,INT), t, d, s ) );
	}

	p = buildtree( UNARY MUL, p, NIL );

	/* if field, build field info */

	if( dsc & FIELD ){
		p = block( FLD, p, NIL, q->stype, 0, q->sizoff );
		p->tn.rval = PKFIELD( dsc&FLDSIZ, q->offset%align );
		}

	return( clocal(p) );
	}

notlval(p) register NODE *p; {

	/* return 0 if p an lvalue, 1 otherwise */

	again:

	switch( p->in.op ){

	case FLD:
		p = p->in.left;
		goto again;

	case UNARY MUL:
		/* fix the &(a=b) bug, given that a and b are structures */
		if( p->in.left->in.op == STASG ) return( 1 );
		/* and the f().a bug, given that f returns a structure */
		if( p->in.left->in.op == UNARY STCALL ||
		    p->in.left->in.op == STCALL ) return( 1 );
	case NAME:
	case OREG:
		if( ISARY(p->in.type) || ISFTN(p->in.type) ) return(1);
	case REG:
		return(0);

	default:
		return(1);

		}

	}

NODE *
bcon( i ){ /* make a constant node with value i */
	register NODE *p;
	register long val;

	p = block( ICON, NIL, NIL, INT, 0, INT );
	/* GB - Use the PASSED value to determine
	  type...
	val = p->tn.lval;
	*/
	val = i;
	if (val>=-128 && val<=127) p->tn.type = CHAR;
	else if (val>=-32768 && val<=32767) p->tn.type = SHORT;
	else p->tn.type = INT;
#ifdef NOTDEF
	p->tn.type = INT;
#endif
	p->tn.lval = i;
	p->tn.rval = NONAME;
	return( clocal(p) );
	}

NODE *
bpsize(p) register NODE *p; {
	return( offcon( psize(p), p->in.type, p->fn.cdim, p->fn.csiz ) );
	}

OFFSZ
psize( p ) NODE *p; {
	/* p is a node of type pointer; psize returns the
	   size of the thing pointed to */

	if( !ISPTR(p->in.type) ){
		uerror( "pointer required");
		return( SZINT );
		}
	/* note: no pointers to fields */
	return( tsize( DECREF(p->in.type), p->fn.cdim, p->fn.csiz ) );
	}

NODE *
convert( p, f )  register NODE *p; {
	/*  convert an operand of p
	    f is either CVTL or CVTR
	    operand has type int, and is converted by the size of the other side
	    */

	register NODE *q, *r;

	q = (f==CVTL)?p->in.left:p->in.right;

	r = block( PMCONV,
		q, bpsize(f==CVTL?p->in.right:p->in.left), INT, 0, INT );
	r = clocal(r);
	if( f == CVTL )
		p->in.left = r;
	else
		p->in.right = r;
	return(p);

	}

econvert( p ) register NODE *p; {

	/* change enums to ints, or appropriate types */

	register TWORD ty;

	if( (ty=BTYPE(p->in.type)) == ENUMTY || ty == MOETY ) {
		if( dimtab[ p->fn.csiz ] == SZCHAR ) ty = CHAR;
		else if( dimtab[ p->fn.csiz ] == SZINT ) ty = INT;
		else if( dimtab[ p->fn.csiz ] == SZSHORT ) ty = SHORT;
		else ty = LONG;
		ty = ctype( ty );
		p->fn.csiz = ty;
		MODTYPE(p->in.type,ty);
		if( p->in.op == ICON && ty != LONG ) p->in.type = p->fn.csiz = INT;
		}
	}

NODE *
pconvert( p ) register NODE *p; {

	/* if p should be changed into a pointer, do so */

	if( ISARY( p->in.type) ){
		p->in.type = DECREF( p->in.type );
		++p->fn.cdim;
		return( buildtree( UNARY AND, p, NIL ) );
		}
	if( ISFTN( p->in.type) )
		return( buildtree( UNARY AND, p, NIL ) );

	return( p );
	}

NODE *
oconvert(p) register NODE *p; {
	/* convert the result itself: used for pointer and unsigned */

	switch(p->in.op) {

	case LE:
	case LT:
	case GE:
	case GT:
		if( ISUNSIGNED(p->in.left->in.type) || ISUNSIGNED(p->in.right->in.type) )  p->in.op += (ULE-LE);
	case EQ:
	case NE:
		return( p );

	case MINUS:
		return(  clocal( block( PVCONV,
			p, bpsize(p->in.left), INT, 0, INT ) ) );
		}

	cerror( "illegal oconvert: %d", p->in.op );

	return(p);
	}

NODE *
ptmatch(p)  register NODE *p; {

	/* makes the operands of p agree; they are
	   either pointers or integers, by this time */
	/* with MINUS, the sizes must be the same */
	/* with COLON, the types must be the same */

	TWORD t1, t2, t;
	int o, d2, d, s2, s;

	o = p->in.op;
	t = t1 = p->in.left->in.type;
	t2 = p->in.right->in.type;
	d = p->in.left->fn.cdim;
	d2 = p->in.right->fn.cdim;
	s = p->in.left->fn.csiz;
	s2 = p->in.right->fn.csiz;

	switch( o ){

	case ASSIGN:
	case RETURN:
	case CAST:
		{  break; }

	case MINUS:
		{  if( psize(p->in.left) != psize(p->in.right) ){
			uerror( "illegal pointer subtraction");
			}
		   break;
		   }
	case COLON:
		{  if( t1 != t2 ) uerror( "illegal types in :");
		   break;
		   }
	default:  /* must work harder: relationals or comparisons */

		if( !ISPTR(t1) ){
			t = t2;
			d = d2;
			s = s2;
			break;
			}
		if( !ISPTR(t2) ){
			break;
			}

		/* both are pointers */
		if( talign(t2,s2) < talign(t,s) ){
			t = t2;
			s = s2;
			}
		break;
		}

	p->in.left = makety( p->in.left, t, d, s );
	p->in.right = makety( p->in.right, t, d, s );
	if( o!=MINUS ){

/* *** again, the result of relationals should ALWAYS be int. (GB) #22/#23
		p->in.type = t;
*/
		if (logop(o))
			p->in.type = INT;
		else p->in.type = t;
		p->fn.cdim = d;
		p->fn.csiz = s;
		}

	return(clocal(p));
	}


NODE *
tymatch(p)  register NODE *p; {

	/* satisfy the types of various arithmetic binary ops */

	/* rules used to be:
		if assignment, op, type of LHS
		if any float or doubles, make double
		if any longs, make long
		otherwise, make int
		if either operand is unsigned, the result is...

	  now the rules are:
		if assignment op, type of LHS
		if any doubles, make double.
		else if any floats, make float.
		else if one is signed, the other unsigned, make both
			unsigned, use unsigned compare and the size of 
			the larger.
		else use the size of the larger.
		
	*/

	register TWORD t1, t2, t, tu;
	register o, u;

	o = p->in.op;

	t1 = p->in.left->in.type;
	t2 = p->in.right->in.type;
	if( (t1==UNDEF || t2==UNDEF) && o!=CAST )
		uerror("void type illegal in expression");

	u = 0;
	if( ISUNSIGNED(t1) ){
		u = 1;
		t1 = DEUNSIGN(t1);
		}
	if( ISUNSIGNED(t2) ){
		u = 1;
		t2 = DEUNSIGN(t2);
		}
/* **** Section removed in response to bugreport #24.
	(GB) SGI 7/25/83.

	if ( logop(o) ) switch (t1) {
#ifdef DOUBLES32BITS
	  case FLOAT:   t = FLOAT;
			break;
#else
	  case FLOAT:
#endif
	  case DOUBLE:	t = DOUBLE;
			break;

	  case INT:	t = INT;
			break;

	  case SHORT:	t = t2==INT ? INT : SHORT;
			break;

	  case CHAR:	t = t2;
			break;
	}
#ifdef DOUBLES32BITS
	 else if (t1==DOUBLE || t2==DOUBLE ) t = DOUBLE;
	 else if (t1==FLOAT  || t2==FLOAT ) t = FLOAT;
#else
	 else if (t1==DOUBLE || t1==FLOAT || t2==DOUBLE || t2==FLOAT ) t = DOUBLE;
#endif
	else t = INT;
*/

/*	In response to the bugreport, the above rules were implemented:
*/

	if( asgop(o) ){
		tu = p->in.left->in.type;
		t = t1;
		}
	else {
		if ((t1==DOUBLE)||(t2==DOUBLE)) t=DOUBLE;
		else if ((t1==FLOAT)||(t2==FLOAT)) t=FLOAT;
		else if ((t1==LONG)||(t2==LONG)) t=LONG;
		else if (logop(o))
		{	
			if ((t1==INT)||(t2==INT)) t=INT;
			else if ((t1==SHORT)||(t2==SHORT)) t=SHORT;
			else t=CHAR;}

		else t = INT;

		tu = (u && UNSIGNABLE(t))?ENUNSIGN(t):t;
		}

	/* because expressions have values that are at least as wide
	   as INT or UNSIGNED, the only conversions needed
	   are those involving FLOAT/DOUBLE, and those
	   from LONG to INT and ULONG to UNSIGNED */

	if( t != t1 ) p->in.left = makety( p->in.left, tu, 0, (int)tu );

	if( t != t2 || o==CAST ) p->in.right = makety( p->in.right, tu, 0, (int)tu );

	if( asgop(o) ){
		p->in.type = p->in.left->in.type;
		p->fn.cdim = p->in.left->fn.cdim;
		p->fn.csiz = p->in.left->fn.csiz;
		}
	else {	

/* **** BUG FIX (GB) relational expressions should always return
	ints.  #22/#23
		p->in.type = tu;
*/
		if (logop(o))
			p->in.type = INT;
		else p->in.type = tu;
		p->fn.cdim = 0;
		p->fn.csiz = t;
		}

# ifndef BUG1
	if( tdebug ) {
		printf( "tymatch(%o):", p );
		tprint(t1);
		printf(" %s ",opst[o]);
		tprint(t2);
		printf(" ==> ");
		tprint(tu);
		printf("\n");
	}
# endif

	return(p);
	}

NODE *
makety( p, t, d, s ) register NODE *p; register TWORD t; {
	/* make p into type t by inserting a conversion */

	if( p->in.type == ENUMTY && p->in.op == ICON ) econvert(p);
	if( t == p->in.type ){
		p->fn.cdim = d;
		p->fn.csiz = s;
		return( p );
		}

	if( t & TMASK ){
		/* non-simple type */
		return( block( PCONV, p, NIL, t, d, s ) );
		}

	if( p->in.op == ICON ){
		if( t==DOUBLE||t==FLOAT ){
			p->in.op = FCON;
			if( ISUNSIGNED(p->in.type) ){
				p->fpn.dval =  ( /*unsigned */ CONSZ)  p->tn.lval;
				}
			else {
				p->fpn.dval = p->tn.lval;
				}

			p->in.type = p->fn.csiz = t;
			return( clocal(p) );
			}
		}

	return( block( SCONV, p, NIL, t, d, s ) );

	}

NODE *
block( o, l, r, t, d, s ) register NODE *l, *r; TWORD t; {

	register NODE *p;

	p = talloc();
	p->in.op = o;
	p->in.left = l;
	p->in.right = r;
	p->in.type = t;
	p->fn.cdim = d;
	p->fn.csiz = s;
	return(p);
	}

icons(p) register NODE *p; {
	/* if p is an integer constant, return its value */
	int val;

	if( p->in.op != ICON )
		screduce(p);

	if( p->in.op != ICON ){
		uerror( "constant expected");
		val = 1;
		}
	else {
		val = p->tn.lval;
		if( val != p->tn.lval ) uerror( "constant too big for cross-compiler" );
		}
	tfree( p );
	return(val);
	}

/* 	the intent of this table is to examine the
	operators, and to check them for
	correctness.

	The table is searched for the op and the
	modified type (where this is one of the
	types INT (includes char and short), LONG,
	DOUBLE (includes FLOAT), and POINTER

	The default action is to make the node type integer

	The actions taken include:
		PUN	  check for puns
		CVTL	  convert the left operand
		CVTR	  convert the right operand
		TYPL	  the type is determined by the left operand
		TYPR	  the type is determined by the right operand
		TYMATCH	  force type of left and right to match, by inserting conversions
		PTMATCH	  like TYMATCH, but for pointers
		LVAL	  left operand must be lval
		CVTO	  convert the op
		NCVT	  do not convert the operands
		OTHER	  handled by code
		NCVTR	  convert the left operand, not the right...

	*/

# define MINT 01  /* integer */
# define MDBI 02   /* integer or double */
# define MSTR 04  /* structure */
# define MPTR 010  /* pointer */
# define MPTI 020  /* pointer or integer */
# define MENU 040 /* enumeration variable or member */

/*****	GB SGI (#32,#37) new void type information 8/9/83 
added line following *****/

# define MVOID 0200 
# define MPTV 0100 /* pointer or VOID */

opact( p )  NODE *p; {

	register mt12, mt1, mt2, o;
	TWORD t;

	mt12 = 0;

	if (tdebug) printf("opact: ");

	switch( optype(o=p->in.op) ){

	case BITYPE:
		mt12=mt2 = moditype( t = p->in.right->in.type );
		if (tdebug) {
			printf("right is (0x%x) ",t);
			tprint(t);
		}
	case UTYPE:
		mt12 &= (mt1 = moditype( t = p->in.left->in.type ));
		if (tdebug) {
			printf(" ; left is (0x%x) ",t);
			tprint(t);
		}

	}

	switch( o ){

	case NAME :
	case STRING :
	case ICON :
	case FCON :
	case CALL :
	case UNARY CALL:
	case UNARY MUL:
		{  return( OTHER ); }
	case UNARY MINUS:
		if( mt1 & MDBI ) return( TYPL );
		break;

	case COMPL:
		if( mt1 & MINT ) return( TYPL );
		break;

	case UNARY AND:
		{  return( NCVT+OTHER ); }
	case INIT:
	case CM:
	case NOT:
	case CBRANCH:
	case ANDAND:
	case OROR:
		return( 0 );

	case MUL:
	case DIV:
		if( mt12 & MDBI ) return( TYMATCH );
		break;

	case MOD:
	case AND:
	case OR:
	case ER:
		if( mt12 & MINT ) return( TYMATCH );
		break;

	case LS:
	case RS:
/* **** bugreport #9.  Dont force int type on shifts, as we may want
	unsigned (GB) 7/25/83.
 		if( mt12 & MINT ) return( LINT+TYMATCH+OTHER );
*/
		if( mt12 & MINT ) return( TYMATCH+OTHER );
		break;

	case EQ:
	case NE:
	case LT:
	case LE:
	case GT:
	case GE:
		if( (mt1&MENU)||(mt2&MENU) ) return( PTMATCH+PUN+NCVT );
		if( mt12 & MDBI ) return( TYMATCH+CVTO );
		else if( mt12 & MPTR ) return( PTMATCH+PUN );
		else if( mt12 & MPTI ) return( PTMATCH+PUN );
		else if ( (mt1&MPTR) && (p->in.right->in.op==ICON)) return ( PTMATCH+PUN+RINT );
		else if ( (mt2&MPTR) && (p->in.left->in.op==ICON)) return ( PTMATCH+PUN+LINT );
		else break;

	case QUEST:
	case COMOP:
		if( mt2&MENU ) return( TYPR+NCVTR );
		return( TYPR );

	case STREF:
		return( NCVTR+OTHER );

	case FORCE:
		return( TYPL );

	case COLON:
		if( mt12 & MENU ) return( NCVT+PUN+PTMATCH );
		else if( mt12 & MDBI ) return( TYMATCH );
		else if( mt12 & MPTR ) return( TYPL+PTMATCH+PUN );
		else if( (mt1&MINT) && (mt2&MPTR) ) return( TYPR+PUN+LINT );
		else if( (mt1&MPTR) && (mt2&MINT) ) return( TYPL+PUN+RINT );
		else if( mt12 & MSTR ) return( NCVT+TYPL+OTHER );
		break;

	case ASSIGN:
	case RETURN:
		if( mt12 & MSTR ) return( LVAL+NCVT+TYPL+OTHER );

/*****	GB SGI (#32)  a function returning void can return a void. 
	The tree is compiled for side-effects only 8/9/83
added next line 
*****/
/** NEW 
		if (mt12 & MPTV) return ( TYPL + NCVT);
**/

	case CAST:

/***** 	GB SGI (#37) casts to void are allowed.  In the new setup,
	mt1 will be MVOID, rather than 0. 8/9/83
replaced
		if(o==CAST && mt1==0)return(TYPL+TYMATCH);
with following ******/
		if(o==CAST && (mt1&MVOID))return(TYPL+TYMATCH);

		if( mt12 & MDBI ) return( TYPL+LVAL+TYMATCH );
		else if( (mt1&MENU)||(mt2&MENU) ) return( LVAL+NCVT+TYPL+PTMATCH+PUN );
/***** 	GB SGI (#32) added following line.  Allows assignment of functions
	returning voids to their pointers 8/9/83
*****/
		/* dont allow (void) results of calls through void function
		   pointers to be assigned or returned (from vax) */
		else if ((mt2 & MVOID) && (p->in.right->in.op == CALL || p->in.right->in.op == UNARY CALL))
			break;
		else if( mt12 == 0 ) break;
		else if( mt1 & MPTR ) return( LVAL+PTMATCH+PUN );
		else if( mt12 & MPTI ) return( TYPL+LVAL+TYMATCH+PUN );

		break;

	case ASG LS:
	case ASG RS:
		if( mt12 & MINT ) return( TYPL+LVAL+OTHER );
		break;

	case ASG MUL:
	case ASG DIV:
		if( mt12 & MDBI ) return( LVAL+TYMATCH );
		break;

	case ASG MOD:
	case ASG AND:
	case ASG OR:
	case ASG ER:
		if( mt12 & MINT ) return( LVAL+TYMATCH );
		break;

	case ASG PLUS:
	case ASG MINUS:
	case INCR:
	case DECR:
		if( mt12 & MDBI ) return( TYMATCH+LVAL );
		else if( (mt1&MPTR) && (mt2&MINT) ) return( TYPL+LVAL+CVTR+RINT );
		break;

	case MINUS:
		if( mt12 & MPTR ) return( CVTO+PTMATCH+PUN );
		if( mt2 & MPTR ) break;
	case PLUS:
		if( mt12 & MDBI ) return( TYMATCH );
		else if( (mt1&MPTR) && (mt2&MINT) ) return( TYPL+CVTR+RINT );
		else if( (mt1&MINT) && (mt2&MPTR) ) return( TYPR+CVTL+LINT );

		}
	if (tdebug)
		uerror( 
		"operands of %s have incompatible types (l = 0x%x, r = 0x%x)", 
			opst[o], mt1, mt2 );
	else
		uerror( "operands of %s have incompatible types", opst[o] );
	return( NCVT );
	}

moditype( ty ) TWORD ty; {

	switch( ty ){

	case TVOID:

/*****	GB SGI (#32) allow assignment of functions returning voids to their
	pointers 8/9/83
added following line
******/
		return(MPTI);

	case UNDEF:

/*****	GB SGI (#32, #37) new void type information. 8/9/83
replaced	return(0);
with *****/
 		return(MVOID|MPTI); /* type is void */

	case ENUMTY:
	case MOETY:
		return( MENU );

	case STRTY:
	case UNIONTY:
		return( MSTR );

	case CHAR:
	case SHORT:
	case UCHAR:
	case USHORT:
		return( MINT|MPTI|MDBI );
	case UNSIGNED:
	case ULONG:
	case INT:
	case LONG:
		return( MINT|MDBI|MPTI );
	case FLOAT:
	case DOUBLE:
		return( MDBI );
	default:

/*****	GB SGI (#32,37) return more void information. Since VOID==0,
	BTYPE(ty)==0 if ty==VOID or TVOID. 8/9/83
replaced	return( MPTR|MPTI );
with ****/

		return((BTYPE(ty))?( MPTR|MPTI ):(MPTI|MPTR));
		}
	}

NODE *
doszof( p )  register NODE *p; {
	/* do sizeof p */
	int i;

	/* whatever is the meaning of this if it is a bitfield? */
	i = tsize( p->in.type, p->fn.cdim, p->fn.csiz )/SZCHAR;

	tfree(p);
	if( i <= 0 ) werror( "sizeof returns 0" );
	return( bcon( i ) );
	}

# ifndef BUG2
eprint( p, down, a, b ) register NODE *p; int *a, *b; {
	register ty;

	*a = *b = down+1;
	while( down > 1 ){
		printf( "\t" );
		down -= 2;
		}
	if( down ) printf( "    " );

	ty = optype( p->in.op );

	printf("%o) %s, ", p, opst[p->in.op] );
	if( ty == LTYPE ){
		if ( p->in.op == FCON )
			printf( "%f, ", p->fpn.dval );
		else{
			printf( p->tn.lval<0?"%ld":CONFMT, p->tn.lval );
			printf( ", %d, ", p->tn.rval );
			}
		}
	tprint( p->in.type );
	printf( ", %d, %d\n", p->fn.cdim, p->fn.csiz );
	fflush( stdout );
	}
# endif

prtdcon( p ) register NODE *p; {
	int i;


	if( p->in.op == FCON ){
		locctr( DATA );
		defalign( ALDOUBLE );
		deflab( i = getlab() );
#ifdef DOUBLES32BITS
		fincode( p->fpn.dval,(p->in.type==DOUBLE)?SZDOUBLE:SZFLOAT);
#else
		fincode( p->fpn.dval, SZDOUBLE );
#endif
		p->tn.lval = 0;
		p->tn.rval = -i;
#ifndef DOUBLES32BITS
		p->in.type = DOUBLE;
#endif
		p->in.op = NAME;
		}
	}


int edebug = 0;
ecomp( p ) register NODE *p; {
# ifndef BUG2
	if( edebug ) fwalk( p, eprint, 0 );
# endif
	if( !reached ){
		werror( "statement not reached" );
		reached = 1;
		}
	p = optim(p);
	walkf( p, prtdcon );
	locctr( PROG );
	ecode( p );
	tfree(p);
	}

# ifdef STDPRTREE
# ifndef ONEPASS

prtree(p) register NODE *p; {

	register struct symtab *q;
	register ty;

# ifdef MYPRTREE
	MYPRTREE(p);  /* local action can be taken here; then return... */
#endif

	ty = optype(p->in.op);

	printf( "%d\t", p->in.op );

	if( ty == LTYPE ) {
		printf( p->tn.lval<0?"%ld":CONFMT, p->tn.lval );
		printf( "\t" );
		}
	if( ty != BITYPE ) {
		if( p->in.op == NAME || p->in.op == ICON ) printf( "0\t" );
		else printf( "%d\t", p->tn.rval );
		}

	printf( "%o\t", p->in.type );

	/* handle special cases */

	switch( p->in.op ){

	case NAME:
	case ICON:
		/* print external name */
		if( p->tn.rval == NONAME ) printf( "\n" );
		else if( p->tn.rval >= 0 ){
			q = &stab[p->tn.rval];
			printf(  "%s\n", exname(q->sname) );
			}
		else { /* label */
			printf( LABFMT, -p->tn.rval );
			}
		break;

	case STARG:
	case STASG:
	case STCALL:
	case UNARY STCALL:
		/* print out size */
		/* use lhs size, in order to avoid hassles with the structure `.' operator */

		/* note: p->in.left not a field... */
		printf( CONFMT, (CONSZ) tsize( STRTY, p->in.left->fn.cdim, p->in.left->fn.csiz ) );
		printf( "\t%d\t\n", talign( STRTY, p->in.left->fn.csiz ) );
		break;

	default:
		printf(  "\n" );
		}

	if( ty != LTYPE ) prtree( p->in.left );
	if( ty == BITYPE ) prtree( p->in.right );

	}

# else

p2tree(p) register NODE *p; {
	register ty;

# ifdef MYP2TREE
	MYP2TREE(p);  /* local action can be taken here; then return... */
# endif

	ty = optype(p->in.op);

	switch( p->in.op ){

	case NAME:
	case ICON:
		if( p->tn.rval == NONAME ) p->in.name[0] = '\0';
		else if( p->tn.rval >= 0 ){ /* copy name from exname */
			register char *cp,*dp;
			register i;
			cp = exname( stab[p->tn.rval].sname );

/*			for( i=0; i<NCHNAM; ++i ) p->in.name[i] = *cp++;*/

			dp = &p->in.name[0];
			i = NCHNAM-1;
			do
				*dp++ = *cp++;
			while (--i != -1);
			}
		else sprintf( p->in.name, LABFMT, -p->tn.rval );
		break;

	case STARG:
	case STASG:
	case STCALL:
	case UNARY STCALL:
		/* set up size parameters */
		p->stn.stsize = (tsize(STRTY,p->in.left->fn.cdim,p->in.left->fn.csiz)+SZCHAR-1)/SZCHAR;
		p->stn.stalign = talign(STRTY,p->in.left->fn.csiz)/SZCHAR;
		break;

	case REG:
		rbusy( p->tn.rval, p->in.type );
	default:
		p->in.name[0] = '\0';
		}

	p->in.rall = NOPREF;

	if( ty != LTYPE ) p2tree( p->in.left );
	if( ty == BITYPE ) p2tree( p->in.right );
	}

# endif
# endif
