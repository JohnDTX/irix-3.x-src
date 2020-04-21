# include "mfile2.h"	/* includes macdefs.h mac2defs.h manifest.h */
#ifdef SGI_REGS
# include "regprefs.h"
#endif

/*****	GB SGI 8/11/83 UNI various speedups introduced *****/

NODE resc[3];

int busy[REGSZ];

int maxa, mina, maxb, minb;

allo0(){ /* free everything */

	register i;

	maxa = maxb = -1;
	mina = minb = 0;

	REGLOOP(i){
		busy[i] = 0;
		if( rstatus[i] & STAREG ){
			if( maxa<0 ) mina = i;
			maxa = i;
			}
		if( rstatus[i] & STBREG ){
			if( maxb<0 ) minb = i;
			maxb = i;
			}
		}
	}

# define TBUSY 01000

allo( p, q ) register NODE *p; struct optab *q; {

	register n, i, j;
	register NODE *rp;

	n = q->needs;
	i = 0;

	/** GB SCR911 --
		
		if this node is a special sgi node which is a
		UNARY CALL and there is a PREF for a particular register,
		make the PREF a MUSTDO, so that the result is put in
		the correct place...

	**/
	if ((p->in.op == UNARY CALL) && (p->in.node_sgi) && (fpatype == NO_FPA)
		&& (p->in.rall) && (!(p->in.rall & NOPREF)))
	{
		p->in.rall |= MUSTDO;
	}

	rp = &resc[0];

	while( n & NACOUNT ){
		rp->in.op = REG;
		rp->tn.rval = freereg( p, n&NAMASK );
		rp->tn.lval = 0;
		rp->in.name[0] = '\0';
		n -= NAREG;
		++i;
		++rp;
		}

	while( n & NBCOUNT ){
		rp->in.op = REG;
		rp->tn.rval = freereg( p, n&NBMASK );
		rp->tn.lval = 0;
		rp->in.name[0] = '\0';
		n -= NBREG;
		++i;
		++rp;
		}

	if( n & NTMASK ){
		rp->in.op = OREG;
		rp->tn.rval = TMPREG;
		if( p->in.op == STCALL || p->in.op == STARG || p->in.op == UNARY STCALL || p->in.op == STASG ){
			rp->tn.lval = freetemp( (SZCHAR*p->stn.stsize + (SZINT-1))/SZINT );
			}
		else {
/* GB (SGI - bugw8) the allocation is in ITEMs of the thing needed.  freetemp expects
   number of words of INTEGERS 
			rp->tn.lval = freetemp( (n&NTMASK)/NTEMP );
*/
			rp->tn.lval = freetemp( ((n&NTMASK)/NTEMP)*szty(p->in.type));
			}
		rp->in.name[0] = '\0';
		rp->tn.lval = BITOOR(rp->tn.lval);
		++i;
		++rp;
		}

	/* turn off "temporarily busy" bit */

	REGLOOP(j){
		busy[j] &= ~TBUSY;
		}

	rp = &resc[0];
	for( j=0; j<i; ++j, ++rp ) if( rp->tn.rval < 0 ) return(0);
	return(1);

	}

freetemp( k ){ /* allocate k integers worth of temp space */
	/* we also make the convention that, if the number of words is more than 1,
	/* it must be aligned for storing doubles... */

	int t;
	if (rdebug) printf(
		"freetmp (%d): maxtemp=%d, maxoff = %d, tmpoff = %d, baseoff= %d\n",
		 k,maxtemp,maxoff, tmpoff, baseoff);
# ifndef BACKTEMP


	if( k>1 ){
		SETOFF( tmpoff, ALDOUBLE );
		}

	t = tmpoff;
	tmpoff += k*SZINT;
	if( tmpoff > maxoff ) maxoff = tmpoff;
	if( tmpoff-baseoff > maxtemp ) maxtemp = tmpoff-baseoff;
	return(t);

# else
	tmpoff += k*SZINT;
	if( k>1 ) {
		SETOFF( tmpoff, ALDOUBLE );
		}
	if( tmpoff > maxoff ) maxoff = tmpoff;
	if( tmpoff-baseoff > maxtemp ) maxtemp = tmpoff-baseoff;
	if (rdebug) printf("\treturned %d. maxtemp = %d, tmpoff=%d, maxoff=%d\n",
						(-tmpoff),maxtemp,tmpoff,maxoff);
	return( -tmpoff );
# endif
	}

