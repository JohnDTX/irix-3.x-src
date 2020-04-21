# include "mfile2.h"	/* includes macdefs.h mac2defs.h manifest.h */
#ifdef SGI_REGS
#include "regprefs.h"
# include  "fpregs.h"
# include "skyopcodes.h"
# include "fpaopcodes.h"
extern int fpatype;
extern int fltftn;
extern int isJuniper;
int suspend_regvars=0;
int tempcc=0;
#endif

/* a lot of the machine dependent parts of the second pass */
/* Incorporated Terman fix to rmove - V. Pratt 2/17/82 */

/* GB SGI UNI speedups */

# define BITMASK(n) ((1L<<n)-1)

lineid( l, fn ) char *fn; {
	/* identify line l and file fn */
	printf( "| line %d, file %s\n", l, fn );
	}

int usedregs;	/* flag word for registers used in current routine */
int maxtoff = 0;

cntbits(i)
  register int i;
  {	register int j,ans;

	for (ans=0, j=0; i!=0 && j<16; j++) { if (i&1) ans++; i>>= 1; }
	return(ans);
}

char *getftnnm();

eobl2(){
	extern int retlab;
	OFFSZ spoff;	/* offset from stack pointer */

	spoff = maxoff;
	spoff /= SZCHAR;
	SETOFF(spoff,2);
	usedregs &= 036374;	/* only save regs used for reg vars */
	spoff += 4*cntbits(usedregs);	/* save at base of stack frame */
	/* GB (SGI) - c2 assumes that labels are on their own statements */
#ifdef NOTDEF
	printf( ".L%d:", retlab);
	if (usedregs) printf( "	moveml	a6@(-.F%d),#0x%x\n", ftnno, usedregs );
	printf( "	unlk	a6\n" );
#endif
	printf( ".L%d:\n", retlab);
#ifdef SGI_REGS
	if ((fpatype == JUNIPER_FPA)&&(fltftn)) {
		if (fltftn == 2)
			printf("\tmovl\td1,0x%x:w\n",FPA_DMOVELSL+0x10);
		printf("\tmovl\td0,0x%x:w\n",
			(fltftn==2)?FPA_DMOVEMSL:FPA_SMOVE);
	}
#endif
	if (usedregs) printf( "\tmoveml	a6@(-.F%d),#0x%x\n", ftnno, usedregs );
	printf( "	unlk	a6\n" );

	printf( "	rts\n" );
	printf( ".F%d = %d\n", ftnno, spoff );
	printf( ".S%d = 0x%x\n", ftnno, usedregs );
/***** GB - check for too many locals *****/
	if (maxtoff > (32767-132) ) 
		werror(
		"%d bytes too much stack space used for arguments in function %s",
				maxtoff - (32767-132), getftnnm());
	if (spoff > 32767)
		werror("%d bytes too many locals in function %s",(spoff - 32767),getftnnm());
/******	GB SGI #42 UNI ********/
#ifdef NATIVE
	printf( ".M%d = %d\t\t| %d + 132\n", ftnno, maxtoff+132, maxtoff );
#else
	printf( "| M%d = %d\n", ftnno, maxtoff+132 );
#endif
	maxtoff = 0;
	printf( "| end\n" );
	if( fltused ) {
		fltused = 0;
		printf( "	.globl	fltused\n");
		}
	}

struct hoptab { int opmask; char * opstring; } ioptab[]= {

	ASG PLUS, "add",
	ASG MINUS, "sub",
	ASG OR,	"or",
	ASG AND, "and",
	ASG ER,	"eor",
	ASG MUL, "mul",
	ASG DIV, "div",
	ASG MOD, "div",
	ASG LS,	"sl",
	ASG RS,	"sr",

	-1, ""    };

hopcode( f, o ){
	/* output the appropriate string from the above table */

	register struct hoptab *q;

	for( q = ioptab;  q->opmask>=0; ++q ){
		if( q->opmask == o ){
			printf( "%s", q->opstring );
			if( f == 'F' ) printf( "f" );
			return;
			}
		}
	cerror( "no hoptab for %s", opst[o] );
	}

char *
rnames[]= {  /* keyed to register number tokens */

	"d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7",
	"a0", "a1", "a2", "a3", "a4", "a5", "a6", "sp"
	};

int rstatus[] = {
/*	SAREG, SAREG,		/* keep for functions results GB (SGI) */

	SAREG|STAREG, SAREG|STAREG,
	SAREG|STAREG, SAREG|STAREG,
	SAREG|STAREG, SAREG|STAREG,
	SAREG|STAREG, SAREG|STAREG,

	SBREG|STBREG, SBREG|STBREG,
	SBREG|STBREG, SBREG|STBREG,
	SBREG|STBREG, SBREG|STBREG,
	SBREG,	      SBREG,
	};

NODE *brnode;
int brcase;

int toff = 0; /* number of stack locations used for args */

int l_pending=0,r_pending=0;
#ifdef SGI_REGS
char *zzzcode();

char *
pusharg(p,cptr) NODE *p; register char *cptr; {

	/* the node p is an argument to a function.  If it is a special
	   SGI node, do special processing. */
	register NODE *pc;
	int special = 0;
	int op = p->in.op;
	int regno;
	int addlong=0,onstack=0;
	char type,child,suf;

	type = *cptr++;
	child = *cptr;

	if ((child != 'L')&&(child != 'R') && ((child < '1')||(child > '3')))
	    cerror("illegal pusharg");
	pc = getlr(p,child);
	if ((child == 'R') || (child == 'L')) 
	    special = (pc->in.node_sgi & SGIARG);

	if (rdebug) 
	    printf("pusharg called for %o, op %o, sgi %x (%.4x), str %s\n",
			    pc,op,pc->in.node_sgi,pc->in.hw_opcode,(cptr-1));

	/* if this is a hardware node, we need to do some special muck... */
	if ((special) && (pc->in.node_sgi & HWOP)&&(fpatype != NO_FPA)) {

		/* if this is a double operation, reverse the order of 
		   writing the longwords to the board on sky moves.
		*/
		if ((fpatype == SKY_FPA)&&(pc->in.node_sgi & DOUBLEOP)&&
				(pc->in.type == DOUBLE))
			type = (type == 'U')?'A':'U';

		/* If this is the operand that the opcode is to be attached to
		   and it is NOT the upper longword, the opcode needs to be moved
		   to the Sky board.  Otherwise, we need to set the opcode
		   for the move on the Juniper fpa.
		*/
		if (fpatype == SKY_FPA) {
			if ((pc->in.node_sgi & WRITE_HWOP)&&(type != 'U'))
				printf("movw\t#0x%x,0x%x\n\t",pc->in.hw_opcode,SKYCOMREG);
		}
		else {
			/* if this is the move of an Upper longword, alter the
			   field of the opcode.
			*/
			if (!(pc->in.node_sgi & WRITE_HWOP)) {
				/* this is the first operand.  Both operands should be zero.
				*/
				pc->in.hw_opcode &= ~FPA_OPERANDS_MASK;
				if (type == 'U') {
					/* Simple write to the upper part of
					   register one.
					*/
					pc->in.hw_opcode = (FPA_DMOVELSL|(1<<4));
				}
				else {
					/* Simple write to the lower part of
					   register one.
					*/
					if (pc->in.type == DOUBLE)
						pc->in.hw_opcode = (FPA_DMOVEMSL|(1<<4));
					else 
						pc->in.hw_opcode = (FPA_SMOVE|(1<<4));
				}
			}
		}
	}
	/* 	register variables are suspended if d0/d1 are not available
		at the time of the call. */
	if (suspend_regvars) special = 0;
	/*	is the operand already in the correct place (perhaps in d0
		as a result of a function call) ? */
	if ((special) && (pc->in.node_sgi & PUSHED)) return(cptr);
	switch (type) {

		case '0':	/* the argument is zero */
					printf("movl\t#0");
					break;

	    case 'W':   suf = 'w';
					goto move;
	    case 'A':	
	A:			suf = 'l';
				if (!special) 
			    	if (op == ICON) {
			    		printf("pea\t");
					onstack++;
			    		acon(pc);
			    		break;
			    	}
	move:		printf("mov%c\t",suf);
				if (adrput(pc)) 
					if (child == 'L') l_pending++;
						else r_pending++;
	    		break;

	    case 'U':
				printf("movl\t");
				upput(pc);
				addlong++;
				break;

	    case 'M':
				/* mask before pushing */
				printf("moveq	#0");
				printf(",%s",
					rnames[regno = getlr(p,*++cptr)->tn.rval]);
				printf("\n\tmov");
				suffix(pc->in.type);
				printf("\t");
				adrput(pc);
				printf(",%s", rnames[regno]);
				goto  common_small;
			
	    case 'X':
				/*extend before pushing */
				if (op == ICON)  {
			    	cptr++;
			    	goto A;
				}
				if (p->in.type == INT) return(cptr);
				printf("mov");
				suffix(pc->in.type);
				printf("\t");
				adrput(pc);
				if (special) {
			    	printf(",%s",rnames[regno = (pc->in.node_sgi & 0xff)]);
			    	cptr++;
				} else {
			    	/* temp 1 better be a register ! */
			    	printf(",%s",
				   	rnames[regno = getlr(p,*++cptr)->tn.rval]);
				}
	
				if ((p->in.type == CHAR)||(p->in.type == UCHAR))
			    	printf("\n\textw\t%s",rnames[regno]);
				printf("\n\textl\t%s",rnames[regno]);
   common_small:
				if (!special) {
			    	printf("\n\tmovl\t%s,sp@-",rnames[regno]);
			    	zzzcode(p,"ZP");
				}
				else if ((pc->in.node_sgi & HWOP)&&(fpatype != NO_FPA))  {
					if (fpatype == SKY_FPA)
						printf("\n\tmovl\t%s,0x%x",rnames[regno],SKYDTREG);
					else 
						printf("\n\tmovl\t%s,0x%x:w",rnames[regno],
								(unsigned short)pc->in.hw_opcode);
				}
				return(cptr);
	    default:
	    bad:
				cerror("illegal pusharg");
		}
	if (special) {
		if ((pc->in.node_sgi & HWOP)&&(fpatype != NO_FPA)) {
			if (fpatype == SKY_FPA) 
				printf(",0x%x",SKYDTREG);
			else {
				/* Juniper FPA. */
				if ((pc->in.node_sgi & WRITE_HWOP)) {
					/* this is the second operand.  
					*/
					if (type == 'U') {
						/* Simple write to the upper part of
					   	register zero.
						*/
						printf(",0x%x:w",(unsigned short)(FPA_DMOVELSL));
						return(cptr);
					}
				}
				printf(",0x%x:w",(unsigned short)pc->in.hw_opcode);
			}
		}
	    else 
			printf(",%s",rnames[(pc->in.node_sgi & 0xff)+addlong]);
	} else {
	    if (!onstack) printf(",sp@-");
	    zzzcode(p,"ZP");
	}
	return(cptr);
}

#endif

#ifdef SGI_FIELDS
TWORD reffield();
#endif
static oldsp;
static unsigned short callregmask_inc,callregmask_dec;

