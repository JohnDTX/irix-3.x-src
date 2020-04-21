/* gf2init.c
 *
 *   Micro_Write(array,basestate,nstates)  unsigned short *array; int states;
 *	writes microcode from array, starting at basestate in microram for
 *		nstates states.
 *   FBC_Reset()
 *	foolproof reset sequence
 *	returns 0x40 if OK
 *   Get_Micro_Version()
 *	returns microcode version MM.mm  MM=major  mm=minor
 *   Table_Init(scratchadr, nwds, &table)
 *	initializes nwds of scratch ram at scratchadr from table
 */

#include "gf2.h"
#include "gl2cmds.h"

#define FBCsend(n)		{ FBCdata = (n);	\
				  FBCflags = CYCINDEBUG; \
				  FBCflags = RUNDEBUG;	\
				}

FBC_Reset()
{
	short _ii,_jj;
	register splsave;

	asm("	movw	sr,d7");
	asm("	orw	#/0700,sr");	/* spl7 */
	GEflags = GERESET1;
	FBCflags = STARTDEV;
#ifdef BADSTUFF
	for (_ii=10000; _ii; --_ii)		/* wait for UC to finish */
		if (!(FBCflags & BPCACK_BIT)) break;
	if (_ii<=0) return(0xbadc);
#endif
	for (_ii=0; _ii<50; _ii++) ;
	FBCflags = STARTDEV & ~FORCEREQ_BIT_;
	FBCflags = STARTDEV & ~FORCEACK_BIT_;
	FBCflags = STARTDEV;
	FBCclrint;
	FBCflags = RUNDEBUG;
	FBCdata = 8;
	for (_ii=0; _ii<20; _ii++) {
		for (_jj=0; ++_jj<50; ) ;
		FBCclrint;
	}
	FBCflags = READOUTRUN;
	_ii = FBCdata;
	GEflags = GEDEBUG;
	FBCflags = RUNDEBUG;
	asm("	movw	d7,sr");	/* restore pl */
	return(_ii);
}

Get_Micro_Version()
{
	short _ii;

	FBCdata = 7;
	for (_ii=0; ++_ii<50; ) ;
	FBCflags = RUNDEBUG & ~FORCEREQ_BIT_;
	FBCflags = RUNDEBUG & ~FORCEACK_BIT_;
	FBCflags = READOUTRUN;
	_ii = FBCdata;
	FBCflags = RUNDEBUG;
	return(_ii);
}

/*
 *	write microcode onto GL2 board
 *	returns -1 if writes correctly
 *	returns failing address otherwise { 0xSWWW : S=slice W=word adr}
 */
Micro_Write(code,basestate,nstates)
	unsigned short code[][4];
	int basestate;
	int nstates;
{
  register i;
  register wd;
  register short mask;
  short got;
  int splsave;

	splsave = spl7();
	nstates += basestate;
	FBCflags = WRITEMICRO;

	for (wd = 0; wd < 4; wd++)
	    for (i=basestate; i<nstates; i++) {
printf("%x ",code[i-basestate][wd]);
		FBCmicroslice(wd,i);
		FBCmicrocode(i) = code[i-basestate][wd];
	    }

	FBCflags = READMICRO;
	mask = 0xffff;
	for (wd = 0; wd < 4; wd++)
	    for (i=basestate; i<nstates; i++) {
		FBCmicroslice(wd,i);
		if (wd==3) mask = 0xff;
		if ( (got = FBCmicrocode(i)&mask)
			!= code[i-basestate][wd]       ) return(i);
	}
	FBCflags = RUNMODE;
	splx(splsave);
	return(-1);
}

Table_Init(adr,nwds,ptr)
	unsigned short adr;
	register nwds;
	register unsigned short *ptr;
{
	FBCflags = RUNDEBUG;
	FBCsend(FBCloadram)
	FBCsend(adr)
	FBCsend(nwds)
	while (nwds-- >0)
		FBCsend(*ptr++)
}
