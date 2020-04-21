# include <stdio.h>
# include <signal.h>
# include <stab.h>
#include <errno.h>
int gdebug,labelno;
int fdefflag;  /* are we within a function definition ? */
#ifndef ONVAX
#include <fpsignal.h>
#endif

# include "mfile1.h"	/* includes macdefs.h manifest.h */

extern int usedregs;	/* bit == 1 if reg was used in subroutine */
extern char *rnames[];
int suppress_warnings;
int verbose,fpatype;
extern int xdebug;
int proflag;
int strftn = 0;	/* is the current function one which returns a value */
#ifdef SGI_REGS
int fltftn = 0;
#endif
FILE *tempfile;
FILE *outfile = stdout;
FILE *errfile;
int isJuniper = 0;
int New_assembler = 0;


branch( n ){
	/* output a branch to label n */
/*****  bug fix #36 (GB) SGI 8/9/83. Branch doesn't alter the 
	destination.  Two return labels are now allocated by bfcode
	if a structure is to be returned, and cgram branches to the
	appropriate one depending on the type of return. *****/

	 printf( "	bra	.L%d\n", n );
	}

int lastloc = PROG;

defalign(n) {
	/* cause the alignment to become a multiple of n */
	n /= SZCHAR;
	if( lastloc != PROG && n > 1 ) printf( "	.even\n" );
	}

locctr( l ){
	register temp;
	/* l is PROG, ADATA, DATA, STRNG, ISTRNG, or STAB */

	if( l == lastloc ) return(l);
	temp = lastloc;
	lastloc = l;
	switch( l ){

	case PROG:
		outfile = stdout;
		printf( "	.text\n" );
		break;

	case DATA:
	case ADATA:
		outfile = stdout;
		if( temp != DATA && temp != ADATA )
			printf( "	.data\n" );
		break;

	case STRNG:
	case ISTRNG:
		outfile = tempfile;
		break;

	case STAB:
		printf( "	.stab\n" );
		break;

	default:
		cerror( "illegal location counter" );
		}

	return( temp );
	}

deflab( n ){
	/* output something to define the current position as label n */
	fprintf( outfile, ".L%d:\n", n );
	}

int crslab = 10;

getlab(){
	/* return a number usable for a label */
	return( ++crslab );
	}

efcode(){
	/* code for the end of a function */

	if( strftn ){  /* copy output (in r0) to caller */
		register struct symtab *p;
		register int stlab;
		register int count;
		int size;

		p = &stab[curftn];

		deflab( retlab );

/****** BUG FIX (#36) GB SGI 8/9/83.  retnovallab is the label for the 
	final exit.  It was allocated by bfcode *****/

		retlab = retnovallab;
		stlab = getlab();
		printf( "	movl	d0,a0\n" );
		printf( "	movl	#.L%d,a1\n" , stlab );
		size = tsize( DECREF(p->stype), p->dimoff, p->sizoff ) / SZCHAR;
		count = size/4;
		while( count-- ) {
			printf( "	movl	a0@+,a1@+\n" );
		}
		if (size&2) {
			printf( "	movw	a0@+,a1@+\n" );
		}
		if (size&1) {
			printf( "	movb	a0@+,a1@+\n" );
		}
		printf( "	movl	#.L%d,d0\n", stlab );

/****   BUGREPORT #35 GB SGI 8/9/83.  Get rid of
	trailing blank in the symbol equate. AND force actual 
	space reservation.
	printf( "	.bss\n	.even\n.L%d	=	.+%d.\n	.text\n", stlab, size );
 ******/
/*
	printf( "	.bss\n	.even\n.L%d    =  . = .+%d\n	.text\n", stlab, size );
*/
	printf("\t.bss\n\t.even\n.L%d:\t.space\t%d\n\t.text\n",stlab,size);

		/* turn off strftn flag, so return sequence will be generated */
		strftn = 0;
		}

	branch( retlab );
	p2bend();
	fdefflag = 0;
	}

/****GB SGI (#36) ****/
int retnovallab;
/*****/