static unsigned node_sgi_save, hw_opcode_save;

char *
zzzcode( p, c ) NODE *p; 
	register char *c; {
	register m,temp;
	switch( *++c ){

	case 'C':
#ifdef SGI_REGS
		if (((p->in.node_sgi & HWNODE)==HWNODE)&&(fpatype)) {
			/* the operation is complete unless the poll bit is set
				in the node */
			if ((p->in.node_sgi & HWPOLL)&&(fpatype == SKY_FPA)) {
				/* generate a polling loop */
				int lab;
				lab = getlab();
				deflab(lab);
				printf("\ttstw\t0x%x\n",SKYSTATREG);
				printf("\tbge \t.L%d\n",lab);
			}
			if (!(p->in.node_sgi & HWSTACKRESULT)) {
				/* and move the result to d0/d1 or the CC, as appropriate */
				if (p->in.node_sgi & HWCCRESULT) {
					/* result is condition codes */
					if (fpatype == SKY_FPA)
						printf("\tmovw\t0x%x,cc\n",SKYDTREG);
					else
						printf("\ttstb\t0x%x:w\n",FPA_CCR);
				} 
				else {
					/* result is real.  Put it in d0 or d0/d1 */
					if (fpatype == SKY_FPA) {
						printf("\tmovl\t0x%x,d0\n",SKYDTREG);
						if (p->in.type == DOUBLE) 
							printf("\tmovl\t0x%x,d1\n",SKYDTREG);
					}
					else {
						/* Juniper FPA */
						if (p->in.type == DOUBLE) {
							printf("\tmovl\t0x%x:w,d0\n",FPA_DMOVEMSL+(0xf<<4));
							printf("\tmovl\t0x%x:w,d1\n",FPA_DMOVELSL+(0xf<<4));
						}
						else 
							printf("\tmovl\t0x%x:w,d0\n",FPA_SMOVE+(0xf<<4));
					}
				}
			}
			break;
		}
#endif
		switch (p->in.left->in.op) {
		  case ICON:	printf("\tjbsr\t");
				if (p->in.op == UNARY FORTCALL)
					facon(p->in.left);
				else
					acon(p->in.left);
				printf("\n");
				return(c);

		  case REG:	printf("\tjbsr\t");
				adrput(p->in.left);
				printf("@\n");
				return(c);

		  case NAME:
		  case OREG:	printf("\tmovl\t");
				adrput(p->in.left);
				printf(",a0\n\tjbsr\ta0@\n");
				return(c);

		  default:	cerror("bad subroutine name");
		}

	case 'B':	/* output b if type is byte */
		suffix(p->in.type);
		return(c);

	case 'L':	/* this is a logical op in which we want the 
			   suffix of the left child. */
		suffix(p->in.left->in.type);
		return(c);


	case 'N':  /* logical ops, turned into 0-1 */
		/* use register given by register 1 */
		cbgen( 0, m=getlab(), 'I' );
		deflab( p->bn.label );
		printf( "	clrl	%s\n", rnames[temp = getlr( p, '1' )->tn.rval] );
		usedregs |= 1<<temp;
		deflab( m );
		p->in.type = INT;
		return(c);

	case 'I':
		cbgen( p->in.op, p->bn.label, c );
		return(c);

#ifdef SGI_REGS

	case 'Q':
		/* lea or pea, depending on whether this is
		   a special SGI arg or not */
		if (p->in.node_sgi & SGIARG) {
			if ((p->in.node_sgi & HWOP)&&(fpatype != NO_FPA)) printf("movl");
			else printf("lea");
		}
		else printf("pea");
		return(c);

	case 'R':
		/* put out the register for the last lea if
		   this is a special SGI node */
		if (p->in.node_sgi & SGIARG)  {
			if (p->in.node_sgi & HWOP) {
				if (fpatype == SKY_FPA) 
					printf(",0x%x",SKYDTREG);
				else {
					/* if this is the last arg, we have to
					   add the opcode.
					*/
					if (p->in.node_sgi & WRITE_HWOP)
						printf(",0x%x:w",(unsigned short)p->in.hw_opcode);
					else
						printf(",0x%x:w",(FPA_SMOVE + (1<<4)));
				}
			}
		    else printf(",%s",rnames[p->in.node_sgi & 0xff]);
		}
		return(c);

	case 'F':
		/* this node should be marked as a special SGI node,
		   and its two children marked for SGI args */
		node_sgi_save = 0;
		if (((p->in.type == DOUBLE)||
			((p->in.left->in.type==DOUBLE)&&(p->in.right->in.type==DOUBLE)))
			&&(fpatype == NO_FPA)) return(c);
		/* SCRXXXX -
		   This node may already be targeted as an sgi node.  Save
		   the special info here and restore it after the table call
		   (in the code for Zc). 
		*/
		if (edebug) printf("...saving sgi node info: (%.8x, %.4x)\n",
			p->in.node_sgi,p->in.hw_opcode);
		node_sgi_save = p->in.node_sgi;
		hw_opcode_save = p->in.hw_opcode;
		if (fpatype != NO_FPA) {
			p->in.node_sgi = SGI_NODE|HWNODE;
			p->in.left->in.node_sgi = SGIARG|D0|HWOP;
			p->in.right->in.node_sgi = SGIARG|D1|HWOP;
		} else {
			p->in.node_sgi = SGI_NODE;
			p->in.left->in.node_sgi = SGIARG|D0;
			p->in.right->in.node_sgi = SGIARG|D1;
			if ((p->in.left->in.op == REG) &&
				(p->in.left->tn.rval == D0)) p->in.left->in.node_sgi |= PUSHED;
			if ((p->in.right->in.op == REG) &&
				(p->in.right->tn.rval == D1)) p->in.right->in.node_sgi |= PUSHED;
		}
		return(c);

	case '-':
		/* pass the indicated part of the node as an argument
		   to a function. */
		c = pusharg(p,++c);
		return(c);
#else
	case '-':
		printf( "sp@-" );
#endif
	case 'P':
#ifdef SGI_REGS
		if (!(p->in.node_sgi & SGIARG)) {
#endif
		    toff += 4;
		    if (toff > maxtoff) maxtoff = toff;
#ifdef SGI_REGS
		}
#endif
		return(c);
#ifndef SGI_REGS
	case '0':
		toff = 0; return(c);
#endif
	case '~':
		/* complimented CR */
		p->in.right->tn.lval = ~p->in.right->tn.lval;
		conput( getlr( p, 'R' ) );
		p->in.right->tn.lval = ~p->in.right->tn.lval;
		return(c);

	case 'M':
		/* negated CR */
		p->in.right->tn.lval = -p->in.right->tn.lval;
	case 'O':
		conput( getlr( p, 'R' ) );
		p->in.right->tn.lval = -p->in.right->tn.lval;
		return(c);

	case 'T':
		/* Truncate longs for type conversions:
		    INT|UNSIGNED -> CHAR|UCHAR|SHORT|USHORT
		   increment offset to second word */

		m = p->in.type;
		p = p->in.left;
		switch( p->in.op ){
		case NAME:
		case OREG:
			if (p->in.type==SHORT || p->in.type==USHORT)
			  p->tn.lval += (m==CHAR || m==UCHAR) ? 1 : 0;
			else p->tn.lval += (m==CHAR || m==UCHAR) ? 3 : 2;
			return(c);
		case REG:
			return(c);
		default:
			cerror( "Illegal ZT type conversion" );
			return(c);

			}

	case 'U':
		cerror( "Illegal ZU" );
		/* NO RETURN */

	case 'W':	/* structure size */
		if( p->in.op == STASG )
			printf( "%d", p->stn.stsize);
		else	cerror( "Not a structure" );
		return(c);

	case 'S':  /* structure assignment */
		{
			register NODE *l, *r;
			register size, i;

			if( p->in.op == STASG ){
				l = p->in.left;
				r = p->in.right;
				}
			else if( p->in.op == STARG ){  /* store an arg onto the stack */
				r = p->in.left;
				}
			else cerror( "STASG bad" );

			if( r->in.op == ICON ) r->in.op = NAME;
			else if( r->in.op == REG ) r->in.op = OREG;
			else if( r->in.op != OREG ) cerror( "STASG-r" );

			size = p->stn.stsize;

			r->tn.lval += size;
			l->tn.lval += size;

			while( size ){ /* simple load/store loop */
				i = (size > 2) ? 4 : 2;
				r->tn.lval -= i;
				expand( r, FOREFF,(i==2)?"\tmovw\tAR,":"\tmovl\tAR," );
				l->tn.lval -= i;
				expand( l, FOREFF, "AR\n" );
				size -= i;
			}

			if( r->in.op == NAME ) r->in.op = ICON;
			else if( r->in.op == OREG ) r->in.op = REG;

			}
		break;

#ifdef SGI_FIELDS
	case 'X':
		/* reference FORCC */
		/* 	simplefld tries to do a simple test - either a
			bit, byte, word, or long test */
		if (!simplefld(p)) {
			/*	not a simple (aligned) field */
			/* 	isolate the field in a data register */
			TWORD ty;
			ty = reffield(p);
			/* 	then test the data register */
			printf("\ttst");
			suffix(ty);
			printf("\t");
			adrput(getlr(p,'1'));
			printf("\n");
		}
		break;

	case 'Y':
		reffield(p);
		break;

	case 'Z':
		setfield(p);
		break;
#endif
	case 'a':
		/* autoincrement or decrement the double precision STARREG 
		   which has just been pushed. */
		{
		    register NODE *tmp;
			if (l_pending) {
				tmp = getlr( p , 'L');
				printf("\t%sql\t#8,",(tmp->in.left->in.op == INCR)?"add":"sub");
				printf( "%s\n", rnames[tmp->in.left->in.left->tn.rval] );
				l_pending = 0;
			} 
			if (r_pending) {
				tmp = getlr( p , 'R');
				printf("\t%sql\t#8,",(tmp->in.left->in.op == INCR)?"add":"sub");
				printf( "%s\n", rnames[tmp->in.left->in.left->tn.rval] );
				r_pending = 0;
			} 
			break;
		}

	case 'b':
	 	/* begin keeping stack offset for table call */
		oldsp = toff;
		break;

#ifdef SGI_REGS
	case 'c':
		/* call the fp compare routine if the compare is done is s/w*/
		/* OR output the opcode with the move, if the compare
		   is done is HW on the JUNIPER FPA
		*/
		if (fpatype == NO_FPA){
			printf("\tjbsr\t%s\n",
					p->in.right->in.type==DOUBLE?"_d_cmp":
					suspend_regvars?"_f_cmp":"_fr_cmp");
			suspend_regvars = 0;
		}
		else if (fpatype == SKY_FPA) {
			/* get the condition codes and store them */
			if (((p->in.node_sgi & (HWCCRESULT|HWSTACKRESULT))==HWCCRESULT)) {
				printf("\tmovw\t0x%x,cc\n",SKYDTREG);
			} 
		}
		else {
			/* Juniper FPA.  Load the condition codes from the
			   FPA CC register.
			*/
			printf("\ttstb\t0x%x:w\n",FPA_CCR);
		}
		if (node_sgi_save) {
			if (edebug) printf(
				"...restoring sgi node info: (%.8x, %.4x)\n",
				node_sgi_save,hw_opcode_save);
			p->in.node_sgi = node_sgi_save;
			p->in.hw_opcode = hw_opcode_save;
		}
		break;
#endif

	case 'e':
		/* clean up stack after table call */
		popargs(toff - oldsp);
#ifdef SGI_REGS
	case 'f':
		restore_volregs(p,callregmask_inc);
		callregmask_inc = 0;
#endif
		break;

#ifdef SGI_REGS
	case 'x':
		/* if this is a h/w floating point compare, generate the opcode */
		if (fpatype != NO_FPA) {
			NODE *tmp;
			if (fpatype == SKY_FPA)
				printf("\tmovw\t#0x%x,0x%x\n",
					p->in.right->in.type==DOUBLE?SKY_DPCMP:SKY_SPCMP,SKYCOMREG);
			else
				p->in.hw_opcode = (p->in.right->in.type==DOUBLE)?
													FPA_DPCMP:FPA_SPCMP;

			if (p->in.right->in.type==DOUBLE) {
				/* put the DOUBLE spec in the argument nodes */
				p->in.right->in.node_sgi |= DOUBLEOP;
				p->in.left->in.node_sgi |= DOUBLEOP;
			}
			/* reverse the arguments */
			tmp = p->in.right;
			p->in.right = p->in.left;
			p->in.left = tmp;
			p->in.node_sgi |= HWCCRESULT;
			if (fpatype == JUNIPER_FPA) {
				p->in.left->in.node_sgi |= WRITE_HWOP;
				p->in.left->in.hw_opcode = p->in.hw_opcode;
			}
		}
		else if (p->in.right->in.type == FLOAT) {
			if (busy[0]) { 
				if (!(p->in.left->in.node_sgi & PUSHED))  {
					suspend_regvars = 1;
					if (((p->in.right->in.op != REG)||
					     (p->in.right->tn.rval != D0))&&
					    ((p->in.left->in.op != REG)||
					     (p->in.left->tn.rval != D0))) {
						callregmask_inc |= (1<<D0);
						callregmask_dec |= (0x08000 >> D0);
						p->in.node_sgi |= SAVE_D0;
						}
					rfree(D0);
				}
			}
			if (busy[1]) { 
				if (!(p->in.right->in.node_sgi & PUSHED)) {
					suspend_regvars = 1;
					if (((p->in.right->in.op != REG)||
				     	(p->in.right->tn.rval != D1))&&
				    	((p->in.left->in.op != REG)||
				     	(p->in.left->tn.rval != D1))) {
						callregmask_inc |= (1<<D1);
						callregmask_dec |= (0x08000 >> D1);
						p->in.node_sgi |= SAVE_D1;
					}
					rfree(D1);
				}
			}
			if (busy[A0]) {
				if (!(p->in.right->in.node_sgi & PUSHED)) {
					callregmask_inc |= (1<<A1);
					callregmask_dec |= (0x08000 >> A1);
					p->in.node_sgi |= SAVE_A0;
					rfree(A0);
				}
			}
			if (p->in.node_sgi & SAVE_REGS) {
				if (tempcc == 0) {
					/* allocate a temp word for
				   	the CC to be saved/restored.
					*/
					tempcc = freetemp(1);
				}
				p->in.node_sgi |= NEED_CC;
			}
		}
		/* SCR1821 */
		if (callregmask_dec) {
			printf("\tmoveml\t#0x%x,sp@-\n",callregmask_dec);
			callregmask_dec = 0;
		}
		break;
#endif

	default:
		cerror( "illegal zzzcode" );
		}
	return(c);
	}