freereg( p, n ) register NODE *p; {
	/* allocate a register of type n */
	/* p gives the type, if floating */

	register j;

	/* not general; means that only one register (the result) OK for call */
	if( callop(p->in.op) ){
		j = callreg(p);
		if( usable( p, n, j ) ) return( j );
		/* have allocated callreg first */
		}
	j = p->in.rall & ~MUSTDO;
	if( j!=NOPREF && usable(p,n,j) ){ /* needed and not allocated */
		return( j );
		}
	if( n&NAMASK ){
		for( j=mina; j<=maxa; ++j ) if( rstatus[j]&STAREG ){
			if( usable(p,n,j) ){
				return( j );
				}
			}
		}
	else if( n &NBMASK ){
		for( j=minb; j<=maxb; ++j ) if( rstatus[j]&STBREG ){
			if( usable(p,n,j) ){
				return(j);
				}
			}
		}

	return( -1 );
	}

usable( p, n, r ) NODE *p; register r; {
	/* decide if register r is usable in tree p to satisfy need n */
 
	/* checks, for the moment */

	if( !istreg(r) ) cerror( "usable asked about nontemp register" );

	if( busy[r] > 1 ) return(0);
 	if(((n&NAMASK) && !(rstatus[r]&SAREG)) || ((n&NBMASK) && !(rstatus[r]&SBREG)))
		return(0);
	if( (szty(p->in.type) == 2) ){ /* only do the pairing for real regs */
		if( r&01 ) return(0);
		if( !istreg(r+1) ) return( 0 );
		if( busy[r+1] > 1 ) return( 0 );
		if( busy[r] == 0 && busy[r+1] == 0  ||
		    busy[r+1] == 0 && shareit( p, r, n ) ||
		    busy[r] == 0 && shareit( p, r+1, n ) ){
			busy[r] |= TBUSY;
			busy[r+1] |= TBUSY;
			return(1);
			}
		else return(0);
		}
	if( busy[r] == 0 ) {
		busy[r] |= TBUSY;
		return(1);
		}

	/* busy[r] is 1: is there chance for sharing */
	return( shareit( p, r, n ) );

	}

shareit( p, r, n ) NODE *p; {
	/* can we make register r available by sharing from p
	   given that the need is n */
	if (rdebug) {
		printf("trying to share reg %d for need 0x%x\n",r,n);
		}
	if( (n&(NASL|NBSL)) && ushare( p, 'L', r ) ) return(1);
	if( (n&(NASR|NBSR)) && ushare( p, 'R', r ) ) return(1);
#ifdef SGI_REGS
	if ( (r==D0) && (p->in.op == UNARY CALL) && 
		(p->in.node_sgi & SGIARG)) {
	    if (radebug) printf("shared as sgi special arg\n");
	    /* D0 was an argument and is a function result */
	    return(1);
	}
#endif
	if (rdebug) { printf("couldn't share it...\n");}
	return(0);
	}

ushare( p, f, r ) register NODE *p; {
	/* can we find a register r to share on the left or right
		(as f=='L' or 'R', respectively) of p */
	p = getlr( p, f );
	if( p->in.op == UNARY MUL ) p = p->in.left;
	if( p->in.op == OREG ){
		if( R2TEST(p->tn.rval) ){
			return( r==R2UPK1(p->tn.rval) || r==R2UPK2(p->tn.rval) );
			}
		else return( r == p->tn.rval );
		}
	if( p->in.op == REG ){
		return( r == p->tn.rval || ( szty(p->in.type) == 2 && r==p->tn.rval+1 ) );
		}
	return(0);
	}

