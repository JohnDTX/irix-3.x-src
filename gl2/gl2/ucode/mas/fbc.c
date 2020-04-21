/* fbc.c  --  function definitions for fbc microassembler
 *
 * for summary, see mas/doc/fbcops.doc
 * new nano-ops:	MULDIV; PROPIN; PROPOUT12; PROPOUT16; CLRFLAG;
 *			MARETC( marcode, etctype);
 * new micro-ops:
 *			IMMRAM( aluop, inc, data, r2, marcode );
 *			REGRAMCOMP( cond, reg, marcode )
 *			IMMRAMCOMP( cond, data, marcode )
 */

#include "mas.h"

/*================================================================*/
/*  parameters
/*================================================================*/

#include "fbcparams.h"

	Paramtype wrcode = {REGWRE,REGWRD};

	Paramtype rdwrtype = {RAMRD,RAMNOP};

	Paramtype qstatus = {QOPERAND,NONQOP};

	Paramtype moredata = {MORE,NOMORE};

	Paramtype injust = {RJUST,LJUST};

	Paramtype marcode = {LOAD,HOLD};

	Paramtype DIinput = {UCONST,MULTIBUS};

	Paramtype busparts = {ALL16,NONE};

	Paramtype bus16bits = {0,0xffff};

	Paramtype qshift = {LDQ,OLDQ};

	Paramtype fshift = {FLL,FF};

	Paramtype incr = {P0,P1};

	Paramtype op2903 = {ADDOP,COMPSOP};

	Paramtype condtype = {IFFALSE,IFNFLAG};

	Paramtype op2910 = {CONT,JZER};

#ifdef UC3
	Paramtype BPCcommand = {CLIPLD,OCT3RVECT};
#endif
#ifdef UC4
	Paramtype BPCcommand = {LOADED,OCT3RVECT};
#endif

	Paramtype addrtype = {0,15};

	Paramtype etctype = {INTERRUPT,FBREAD};

/*================================================================*/
/*   outputs
/*================================================================*/

Output NextAddress =	{"di",		0,	15,	0};
Output i0 = 		{"i0",		16,	16,	0};
Output i1 =		{"i1",		17,	20,	6};
Output i2 =		{"i2",		21,	24,	4};  /* deflt: WRE */
Output cin =		{"cin",		25,	25,	0};
Output earbar =		{"earbar",	26,	26,	0};
Output ealbar =		{"ealbar",	27,	27,	0};
Output addra =		{"addra",	28,	31,	0};
Output addrb =		{"addrb",	32,	35,	0};
Output clklong =	{"clklong",	36,	36,	0};
Output get = 		{"get",		37,	37,	0};
Output put =		{"put",		38,	38,	0};
Output loadout = 	{"loadout",	39,	39,	0};
Output enram = 		{"enram",	40,	40,	0};
Output rdram =		{"rdram",	41,	41,	0};
Output highbyte =	{"highbyte",	42,	42,	0};
Output rightjust =	{"rightjust",	43,	43,	0};
Output seqop =		{"seqop",	44,	47,	14};
Output ccsel =		{"ccsel",	48,	50,	0};
Output fbccode =	{"fbccode",	51,	54,	0};
Output reverse =	{"reverse",	55,	55,	0};

Output DIsrc = 		{"DIsrc",	0,	7,	0};
		/* pseudo-field for recording intended use of DI bus	*/
Output seqtype = 	{"seqtype",	0,	7,	0};
		/* pseudo-field for categorizing 2910 opcode used	*/
Output microconst = 	{"microconst",	-1,	-1,	0};
		/* pseudo-field for recording whether MICROCONST invoked */


/*================================================================*/
/*  Nano-Ops
/*================================================================*/


MICROCONST(x)
   Param x;
{
    Procname(MICROCONST);

    Typecheck(x,bus16bits);
    Setfield(microconst,1);
/*    Setfield(constproc,1);	/* pseudo-field: constant procedure */
/*		constproc set by LOADDI -- this is just a holdover from
 *			SLIM simulator where constant value needed first
 */
}

LOADDI(x)
    Param x;
{
    Procname(LOADDI);
    Typecheck(x,DIinput);
    Setfield(DIsrc,x);		/* record intended use of DI bus */

    switch (x) {
	case UCOUNT:
	case OUTPUTREG: 
	case OUTPUTADDRESS:
	case INLJUST:
			Setfield(rightjust,0);
			break;
	case INRJUST:
	case BPCDATA:
	case OUTPUTCOUNT:
	case MULTIBUS: Setfield(rightjust,1);
			break;
/*	case UCONST:	(do nothing)	*/
    }
}