#ifdef SGI_REGS
restore_volregs(p,mask) register NODE *p;
unsigned short mask;
{
		if (p->in.node_sgi & SAVE_REGS) {
			/* if we are expecting the CC to be valid
			   after the call, we need to save it... */
			if (p->in.node_sgi & NEED_CC) {
				/* save away the CC */
				printf("\tmovw\tcc,a6@(%d)\n",tempcc);
			}
			if (p->in.node_sgi & SAVE_A0) {
				/* restore A0 */
				if (busy[A0])
					cerror("arg register a0 busy at restoration");
				rbusy(A0,INT);
			}
			if (p->in.node_sgi & SAVE_D1) {
				/* restore D1 */
				if (busy[D1])
					cerror("arg register d1 busy at restoration");
				rbusy(D1,INT);
			}
			if (p->in.node_sgi & SAVE_D0) {
				/* restore D0 */
				if (busy[D0])
					cerror("arg register d0 busy at restoration");
				rbusy(D0,INT);
			}
		}
		if (mask)
			printf("\tmoveml\tsp@+,#0x%x\n",mask);
		if (p->in.node_sgi & SAVE_REGS) {

			if (p->in.node_sgi & NEED_CC) {
				/* restore the CC */
				printf("\tmovw\ta6@(%d),cc\n",tempcc);
			}
			p->in.node_sgi &= ~SAVE_REGS;
		}
}
#endif
#ifdef SGI_FIELDS

extern int fldtyp,fldsz,fldshf;

TWORD 
reffield(p) register NODE *p;
{
	/*	reffield assumes that the destination (RESULT 1) is an A-type
		(data) register. */

	register NODE *source,*dest;
	int setlab,endlab,i;
	unsigned long mask,compl;
	int isfldop,o;
	char suf;

	if (xdebug)  {
		printf("reffield: fldshf = 0x%x, fldsz= 0x%x, fldtyp = ",
				fldshf,fldsz);
		tprint(fldtyp);
		printf(" (0x%x)\n", fldtyp);
	}
	/* result of a reffield is an INT 
	p->in.type = INT;
	*/

	source = getlr(p,'R');
	if (source->in.op != FLD) 
		cerror("reffield called with other than field operator");

	/* 	get the target address */
	source = getlr(source,'L');
	isfldop =  (((o = source->in.op) == OREG)||(o == NAME) || (o == REG));
	if (!isfldop)
		cerror("unexpected field address in reffield");

	dest = getlr(p,'1');


	if (fldsz == 1) {
		/* single bit field.  Set the dest to 0/1. */

		if ((source->in.op != REG)&&(fldtyp == SZSHORT)) {
			/*	increment to correct byte */
			/* 	this is only necessary if the field type is short. */

			/* GB BUGXX AAAKKK!  replaced following...
			if (fldshf<8) {
				source->tn.lval++;
				fldshf -= SZCHAR;
			}
				*/
			/* with this.... */
			if (fldshf<8) {
				source->tn.lval++;
			}
			else fldshf -= SZCHAR;

		}

		printf("\tbtst\t#0x%x,",fldshf);
		adrput(source);
		printf("\n");
		setlab = getlab();
		endlab = getlab();
		printf("\tjne\t.L%d\n",setlab);
		printf("\tclrl\t");
		adrput(dest);
		printf("\n\tjra\t.L%d\n",endlab);
		deflab(setlab);
		printf("\tmoveq\t#1,");
		adrput(dest);
		printf("\n");
		deflab(endlab);
		return(CHAR);
	}

	/* field is not single bit.  Is it a byte,long or word which is aligned? */
	/* if the field is in a register, and the shift is nonzero, we cant
		do this optimization! */
	if (!((source->in.op == REG)&&(fldshf))) {
		if (((!(fldshf % 8)) && (fldsz == SZCHAR) ) ||
			((!(fldshf % 16)) && ((fldsz == SZSHORT) || (fldsz == SZLONG))))
		{	
			/* 	move to byte alignment if quantity is byte */
			if ((fldsz == SZCHAR)&&(fldtyp != SZCHAR)) {
				if (fldtyp != SZSHORT) 
					cerror("inconsistent type in byte-aligned field ref");
				if (!fldshf) {
					source->tn.lval++;
					fldshf -= SZCHAR;
				}
			}
			/* and simply store the result */
			if (fldsz != SZLONG) {
				printf("\tclrl\t");
				adrput(dest);
				printf("\n");
			}
			printf("\tmov%c\t",(fldsz==SZCHAR)?'b':(fldsz==SZSHORT)?'w':'l');
			adrput(source);
			printf(",");
			adrput(dest);
			printf("\n");
			return((fldsz == SZCHAR)?CHAR:(fldsz == SZSHORT)?SHORT:LONG);
		}
	}

	/*  field is really a nasty little object.  We need to 
		load the quantity containing the source, mask and
		shift it . 
	/*

	/* 	adjust to the next btye boundary if the field is smaller than
		a byte and is entirely in the most signif byte */
	
#ifdef NOTDEF
	if (fldtyp == SZLONG) 
		if ((fldsz + fldshf)<SZSHORT) {
			source->tn.lval += 2; 
			fldtyp = SZSHORT;
		}
#endif
	if (fldtyp == SZSHORT)  {
	 	if ((fldsz + fldshf) < SZCHAR) {
			source->tn.lval++;
			fldtyp = SZCHAR;
		}
#ifdef NOTDEF
		else if ((fldsz < SZCHAR) && (fldshf >= SZCHAR)) {
			fldtyp = SZCHAR;
			fldshf -= SZCHAR;
		}
#endif
	}
	/* 	mask has all the bits of the field ON, compl has them OFF */
	mask = 0;
	for (i=0;i<fldsz;i++) mask = (mask<<1)+1;
	mask <<= fldshf;
	compl = ~mask;
	
	suf = (fldtyp==SZLONG)?'l':(fldtyp==SZSHORT)?'w':'b';

#ifdef NOTDEF
	/* 	now clear the destination */
	if (fldtyp != SZLONG) {
		printf("\tclrl\t");
		adrput(dest);
		printf("\n");
	}
#endif

	/*	move a copy of the quantity containing the field */
	printf("\tmov%c\t",suf);
	adrput(source);
	printf(",");
	adrput(dest);
	
	/* 	mask it */
	printf("\n\tandl\t#0x%x,",mask);
	adrput(dest);

	/* and shift it */
	while (fldshf > 0) {
		printf("\n\tlsrl\t#0x%x,",(fldshf>8)?8:fldshf);
		adrput(dest);
		fldshf -= 8;
	}
	printf("\n");
	return((fldsz == SZCHAR)?CHAR:(fldsz == SZSHORT)?SHORT:LONG);
}


