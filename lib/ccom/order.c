# include "mfile2.h"	/* includes macdefs.h mac2defs.h manifest.h */
#ifdef SGI_REGS
# include "regprefs.h"
extern int fpatype;
#endif

int fltused = 0;

stoasg( p, o ) register NODE *p; {
	/* should the assignment op p be stored,
	   given that it lies as the right operand of o
	   (or the left, if o==UNARY MUL) */
	return( shltype(p->in.left->in.op, p->in.left ) );

	}

deltest( p ) register NODE *p; {
	/* should we delay the INCR or DECR operation p */
# ifndef MULTILEVEL
	if( p->in.op == INCR && p->in.left->in.op == REG && 
/* 
   GB - 10/28/87 ensure that the type of the increment is the same as the
	base pointer.  Otherwise, autoincr/decr addressing mode will be
	used (incorrectly).  Added one line.
*/
/**/
	    spsz( p->in.left->in.type, p->in.right->tn.lval ) ){
		/* STARREG */
		if (!(spsz(p->in.type, p->in.right->tn.lval)))
			werror(
"pointer may be incorrectly incremented by sizeof(CAST) rather than sizeof(object)");
		return( 0 );
		}
# else
	if( mlmatch(p,DEFINCDEC,INCR) && spsz( p->in.left->in.type, p->in.right->tn.lval ) ){
		/* STARREG */
		return( 0 );
		}
# endif

	p = p->in.left;
	if( p->in.op == UNARY MUL ) p = p->in.left;
	return( p->in.op == NAME || p->in.op == OREG || p->in.op == REG );
	}

mkadrs(p) register NODE *p; {
	register o;

	o = p->in.op;

	if( asgop(o) ){
		if( p->in.left->in.su >= p->in.right->in.su ){
			if( p->in.left->in.op == UNARY MUL ){
				if( p->in.left->in.su > 0 )
					SETSTO( p->in.left->in.left, INTEMP );
				else {
					if( p->in.right->in.su > 0 ) SETSTO( p->in.right, INTEMP );
					else cerror( "store finds both sides trivial" );
					}
				}
			else if( p->in.left->in.op == FLD && p->in.left->in.left->in.op == UNARY MUL ){
				SETSTO( p->in.left->in.left->in.left, INTEMP );
				}
			else { /* should be only structure assignment */
				SETSTO( p->in.left, INTEMP );
				}
			}
		else SETSTO( p->in.right, INTEMP );
		}
	else {
		if( p->in.left->in.su > p->in.right->in.su ){
			SETSTO( p->in.left, INTEMP );
			}
		else {
			SETSTO( p->in.right, INTEMP );
			}
		}
	
# ifndef BUG4
		if( edebug ) {
			printf("mkadrs: storing tree-\n");
			fwalk( stotree, eprint, 0 );
		    }
# endif
	}

notoff( t, r, off, cp) TWORD t; CONSZ off; char *cp; {
	/* is it legal to make an OREG or NAME entry which has an
	/* offset of off, (from a register of r), if the
	/* resulting thing had type t */

	if ( off>=-32768 && off<=32767 && *cp=='\0' && r>=A0 && r<=SP )
	  return(0);	/* YES */
	return( 1 );  /* NO */
	}

# define max(x,y) ((x)<(y)?(y):(x))
# define min(x,y) ((x)<(y)?(x):(y))


# define ZCHAR 01
# define ZLONG 02
# define ZFLOAT 04

zum( p, zap ) register NODE *p; {
	/* zap Sethi-Ullman number for chars, longs, floats */
	/* in the case of longs, only STARNM's are zapped */
	/* ZCHAR, ZLONG, ZFLOAT are used to select the zapping */

	register su;

	su = p->in.su;

	switch( p->in.type ){

	case CHAR:
	case UCHAR:
		if( !(zap&ZCHAR) ) break;
		if( su==0 && p->in.op!=ICON ) p->in.su = su = 1;
		break;

	case LONG:
	case ULONG:
		if( !(zap&ZLONG) ) break;
		if( p->in.op == UNARY MUL && su == 0 ) p->in.su = su = 2;
		break;

	case FLOAT:
		if( !(zap&ZFLOAT) ) break;
		if( su == 0 ) p->in.su = su = 1;

		}

	return( su );
	}