SETROP(reg, halves)
    Param reg,halves;
{
    Procname(SETROP);
    Typecheck(reg,addrtype);
    Typecheck(halves,busparts);

    Setfield(addra,reg);
    switch (halves) {
	case ALL16:  Setfield(ealbar,1); Setfield(earbar,1); break;
	case HI8:    Setfield(ealbar,1); Setfield(earbar,0); break;
	case LO8:    Setfield(ealbar,0); Setfield(earbar,1); break;
	case NONE:   Setfield(ealbar,0); Setfield(earbar,0);
    }
}

SETSOP(qop, reg, rdwrram)
    Param qop,reg,rdwrram;
{
    Procname(SETSOP);
    Typecheck(qop,qstatus);
    Typecheck(reg,addrtype);
    Typecheck(rdwrram,rdwrtype);

    Setfield(addrb,reg);
    if (qop == QOPERAND) Setfield(i0,1);
    RAMOP(rdwrram);
}


RAMOP(rdwrram)
    Param rdwrram;
{
    Procname(RAMOP);
    Typecheck(rdwrram,rdwrtype);

    switch (rdwrram) {
	case RAMRD:	Setfield(enram,1); Setfield(rdram,1); break;
	case RAMWR:	Setfield(enram,1); Setfield(rdram,0); break;
	case RAMNOP:	Setfield(enram,0); Setfield(rdram,0);
    }
}

ALUOP(op,inc)
    Param op,inc;
{
    Procname(ALUOP);
    Typecheck(op,op2903);
    Typecheck(inc,incr);

    switch (op) {
	case ADDOP:	Setfield(i1,3); break;
	case SUBRSOP:	Setfield(i1,2); break;
	case SUBSROP:	Setfield(i1,1); break;
	case FHIGHOP:	Setfield(i1,0); Setfield(i0,1); break;
	case SONLYOP:	Setfield(i1,4); break;
	case RONLYOP:	Setfield(i1,6); break;
	case FLOWOP:	Setfield( i1,8); break;
	case ANDOP:	Setfield(  i1,12); break;
	case XNOROP:	Setfield( i1,10); break;
	case XOROP:	Setfield(  i1,11); break;
	case NOROP:	Setfield(  i1,13); break;
	case NANDOP:	Setfield( i1,14); break;
	case IOROP:	Setfield(  i1,15); break;
	case COMPROP:	Setfield(i1,7); break;
	case COMPSOP:	Setfield(i1,5);
	}
    if (inc==P1) Setfield(cin,1);
}

FTOYANDQ(f,q,w)
    Param f,q,w;
{
    Procname(FTOYANDQ);
    Typecheck(f,fshift);
    Typecheck(q,qshift);
    Typecheck(w,wrcode);

    switch (f) {
	case FAR:
	    if (q==OLDQ && w==REGWRE) Setfield(i2,0);
	    else if (q==QR && w==REGWRE) Setfield(i2,2);
	    else FQerr();
	    break;
	case FLR:
	    if (q==OLDQ && w==REGWRE) Setfield(i2,1);
	    else if (q==QR && w==REGWRE) Setfield(i2,3);
	    else FQerr();
	    break;
	case FF:
	    switch (q) {
		case OLDQ:
		    if (w==REGWRE) Setfield(i2,4);
		    else	Setfield(i2,12);
		    break;
		case QR: 
		    if (w==REGWRD) Setfield(i2,5);
		    else FQerr();
		    break;
		case LDQ:
		    if (w==REGWRD) Setfield(i2,6);
		    else	Setfield(i2,7);
		    break;
		case QL:
		    if (w==REGWRD) Setfield(i2,13);
		    else FQerr();
		    break;
		}
	    break;
	case FLL:
	    if (q==OLDQ && w==REGWRE) Setfield(i2,9);
	    else if (q==QL && w==REGWRE) Setfield(i2,11);
	    else FQerr();
	    break;
	case FAL:
	    if (q==OLDQ && w==REGWRE) Setfield(i2,8);
	    else if (q==QL && w==REGWRE) Setfield(i2,10);
	    else FQerr();
	}
}