simplefld(p) register NODE *p;
{

	register NODE *tgt;
	NODE *tmp;
	int setlab,endlab,i;
	unsigned long mask,compl;
	char suf;
	int isfldop,o;

	if (xdebug) 
		printf("simplefield: fldshf = 0x%x, fldsz= 0x%x, flstyp = 0x%x\n",
				fldshf,fldsz,fldtyp);
	tgt = getlr(p,'R');
	if (tgt->in.op != FLD) 
		cerror("simplefield called with other than field operator");

	/* 	get the target address */
	tgt = getlr(tgt,'L');

	isfldop =  (((o = tgt->in.op) == OREG)||(o == NAME) || (o == REG));
	if (!isfldop)
		cerror("unexpected field address in simplefield");


	if (fldsz == 1) {
		/* single bit field.  Test the ls bit of the value */

		if ((tgt->in.op != REG)) {
			/*	increment to test the lsb of the correct byte */
			tgt->tn.lval += nbytes(tgt->in.type)-1;
		}

		printf("\tbtst\t#0x%x,",fldshf);
		adrput(tgt);
		printf("\n");

		return(1);
	}

	/* field is not single bit.  Is it a byte,long or word which is aligned? */
	/* if the field is in a register, and the shift is nonzero, we cant
		do this optimization! */
	if ((tgt->in.op != REG)||(!fldshf)) {
		if (((!(fldshf % 8)) && (fldsz == SZCHAR) ) ||
			((!(fldshf % 16)) && ((fldsz == SZSHORT) || (fldsz == SZLONG))))
		{	
			/* 	move to byte alignment if quantity is byte */
			if ((fldsz == SZCHAR)&&(fldtyp != SZCHAR)) {
				if (fldtyp != SZSHORT) 
					cerror("inconsistent field type in byte field");
				tgt->tn.lval++;
				fldshf -= SZCHAR;
			}

			/* and simply test the result */
			printf("\ttst%c\t",(fldsz==SZCHAR)?'b':(fldsz==SZSHORT)?'w':'l');
			adrput(tgt);
			printf("\n");
			return(1);
		}
	}


	return(0);
}


setfield(p) register NODE *p;
{
	/* 	setfield is called when an SFLD (left), EA(right) tree is found
		for an ASSIGN type op.  Two type A (data) registers 
		have been allocated for
		the operation.  Certain field parameters have already been set
		up for us by tshape().  They correspond to the following quantities:

								 fldsz	  fldshf
								<----><---------->
					______________________________
					|			XXXXXX			 |
					------------------------------
					< --------- fldty ---------- >

		Here, fldty is SZCHAR, SZSHORT or SZLONG.  fields are not allowed
		to cross an INT boundary.  If they do, they are aligned on the
		following INT boundary.  Fields are inherantly unsigned.

		setfield attempts to optimize single-bit and byte-aligned fields.

	*/

	register NODE *r,*l;
	NODE *tmp;
	int setlab,endlab,i;
	unsigned long mask,compl;
	char suf;
	int isfldop,o;
	if (xdebug) 
		printf("setfield: fldshf = 0x%x, fldsz= 0x%x, fldtyp = 0x%x\n",
				fldshf,fldsz,fldtyp);
	r = getlr(p,'R');
	l = getlr(p,'L');
	if (l->in.op != FLD) 
		cerror("setfield called with illegal lhs of field operator");

	/* 	get the target address */
	l = getlr(l,'L');

	isfldop =  (((o = l->in.op) == OREG)||(o == NAME) || (o == REG));
	if (!isfldop)
		cerror("unexpected field address in setfield");

	if (r->in.op == REG)
		/*  if the source is a register, the data will
			always be in the ls portion.  Just force 
			type matching
		*/
		r->in.type = l->in.type;

	if (fldsz == 1) {
		/* single bit field.  Text the ls bit of the value to
		   determine whether to use a bset or bclr. */

		if ((l->in.op != REG)&&(fldtyp != SZCHAR)) {
			/*	increment to correct byte */
			if (fldtyp == SZLONG)
				cerror("illegal long field type in single bit field set");
			/* GB BUGXX AAAKKK!  replaced following...
			if (fldshf<8) {
				l->tn.lval++;
				fldshf -= SZCHAR;
			}
				*/
			/* with this.... */
			if (fldshf<8) {
				l->tn.lval++;
			}
			else fldshf -= SZCHAR;
		}


		switch (r->in.op) {

			case ICON:	
						printf("\t%s\t",(r->tn.lval & 1)?"bset":"bclr");
						printf("#0x%x,", fldshf);
						adrput(l);
						printf("\n");
						break;

			case NAME:
			case OREG:
						r->tn.lval += nbytes(r->in.type)-1;
			case REG:
						printf("\tbtst\t#0,");
						adrput(r);
						printf("\n");
						setlab = getlab();
						endlab = getlab();
						printf("\tjne\t.L%d\n",setlab);
						printf("\tbclr\t#0x%x,", fldshf);
						adrput(l);
						printf("\n\tjra\t.L%d\n",endlab);
						deflab(setlab);
						printf("\tbset\t#0x%x,", fldshf);
						adrput(l);
						printf("\n");
						deflab(endlab);
						break;

			default:
						cerror("cannot decompose value for field assign");
			}

		return(0);
	}

	/* field is not single bit.  Is it a byte,long or word which is aligned? */
	/* if the field is in a register, and the shift is nonzero, we cant
		do this optimization! */
	if ((l->in.op != REG)||(!fldshf)) {
		if (((!(fldshf % 8)) && (fldsz == SZCHAR) ) ||
			((!(fldshf % 16)) && ((fldsz == SZSHORT) || (fldsz == SZLONG))))
		{	
			/* 	align correctly if quantity is byte */
			if (fldsz == SZCHAR) {
				/* source must be updated */
				/* move source to correct byte alignment */
				if ((i = nbytes(r->in.type)) == 4)
					cerror("illegal source type in byte field asmt");
				if (i==2) {
					if ((r->in.op == OREG)||(r->in.op == NAME)) r->tn.lval++;
					r->in.type = UCHAR;
				}
				if (fldtyp != SZCHAR) {
					/*  field is not aligned properly */
					if (fldtyp != SZSHORT)
						cerror("illegal field type in byte field asmt");
					l->tn.lval++;
					fldshf -= 8;
					fldtyp = SZCHAR;
				}
			} else if (fldsz == SZSHORT) {
				/* source better be a short! */
				if (nbytes(r->in.type) != 2)
					cerror("illegal source type in short field asmt");
			}
	
			/* adjust the source address if necessary */
#ifdef NOTDEF
			if ((r->in.op == OREG) || (r->in.op == NAME)) {
				if ((i = nbytes(r->in.type)) != fldsz/SZCHAR) {
					if (fldsz == SZCHAR) i = (i==4)?3:1;
					else 
						/* fldsz must be SZSHORT, and i must be 4 (long) */
						i = 2;
					r->tn.lval += i;
				}
			}
#endif
	
			/* and simply store the result */
			printf("\tmov%c\t",(fldsz==SZCHAR)?'b':(fldsz==SZSHORT)?'w':'l');
			adrput(r);
			printf(",");
			adrput(l);
			printf("\n");
			return(0);
		}
	}

	/*  field is really a nasty little object.  We need to clear the 
		destination field, mask and shift the source, and OR it in. */

	/* 	adjust to the next btye boundary if the field is smaller than
		a byte and is entirely in the most signif byte */
	
#ifdef NOTDEF
	if (fldtyp == SZLONG) 
		if ((fldsz + fldshf)<SZSHORT) {
			l->tn.lval += 2; 
			fldtyp = SZSHORT;
		}
#endif
	if (fldtyp == SZCHAR) {
		/*  may have to increment the source address */
		if (nbytes(r->in.type) == 2) {
			if ((r->in.op == OREG)||(r->in.op == NAME)) r->tn.lval++;
			r->in.type = UCHAR;
		}
	}
	if (fldtyp == SZSHORT)  {
	 	if ((fldsz + fldshf) < SZCHAR) {
			l->tn.lval++;
			if ((r->in.op == OREG)||(r->in.op == NAME)) r->tn.lval++;
			fldtyp = SZCHAR;
		}
#ifdef NOTDEF
		else if ((fldsz < SZCHAR) && (fldshf >= SZCHAR)) {
			fldtyp = SZCHAR;
			fldshf -= SZCHAR;
		}
#endif
	}

	/* 	mask has all the bits of the field ON, compl has them OFF */
	mask = 0;
	for (i=0;i<fldsz;i++) mask = (mask<<1)+1;
	mask <<= fldshf;
	compl = ~mask;
	
	suf = (fldtyp==SZLONG)?'l':(fldtyp==SZSHORT)?'w':'b';

	/* 	now clear the destination field */
	printf("\tand%c\t#0x%x,",suf,compl);
	adrput(l);
	printf("\n");

	/* 	prepare the source for or'ing in */
	if (r->in.op == ICON) {
		r->tn.lval <<= fldshf;
		r->tn.lval &= mask;
	} 
	else  {
		/* get the source in a temporary register */
#ifdef NOTDEF /* scr 0749 */
		if ((r->in.op != REG)||(!istreg(r->tn.rval))) 
#endif
		{
			tmp = getlr(p,'1');
			/* 	if we dont need the entire quantity, adjust to the
				correct portion and only retrieve it. */
			if (fldtyp != SZLONG) {
				tmp->in.type = (fldtyp == SZSHORT)?USHORT:UCHAR;
#ifdef NOTDEF
				r->tn.lval += (nbytes(r->in.type) - fldtyp/SZCHAR);
#endif
			}
			/*  added else clause for scr 1245 (GB) 11/6/85 */
			else 
				tmp->in.type = ULONG;

			printf("\tmov");
			suffix(tmp->in.type);
			printf("\t");
			adrput(r);
			printf(",");
			adrput(tmp);
			printf("\n");
			r=tmp;
		}

		/* 	now, if the shift is too large, put it in the
			second temp register */

		if (fldshf) {
			if (fldshf > 8) {
				tmp = getlr(p,'2');
				printf("\tmoveq\t#0x%x,",fldshf);
				adrput(tmp);
				printf("\n\tlsll\t");
				adrput(tmp);
				printf(",");
			}
			else 
				printf("\tlsl%c\t#0x%x,",suf,fldshf);
			adrput(r);
			printf("\n");
		}

		/* 	and mask off all but the pertinent bits */
		printf("\tand%c\t#0x%x,",suf,mask);
		adrput(r);
		printf("\n");
	}

	/* 	finally, OR the source to the destination */
	printf("\tor%c\t",suf);
	adrput(r);
	printf(",");
	adrput(l);
	printf("\n");
	return(0);
}

nbytes( t) TWORD t; {

	/* how many bytes occupy something of type t ? */
	if( ISPTR(t) ) t = DECREF(t);

	if( ISPTR(t) ) return( 4 );

	switch( t ){

	case UCHAR:
	case CHAR:
		return( 1 );

	case SHORT:
	case USHORT:
		return( 2 );

	case INT:
	case UNSIGNED:
	case FLOAT:
		return( 4 );

	case DOUBLE:
		return( 8 );
		}

	return( 0 );
	}

#endif /* SGI_FIELDS */


suffix (t ) TWORD t; {
		if( t == CHAR || t == UCHAR ) printf( "b" );
		else if( t == SHORT || t == USHORT ) printf( "w" );
		else printf( "l" );
		return;
	}



rmove( rt, rs, t ) TWORD t; {
	if (rt == rs) return;	/* dont move a reg to itself (GB REG) */
	if ( t == DOUBLE ) {
	  printf( "	movl	%s,%s\n", rnames[rs+1], rnames[rt+1] );
	  usedregs |= 1<<(rs+1);
	  usedregs |= 1<<(rt+1);
	}
	
	printf( "	movl	%s,%s\n", rnames[rs], rnames[rt] );
	usedregs |= 1<<rs;
	usedregs |= 1<<rt;
	}