bfcode( a, n ) int a[]; {
	/* code for the beginning of a function; a is an array of
		indices in stab for the arguments; n is the number */
	register i;
	register temp;
	register struct symtab *p;
	int off;
	char type;

	locctr( PROG );
	p = &stab[curftn];
	/* GB - Fortran Change */
	if (isfortran ) 
		deffortnam(p);
	else 
		defnam( p );

	isfortran =0;
	temp = p->stype;
	temp = DECREF(temp);
	strftn = (temp==STRTY) || (temp==UNIONTY);
#ifdef SGI_REGS
	if (temp == FLOAT) fltftn = 1;
	else if (temp == DOUBLE) fltftn = 2;
	else fltftn=0;
#endif

/*****  BUG FIX (#36) GB SGI  retlab is the default return label
	when the function is returning a value.  retnovallab is the
	label when the function is NOT returning a value.  Unless
	the function returns a structure, these are the same, as the 
	function result is undefined ******/

	retlab = getlab();
	if (!strftn) retnovallab=retlab;
	 else retnovallab=getlab();
/******/

	if( proflag ){
		int plab;
		plab = getlab();
		printf( "	movl	#.L%d,a0\n", plab );
		printf( "	jbsr	mcount\n" );
		printf( "	.data\n.L%d:	.long 0\n	.text\n", plab );
		}

	/* routine prolog */
	printf( "	link	a6,#-.F%d\n", ftnno );

/******	GB SGI bug#42 stackprobe UNI *******/
#ifdef STACKPROBE
	printf("	tstb\tsp@(-.M%d)\n", ftnno);
#endif 

	if (New_assembler)
		printf( "	moveml	#.S%d,a6@(-.F%d:w)\n", ftnno, ftnno );
	else
		printf( "	moveml	#.S%d,a6@(-.F%d)\n", ftnno, ftnno );

	usedregs = 0;

	if (gdebug) {
#ifdef STABDOT
		pstabdot(N_SLINE, lineno);
#else
		pstab(NULLNAME, N_SLINE);
		printf("0,%d,.LL%d\n", lineno, labelno);
		printf(".LL%d:\n", labelno++);
#endif
	}
	off = ARGINIT;

	for( i=0; i<n; ++i ){
		p = &stab[a[i]];
		if( p->sclass == REGISTER ){
			temp = p->offset;  /* save register number */
			p->sclass = PARAM;  /* forget that it is a register */
			p->offset = NOOFFSET;
			oalloc( p, &off );
			if (p->stype==CHAR || p->stype==UCHAR) type = 'b';
			else if (p->stype==SHORT || p->stype==USHORT) type = 'w';
			else type = 'l';
			printf( "	mov%c	a6@(%d),%s\n", type, p->offset/SZCHAR,
			  rnames[temp] );
			usedregs |= 1<<temp;
			p->offset = temp;  /* remember register number */
			p->sclass = REGISTER;   /* remember that it is a register */
			}
		else {
			if( oalloc( p, &off ) ) cerror( "bad argument" );
			}

		}
	printf("| A%d = %d\n", ftnno, off/SZCHAR);
	fdefflag = 1;
	}

bccode(){ /* called just before the first executable statment */
		/* by now, the automatics and register variables are allocated */
	SETOFF( autooff, SZINT );
	/* set aside store area offset */
	p2bbeg( autooff, regvar );
	}

ejobcode( flag ){
#ifdef NOTDEF
	/* called just before final exit */
	/* flag is 1 if errors, 0 if none */
	extern int errno,sys_nerr;
	if ((errno)&& (errno != ENOTTY))
	{
		where('u');
		perror("");
		return(errno);
	} 
	return(0);
#endif
}

aobeg(){
	/* called before removing automatics from stab */
	}

aocode(p) struct symtab *p; {
	/* called when automatic p removed from stab */
	}

aoend(){
	/* called after removing all automatics from stab */
	}

char *fortexname();
char *exname();