FQerr()
{
   printf("FTOYANDQ: bad parameter combination");
    printstate();
}


YTODO(y)
    Param y;
{
    Procname(YTODO);
    Typecheck(y,busparts);

    if (y==HI8) Setfield(highbyte,1);
}


_DOToOutreg()
{
    Procname(DOTOOUTREG);
    Setfield(loadout,1);
}


DOTOMAR(act)
   Param act;
{
    Procname(DOTOMAR);
    Typecheck(act,marcode);

#ifdef UC3
    if (act != HOLD) {
	Setfield(put,0);
	Setfield(reverse,0);
    }
    switch (act) {
	case LOAD:  Setfield(fbccode,1); break;
	case INC:  Setfield(fbccode,2); break;
	case DEC:  Setfield(fbccode,3);
	}
#endif
#ifdef UC4
    if (act != HOLD) {
	Setfield(put,0);
	Setfield(reverse,1);
    }
    switch (act) {
	case LOAD:  Setfield(fbccode,3); break;
	case INC:  Setfield(fbccode,4); break;
	case DEC:  Setfield(fbccode,5);
	}
#endif
}


MARETC(mar,etc)
   Param mar,etc;
{
    Procname(MARETC);
    Typecheck(mar,marcode);
    Typecheck(etc,etctype);

#ifdef UC3
    printf("MARETC not supported "); printstate();
#endif
#ifdef UC4
    Setfield(put,0);
    Setfield(reverse,1);
    switch (mar) {
	case LOAD:	if (etc==INTERRUPT) Setfield(fbccode,9);
			else Setfield(fbccode,12);
			break;
	case INC:	if (etc==INTERRUPT) Setfield(fbccode,10);
			else Setfield(fbccode,13);
			break;
	case DEC:	if (etc==INTERRUPT) Setfield(fbccode,11);
			else Setfield(fbccode,14);
			break;
	case HOLD:	if (etc==INTERRUPT) Setfield(fbccode,6);
			else Setfield(fbccode,8);
	}
#endif
}