recl2( p ) register NODE *p; {
	register r = p->tn.rval;
	if( p->in.op == REG ) rfree( r, p->in.type );
	else if( p->in.op == OREG ) {
		if( R2TEST( r ) ) {
			rfree( R2UPK1( r ), PTR+INT );
			rfree( R2UPK2( r ), INT );
			}
		else {
			rfree( r, PTR+INT );
			}
		}
	}

int rdebug = 0;

rfree( r, t ) TWORD t; {
	/* mark register r free, if it is legal to do so */
	/* t is the type */

	if( rdebug ){
		printf( "rfree( %s ), size %d", rnames[r], szty(t) );
		}

	if( istreg(r) ){
		if( --busy[r] < 0 ) cerror( "register overfreed");
		if( szty(t) == 2 ){
			if( (r&01) ) cerror( "illegal free" );
			if( --busy[r+1] < 0 ) cerror( "register overfreed" );
			}
		}
	if (rdebug) printf("\tnow %d users.\n",busy[r]);
	}

rbusy(r,t) register r; TWORD t; {
	/* mark register r busy */
	/* t is the type */

	if( rdebug ){
		printf( "rbusy( %s ), type %d, size %d",
			rnames[r], t, szty(t) );
		}


	if( istreg(r) ) ++busy[r];
	if( szty(t) == 2 ){
		if( istreg(r+1) ) ++busy[r+1];
		if( (r&01) ) cerror( "illegal register pair freed" );
		}
	if (rdebug) printf("\tnow %d users.\n",busy[r]);
	}

rwprint( rw ){ /* print rewriting rule */
	register i, flag;
	static char * rwnames[] = {

		"RLEFT",
		"RRIGHT",
		"RESC1",
		"RESC2",
		"RESC3",
		0,
		};

	if( rw == RNULL ){
		printf( "RNULL" );
		return;
		}

	if( rw == RNOP ){
		printf( "RNOP" );
		return;
		}

	flag = 0;
	for( i=0; rwnames[i]; ++i ){
		if( rw & (1<<i) ){
			if( flag ) printf( "|" );
			++flag;
			printf( rwnames[i] );
			}
		}
	}