struct respref
respref[] = {
	INTAREG|INTBREG,	INTAREG|INTBREG,
	INAREG|INBREG,	INAREG|INBREG|SOREG|STARREG|SNAME|STARNM|SCON,
	INTEMP,	INTEMP,
	FORARG,	FORARG,
	INTAREG,	SOREG|SNAME,
	0,	0 };

setregs(){ /* set up temporary registers */
	register i;
	int maxr;
	register int naregs = (((maxtreg>>8)&0377) + 1);

	/* use any unused variable registers as scratch registers */
	maxtreg & = 0377;
	fregs = maxtreg>=MINDRVAR ? maxtreg + 1 : MINDRVAR;
/*	if( xdebug ){*/
		/* -x changes number of free regs to 2, -xx to 3, etc. */
/*		if( (xdebug+1) < fregs ) fregs = xdebug+1;
		}
*/
	for( i=MINDRVAR; i<=MAXRVAR; i++ )
		rstatus[i] = i<fregs ? SAREG|STAREG : SAREG;
/***** GB - Kipp's hack for getting an A register to use.
	    if the reservea5 flag (macdefs.h) is set by main
	    (flag -R to ccom or -XCR to cc), MINRVAR for the a
	    registers = MINRVAR for the d registers plus one.
***/
	maxr = MAXRVAR;
	if (reservea5)  {
		maxr--;
		rstatus[5+8] = 0;
		naregs--;
	}
	for( i=MINARVAR; i<=maxr; i++ )
		rstatus[i+8] = i<naregs ? SBREG|STBREG : SBREG;
	}

szty(t) TWORD t; { /* size, in words, needed to hold thing of type t */
	/* really is the number of registers to hold type t */
/* bug fix 4/28/83.  This appeared wrong.  Try the fix and see if
   it clears up a possibly related problem. */
/*	return(1);*/
	return(t==DOUBLE ? 2 : 1);
	}

rewfld( p ) NODE *p; {
	return(1);
	}

callreg(p) NODE *p; {
	return( D0 );
	}

shltype( o, p ) NODE *p; {
	if( o == NAME|| o==REG || o == ICON || o == OREG ) return( 1 );
	return( o==UNARY MUL && shumul(p->in.left) );
	}

flshape( p ) register NODE *p; {
	register o = p->in.op;
	if( o==NAME || o==REG || o==ICON || o==OREG ) return( 1 );
	return( o==UNARY MUL && shumul(p->in.left)==STARNM );
	}

shtemp( p ) register NODE *p; {
	if( p->in.op == UNARY MUL ) p = p->in.left;
	if( p->in.op == REG || p->in.op == OREG ) return( !istreg( p->tn.rval ) );
	return( p->in.op == NAME || p->in.op == ICON );
	}

spsz( t, v ) TWORD t; CONSZ v; {

	/* is v the size to increment something of type t */

	if( !ISPTR(t) ) return( 0 );
	t = DECREF(t);

	if( ISPTR(t) ) return( v == 4 );

	switch( t ){

	case UCHAR:
	case CHAR:
		return( v == 1 );

	case SHORT:
	case USHORT:
		return( v == 2 );

	case INT:
	case UNSIGNED:
	case FLOAT:
		return( v == 4 );

	case DOUBLE:
		return( v == 8 );
		}

	return( 0 );
	}

indexreg( p ) register NODE *p; {
	if ( p->in.op==REG && p->tn.rval>=A0 && p->tn.rval<=SP) return(1);
	return(0);
}

shumul( p ) register NODE *p; {
	register o;

	o = p->in.op;
	if( indexreg(p) ) return( STARNM );

	if( o == INCR && indexreg(p->in.left) && p->in.right->in.op==ICON &&
	    p->in.right->in.name[0] == '\0' &&
	    spsz( p->in.left->in.type, p->in.right->tn.lval ) )
		return( STARREG );

	return( 0 );
	}

adrcon( val ) CONSZ val; {
	printf( CONFMT, val );
	}

conput( p ) register NODE *p; {
	switch( p->in.op ){

	case ICON:
		acon( p );
		return;

	case REG:
		printf( "%s", rnames[p->tn.rval] );
		usedregs |= 1<<p->tn.rval;
		return;

	default:
		cerror( "illegal conput" );
		}
	}

insput( p ) NODE *p; {
	cerror( "insput" );
	}

upput( p ) NODE *p; {
	/* output the address of the second word in the
	   pair pointed to by p (for LONGs)*/
	CONSZ save;

	if( p->in.op == FLD ){
		p = p->in.left;
		}

	save = p->tn.lval;
	switch( p->in.op ){

	case NAME:
		p->tn.lval += SZINT/SZCHAR;
		acon( p );
		break;

	case ICON:
		/* addressable value of the constant */
		p->tn.lval &= BITMASK(SZINT);
		printf( "#" );
		acon( p );
		break;

	case REG:
		printf( "%s", rnames[p->tn.rval+1] );
		usedregs |= 1<<(p->tn.rval + 1);
		break;

	case OREG:
		p->tn.lval += SZINT/SZCHAR;
		if( p->tn.rval == A6 ){  /* in the argument region */
			if( p->in.name[0] != '\0' ) werror( "bad arg temp" );
			}
		printf( "%s@", rnames[p->tn.rval] );
		usedregs |= 1<<p->tn.rval;
		if( p->tn.lval != 0 || p->in.name[0] != '\0' )
		  { printf("("); acon( p ); printf(")"); }
		break;

/* GB bug #84 8/31/84 **/
	case UNARY MUL:
		/* STARNM or STARREG found */
		if( tshape(p, STARNM) ) {
			adrput( p->in.left);
			printf( "@(%d)",SZINT/SZCHAR );
			}
		else {	/* STARREG - really auto inc or dec */
			/* when we adrput the lower long of this
			   double, the autoincrementing will take place. */
			adrput( p->in.left->in.left);
			printf( "@(%d)",SZINT/SZCHAR );
		}
		return;
/** GB **/
	default:
		cerror( "illegal upper address" );
		break;

		}
	p->tn.lval = save;

	}

adrput( p ) register NODE *p; {
	/* output an address, with offsets, from p */
/* GB bug #84 8/31/84 **/
	/* returns 1 if an autoincrement/autodecrement is pending,
	   0 otherwise */

	if( p->in.op == FLD ){
		p = p->in.left;
		}
	switch( p->in.op ){

	case NAME:
		acon( p );
		break;

	case ICON:
		/* addressable value of the constant */
		if( szty( p->in.type ) == 2 ) {
			/* print the high order value */
			CONSZ save;
			save = p->tn.lval;
			p->tn.lval = ( p->tn.lval >> SZINT ) & BITMASK(SZINT);
			printf( "#" );
			acon( p );
			p->tn.lval = save;
			break;
			}
		printf( "#" );
		acon( p );
		break;

	case REG:
		printf( "%s", rnames[p->tn.rval] );
		usedregs |= 1<<p->tn.rval;
		break;

	case OREG:
		if( p->tn.rval == A6 ){  /* in the argument region */
			if( p->in.name[0] != '\0' ) werror( "bad arg temp" );
			printf( "a6@(" );
			printf( p->tn.lval<0?"%ld":CONFMT, p->tn.lval );
			printf( ")" );
			break;
			}
		printf( "%s@", rnames[p->tn.rval] );
		usedregs |= 1<<p->tn.rval;
		if( p->tn.lval != 0 || p->in.name[0] != '\0' ) { 
			printf("("); 
			/* GB - put out negative address offsets in decimal */
			if (p->in.name[0] == '\0')
				printf(p->tn.lval<0? "%ld":CONFMT, p->tn.lval);
		  	else 
				acon( p ); 
		  	printf(")"); 
		}
		break;

	case UNARY MUL:
		/* STARNM or STARREG found */
		if( tshape(p, STARNM) ) {
			adrput( p->in.left);
			printf( "@" );
			}
		else { 	/* STARREG - really auto inc or dec */
/** GB bug #84 ***/
			if (BTYPE(p->in.type) == DOUBLE) {
				adrput( p->in.left->in.left);
				printf( "@" );
				return(1);
			}
			else {
				/* turn into OREG so replacement node will
			   	reflect the value of the expression */
				register i;
				register NODE *q, *l;

				l = p->in.left;
				q = l->in.left;
				p->in.op = OREG;
				p->in.rall = q->in.rall;
				p->tn.lval = q->tn.lval;
				p->tn.rval = q->tn.rval;
				for( i=0; i<NCHNAM; i++ )
					p->in.name[i] = q->in.name[i];
				if( l->in.op == INCR ) {
					adrput( p );
					printf( "+" );
					p->tn.lval -= l->in.right->tn.lval;
					}
				else {	/* l->in.op == ASG MINUS */
					printf( "-" );
					adrput( p );
					}
				tfree( l );
				}
		}
		break;

	default:
		cerror( "illegal address" );
		break;

		}
	return(0);

	}

facon( p ) register NODE *p; { /* print out a constant */

	if( p->in.name[0] == '\0' ){	/* constant only */
		printf( CONFMT, p->tn.lval);
		}
	else if( p->tn.lval == 0 ) {	/* name only */
		printf( NAME_FMT, &p->in.name[1] );
		}
	else {				/* name + offset */
		printf( NAME_FMT, &p->in.name[1] );
		putchar('+');
		printf(CONFMT, p->tn.lval);
		}
	}

acon( p ) register NODE *p; { /* print out a constant */

	if( p->in.name[0] == '\0' ){	/* constant only */
		printf( CONFMT, p->tn.lval);
		}
	else if( p->tn.lval == 0 ) {	/* name only */
		printf( NAME_FMT, p->in.name );
		}
	else {				/* name + offset */
		printf( NAME_FMT, p->in.name );
		putchar('+');
		printf(CONFMT, p->tn.lval);
		}
	}

genscall( p, cookie ) register NODE *p; {
	/* structure valued call */
	return( gencall( p, cookie ) );
	}