BPCCMD(cmd)
    Param cmd;
{
    Procname(BPCCMD);
    Typecheck(cmd,BPCcommand);

    switch (cmd) {
#ifdef UC3
/*		strobes		*/
	case MASKREAD : Setfield( put,0); Setfield( fbccode,5); break;
	case FONTREAD : Setfield( put,0); Setfield( fbccode,6); break;
	case LDCONFIG : Setfield( put,0); Setfield( fbccode,8); break;
	case LOADED   : Setfield( put,0); Setfield( fbccode,9); break;
	case LOADEC   : Setfield( put,0); Setfield( fbccode,10); break;
	case LOADXS   : Setfield( put,0); Setfield( fbccode,11); break;
	case LOADXE   : Setfield( put,0); Setfield( fbccode,12); break;
	case LOADYS   : Setfield( put,0); Setfield( fbccode,13); break;
	case LOADYE   : Setfield( put,0); Setfield( fbccode,14); break;
	case LOADFA   : Setfield( put,0); Setfield( fbccode,15); break;
/*		commands	*/
	case CLIPLD	 :Setfield( put,1); Setfield(fbccode,0); break;
	case COLORLD	 :Setfield( put,1); Setfield(fbccode,2); break;
	case FONTWR	 :Setfield( put,1); Setfield(fbccode,1); break;
	case WDREAD	 :Setfield( put,1); Setfield(fbccode,4); break;
	case WDWR	 :Setfield( put,1); Setfield(fbccode,5); break;
	case PIXSTRRD	 :Setfield( put,1); Setfield(fbccode,6); break;
	case PIXSTRWR	 :Setfield( put,1); Setfield(fbccode,8); break;
	case CHDRAW	 :Setfield( put,1); Setfield(fbccode,9); break;
	case RECTNGL    :Setfield( put,1); Setfield(fbccode,10); break;
	case OCT0VECT   :Setfield( put,1); Setfield(fbccode,12); break;
	case OCT1VECT   :Setfield( put,1); Setfield(fbccode,13); break;
	case OCT2VECT   :Setfield( put,1); Setfield(fbccode,14); break;
	case OCT3VECT   :Setfield( put,1); Setfield(fbccode,15); break;
	case OCT0RVECT  :Setfield( put,1); Setfield(fbccode,12); Setfield(reverse,1); break;
	case OCT1RVECT  :Setfield( put,1); Setfield(fbccode,13); Setfield(reverse,1); break;
	case OCT2RVECT  :Setfield( put,1); Setfield(fbccode,14); Setfield(reverse,1); break;
	case OCT3RVECT  :Setfield( put,1); Setfield(fbccode,15); Setfield(reverse,1); break;
	case CLEAR	 :Setfield( put,1); Setfield(fbccode,11); break;
	case XADDRLD	 :Setfield( put,1); Setfield(fbccode,3); break;
	case BPCNOP	 :Setfield( put,1); Setfield(fbccode,7); break;
    }
#endif UC3
#ifdef UC4
/*		strobes		*/
	case LOADED:	Setfield(fbccode,1); break;
	case LOADEC:	Setfield(fbccode,2); break;
	case LOADXS:	Setfield(fbccode,3); break;
	case LOADXE:	Setfield(fbccode,4); break;
	case LOADYS:	Setfield(fbccode,5); break;
	case LOADYE:	Setfield(fbccode,6); break;
	case LOADFA:	Setfield(fbccode,7); break;
	case LOADSAF:	Setfield(fbccode,8); break;
	case LOADSAI:	Setfield(fbccode,9); break;
	case LOADEAF:	Setfield(fbccode,10); break;
	case LOADEAI:	Setfield(fbccode,11); break;
	case LOADSDF:	Setfield(fbccode,12); break;
	case LOADSDI:	Setfield(fbccode,13); break;
	case LOADEDF:	Setfield(fbccode,14); break;
	case LOADEDI:	Setfield(fbccode,15); break;
	case LOADMODE:	Setfield(fbccode,0); break;
	case LOADREPEAT:Setfield(fbccode,1); break;
	case LOADCONFIG:Setfield(fbccode,2); break;
 /*		commands	*/
	case READFONT:	Setfield(fbccode,0); break;
 	case WRITEFONT:	Setfield(fbccode,1); break;
	case READREPEAT:Setfield(fbccode,2); break;
	case SETADDRS:	Setfield(fbccode,3); break;
	case SAVEWORD:	Setfield(fbccode,4); break;
	case DRAWWORD:	Setfield(fbccode,5); break;
	case READLSTIP:	Setfield(fbccode,6); break;
	case NOOP:	Setfield(fbccode,7); break;
	case DRAWCHAR:	Setfield(fbccode,9); break;
	case FILLRECT:	Setfield(fbccode,10); break;
	case FILLTRAP:	Setfield(fbccode,11); break;
	case OCT0VECT:	Setfield(fbccode,12); break;
	case OCT1VECT:	Setfield(fbccode,13); break;
	case OCT2VECT:	Setfield(fbccode,14); break;
	case OCT3VECT:	Setfield(fbccode,15); break;
	case SETSCRMASKX:Setfield(fbccode,0); break;
	case SETSCRMASKY:Setfield(fbccode,1); break;
	case SETCOLORAB:Setfield(fbccode,5); break;
	case SETCOLORCD:Setfield(fbccode,4); break;
	case SETWEAB:	Setfield(fbccode,7); break;
	case SETWECD:	Setfield(fbccode,6); break;
	case READPIXELAB:Setfield(fbccode,9); break;
	case READPIXELCD:Setfield(fbccode,8); break;
	case DRAWPIXELAB:Setfield(fbccode,11); break;
	case DRAWPIXELCD:Setfield(fbccode,10); break;
	case OCT0RVECT:	Setfield(fbccode,12); break;
	case OCT1RVECT:	Setfield(fbccode,13); break;
	case OCT2RVECT:	Setfield(fbccode,14); break;
	case OCT3RVECT:	Setfield(fbccode,15); break;
    }
    if (cmd <= LOADCONFIG) {		/* strobes */
	Setfield(put,0);
	if (cmd <= LOADEDI) Setfield(reverse,0);   /* strobes below 0x10 */
	else Setfield(reverse,1);
    }
    else {				/* commands */
	Setfield(put,1);
	if (cmd < SETSCRMASKX) Setfield(reverse,0);	/* cmds below 0x10 */
	else Setfield(reverse,1);
    }
#endif UC4
}


_InterruptHost()
{
    Procname(INTERRUPTHOST);
    Setfield(put,0);
#ifdef UC3
    Setfield(fbccode,4);
    Setfield(reverse,0);
#endif
#ifdef UC4
    Setfield(fbccode,6);
    Setfield(reverse,1);
#endif
}