sucomp( p ) register NODE *p; {

	/* set the su field in the node to the sethi-ullman
	   number, or local equivalent */

	register o, ty, sul, sur;
	register nr;

	ty = optype( o=p->in.op);
	nr = szty( p->in.type );
	p->in.su = 0;
	/*** GB SGI bug #69 - 
		if right op is NOT a constant between 8 and -7 and
		the op is a shift op, the right must be placed in a register.
		Up the register count */
	if (shiftop(o)) {
		if ((p->in.right->in.op != ICON) || 
			(p->in.right->tn.lval < (-7)) || 
			(p->in.right->tn.lval > 8))
		p->in.right->in.su++;
	}

	/* GB SGI SCR911 - floating point compares unless an fpa is in
	   use, necessitate a fake call.  Thus, all registers are needed.
	*/
#ifdef SGI_REGS
	if ((o >= _FIRST_FCMPOP)&&(o<= _LAST_FCMPOP)&&(fpatype == NO_FPA)) {
		TWORD typ;
		if (
#ifdef NOTDEF
		    (((typ = p->in.left->in.type) == FLOAT)||(typ == DOUBLE))||
		    (((typ = p->in.right->in.type) == FLOAT)||(typ == DOUBLE))
#endif
		    (((p->in.left->in.type) == FLOAT))||
		    (((p->in.right->in.type) == FLOAT))
		   ) {
			p->in.su = fregs;
			return;
		}
	}
#endif

	if( ty == LTYPE ) {
		if( p->in.type==FLOAT ) p->in.su = 1;
		return;
		}
	else if( ty == UTYPE ){
		switch( o ) {
		case UNARY CALL:
		case UNARY STCALL:
			p->in.su = fregs;  /* all regs needed */
			return;

		case UNARY MUL:
			if( shumul( p->in.left ) ) return;
/*** GB - bug81 *** UNARY MUL needs A register - dont su a D register ****/
#ifdef NOTDEF
			nr = 0;
#endif

		default:
			p->in.su = max( p->in.left->in.su, nr);
			return;
			}
		}


	/* If rhs needs n, lhs needs m, regular su computation */

	sul = p->in.left->in.su;
	sur = p->in.right->in.su;

		
	if( o == ASSIGN ){
		asop:  /* also used for +=, etc., to memory */
		if( sul==0 ){
			/* don't need to worry about the left side */
			p->in.su = max( sur, nr );
			}
		else {
			/* right, left address, op */
			if( sur == 0 ){
				/* just get the lhs address into a register, and mov */
				/* the `nr' covers the case where value is in reg afterwards */
				p->in.su = max( sul, nr );
				}
			else {
				/* right, left address, op */
				p->in.su = max( sur, nr+sul );
				}
			}
		return;
		}

	if( o == CALL || o == STCALL ){
		/* in effect, takes all free registers */
		p->in.su = fregs;
		return;
		}

	if( o == STASG ){
		/* right, then left */
		p->in.su = max( max( sul+nr, sur), fregs );
		return;
		}

	if( logop(o) ){
		/* do the harder side, then the easier side, into registers */
		/* left then right, max(sul,sur+nr) */
		/* right then left, max(sur,sul+nr) */
		/* to hold both sides in regs: nr+nr */
		nr = szty( p->in.left->in.type );
		sul = zum( p->in.left, ZLONG|ZCHAR|ZFLOAT );
		sur = zum( p->in.right, ZLONG|ZCHAR|ZFLOAT );
		p->in.su = min( max(sul,sur+nr), max(sur,sul+nr) );
		return;
		}

	if( asgop(o) ){
		/* computed by doing right, doing left address, doing left, op, and store */
		switch( o ) {
		case INCR:
		case DECR:
			/* do as binary op */
			break;

		case ASG PLUS:
		case ASG MINUS:
		case ASG AND:
		case ASG ER:
		case ASG OR:
			if( p->in.type == INT || p->in.type == UNSIGNED || ISPTR(p->in.type) ) goto asop;

		gencase:
		default:
			sur = zum( p->in.right, ZCHAR|ZLONG|ZFLOAT );
			if( sur == 0 ){ /* easy case: if addressable,
				do left value, op, store */
				if( sul == 0 ) p->in.su = nr;
				/* harder: left adr, val, op, store */
				else p->in.su = max( sul, nr+1 );
				}
			else { /* do right, left adr, left value, op, store */
				if( sul == 0 ){  /* right, left value, op, store */
					p->in.su = max( sur, nr+nr );
					}
				else {
					p->in.su = max( sur, max( sul+nr, 1+nr+nr ) );
					}
				}
			return;
			}
		}

	switch( o ){
	case ANDAND:
	case OROR:
	case QUEST:
	case COLON:
	case COMOP:
		p->in.su = max( max(sul,sur), nr);
		return;
		}

	if( ( o==DIV || o==MUL )) {
		/* have to do the left in a register. */
		nr = 1;
	}
	if( o==MOD ) {
		/* need TWO regs to do the left */
		nr=2;
	}
#ifdef NOTDEF
	if( ( o==DIV || o==MOD || o==MUL )
	    && p->in.type!=FLOAT && p->in.type!=DOUBLE ) nr = fregs;
#endif
	if( o==PLUS || o==MUL || o==OR || o==ER ){
		/* AND is ruined by the hardware */
		/* permute: get the harder on the left */

		register rt, lt;

		if( istnode( p->in.left ) || sul > sur ) goto noswap;  /* don't do it! */

		/* look for a funny type on the left, one on the right */


		lt = p->in.left->in.type;
		rt = p->in.right->in.type;

		if( rt == FLOAT && lt == DOUBLE ) goto swap;

		if( (rt==CHAR||rt==UCHAR) && (lt==INT||lt==UNSIGNED||ISPTR(lt)) ) goto swap;

		if( lt==LONG || lt==ULONG ){
			if( rt==LONG || rt==ULONG ){
				/* if one is a STARNM, swap */
				if( p->in.left->in.op == UNARY MUL && sul==0 ) goto noswap;
				if( p->in.right->in.op == UNARY MUL && p->in.left->in.op != UNARY MUL ) goto swap;
				goto noswap;
				}
			else if( p->in.left->in.op == UNARY MUL && sul == 0 ) goto noswap;
			else goto swap;  /* put long on right, unless STARNM */
			}

		/* we are finished with the type stuff now; if one is addressable,
			put it on the right */
		if( sul == 0 && sur != 0 ){

			NODE *s;
			int ssu;

		swap:
			ssu = sul;  sul = sur; sur = ssu;
			s = p->in.left;  p->in.left = p->in.right; p->in.right = s;
			}
		}
	noswap:

	sur = zum( p->in.right, ZCHAR|ZLONG|ZFLOAT );
	if( sur == 0 ){
		/* get left value into a register, do op */
		p->in.su = max( nr, sul );
		}
	else {
		/* do harder into a register, then easier */
		p->in.su = max( nr+nr, min( max( sul, nr+sur ), max( sur, nr+sul ) ) );
		}
	}