/* GB - FORTRAN */
defnam( p ) register struct symtab *p; {
	/* define the current location as the name p->sname */

	if( p->sclass == FORTRAN ){
		printf( "	.globl	%s\n", fortexname( p->sname ) );
		printf( "	.globl	%s\n", exname( p->sname ) );
		}
	else if( p->sclass == EXTDEF ){
		printf( "	.globl	%s\n", exname( p->sname ) );
		}
	if( p->sclass == STATIC && p->slevel>1 ) deflab( p->offset );
	else {
		printf( "%s:\n", exname( p->sname ) );
		if (p->sclass == FORTRAN)
			printf( "%s:\n", fortexname( p->sname ) );

	}
}

deffortnam( p ) register struct symtab *p; {
	/* define the current location as the name p->sname */

	char *fortexname();

	if( p->sclass == EXTDEF ){
		printf( "	.globl	%s\n", fortexname( p->sname ) );
		}
	if( p->sclass == STATIC && p->slevel>1 ) 
	{
		uerror("static fortran functions disallowed. - ignored fortran spec");
		deflab( p->offset );
	}
	else printf( "%s:\n", fortexname( p->sname ) );

	}

#define ASCII 0
#ifdef ASCII
int byinit=1;
bycode( t, i ){
	/* put byte i+1 in a string */

	i &= 31 ;
	if( t < 0 ){ /* end of the string */
		if(( i != 0) && !byinit ) {
			fprintf( outfile, "\"\n" );
			byinit++;
		}

	}
	else { /* stash byte t into string */
		/* t &= 0x7f; */
		if((byinit)&&(t)) {
			fprintf( outfile, "\t.ascii\t\"" );
			byinit=0;
		}

		/* put the character, either as text or special */
		if (t == '\134') putc('\134',outfile);
		if ((t>=0x20)&&(t<0x7f)&&(t!=0x22)) putc(t,outfile);
		/*else if (t) fprintf(outfile,"\\%03.3o",t);*/
		else {
			if (!byinit) fprintf(outfile,"\"\n");
			fprintf(outfile,"\t.byte\t0x%x\n",t);
			byinit++;
		}

		if( (i == 31)&&!byinit ) {
			fprintf( outfile, "\"\n" );
			byinit++;
		}
	   }

	}
#else
bycode( t, i ){
	/* put byte i+1 in a string */

	i &= 07;
	if( t < 0 ){ /* end of the string */
		if( i != 0 ) fprintf( outfile, "\n" );
		}

	else { /* stash byte t into string */
		if( i == 0 ) fprintf( outfile, "	.byte	" );
		else fprintf( outfile, "," );
		fprintf( outfile, "%d", t );
		if( i == 07 ) fprintf( outfile, "\n" );
		}
	}
#endif
zecode( n ){
	/* n integer words of zeros */
	OFFSZ temp;
/*	register i;*/

	if( n <= 0 ) return;
/*################
 *
 *	ALTERATION TO INITIALIZE STATIC ARRAYS BY .space 
 *
 *	G.Boyd (SGI) 3/31/83 re-altered to .space for new as 7/4/83.
 *
 *################*/
/*	for( i=1; i<=n; i++ ) printf( "	.long	0\n" );*/
	printf("\t.space\t%d\n",(SZINT/SZCHAR)*n);
	inoff += n*SZINT;
	}

fldal( t ) unsigned t; { /* return the alignment of field of type t */
	uerror( "illegal field type" );
	return( ALINT );
	}

fldty( p ) struct symtab *p; { /* fix up type of field p */
#ifdef SGI_FIELDS
	int offset = p->offset%SZLONG;
	int itemp = offset + UPKFSZ(p->sclass);

	if (xdebug) {
		printf("fldty(): ");
		tprint(p->stype);
		printf(" (0x%x) (o= 0x%x, sz = 0x%x) -> ",
				p->stype,offset,UPKFSZ(p->sclass));
	}

	if (( itemp <= 16)||((offset >= 16)&&(itemp <= 32)))
		p->stype = USHORT;
	if ((itemp <= 8) || ((offset >= 16) && (itemp <= 24)))
		p->stype = UCHAR;
	if (xdebug) {tprint(p->stype);printf("(0x%x)\n",p->stype); }
#endif
	}