gencall( p, cookie ) register NODE *p; {
	/* generate the call given by p */
	register temp;
	register m;
	register ishw;
#ifdef SGI_REGS
	unsigned short gencallregmask_inc= 0,gencallregmask_dec = 0;
#endif

	int isfortran=0;
	int first;


#ifdef SGI_REGS
	/* GB SCR911 10/8/85. At the call, d0 and/or d1 may be busy due
	   to arguments in registers.  If this is true, the arg regs must
	   be pushed and popped across the call.
	*/

	if (busy[0]) {
		gencallregmask_inc |= (1<<D0);
		gencallregmask_dec |= (0x08000 >> D0);
		p->in.node_sgi |= SAVE_D0;
		rfree(D0);
	}
	if (busy[1] ) {
		gencallregmask_inc |= (1<<D1);
		gencallregmask_dec |= (0x08000 >> D1);
		p->in.node_sgi |= SAVE_D1;
		rfree(D1);
	}
	if (busy[A0]) {
		gencallregmask_inc |= (1<<A0);
		gencallregmask_dec |= (0x08000 >> A0);
		p->in.node_sgi |= SAVE_A0;
		rfree(A0);
	}
#endif


	/* GB REG fix argsize to not add sizes for args that will
	   be placed in registers */
	if( (p->in.right) 
#ifdef SGI_REGS
	    && (!(p->in.node_sgi & SGI_NODE)) 
#endif
		) 
		temp = argsize( p->in.right );
	else temp = 0;

	if (p->in.op == UNARY FORTCALL) {
		isfortran++;
		/* save a4/a5 and restore FORTRAN's global registers */
		usedregs |= ((1<<D2)|(1<<A5));
#ifdef NOTDEF
		if (!isJuniper) usedregs |= (1<<A4);
#endif
		if ((!(rstatus[D2] & STAREG)) || (busy[D2]))  {
			gencallregmask_inc |= (1<<D2);
			gencallregmask_dec |= (0x08000 >> D2);
		}
#ifdef NOTDEF
		if (!isJuniper) {
			if ((!(rstatus[A4] & STBREG)) || (busy[A4])) {
				gencallregmask_inc |= (1<<A4);
				gencallregmask_dec |= (0x08000 >> A4);
			}
		}
#endif
		if ((!(rstatus[A5] & STBREG)) || (busy[A5])) {
			gencallregmask_inc |= (1<<A5);
			gencallregmask_dec |= (0x08000 >> A5);
		}
	}
	if (gencallregmask_dec) {
		printf("\tmoveml\t#0x%x,sp@-\n",gencallregmask_dec);
		gencallregmask_dec = 0;
	}

	if( p->in.right ){ /* generate args */
		/* added argument to genargs GB */
		genargs( p->in.right 
#ifdef SGI_REGS
		        ,(p->in.node_sgi & (SGI_NODE))?p->in.node_sgi:0 ,p->in.hw_opcode
#endif
			);
		}

	
	if( !shltype( p->in.left->in.op, p->in.left ) ) {
		order( p->in.left, INBREG|SOREG );
		}

	if (p->in.op != UNARY FORTCALL)
		p->in.op = UNARY CALL;
	if (isfortran) {
		printf("\tmovl\t_$a5_save,a5\n");
#ifdef NOTDEF
		if (!isJuniper)
			printf("\tmovl\t_$a4_save,a4\n");
#endif
	}
	m = match( p, INTAREG|INTBREG );
	popargs( temp );


#ifdef SGI_REGS
	restore_volregs(p,gencallregmask_inc);
#endif
	return(m != MDONE);
	}

popargs( size ) register size; {
	/* pop arguments from stack */

	if (tdebug) printf("popping args from stack. size = %d, toff = %d\n",
			   size,toff);
	toff -= size/2;
	if( toff == 0 && size >= 2 ) size -= 2;
	switch( size ) {
	case 0:
		break;
	default:
		printf( "\t%s\t#%d,sp\n", 
		   size<=8 ? "addql":(size<=32767)?"addw":"addl", size);
		}
	}

char *
ccbranches[] = {
	"	beq	.L%d\n",
	"	bne	.L%d\n",
	"	ble	.L%d\n",
	"	blt	.L%d\n",
	"	bge	.L%d\n",
	"	bgt	.L%d\n",
	"	bls	.L%d\n",
	"	bcs	.L%d\n",		/* blo */
	"	bcc	.L%d\n",		/* bhis */
	"	bhi	.L%d\n",
	};

/*	long branch table

   This table, when indexed by a logical operator,
   selects a set of three logical conditions required
   to generate long comparisons and branches.  A zero
   entry indicates that no branch is required.
   E.G.:  The <= operator would generate:
	cmp	AL,AR
	jlt	lable	/ 1st entry LT -> lable
	jgt	1f	/ 2nd entry GT -> 1f
	cmp	UL,UR
	jlos	lable	/ 3rd entry ULE -> lable
   1:
 */

int lbranches[][3] = {
	/*EQ*/	0,	NE,	EQ,
	/*NE*/	NE,	0,	NE,
	/*LE*/	LT,	GT,	ULE,
	/*LT*/	LT,	GT,	ULT,
	/*GE*/	GT,	LT,	UGE,
	/*GT*/	GT,	LT,	UGT,
	/*ULE*/	ULT,	UGT,	ULE,
	/*ULT*/	ULT,	UGT,	ULT,
	/*UGE*/	UGT,	ULT,	UGE,
	/*UGT*/	UGT,	ULT,	UGT,
	};

/* logical relations when compared in reverse order (cmp R,L) */
extern short revrel[] ;

cbgen( o, lab, mode ) { /*   printf conditional and unconditional branches */
	register *plb;
	int lab1f;

	if( o == 0 ) printf( "	bra	.L%d\n", lab );
	else	if( o > UGT ) cerror( "bad conditional branch: %s", opst[o] );
	else {
		switch( brcase ) {

		case 'A':
		case 'C':
			plb = lbranches[ o-EQ ];
			lab1f = getlab();
			expand( brnode, FORCC, brcase=='C' ? "\tcmp\tAL,AR\n" : "\ttst\tAR\n" );
			if( *plb != 0 )
				printf( ccbranches[*plb-EQ], lab);
			if( *++plb != 0 )
				printf( ccbranches[*plb-EQ], lab1f);
			expand( brnode, FORCC, brcase=='C' ? "\tcmp\tUL,UR\n" : "\ttst\tUR\n" );
			printf( ccbranches[*++plb-EQ], lab);
			deflab( lab1f );
			reclaim( brnode, RNULL, 0 );
			break;

		default:
			if( mode=='F' ) o = revrel[ o-EQ ];
			printf( ccbranches[o-EQ], lab );
			break;
			}

		brcase = 0;
		brnode = 0;
		}
	}

ubgen(lab) {
	/* generate an unconditional branch to lab */

	printf("\tbra\t.L%d\n",lab);
}

#ifdef NOTDEF
nextcook( p, cookie ) NODE *p; {
	/* we have failed to match p with cookie; try another */
	if( cookie == FORREW ) return( 0 );  /* hopeless! */
	if (notlval(p)) {
		if (!(cookie&INTAREG)) return(INTAREG|INAREG);
		if ( (!(cookie&INTEMP)) && asgop(p->in.op) ) 
			return(INTEMP|INAREG|INTAREG);
	} else {
		if (!(cookie&INTBREG)) return(INTBREG|INBREG);
		if ( (!(cookie&INTEMP)) && asgop(p->in.op) ) 
			return(INTEMP|INTBREG|INTAREG);
	}	
	return( FORREW );
}
#endif

nextcook( p, cookie ) NODE *p; {
	/* we have failed to match p with cookie; try another */
	if( cookie == FORREW ) return( 0 );  /* hopeless! */
	if( !(cookie&(INTAREG|INTBREG)) ) return( INTAREG|INTBREG );
	if( !(cookie&INTEMP) && asgop(p->in.op) ) return( INTEMP|INAREG|INTAREG|INTBREG|INBREG );
	return( FORREW );
	}

lastchance( p, cook ) NODE *p; {
	/* forget it! */
	return(0);
	}


struct functbl {
	int fop;
	char *func;
	long argtype;
	short sky_opcode;
	short fpa_opcode;
	} opfunci[] = {	/* INT */
#ifdef SGI_REGS
	MUL,		"rlmul",	COMMUTEOP|SGI_NODE|HWNODE,SKY_IMUL,0,
	DIV,		"rldiv", 	SGI_NODE,	0,0,
	MOD,		"rlrem", 	SGI_NODE,	0,0,
	ASG MUL,	"ralmul", 	SGI_NODE|INDIRECTOP,	0,0,
	ASG DIV,	"raldiv", 	SGI_NODE|INDIRECTOP,	0,0,
	ASG MOD,	"ralrem", 	SGI_NODE|INDIRECTOP,	0,0,
	0,	0, 0 },
	opfuncu[] = {	/* UNSIGNED */
	MUL,		"rulmul", 	COMMUTEOP|SGI_NODE,	0,0,
	DIV,		"ruldiv", 	SGI_NODE,	0,0,
	MOD,		"rulrem", 	SGI_NODE,	0,0,
	ASG MUL,	"raulmul", 	SGI_NODE|INDIRECTOP,	0,0,
	ASG DIV,	"rauldiv", 	SGI_NODE|INDIRECTOP,	0,0,
	ASG MOD,	"raulrem", 	SGI_NODE|INDIRECTOP,	0,0,
	0,	0, 	0 ,0,0 },	

	opfuncp[] = {	/* PTR */
	MUL,		"rlmul",	COMMUTEOP|SGI_NODE|HWNODE,SKY_IMUL,0,
	DIV,		"rldiv", 	SGI_NODE,	0,0,
	MOD,		"rlrem", 	SGI_NODE,	0,0,
	0,	0 ,0,0 },	
#else
	MUL,		"lmul",	0,	0,0,
	DIV,		"ldiv", 0,	0,0,
	MOD,		"lrem", 0,	0,0,
	ASG MUL,	"almul", 0,	0,0,
	ASG DIV,	"aldiv", 0,	0,0,
	ASG MOD,	"alrem", 0,	0,0,
	0,	0, 0,0 },
	opfuncu[] = {	/* UNSIGNED */
	MUL,		"ulmul", 0,	0,0,
	DIV,		"uldiv", 0,	0,0,
	MOD,		"ulrem", 0,	0,0,
	ASG MUL,	"aulmul", 0,	0,0,
	ASG DIV,	"auldiv", 0,	0,0,
	ASG MOD,	"aulrem", 0,	0,0,
	0,	0,	0, 	0,0 },

	opfuncp[] = {	/* PTR */
	MUL,		"lmul", 0,	0,0,
	DIV,		"ldiv", 0,	0,0,
	MOD,		"lrem", 0,	0,0,
	0,	0 ,0,0 },	
#endif
#ifndef DOUBLES32BITS
	opfuncf[] = {	/* FLOAT */
	PLUS,		"fadd", 0,	0,0,
	MINUS,		"fsub", 0,	0,0,
	MUL,		"fmul", 0,	0,0,
	DIV,		"fdiv", 0,	0,0,
	UNARY MINUS,	"fneg", 0,	0,0,
	ASG PLUS,	"afaddf", 0,	0,0,
	ASG MINUS,	"afsubf", 0,	0,0,
	ASG MUL,	"afmulf", 0,	0,0,
	ASG DIV,	"afdivf", 0,	0,0,
	0,	0, 0,0 },
	opfuncd[] = {	/* DOUBLE */
	PLUS,		"fadd", 0,	0,0,
	MINUS,		"fsub", 0,	0,0,
	MUL,		"fmul", 0,	0,0,
	DIV,		"fdiv", 0,	0,0,
	UNARY MINUS,	"fneg", 0,	0,0,
	ASG PLUS,	"afadd", 0,	0,0,
	ASG MINUS,	"afsub", 0,	0,0,
	ASG MUL,	"afmul", 0,	0,0,
	ASG DIV,	"afdiv", 0,	0,0,
	0,	0 ,0,0 };	