_ReadBPCBus()
{
    Procname(READBPCBUS);
    Setfield(put,0);
#ifdef UC3
    Setfield(fbccode,6);
    Setfield(reverse,0);
#endif
#ifdef UC4
    Setfield(fbccode,8);
    Setfield(reverse,1);
#endif
}


_SetLED()
{
    Procname(SETLED);
    Setfield(put,0);
#ifdef UC3
    Setfield(fbccode,5);
    Setfield(reverse,0);
#endif
#ifdef UC4
    Setfield(fbccode,7);
    Setfield(reverse,1);
#endif
}


_ClrFlag()
{
    Procname(CLRFLAG);
    Setfield(put,0);
    Setfield(fbccode,15);
    Setfield(reverse,1);
}


/* new carry-in selection for multiply, divide, doubleword cycles:
 * MULDIV, PROPAGATE
 */

_MulDiv()
{
    Procname(MULDIV);
    Setfield(clklong,1);
    Setfield(rightjust,0);
}

_PropIn()
{
    Procname(PROPIN);
    Setfield(clklong,1);
    Setfield(rightjust,1);
}

_PropOut16()
{
    Procname(PROPOUT16);
    Setfield(clklong,0);
    Setfield(rightjust,0);
}

_PropOut12()
{
    Procname(PROPOUT12);
    Setfield(clklong,0);
    Setfield(rightjust,1);
}

_GeomEngData()
{
    Procname(GEOMENGDATA);
    Setfield(get,1);
}


/*================================================================*/
/*   micro-ops
/*================================================================*/

LOADREG(reg,bus,data)
   Param reg,bus,data;
{
    Procname(LOADREG);
    Typecheck(reg,addrtype);
    Typecheck(bus,busparts);
    Typecheck(data,moredata);

    switch (bus) {
	case ALL16:  LOADDI(INRJUST); break;
	case HI8:
	case LO8:    LOADDI(INLJUST); break;
	default:	printf("Misuse of bus in LOADREG() "); printstate();
    }
    if (data==MORE) _GeomEngData();
    SETROP(reg,bus);  SETSOP(NONQOP,reg,RAMNOP);  ALUOP(RONLYOP,P0);
    FTOYANDQ(FF,OLDQ,REGWRE);  YTODO(ALL16);
}


LOADIMM(reg,data)
    Param reg,data;
{
    Procname(LOADIMM);
    Typecheck(reg,addrtype);
    Typecheck(data,bus16bits);
    LOADDI(UCONST); SETROP(reg,ALL16);
    SETSOP(NONQOP,reg,RAMNOP);  ALUOP(RONLYOP,P0);
    FTOYANDQ(FF,OLDQ,REGWRE);  YTODO(ALL16);
}


LOADMAR(data)
   Param data;
{
    Procname(LOADMAR);
    Typecheck(data,bus16bits);
    LOADDI(UCONST);  SETROP(0,ALL16);
    SETSOP(NONQOP,0,RAMNOP);  ALUOP(RONLYOP,P0);
  FTOYANDQ(FF,OLDQ,REGWRD);  YTODO(ALL16);  DOTOMAR(LOAD);
}


RAM(rdwr,reg,mar)
    Param rdwr,reg,mar;
{
    Procname(RAM);
    Typecheck(rdwr,rdwrtype);
    Typecheck(reg,addrtype);
    Typecheck(mar,marcode);
    SETSOP(NONQOP,reg,rdwr);  ALUOP(SONLYOP,P0);
    FTOYANDQ(FF,OLDQ,REGWRE);  YTODO(ALL16);  DOTOMAR(mar);
}


REGREG(op,inc,r1,r2)
    Param op,inc,r1,r2;
{
    Procname(REGREG);
    Typecheck(op,op2903);
    Typecheck(inc,incr);
    Typecheck(r1,addrtype);
    Typecheck(r2,addrtype);
    SETROP(r1,NONE);   SETSOP(NONQOP,r2,RAMNOP);  ALUOP(op,inc);
    FTOYANDQ(FF,OLDQ,REGWRE);  YTODO(ALL16);
}