where(c){ /* print location of error  */
	/* c is either 'u', 'c', or 'w' */
	fprintf( errfile, "%s, line %d: ", ftitle, lineno );
	}

fperror(sig) int sig; {
	/* floating point exception occurred.  Report the line number*/
	werror("results of all floating point exceptions set to zero");
#ifdef ONVAX
	signal(SIGFPE, fperror);
#endif
}

/****
*****	(KIPP)	Fill in alignment variables with default values
****/
#ifdef	OLD_SGI_DEFAULT
int	ALINT = AL16_ALINT,
	ALFLOAT = AL16_ALFLOAT,
	ALDOUBLE = AL16_ALDOUBLE,
	ALLONG = AL16_ALLONG,
	ALPOINT = AL16_ALPOINT,
	ALSTRUCT = AL16_ALSTRUCT,
	ALSTACK = AL16_ALSTACK;
#else
int	ALINT = AL32_ALINT,
	ALFLOAT = AL32_ALFLOAT,
	ALDOUBLE = AL32_ALDOUBLE,
	ALLONG = AL32_ALLONG,
	ALPOINT = AL32_ALPOINT,
	ALSTRUCT = AL32_ALSTRUCT,
	ALSTACK = AL32_ALSTACK;
#endif

char *tmpname = "/tmp/pcXXXXXX";

main( argc, argv ) char *argv[]; {
	int dexit();
	register int c;
	register int i;
	int r;

	char *ipfilenm=0,*opfilenm=0;
	verbose = 0;fpatype = NO_FPA;
	reservea5=0;
	for( i=1; i<argc; ++i ) {
		if( argv[i][0] == '-' ) {
			if ( argv[i][1] == 'X' ) {
				if ( argv[i][2] == 'p' )  proflag = 1;
			}
			else switch(argv[i][1]) {
				case 'A':
						if (argv[i][2] == 'S') {
						    ALINT = AL16_ALINT;
						    ALFLOAT = AL16_ALFLOAT;
						    ALDOUBLE = AL16_ALDOUBLE;
						    ALLONG = AL16_ALLONG;
						    ALPOINT = AL16_ALPOINT;
						    ALSTRUCT = AL16_ALSTRUCT;
						    ALSTACK = AL16_ALSTACK;
						} else
						if (argv[i][2] == 'L') {
						    ALINT = AL32_ALINT;
						    ALFLOAT = AL32_ALFLOAT;
						    ALDOUBLE = AL32_ALDOUBLE;
						    ALLONG = AL32_ALLONG;
						    ALPOINT = AL32_ALPOINT;
						    ALSTRUCT = AL32_ALSTRUCT;
						    ALSTACK = AL32_ALSTACK;
						} else
			    cerror("illegal structure alignment boundary");
						break;

				case 'v':	verbose++;
						break;

				case 'w':	suppress_warnings++;
						break;

				case 'f':	if(isJuniper)
							fpatype = JUNIPER_FPA;
						else
							fpatype = SKY_FPA;
						break;
				case 'J':
						isJuniper=1;
						New_assembler++;
						if (fpatype == SKY_FPA)
							fpatype = JUNIPER_FPA;
						break;
				case 'N':
						New_assembler++;
						break;
				case 'R':
						reservea5++;
						break;
			}
		} else {
			if (!ipfilenm) ipfilenm = argv[i];
			else opfilenm = argv[i];
		}
	}

	/* just for clarity.  This would happen in common.c anyway. */
	if (suppress_warnings) verbose = 0;

	mktemp(tmpname);
	if(signal( SIGHUP, SIG_IGN) != SIG_IGN) signal(SIGHUP, dexit);
	if(signal( SIGINT, SIG_IGN) != SIG_IGN) signal(SIGINT, dexit);
	if(signal( SIGTERM, SIG_IGN) != SIG_IGN) signal(SIGTERM, dexit);
#ifndef ONVAX
	fpsigset(fperror,INHIBIT_DUMP|CONTINUE_AFTER_FPERROR);
#else
	if (signal(SIGFPE, SIG_IGN) != SIG_IGN) signal(SIGFPE, fperror);
#endif

	if ((ipfilenm) && (freopen(ipfilenm,"r",stdin) == NULL)) {
		fprintf(errfile,"ccom: cannot open input file %s\n",ipfilenm);
		exit(-1);
	}

	if ((opfilenm) && (freopen(opfilenm,"w",stdout) == NULL)) {
		fprintf(errfile,"ccom: cannot open output file %s\n",opfilenm);
		exit(-1);
	}

	if (fpatype) 
		cwerror((fpatype == SKY_FPA)?
			"Sky floating point option selected":
			"Juniper floating point option selected");

	tempfile = fopen( tmpname, "w" );
	if(tempfile == NULL) cerror( "Cannot open temp file" );

	r = mainp1( argc, argv );

	tempfile = freopen( tmpname, "r", tempfile );
	if( tempfile != NULL )
		while((c=getc(tempfile)) != EOF )
			putchar(c);
	else cerror( "Lost temp file" );

#ifdef NOTDEF
	/*** GB BUG71 check status of stdout and set exit code appropriately **/
	if ((!r) && ferror(stdout)) r=1;
#endif

#ifdef juniper
	if (r) printf("\t.abort\n");
#endif
	unlink(tmpname);
	return( r );
	}