int radebug = 0;

mkrall( p, r ) register NODE *p; {
	/* insure that the use of p gets done with register r; in effect, */
	/* simulate offstar */

	if( p->in.op == FLD ){
		p->in.left->in.rall = p->in.rall;
		p = p->in.left;
		}

	if( p->in.op != UNARY MUL ) return;  /* no more to do */
	p = p->in.left;
	if( p->in.op == UNARY MUL ){
		p->in.rall = r;
		p = p->in.left;
		}
	if( p->in.op == PLUS && p->in.right->in.op == ICON ){
		p->in.rall = r;
		p = p->in.left;
		}
	rallo( p, r );
	}

rallo( p, down ) register NODE *p; {
	/* do register allocation */
	register o, type, down1, down2, ty;

	if( radebug ) printf( "rallo( %o, %o )\n", p, down );

	down2 = NOPREF;

	ty = optype( o = p->in.op );
	type = p->in.type;

	p->in.rall = down;
	down1 = ( down &= ~MUSTDO );


	if( type == DOUBLE || type == FLOAT ){
		if( o == FORCE ) down1 = D0|MUSTDO;
		++fltused;
		}
	else switch( o ) {
	case ASSIGN:	
		down1 = NOPREF;
		down2 = down;
		break;

	case ASG MUL:
	case ASG DIV:
	case ASG MOD:
		/* keep the addresses out of the hair of (r0,r1) */
		if(fregs == 2 ){
			down1 = D1;
			down2 = NOPREF;
			break;
			}
		/* at least 3 regs free */
		/* compute lhs in (r0,r1), address of left in r2 */
		p->in.left->in.rall = D1;
		mkrall( p->in.left, D2);
		/* now, deal with right */
		if( fregs == 3 ) rallo( p->in.right, NOPREF );
		else {
			/* put address of long or value here */
			p->in.right->in.rall = D3;
			mkrall( p->in.right, D3);
			}
		return;

	case MUL:
	case DIV:
	case MOD:
		rallo( p->in.left, D1);

		if( fregs == 2 ){
			rallo( p->in.right, NOPREF );
			return;
			}
		/* compute addresses, stay away from (r0,r1) */

		p->in.right->in.rall = (fregs==3) ? D2: D3;
		mkrall( p->in.right, D2);
		return;
#ifdef NOTDEF
		/* keep the addresses out of the hair of (r0,r1) */
		if(fregs == 2 ){
			/* lhs in (r0,r1), nothing else matters */
			down1 = D1|MUSTDO;
			down2 = NOPREF;
			break;
			}
		/* at least 3 regs free */
		/* compute lhs in (r0,r1), address of left in r2 */
		p->in.left->in.rall = D1|MUSTDO;
		mkrall( p->in.left, D2|MUSTDO );
		/* now, deal with right */
		if( fregs == 3 ) rallo( p->in.right, NOPREF );
		else {
			/* put address of long or value here */
			p->in.right->in.rall = D3|MUSTDO;
			mkrall( p->in.right, D3|MUSTDO );
			}
		return;

	case MUL:
	case DIV:
	case MOD:
		rallo( p->in.left, D1|MUSTDO );

		if( fregs == 2 ){
			rallo( p->in.right, NOPREF );
			return;
			}
		/* compute addresses, stay away from (r0,r1) */

		p->in.right->in.rall = (fregs==3) ? D2|MUSTDO : D3|MUSTDO ;
		mkrall( p->in.right, D2|MUSTDO );
		return;
#endif
	case EQ:
	case NE:
	case GT:
	case GE:
	case LT:
	case LE:
#ifdef SGI_REGS
		if (ty != LTYPE) {
			if (p->in.left->in.type == FLOAT) {
				/* lhs in (r0,r1), nothing else matters */
				down1 = D0|MUSTDO;
				down2 = D1|MUSTDO;
				break;
			}
		}
#endif
	case CALL:
	case STASG:
	case NOT:
	case ANDAND:
	case OROR:
		down1 = NOPREF;
		break;

	case FORCE:	
		down1 = D0|MUSTDO;
		break;

		}

	if( ty != LTYPE ) rallo( p->in.left, down1 );
	if( ty == BITYPE ) rallo( p->in.right, down2 );

	}