REGRAM(op,inc,r1,r2,mar)
    Param op,inc,r1,r2,mar;
{
    Procname(REGRAM);
    Typecheck(op,op2903);
    Typecheck(inc,incr);
    Typecheck(r1,addrtype);
    Typecheck(r2,addrtype);
    Typecheck(mar,marcode);
    SETROP(r1,NONE); SETSOP(NONQOP,r2,RAMRD);  ALUOP(op,inc);
    FTOYANDQ(FF,OLDQ,REGWRE);  YTODO(ALL16);  DOTOMAR(mar);
}

IMMRAM(op,inc,dat,r2,mar)
    Param op,inc,dat,r2,mar;
{
    Procname(IMMRAM);
    Typecheck(op,op2903);
    Typecheck(inc,incr);
    Typecheck(dat,bus16bits);
    Typecheck(r2,addrtype);
    Typecheck(mar,marcode);
    LOADDI(UCONST);
    SETROP(0,ALL16); SETSOP(NONQOP,r2,RAMRD);  ALUOP(op,inc);
    FTOYANDQ(FF,OLDQ,REGWRE);  YTODO(ALL16);  DOTOMAR(mar);
}


IMMREG(op,inc,data,reg)
    Param op,inc,data,reg;
{
    Procname(IMMREG);
    Typecheck(op,op2903);
    Typecheck(inc,incr);
    Typecheck(data,bus16bits);
    Typecheck(reg,addrtype);
    LOADDI(UCONST);  SETROP(0,ALL16);  SETSOP(NONQOP,reg,RAMNOP);
    ALUOP(op,inc);   FTOYANDQ(FF,OLDQ,REGWRE);  YTODO(ALL16);
}


IMMCOMP(inc,data,reg)	  /* compare immediate data and reg (nonwrite) */
    Param inc,data,reg;
{
    Procname(IMMCOMP);
    Typecheck(inc,incr);
    Typecheck(data,bus16bits);
    Typecheck(reg,addrtype);
    LOADDI(UCONST);  SETROP(0,ALL16);  SETSOP(NONQOP,reg,RAMNOP);
    ALUOP(SUBRSOP,inc);  FTOYANDQ(FF,OLDQ,REGWRD);  YTODO(ALL16);
}


REGCOMP(inc,reg1,reg2)			/* compare two registers (nonwrite) */
    Param inc,reg1,reg2;
{
    Procname(REGCOMP);
    Typecheck(inc,incr);
    Typecheck(reg1,addrtype);
    Typecheck(reg2,addrtype);
    SETROP(reg1,NONE);  SETSOP(NONQOP,reg2,RAMNOP);
    ALUOP(SUBRSOP,inc);  FTOYANDQ(FF,OLDQ,REGWRD);  YTODO(ALL16);
}


REGRAMCOMP(inc,reg,marop)	/* compare register and scratch loc */
    Param inc,reg,marop;
{
    Procname(REGRAMCOMP);
    Typecheck(inc,incr);
    Typecheck(reg,addrtype);
    Typecheck(marop,marcode);
    SETROP(reg,NONE); SETSOP(NONQOP,reg,RAMRD);
    ALUOP(SUBRSOP,inc);  FTOYANDQ(FF,OLDQ,REGWRD);
    YTODO(ALL16);  DOTOMAR(marop);
}


IMMRAMCOMP(inc,data,marop)	/* compare immediate data and scratch loc */
    Param inc,data,marop;
{
    Procname(IMMRAMCOMP);
    Typecheck(inc,incr);
    Typecheck(data,bus16bits);
    Typecheck(marop,marcode);
    LOADDI(UCONST); SETROP(0,ALL16); SETSOP(NONQOP,0,RAMRD);
    ALUOP(SUBRSOP,inc);  FTOYANDQ(FF,OLDQ,REGWRD);
    YTODO(ALL16);  DOTOMAR(marop);
}

/*================================================================*/
/*  sequencing procedures
/*================================================================*/

COND(cond)
    Param cond;
{
    Procname(COND);
    Typecheck(cond,condtype);

    switch(cond) {
	case IFTRUE:  Setfield(ccsel,0); break;
	case IFNEG:   Setfield(ccsel,1); break;
	case IFOVF:  Setfield(ccsel,2); break;
	case IFNZ:  Setfield(ccsel,3); break;
	case IFZ:  Setfield(ccsel,4); break;
	case IFNNEG:  Setfield(ccsel,5); break;
	case IFNFLAG:  Setfield(ccsel,6); break;
	case IFFALSE:  Setfield(ccsel,7); break;
	}
}