dexit( v ) {
#ifdef juniper
	if (nerrors) printf("\t.abort\n");
#endif
	unlink(tmpname);
	exit(1);
	}

genswitch(p,n) register struct sw *p;{
	/*	p points to an array of structures, each consisting
		of a constant value and a label.
		The first is >=0 if there is a default label;
		its value is the label number
		The entries p[1] to p[n] are the nontrivial cases
		*/

/******	GB SGI switch converted from long to word offsets. UNI
	see .v6 source for old code 
******/

	register i;
	register CONSZ j, range;
	register dlab, swlab;

	range = p[n].sval-p[1].sval;

	if( range>0 && range <= 3*n && n>=4 ){ /* implement a direct switch */

		dlab = p->slab >= 0 ? p->slab : getlab();

		if( p[1].sval ){
			printf( "	subl	#" );
			printf( CONFMT, p[1].sval );
			printf( ",d0\n" );
			}

		/* note that this is a cl; it thus checks
		   for numbers below range as well as out of range.
		   */
		printf( "	cmpl	#%ld,d0\n", range );
		printf( "	bhi	.L%d\n", dlab );
		printf( "	addw	d0,d0\n" );

		printf( "	movw	pc@(6,d0:w),d0\n" );
		printf( "	jmp	pc@(2,d0:w)\n" );
		printf( ".L%d:\n", swlab=getlab() ); /* output table */

		/* output table */
		for( i=1,j=p[1].sval; i<=n; ++j ){

			printf( "	.word	.L%d-.L%d\n", ( j == p[i].sval )
				? p[i++].slab : dlab, swlab );

			}
		printf("	.text\n");

		if( p->slab< 0 ) deflab( dlab );
		return;

		}

	genbinary(p,1,n,0);
}

genbinary(p,lo,hi,lab)
  register struct sw *p;
  {	register int i,lab1;

	if (lab) printf(".L%d:",lab);	/* print label, if any */

	if (hi-lo > 4) {		/* if lots more, do another level */
	  i = lo + ((hi-lo)>>1);	/* index at which we'll break this time */
	  printf( "	cmpl	#" );
	  printf( CONFMT, p[i].sval );
	  printf( ",d0\n	beq	.L%d\n", p[i].slab );
	  printf( "	bgt	.L%d\n", lab1=getlab() );
	  genbinary(p,lo,i-1,0);
	  genbinary(p,i+1,hi,lab1);
	} else {			/* simple switch code for remaining cases */
	  for( i=lo; i<=hi; ++i ) {
	    printf( "	cmpl	#" );
	    printf( CONFMT, p[i].sval );
	    printf( ",d0\n	beq	.L%d\n", p[i].slab );
	  }
	  if( p->slab>=0 ) branch( p->slab );
	}
}