offstar( p ) register NODE *p; {
	/* handle indirections */

	if( p->in.op == UNARY MUL ) p = p->in.left;

	if( p->in.op == PLUS || p->in.op == MINUS ){
		if( p->in.right->in.op == ICON && p->in.right->in.name[0]=='\0' &&
		    p->in.right->tn.lval>=-32768 && p->in.right->tn.lval<=32767 ){
			order( p->in.left , INTBREG|INBREG );
			return;
			}
		}
	order( p, INTBREG|INBREG );
	}

setincr( p ) NODE *p; {
	return( 0 );	/* for the moment, don't bother */
	}

niceuty( p ) register NODE *p; {
	register TWORD t;

	return( p->in.op == UNARY MUL && (t=p->in.type)!=CHAR &&
		t!= UCHAR && t!= FLOAT &&
		shumul( p->in.left) != STARREG );
	}
setbin( p ) register NODE *p; {
	register NODE *r, *l;

	r = p->in.right;
	l = p->in.left;

	if( p->in.right->in.su == 0 ){ /* rhs is addressable */
		if( logop( p->in.op ) ){
			if( l->in.op == UNARY MUL && l->in.type != FLOAT && shumul( l->in.left ) != STARREG ) offstar( l->in.left );
			else order( l, INAREG|INTAREG|INBREG|INTBREG|INTEMP );
			return( 1 );
			}
		if( !istnode( l ) ){
			order( l, INTAREG|INTBREG );
			return( 1 );
			}
		/* rewrite */
		return( 0 );
		}
	/* now, rhs is complicated: must do both sides into registers */
	/* do the harder side first */

	if( logop( p->in.op ) ){
		/* relational: do both sides into regs if need be */

		if( r->in.su > l->in.su ){
			if( niceuty(r) ){
				offstar( r->in.left );
				return( 1 );
				}
			else if( !istnode( r ) ){
				order( r, INTAREG|INAREG|INTBREG|INBREG|INTEMP );
				return( 1 );
				}
			}
		if ((r->in.type == FLOAT)||(l->in.type == FLOAT)) {
		/* if one subtree is simple, do the other.
		   else 
			do the right subtree
		*/
		if ((r->in.su != 8)&&(!istnode(l))) {
			order( l, INTAREG|INAREG|INTBREG|INBREG|INTEMP );
			return( 1 );
			}
		if( !istnode( r ) ){
			order( r, INTAREG|INAREG|INTBREG|INBREG|INTEMP );
			return( 1 );
			}
		if( !istnode( l ) ){
			order( l, INTAREG|INAREG|INTBREG|INBREG|INTEMP );
			return( 1 );
			}
		}
		else 
		{
		if( niceuty(l) ){
			offstar( l->in.left );
			return( 1 );
			}
		else if( !istnode( l ) ){
			order( l, INTAREG|INAREG|INTBREG|INBREG|INTEMP );
			return( 1 );
			}
		else if( niceuty(r) ){
			offstar( r->in.left );
			return( 1 );
			}
		if( !istnode( r ) ){
			order( r, INTAREG|INAREG|INTBREG|INBREG|INTEMP );
			return( 1 );
			}
		}
		cerror( "setbin can't deal with %s", opst[p->in.op] );
		}

	/* ordinary operator */

	if( !istnode(r) && r->in.su > l->in.su ){
		/* if there is a chance of making it addressable, try it... */
		if( niceuty(r) ){
			offstar( r->in.left );
			return( 1 );  /* hopefully, it is addressable by now */
			}
		order( r, INTAREG|INAREG|INTBREG|INBREG|INTEMP );  /* anything goes on rhs */
		return( 1 );
		}
	else {
		if( !istnode( l ) ){
			order( l, INTAREG|INTBREG );
			return( 1 );
			}
		/* rewrite */
		return( 0 );
		}
	}