SEQ(op)
    Param op;
{
    Procname(SEQ);
    Typecheck(op,op2910);

    switch(op) {
	case JZER:  Setfield(seqop,0); Setfield(seqtype,NONBRANCH); break;
	case JSUB:  Setfield(seqop,1); Setfield(seqtype,BRANCH); break;
	case JMAP:  Setfield(seqop,2); Setfield(seqtype,SPECIAL); break;
	case JUMP:  Setfield(seqop,3); Setfield(seqtype,BRANCH); break;
	case PUSH:  Setfield(seqop,4); Setfield(seqtype,COUNTER); break;
	case JSRP:  Setfield(seqop,5); Setfield(seqtype,BRANCH); break;
	case VECT:  Setfield(seqop,6); Setfield(seqtype,NONBRANCH); break;
	case CJRP:  Setfield(seqop,7); Setfield(seqtype,BRANCH); break;
	case LOUP:  Setfield(seqop,8); Setfield(seqtype,NONBRANCH); break;
	case RPCT:  Setfield(seqop,9); Setfield(seqtype,BRANCH); break;
	case RETN:  Setfield(seqop,10); Setfield(seqtype,NONBRANCH); break;
	case CJPP:  Setfield(seqop,11); Setfield(seqtype,SPECIAL); break;
	case LDCT:  Setfield(seqop,12); Setfield(seqtype,COUNTER); break;
	case TEST:  Setfield(seqop,13); Setfield(seqtype,NONBRANCH); break;
	case CONT:  Setfield(seqop,14); Setfield(seqtype,NONBRANCH); break;
	case THWB:  Setfield(seqop,15); Setfield(seqtype,BRANCH); break;
	}
}

/* ================================================================
/*	New MUL  DIV  micro-ops
/* ================================================================*/

MUL(r1,r2)
    Param r1,r2;
{
    Procname(MUL);
    SETROP(r1,NONE);  SETSOP(NONQOP,r2,RAMNOP);
    Setfield(i0,0);  Setfield(i1,0);  Setfield(i2,2);  Setfield(cin,0);
    YTODO(ALL16);
    _PropIn();
    COND(IFTRUE);
/* programmer add   SEQ(RPCT); NEXT(state); */
}

MULLAST(r1,r2)
    Param r1,r2;
{
    Procname(MULLAST);
    SETROP(r1,NONE);  SETSOP(NONQOP,r2,RAMNOP);
    Setfield(i0,0);  Setfield(i1,0);  Setfield(i2,6);  Setfield(cin,0);
    YTODO(ALL16);
    _MulDiv();
/* programmer add SEQ() and NEXT() */
}

DIVFIRST(r1,r2)
    Param r1,r2;
{
    Procname(DIVFIRST);
    SETROP(r1,NONE);  SETSOP(NONQOP,r2,RAMNOP);
    Setfield(i0,0);  Setfield(i1,0);  Setfield(i2,10);  Setfield(cin,0);
    YTODO(ALL16);
    _MulDiv();
}

DIV(r1,r2)
    Param r1,r2;
{
    Procname(DIV);
    SETROP(r1,NONE);  SETSOP(NONQOP,r2,RAMNOP);
    Setfield(i0,0);  Setfield(i1,0);  Setfield(i2,12);  Setfield(cin,0);
    YTODO(ALL16);
    _MulDiv();
    COND(IFTRUE);
/* programmer add SEQ(RPCT); NEXT(state); */
}

DIVCOR(r1,r2)
    Param r1,r2;
{
    Procname(DIVCOR);
    SETROP(r1,NONE);  SETSOP(NONQOP,r2,RAMNOP);
    Setfield(i0,0);  Setfield(i1,0);  Setfield(i2,14);  Setfield(cin,0);
    YTODO(ALL16);
    _MulDiv();
}


/*================================================================*/
/*  output generator
/*================================================================*/

generate()
{
Generate(NextAddress)
Generate(i0)
Generate(i1)
Generate(i2)
Generate(cin)
Generate(earbar)
Generate(ealbar)
Generate(addra)
Generate(addrb)
Generate(clklong)
Generate(get)
Generate(put)
Generate(loadout)
Generate(enram)
Generate(rdram)
Generate(highbyte)
Generate(rightjust)
Generate(seqop)
Generate(ccsel)
Generate(fbccode)
Generate(reverse)
}