reclaim( p, rw, cookie ) register NODE *p; {
	register NODE **qq;
	register NODE *q;
	register i;
	NODE *recres[5];
	struct respref *r;

	/* allo (called by match()) has filled in the resc[] array with where
	   the results are.  We have been passed the cookie to reclaim the
	   tree and put away the results */

	if( rdebug ){
		printf( "reclaim( %o, ", p );
		rwprint( rw );
		printf( ", " );
		prcook( cookie );
		printf(", sgi = %x, hwop = %x",p->in.node_sgi,p->in.hw_opcode);
		printf( " )\n" );
		}

	if( rw == RNOP || ( p->in.op==FREE && rw==RNULL ) ) return;  /* do nothing */

	walkf( p, recl2 );

	if( callop(p->in.op) ){
		/* check that all scratch regs are free */
		callchk(p);  /* ordinarily, this is the same as allchk() */
		}

	if( rw == RNULL || (cookie&FOREFF) ){ /* totally clobber, leaving nothing */
		tfree(p);
		return;
		}

	/* handle condition codes specially */

	if( (cookie & FORCC) && (rw&RESCC)) {
		/* result is CC register */
		tfree(p);
		p->in.op = CCODES;
		p->tn.lval = 0;
		p->tn.rval = 0;
		return;
		}

	/* locate results */

	qq = recres;

	if( rw&RLEFT) *qq++ = getlr( p, 'L' );;
	if( rw&RRIGHT ) *qq++ = getlr( p, 'R' );
	if( rw&RESC1 ) *qq++ = &resc[0];
	if( rw&RESC2 ) *qq++ = &resc[1];
	if( rw&RESC3 ) *qq++ = &resc[2];

	if( qq == recres ){
		cerror( "illegal reclaim");
		}

	*qq = NIL;

	/* now, select the best result, based on the cookie */

	for( r=respref; r->cform; ++r ){
		if( cookie & r->cform ){
			for( qq=recres; (q= *qq) != NIL; ++qq ){
				if( tshape( q, r->mform ) ) goto gotit;
				}
			}
		}

	/* we can't do it; die */
	cerror( "cannot reclaim");

	gotit:

	if( p->in.op == STARG ) p = p->in.left;  /* STARGs are still STARGS */

	q->in.type = p->in.type;  /* to make multi-register allocations work */
		/* maybe there is a better way! */
	q = tcopy(q);

	tfree(p);

	p->in.op = q->in.op;
	p->tn.lval = q->tn.lval;
	p->tn.rval = q->tn.rval;
	for( i=0; i<NCHNAM; ++i )
		p->in.name[i] = q->in.name[i];

	q->in.op = FREE;

	/* if the thing is in a register, adjust the type */

	switch( p->in.op ){

	case REG:
		if( ! (p->in.rall & MUSTDO ) ) return;  /* unless necessary, ignore it */
		i = p->in.rall & ~MUSTDO;
		if( i & NOPREF ) return;
		if( i != p->tn.rval ){
			if( busy[i] || ( szty(p->in.type)==2 && busy[i+1] ) ){
				cerror( "faulty register move" );
				}
			rbusy( i, p->in.type );
			rfree( p->tn.rval, p->in.type );
			rmove( i, p->tn.rval, p->in.type );
			p->tn.rval = i;
			}

	case OREG:
		if( R2TEST(p->tn.rval) ){
			int r1, r2;
			r1 = R2UPK1(p->tn.rval);
			r2 = R2UPK2(p->tn.rval);
			if( (busy[r1]>1 && istreg(r1)) || (busy[r2]>1 && istreg(r2)) ){
				cerror( "potential register overwrite" );
				}
			}
		else if( (busy[p->tn.rval]>1) && istreg(p->tn.rval) ) cerror( "potential register overwrite");
		}

	}

ncopy( q, p ) register NODE *p, *q; {
	/* copy the contents of p into q, without any feeling for
	   the contents */
	/* this code assume that copying rval and lval does the job;
	   in general, it might be necessary to special case the
	   operator types */
	register i;

	q->in.op = p->in.op;
	q->in.rall = p->in.rall;
#ifdef SGI_REGS
	q->in.node_sgi = p->in.node_sgi;
	q->in.hw_opcode = p->in.hw_opcode;
#endif
	q->in.type = p->in.type;
	q->tn.lval = p->tn.lval;
	q->tn.rval = p->tn.rval;
	for( i=0; i<NCHNAM; ++i ) q->in.name[i]  = p->in.name[i];

	}

NODE *
tcopy( p ) register NODE *p; {
	/* make a fresh copy of p */

	register NODE *q;
	register r;

	ncopy( q=talloc(), p );

	r = p->tn.rval;
	if( p->in.op == REG ) rbusy( r, p->in.type );
	else if( p->in.op == OREG ) {
		if( R2TEST(r) ){
			rbusy( R2UPK1(r), PTR+INT );
			rbusy( R2UPK2(r), INT );
			}
		else {
			rbusy( r, PTR+INT );
			}
		}

	switch( optype(q->in.op) ){

	case BITYPE:
		q->in.right = tcopy(p->in.right);
	case UTYPE:
		q->in.left = tcopy(p->in.left);
		}

	return(q);
	}

allchk(){
	/* check to ensure that all register are free */

	register i;

	REGLOOP(i){
		if( istreg(i) && busy[i] ){
			if (rdebug) printf("temp register %d busy\n",i);
			cerror( "register allocation error");
			}
		}

	}