setstr( p ) register NODE *p; { /* structure assignment */
	if( p->in.right->in.op != REG ){
		order( p->in.right, INTBREG );
		return(1);
		}
	p = p->in.left;
	if( p->in.op != NAME && p->in.op != OREG ){
		if( p->in.op != UNARY MUL ) cerror( "bad setstr" );
		order( p->in.left, INTBREG );
		return( 1 );
		}
	return( 0 );
	}

setasg( p ) register NODE *p; {
	/* setup for assignment operator */

	if( p->in.right->in.su != 0 && p->in.right->in.op != REG ) {
		if( p->in.right->in.op == UNARY MUL )
			offstar( p->in.right->in.left );
		else
			order( p->in.right, INAREG|INBREG|SOREG|SNAME|SCON );
		return(1);
		}
	if( p->in.left->in.op == UNARY MUL && !tshape( p->in.left, STARREG|STARNM ) ){
		offstar( p->in.left->in.left );
		return(1);
		}
	if( p->in.left->in.op == FLD && p->in.left->in.left->in.op == UNARY MUL ){
		offstar( p->in.left->in.left->in.left );
		return(1);
		}
	/* if things are really strange, get rhs into a register */
	if( p->in.right->in.op != REG ){
		order( p->in.right, INAREG|INBREG );
		return( 1 );
		}
	return(0);
	}