#else /* not DOUBLES32BITS */
#ifdef SGI_REGS
	opfuncf[] = {	/* FLOAT */
	PLUS,		"_fr_add",	COMMUTEOP|SGI_NODE|HWNODE,SKY_SPADD,FPA_SPADD,
	MINUS,		"_fr_sub",	SGI_NODE|HWNODE,	SKY_SPSUB,FPA_SPSUB,
	MUL,		"_fr_mul",	COMMUTEOP|SGI_NODE|HWNODE,SKY_SPMUL,FPA_SPMUL,
	DIV,		"_fr_div",	SGI_NODE|HWNODE|HWPOLL,	SKY_SPDIV,FPA_SPDIV,
/*	UNARY MINUS,	"_fr_neg",	UNARYOP|SGI_NODE,	0, */
	ASG PLUS,	"_fr_iadd",	INDIRECTOP|SGI_NODE,	0,0,
	ASG MINUS,	"_fr_isub",	INDIRECTOP|SGI_NODE,	0,0,
	ASG MUL,	"_fr_imul",	INDIRECTOP|SGI_NODE,	0,0,
	ASG DIV,	"_fr_idiv",	INDIRECTOP|SGI_NODE,	0,0,
	0,	0,	0,0 },
	opfuncd[] = {	/* DOUBLE */
	PLUS,		"_d_add",	SGI_NODE|DOUBLEOP|HWNODE|HWPOLL,		SKY_DPADD,FPA_DPADD,
	MINUS,		"_d_sub",	SGI_NODE|DOUBLEOP|HWNODE|HWPOLL,		SKY_DPSUB,FPA_DPSUB,
	MUL,		"_d_mul",	SGI_NODE|DOUBLEOP|HWNODE|HWPOLL,	SKY_DPMUL,FPA_DPMUL,
	DIV,		"_d_div",	SGI_NODE|DOUBLEOP|HWNODE|HWPOLL,	SKY_DPDIV,FPA_DPDIV,
/*	UNARY MINUS,	"_dr_neg",	SGI_NODE|UNARYOP|DOUBLEOP,	0,*/
	ASG PLUS,	"_dr_iadd",	SGI_NODE|DOUBLEOP|INDIRECTOP,	0,0,
	ASG MINUS,	"_dr_isub",	SGI_NODE|DOUBLEOP|INDIRECTOP,	0,0,
	ASG MUL,	"_dr_imul",	SGI_NODE|DOUBLEOP|INDIRECTOP,	0,0,
	ASG DIV,	"_dr_idiv",	SGI_NODE|DOUBLEOP|INDIRECTOP,	0,0,
	0,	0 	,0,0};	
#else SGI_REGS
	opfuncf[] = {	/* FLOAT */
	PLUS,		"_f_add",	0,	0,0,
	MINUS,		"_f_sub",	0,	0,0,
	MUL,		"_f_mul",	0,	0,0,
	DIV,		"_f_div",	0,	0,0,
	UNARY MINUS,	"_f_neg",	0,	0,0,
	ASG PLUS,	"_f_iadd",	0,	0,0,
	ASG MINUS,	"_f_isub",	0,	0,0,
	ASG MUL,	"_f_imul",	0,	0,0,
	ASG DIV,	"_f_idiv",	0,	0,0,
	0, 0,	0,0 },
	opfuncd[] = {	/* DOUBLE */
	PLUS,		"_d_add",	0,	0,0,
	MINUS,		"_d_sub",	0,	0,0,
	MUL,		"_d_mul",	0,	0,0,
	DIV,		"_d_div",	0,	0,0,
	UNARY MINUS,	"_d_neg",	0,	0,0,
	ASG PLUS,	"_d_iadd",	0,	0,0,
	ASG MINUS,	"_d_isub",	0,	0,0,
	ASG MUL,	"_d_imul",	0,	0,0,
	ASG DIV,	"_d_idiv",	0,	0,0,
	0,	0 , 0,0};	
#endif
#endif

hardops(p)  register NODE *p; {
	/* change hard to do operators into function calls. */
	register NODE *q;
	register struct functbl *f;
	register o;
	register TWORD t, t1;
	register val;
	register NODE *tp;

	o = p->in.op;
	if (o==SCONV) {
		hardconv(p);
		return;
	}
	/* certain multiplies, divides, and mods can be done directly */
	tp = p->in.left;
	if (o == MUL || o == DIV || o == MOD) {
		if (tp->in.op != SCONV) goto xx;
		if (p->in.right->in.op != ICON) goto ww;
		t = tp->in.left->in.type;
		if (t != SHORT && t != USHORT && t != CHAR && t != UCHAR)
			goto xx;
		val = p->in.right->tn.lval;
		if ( (t==SHORT || t==CHAR) && val >= -32768 && val <= 32767 )
			t = SHORT;
		else
		if ( (t==USHORT || t==UCHAR) && val >= 0 && val <= 65535 )
			t = USHORT;
		else
			goto ww;
		tp->in.type = p->in.right->in.type = t;
		if (o != MUL) {
			q = talloc();
			q->in.op = p->in.op;
			q->in.type = t;
			q->in.left = tp;
			q->in.right = p->in.right;
			p->in.op = SCONV;
			p->in.left = q;
			p->in.right = NIL;
		}
		return;
	}
	ww:
	/* certain multiplies, divides, and mods can be done directly */
	if (o == MUL || o == DIV || o == MOD) {
		if (tp->in.op != SCONV) goto xx;
		if (p->in.right->in.op != SCONV) goto xx;
		t = tp->in.left->in.type;
		if (t != SHORT && t != USHORT && t != CHAR && t != UCHAR)
			goto xx;
		t1 = p->in.right->in.left->in.type;
		if (t1 != SHORT && t1 != USHORT && t1 != CHAR && t1 != UCHAR)
			goto xx;
		if (t == USHORT || t1 == USHORT)
			t = USHORT;
		else
			t = SHORT;
		tp->in.type = p->in.right->in.type = t;
		if (o != MUL) {
			q = talloc();
			q->in.op = p->in.op;
			q->in.type = t;
			q->in.left = tp;
			q->in.right = p->in.right;
			p->in.op = SCONV;
			p->in.left = q;
			p->in.right = NIL;
		}
		return;
	}
	xx:

	t = p->in.type;

	/*
	 * for float and double increment/decrement
	 * convert to (x +/-= 1) -/+ 1
	 */
	if ((o == INCR || o == DECR) && (t == FLOAT || t == DOUBLE)) {
		int eprint();

		if (edebug) {
			printf("hardop: converting:\n");
			fwalk(p, eprint, 0);
		}
		q = talloc();
		q->in.op = o == INCR ? ASG PLUS : ASG MINUS;
		q->in.rall = p->in.rall;
		q->in.type = p->in.type;
		q->in.left = p->in.left;
		q->in.right = talloc();
		ncopy(q->in.right, p->in.right);
		o = p->in.op = o == INCR ? MINUS : PLUS;
		p->in.left = q;
		if (edebug)
			fwalk(p, eprint, 0);
		hardops(q);	/* do the +=/-= node */
		if (edebug)
			fwalk(p, eprint, 0);
	}

	/* GB - SCR911 - change comparisons with at least one
	   float argument from OPLOG trees to the appropriate
	   calls.  This will save us the hardship of having
	   to do all the special jockeying with f_cmp nodes
	   since they were by-passing the checks of the normal
	   call mechanism.
	*/
	/* GB - SCR1744 - generate hardware divide and multiply
	   instructions on the 68020.
	*/
	if ((( t == INT) || (t == UNSIGNED)) && (isJuniper))
		return;
	if ( t==INT )
		for( f=opfunci; f->fop; f++ ) {
			if( o==f->fop ) goto convert;
			}
	else if ( t==UNSIGNED )
		for( f=opfuncu; f->fop; f++ ) {
			if( o==f->fop ) goto convert;
			}
	else if ( t==FLOAT )
		for( f=opfuncf; f->fop; f++ ) {
			if( o==f->fop ) goto convert;
			}
	else if ( t==DOUBLE )
		for( f=opfuncd; f->fop; f++ ) {
			if( o==f->fop ) goto convert;
			}

	t &= TMASK;
	if ( t==PTR )
		for( f=opfuncp; f->fop; f++ ) {
			if( o==f->fop ) goto convert;
			}
	return;

	/* need address of left node for ASG OP */
	/* WARNING - this won't work for long in a REG */
	convert:
#ifdef SGI_REGS
	/* GB - cant pass args to double non-indirect routines unless in h/w */
	if ((f->argtype & DOUBLEOP)&&(!(f->argtype & INDIRECTOP)) && (!fpatype))
		f->argtype = 0;

#endif
	if( asgop( o ) ) {
#ifdef SGI_REGS

		int sucomp();

		if ((fpatype)&&
			((p->in.type == DOUBLE)||(p->in.type == FLOAT))) { 
		/* GB - we must do the following here:

			if the op is an assign op, and we are using the h/w floating point
			option, the call must be replaced by the normal op and an assignment
			tacked on. Thus,

						+=									=
				   	   .  .     must be replaced by  	  .    .
				  	  .    .						op1 addr     +
				op1	addr  op2 val								.  .  
														  	   op1  op2
														       val  val

			**********************************************************
			BUG98 (scr1126) 9/6/85:

				We cannot always simply copy the l.h.s of this tree, as
				it may have side effects.  Thus, we have to do the following:

					* copy the l.h.s to a temp location.
					* do a SU comp on it.
					* mimick the algorithm in order.c (setasgop) to decide
						if the tree can be simply copied.
					* IF the tree can be simply copied (has no side effects),
						do it.
					* Otherwise, turn off the node as a hardware node, and
						re-do the hardop (thus substituting the software
						counterpart).
	
			**********************************************************
		*/

			if (edebug) {
				printf("hardop: before <op>= conversion:\n");
				fwalk(p,eprint,0);	
				fflush(stdout);
			    }
			q = tcopy( p->in.left );
			if (!has_autoincrdecr( q )) {
				walkf( q, sucomp);
				if ((q->in.su == 0)||
					((q->in.op == UNARY MUL )&&(q->in.left->in.op == NAME))) {
					/* we can simply copy the left subtree */
					tfree(q);
					q = talloc();
					q->in.op = p->in.op - 1; /* change <op>= to <op> */
					q->in.rall = p->in.rall;
					q->in.type = p->in.type;
					q->in.right = p->in.right;
					q->in.left = tcopy( p->in.left);
					p->in.op = ASSIGN;
					p->in.right = q;
					if (edebug) {
						printf("hardop: after <op>= conversion:\n");
						fwalk(p,eprint,0);	
						fflush(stdout);
				    	}
					/* we transformed the tree.  Call hardops() with the new tree */
					hardops(q);
					return;
				}
				else if (edebug) 
					printf("hardop: tree has ++/--\n");	
			}
			/* tree may have side-effects. Use software floating point. */
			tfree(q);
			if (edebug) printf(
				  "hardop: <op>= conversion disallowed due to side-effects\n");
			fflush(stdout);
		}
#endif
		tp = p->in.left;
		switch( tp->in.op ) {

		case UNARY MUL:	/* convert to address */
			tp->in.op = FREE;
			p->in.left = tp->in.left;
			break;

		case NAME:	/* convert to ICON pointer */
			tp->in.op = ICON;
			tp->in.type = INCREF( tp->in.type );
			break;

		case OREG:	/* convert OREG to address */
			tp->in.op = REG;
			tp->in.type = INCREF( tp->in.type );
			if( tp->tn.lval != 0 ) {
				q = talloc();
				q->in.op = PLUS;
				q->in.rall = NOPREF;
				q->in.type = tp->in.type;
				q->in.left = tp;
				q->in.right = talloc();

				tp = q->in.right;
				tp->in.op = ICON;
				tp->in.rall = NOPREF;
				tp->in.type = INT;
				tp->in.name[0] = '\0';
				tp->tn.lval = p->in.left->tn.lval;
				tp->tn.rval = 0;

				p->in.left->tn.lval = 0;
				p->in.left = q;
				}
			break;

		/* rewrite "foo <op>= bar" as "foo = foo <op> bar" for foo in a reg */
		case REG:
			q = talloc();
			q->in.op = p->in.op - 1; /* change <op>= to <op> */
			q->in.rall = p->in.rall;
			q->in.type = p->in.type;
			q->in.left = talloc();
			q->in.right = p->in.right;
			ncopy(q->in.left, p->in.left);
			p->in.op = ASSIGN;
			p->in.right = q;
			hardops(q);
			return;

		default:
			cerror( "Bad address for hard ops" );
			/* NO RETURN */

			}

		}

	/* build comma op for args to function */
	if ( optype(p->in.op) == BITYPE ) {
	  q = talloc();
	  q->in.op = CM;
	  q->in.rall = NOPREF;
	  q->in.type = INT;
	  q->in.left = p->in.left;
	  q->in.right = p->in.right;
	} else q = p->in.left;

	p->in.op = CALL;
	p->in.right = q;
#ifdef SGI_REGS
/****GB  REG FIX ****/
	p->in.node_sgi = f->argtype;
	if (fpatype == SKY_FPA)
		p->in.hw_opcode = f->sky_opcode;
	else
		p->in.hw_opcode = f->fpa_opcode;
	/* if the fpa in question cant handle this, turn off the 
	   hardware bits in the sginode
	*/
	if (p->in.hw_opcode == 0)
		p->in.node_sgi &= ~(HWNODE|HWOP);
/******/
#endif
	/* put function name in left node of call */
	p->in.left = q = talloc();
#ifdef SGI_REGS
	q->in.node_sgi = f->argtype;
	if (fpatype == SKY_FPA)
		q->in.hw_opcode = f->sky_opcode;
	else
		q->in.hw_opcode = f->fpa_opcode;
	/* if the fpa in question cant handle this, turn off the 
	   hardware bits in the sginode
	*/
	if (q->in.hw_opcode == 0)
		q->in.node_sgi &= ~(HWNODE|HWOP);

	/*  if we are using an fpa, we have to interchange the operands.
		The sky board needs them written first to last, and the
		juniper fpa is easier if they are moved in that order.
	*/
	if ((fpatype != NO_FPA) && (p->in.node_sgi & HWNODE) && 
			!(p->in.node_sgi & UNARYOP)) {
		register NODE *tmp;
		if (p->in.right->in.op != CM) cerror("inconsistent floating point tree");
		tmp = p->in.right->in.left;
		p->in.right->in.left = p->in.right->in.right;
		p->in.right->in.right = tmp;
	}
	/* on the Sky board, the opcode is attached to the FIRST
	   operand moved to the board.  On the Juniper FPA, it
	   is attached to the last one.
	*/
	{
		register NODE *tmp;
		tmp = p->in.right;
		if (fpatype == JUNIPER_FPA) {
			if (tmp->in.op == CM)
				tmp = tmp->in.left;
			tmp->in.hw_opcode = f->fpa_opcode;
		}
		else {
			while (tmp->in.op == CM)
				tmp = tmp->in.right;
			tmp->in.hw_opcode = f->sky_opcode;
		}
		tmp->in.node_sgi |= WRITE_HWOP;
	}
#endif
	q->in.op = ICON;
	q->in.rall = NOPREF;
	q->in.type = INCREF( FTN + p->in.type );
	strcpy( q->in.name, f->func );
	q->tn.lval = 0;
	q->tn.rval = 0;

	return;

	}