setasop( p ) register NODE *p; {
	/* setup for =ops */
	register sul, sur;
	register NODE *q, *p2;

	sul = p->in.left->in.su;
	sur = p->in.right->in.su;

	switch( p->in.op ){
	  case ASG PLUS:
	  case ASG MINUS:
	  case ASG OR:
	  case ASG ER:
	  case ASG AND:
		if (p->in.right->in.op != REG && sul == 0) {
		  order(p->in.right,INAREG|INTAREG);
		  return(1);
		} else break;

	  case ASG LS:
	  case ASG RS:
		if (p->in.left->in.op != REG) return(0);
		if (p->in.right->in.op == REG ||
		    (p->in.right->in.op==ICON && p->in.right->tn.lval>=1 && p->in.right->tn.lval<=8))
		  break;
		order(p->in.right,INAREG|INTAREG);
		return(1);

#ifdef NOTDEF
/* GB 7/17/87.  fix for scr2649.  */
	  case ASG MUL:
	  case ASG DIV:
	  case ASG MOD:
		if (p->in.left->in.op != REG) return(0) ;
#endif
	}

	if( sur == 0 ){

	leftadr:
		/* easy case: if addressable, do left value, op, store */
		if( sul == 0 ) goto rew;  /* rewrite */

		/* harder; make aleft address, val, op, and store */

		if( p->in.left->in.op == UNARY MUL ){
			offstar( p->in.left->in.left );
			return( 1 );
			}
		if( p->in.left->in.op == FLD && p->in.left->in.left->in.op == UNARY MUL ){
			offstar( p->in.left->in.left->in.left );
			return( 1 );
			}
	rew:	/* rewrite, accounting for autoincrement and autodecrement */

		q = p->in.left;
		if( q->in.op == FLD ) q = q->in.left;
		if( q->in.op != UNARY MUL || shumul(q->in.left) != STARREG ) return(0); /* let reader.c do it */

		/* mimic code from reader.c */

		p2 = tcopy( p );
		p->in.op = ASSIGN;
		reclaim( p->in.right, RNULL, 0 );
		p->in.right = p2;

		/* now, zap INCR on right, ASG MINUS on left */

		if( q->in.left->in.op == INCR ){
			q = p2->in.left;
			if( q->in.op == FLD ) q = q->in.left;
			if( q->in.left->in.op != INCR ) cerror( "bad incr rewrite" );
			}
		else if( q->in.left->in.op != ASG MINUS )  cerror( " bad -= rewrite" );

		q->in.left->in.right->in.op = FREE;
		q->in.left->in.op = FREE;
		q->in.left = q->in.left->in.left;

		/* now, resume reader.c rewriting code */

		canon(p);
		rallo( p, p->in.rall );
		order( p2->in.left, INTBREG|INTAREG );
		order( p2, INTBREG|INTAREG );
		return( 1 );
		}

	/* harder case: do right, left address, left value, op, store */

	if( p->in.right->in.op == UNARY MUL && p->in.left->in.op==REG){
		offstar( p->in.right->in.left );
		return( 1 );
		}
	/* sur> 0, since otherwise, done above */
	if( p->in.right->in.op == REG ) goto leftadr;  /* make lhs addressable */
	order( p->in.right, INAREG|INBREG );
	return( 1 );
	}

int crslab = 10000;

getlab(){
	return( crslab++ );
	}

deflab( l ){
	printf( ".L%d:\n", l );
	}

genargs( p
#ifdef SGI_REGS
	,specialargtype,hw_opcode
#endif
	) register NODE *p; {
	register size,inc;

	/* generates the code for the right-most argument first.  This
	   corresponds to the last argument in a call: func(a,b). */

	/* generate code for the arguments */

#ifdef SGI_REGS
	if (edebug) printf("genargs : %o argtype %x\n",p
				    ,specialargtype
				    );
	/*  first, do the arguments on the right (last->first) */
	/*  GB REG - in the special case of passing arguments in registers ***/
	if (specialargtype) {
		int d0=D0,d1=D1;
		if ((specialargtype & HWNODE)&&(fpatype)) {d0 |= HWOP; d1 |= HWOP;}
	    if (specialargtype & UNARYOP) {
			/* the passed node is the single argument */
			p->in.node_sgi |= SGIARG|d0;
			if (specialargtype & DOUBLEOP) p->in.node_sgi |= DOUBLEOP;
	    }
	    else {
	    	register NODE *tl = p->in.left, *tr = p->in.right;
			/* assign the registers */
	        if (specialargtype & DOUBLEOP) {
		    	tr->in.node_sgi |= DOUBLEOP;
		    	tl->in.node_sgi |= DOUBLEOP;
	    	}
			if (specialargtype & INDIRECTOP) {
		    	tl->in.node_sgi |= SGIARG|A0;
		    	tr->in.node_sgi |= SGIARG|d0;
			}
			else {
		    	tl->in.node_sgi |= SGIARG|d0;
		    	tr->in.node_sgi |= SGIARG|d1;
			}
			if ((!(specialargtype & HWNODE))||(fpatype == NO_FPA)) {
				/*  pass thru each node and generate it so that
	   	    		only a simple op is left.  */
	    		/* if the left node is not simple, we may have work to do. */
	    		if (dope[tl->in.op] != LTYPE) {
			    	/* left node is not simple.  */
		    		/* if the right node is simple, we want to
					   evaluate the left first.  If the
					   right is complex, evaluate the left into
					   a temp
		     		*/
		     		if (dope[tr->in.op] == LTYPE) {
						/* just evaluate the left first,
			   		   	and leave it in d0. */
						order(tl,FORARG);
						/* and mark it as finished */
						/* unless we need to move it from d0 */
						tl->in.node_sgi |= PUSHED;
		     		} else {
						/* neither node is simple.*/  
						/* evaluate left into a temp */
						order(tl,INTEMP);
		    		} 
				}
	    	} else {
			/* NODE is for a hardware op.  
			   This has slightly different properties 
			*/
	    		/* if the left node is not simple, we may have work to do. */
	    		if (dope[tl->in.op] != LTYPE) {
		    		/* left node is not simple.  */
					/* we must evaluate the left in a temp unless the 
					   right is simple and the op is commutative */
		     		if ((dope[tr->in.op] == LTYPE)
						&&(specialargtype & COMMUTEOP)) {
						/* right is simple and the node is
						   commutative. Swap the children so we
						   dont have evaluate the left in a temp 
						*/
			    		NODE *n;
						if (edebug) printf("genargs(h/w): swapping children\n");
						if (tr->in.node_sgi & WRITE_HWOP) {
							/* SKY_FPA case */
							tl->in.node_sgi |= WRITE_HWOP;
							tl->in.hw_opcode = tr->in.hw_opcode;
							tr->in.node_sgi &= ~WRITE_HWOP;
						}
						else if (tl->in.node_sgi & WRITE_HWOP) {
							/* JUNIPER_FPA case */
							tr->in.node_sgi |= WRITE_HWOP;
							tr->in.hw_opcode = tl->in.hw_opcode;
							tl->in.node_sgi &= ~WRITE_HWOP;
						}
						n = tl;
						p->in.left = tl = tr;
						p->in.right = tr = n;
		        	} else {
						order(tl,INTEMP);
						tl->in.node_sgi |= HWOP;
					}
				}
			}
		}
	}
#endif
		
    while( p->in.op == CM ){
		genargs( p->in.right
#ifdef SGI_REGS
			,0 ,0
#endif
			);
		p->in.op = FREE;
		p = p->in.left;
	}

	if( p->in.op == STARG ){ /* structure valued argument */

		size = p->stn.stsize;
		if( p->in.left->in.op == ICON ){
			/* make into a name node */
			p->in.op = FREE;
			p= p->in.left;
			p->in.op = NAME;
			}
		else {
			/* make it look beautiful... */
			p->in.op = UNARY MUL;
			canon( p );  /* turn it into an oreg */
			if( p->in.op != OREG ){
				offstar( p->in.left );
				canon( p );
#ifdef NOTDEF
				/* old code */
				if( p->in.op != OREG ) cerror( "stuck starg" );
#else
				/* fix for scr1464 */
				if (p->in.op != OREG) {
					offstar( p->in.left );
					canon( p );
					if( p->in.op != OREG ) 
						cerror( "stuck starg" );
				}
#endif
			}
		}

		p->tn.lval += size;  /* end of structure */
		/* put on stack backwards */
		for( ; size>0; size -= inc ){
			inc = (size>2) ? 4 : 2;
			p->tn.lval -= inc;
#ifdef SGI_REGS
			expand( p, RNOP,(inc==4)?"\tZ-AR\n":"\tZ-WR\n" );
#else
			expand( p, RNOP,(inc==4)?"\tmovl\tAR,Z-\n":"\tmovw\tAR,Z-\n" );
#endif
			}
		reclaim( p, RNULL, 0 );
		return;
		}

	/* ordinary case */
#ifdef SGI_REGS
	/* if the result is where we want it, dont evaluate FORARG */
	if ((p->in.node_sgi & (SGIARG|PUSHED))== (SGIARG|PUSHED))
		{if (p->in.op == REG)
			rfree(p->tn.rval);
		}
	else
#ifdef NOTDEF
	if (!((p->in.node_sgi & (SGIARG|PUSHED))== (SGIARG|PUSHED)))
#endif
#endif
	    order( p, FORARG );
	}

argsize( p ) register NODE *p; {
	register t;
	t = 0;
	if( p->in.op == CM ){
		t = argsize( p->in.left );
		p = p->in.right;
		}
#ifdef DOUBLES32BITS
	if( p->in.type == DOUBLE ){
		SETOFF( t, 2 );
		return( t+8 );
		}
	else if ( p->in.type == FLOAT ){
		SETOFF( t, 2 );
		return( t+4 );
		}
#else DOUBLES32BITS
	if( p->in.type == DOUBLE || p->in.type == FLOAT ){
		SETOFF( t, 2 );
		return( t+8 );
		}
#endif
	else if( p->in.op == STARG ){
		SETOFF( t, p->stn.stalign );  /* alignment */
		return( t + p->stn.stsize );  /* size */
		}
	else {
		SETOFF( t, 2 );
		return( t+4 );
		}
	}