has_autoincrdecr( t ) register NODE *t;  
	{
	register opty;
	int result = 0;

	opty = optype(t->in.op);

	if( opty != LTYPE ) result |= has_autoincrdecr( t->in.left );
	if( opty == BITYPE ) result |= has_autoincrdecr( t->in.right);
	if ((t->in.op == INCR)||(t->in.op == DECR)) result = 1;
	return(result);

	}

/* do fix and float conversions */
hardconv(p)
  register NODE *p;
  {	register NODE *q;
	register TWORD t,tl;
	int m,ml;

	t = p->in.type;
	tl = p->in.left->in.type;

	m = t==DOUBLE || t==FLOAT;
	ml = tl==DOUBLE || tl==FLOAT;

/* replaced the following line with below to fix bugreport #18.
   Suggested by L version. GB (SGI) 4/28/83.
	if (m==ml || logop(p->in.left->in.op)) return;
*/
#ifndef IEEE
	if (m==ml) return;
#else
	if ((!m&&!ml)||(t==tl)) return;
#endif
	p->in.op = CALL;
	p->in.right = p->in.left;

	/* put function name in left node of call */
	p->in.left = q = talloc();
	q->in.op = ICON;
	q->in.rall = NOPREF;
	q->in.type = INCREF( FTN + p->in.type );
#ifdef IEEE
	if (m==ml) {
#ifdef SGI_REGS
	    p->in.node_sgi = SGI_NODE|UNARYOP|HWNODE;
		if (t==FLOAT) p->in.node_sgi |= DOUBLEOP;
		if (fpatype == SKY_FPA)
			p->in.hw_opcode = (t==FLOAT)? SKY_DPSP:SKY_SPDP;
		else
			p->in.hw_opcode = (t==FLOAT)? FPA_DPSP:FPA_SPDP;
	    strcpy( q->tn.name, (t==FLOAT) ? "_dr_2_f" : "_fr_2_d" );
#else
	    strcpy( q->tn.name, (t==FLOAT) ? "_d_2_f" : "_f_2_d" );
#endif
	}
	else
	if (ml) {
#ifdef SGI_REGS
	    p->in.node_sgi = SGI_NODE|UNARYOP|HWNODE;
		if (tl==DOUBLE) p->in.node_sgi |= DOUBLEOP;

			if (fpatype == SKY_FPA)
				p->in.hw_opcode = (tl==FLOAT)? SKY_SPFIX:SKY_DPFIX;
			else
				p->in.hw_opcode = (tl==FLOAT)? FPA_SPFIX:FPA_DPFIX;

			/* for now, turn off this node as a hardware node. */
#ifndef NOTDEF
		    if(fpatype == JUNIPER_FPA)
				p->in.node_sgi &= ~HWNODE;
#endif

	    strcpy( q->tn.name, (tl==FLOAT) ? "_fr_2_i" : "_dr_2_i");
#else /* SGI_REGS */
	    strcpy( q->tn.name, (tl==FLOAT) ? "_f_2_i" : "_d_2_i");
#endif /* SGI_REGS */
	}
	else {
#ifdef SGI_REGS
	    p->in.node_sgi = HWNODE|SGI_NODE|UNARYOP;
		if (t==DOUBLE) p->in.node_sgi |= DOUBLEOP;

			if (fpatype == SKY_FPA)
				p->in.hw_opcode = (t==FLOAT)?SKY_SPFLOAT:SKY_DPFLOAT;
			else
				p->in.hw_opcode = (t==FLOAT)?FPA_SPFLOAT:FPA_DPFLOAT;

	    strcpy( q->tn.name, (t==FLOAT) ? "_ir_2_f" : "_ir_2_d");
#else
	    strcpy( q->tn.name, (t==FLOAT) ? "_i_2_f" : "_i_2_d");
#endif
	}
#else
	strcpy( q->tn.name, m ? "float" : "fix" );
#endif
	q->tn.lval = 0;
	q->tn.rval = 0;
#ifdef SGI_REGS
	/* last, copy the opcode into the rightmost node, and indicate
	   that the opcode must be attached to that node.
	*/
	{
		register NODE *tmp;
		tmp = p->in.right;
		if (fpatype == JUNIPER_FPA) {
			if (tmp->in.op == CM)
				tmp = tmp->in.left;
		}
		else {
			while (tmp->in.op == CM)
				tmp = tmp->in.right;
		}
		tmp->in.node_sgi |= WRITE_HWOP;
		tmp->in.hw_opcode = p->in.hw_opcode;
	}
#endif
}

optim2( p ) register NODE *p; {
	/* do local tree transformations and optimizations */

	register NODE *r;

	switch( p->in.op ) {
		}
	}

myreader(p) register NODE *p; {
	walkf( p, hardops );	/* convert ops to function calls */
	canon( p );		/* expands r-vals for fileds */
	/* walkf( p, optim2 ); 	/* doesn't do anything anyway */
	toff = 0;  /* stack offset swindle */
	}

special( p, shape ) register NODE *p; {
	/* special shape matching routine */

	switch( shape ) {

	case SCCON:
		if( p->in.op == ICON && p->in.name[0]=='\0' && p->tn.lval>= -128 && p->tn.lval <=127 ) return( 1 );
		break;

	case SICON:
		if( p->in.op == ICON && p->in.name[0]=='\0' && p->tn.lval>= 0 && p->tn.lval <=32767 ) return( 1 );
		break;

	case S8CON:
		if( p->in.op == ICON && p->in.name[0]=='\0' && p->tn.lval>= 1 && p->tn.lval <= 8) return( 1 );
		break;

	default:
		cerror( "bad special shape" );

		}

	return( 0 );
	}

# ifndef ONEPASS
main( argc, argv ) char *argv[]; {
	return( mainp2( argc, argv ) );
	}
# endif

# ifdef MULTILEVEL
# include "mldec.h"

struct ml_node mltree[] ={

DEFINCDEC,	INCR,	0,
	INCR,	SANY,	TANY,
		OPANY,	SAREG|STAREG,	TANY,
		OPANY,	SCON,	TANY,

DEFINCDEC,	ASG MINUS,	0,
	ASG MINUS,	SANY,	TANY,
		REG,	SANY,	TANY,
		ICON,	SANY,	TANY,

TSOREG,	1,	0,
	UNARY MUL,	SANY,	TANY,
		REG,	SANY,	TANY,

TSOREG,	2,	0,
	UNARY MUL,	SANY,	TANY,
		PLUS,	SANY,	TANY,
			REG,	SANY,	TANY,
			ICON,	SANY,	TCHAR|TUCHAR|TSHORT|TUSHORT|TINT|TUNSIGNED|TPOINT,

TSOREG,	2,	0,
	UNARY MUL,	SANY,	TANY,
		MINUS,	SANY,	TANY,
			REG,	SANY,	TANY,
			ICON,	SANY,	TCHAR|TUCHAR|TSHORT|TUSHORT|TINT|TUNSIGNED|TPOINT,
0,0,0};
# endif
