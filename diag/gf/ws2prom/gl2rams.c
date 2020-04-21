/* gl2rams.c   ---  fbc ram tests for GL2 microcode
 *
 * define
 *	GFALPHA:	reflects ALPHA FBC and newest diagnostic microcode
 *	GF2:		reflects new GF and microcode
 */

#include "gums.h"
#include "gl2cmds.h"

extern unsigned short _ucode[][4];
extern short devstatus;
extern short GEstatus;
short randsave;
extern char ucerr();
short intcount;
short ramerrs;
extern short Verbose;

int scratchtest()
{
	register short size;
	int scratcherrs = 0;

	if ((size = scratchsetup())==0) return(1);
						/* get scratch size  */
	if (Verbose) printf("testing %d words: ",size);
	scratcherrs += scratchloop(size,0x5555,0xaaaa);
	scratcherrs += scratchloop(size,0xaaaa,0x5555);
	scratcherrs += scratchrand(size);
	hardinit();
	if (Verbose) printf("scratch test done.\n");
	return(scratcherrs);
}


int scratchloop(siz,even,odd)
	short siz,even,odd;
{
	register i,j;
	fsetup;

	if (scratchsetup()==0) return(1);

    for (i=0; i < siz; )	{	/* write loop */
	fshort(even);
	i++;
	fshort(odd);
	i++;
    }

    for (intcount = 0; intcount < siz; intcount+=16) {	/* read loop */
	fshort(FBCdumpram);
	fshort(intcount);
	buzz(50);	/* wait for interrupt */
	if (FBCflags & INTERRUPT_BIT_) {
		printf("no feedback\n");
		return(1);
	}
	for (j=0; j<16; j++) {
	    FBCclrint;
	    if (even != FBCdata) screrr("data",intcount+j,even);
	    FBCclrint;
	    ++j;
	    if (odd != FBCdata) screrr("data",intcount+j,odd);
	}
	FBCclrint;
    }
    return(ramerrs);
}


scratchrand(siz)
	short siz;
{
	register i,j;
	register short rand;
	fsetup;

/* perform scratchsetup first */
	ramerrs = 0;
	if (scratchsetup()==0) return(1);

    rand = randsave;
    for (i=0; i < siz; i++) {	/* write loop */
	rand = rand*4 + rand + 17625;
	fshort(rand);
    }

    rand = randsave;
    for (intcount = 0; intcount < siz; intcount+=16) { /* read loop */
	fshort(FBCdumpram);
	fshort(intcount);
	buzz(50);
	if (FBCflags & INTERRUPT_BIT_) {
		printf("no feedback\n");
		return(1);
	}
	for (j=0; j<16; j++) {
	    rand = rand*4 + rand + 17625;
	    FBCclrint;
	    if (rand != FBCdata) screrr("addr",intcount+j,rand);
	}
	FBCclrint;
    }
    randsave = rand;
    return(ramerrs);
}


scratchsetup()
{
   register short i;
   register j;
   fsetup;

	i = hardinit();
	i++;		/* i is now 2<<n  */

	GFdisabvert(GERESET3|ENABTRAPINT_BIT_|ENABFBCINT_BIT_,RUNDEBUG);
	devstatus = RUNDEBUG;

	ramerrs = 0;
	if (Verbose) putchar('<');
	fshort(FBCloadram);		/* command code for testing */
	if ((FBCdata <0x200) || (FBCdata >0x400))  {
		printf("microcode setup failure: loc %x\n",FBCdata);
		return(0);
	}
	fshort(0); 	/* starting scratch adr */
	fshort(i);	/* no. wds */
	return(i);
}


/*=====================================================================*/
/*		MICROCODE STUFF						*/

int ramtest(lim)	/* microcode ram test */
	short lim;
{
	int errs = 0;

	errs += testdata(0,0);
	errs += testdata(0xffff,0xffff);
	errs += testdata(0x5555,0xaaaa);
	errs += testdata(0xaaaa,0x5555);
	errs += testadr(lim);
	GEflags = GEstatus;
	return(errs);
}


int testdata(eval,oval)		/* microcode ram: data test */
	short eval,oval;
{
	short i,got,val,err=0;
	short slice = 0;
	register short mask = 0xffff;

	FBCflags = devstatus = WRITEMICRO;
	for (slice=0; slice<32; slice++) {
		FBCmicroslice(slice%4,slice<<9);
		for (i=0; i<512; i+=2) {
			FBCmicrocode(i) = eval;
			FBCmicrocode(i+1) = oval;
		}
	}
	FBCflags = devstatus = READMICRO;
	for (slice=0; slice<32; slice++) {
		FBCmicroslice(slice%4,slice<<9);
		if ((slice%4)==3) mask = 0xff;
		else mask = 0xffff;
		for (i=0; i<512; i++)
		   if (( (got = FBCmicrocode(i)&mask)
			- (val = i&1 ? oval : eval)   ) & mask ) {
			if (ucerr("data",slice,i,val,got)=='q') return(err);
			err++;
		   }
	}
	return(err);
   }


int testadr(lim)	/* microcode adressing test */
	short lim;
   {
	register i;
	short got,val,rand,err=0;
	short slice = 0;
	register short mask;

	rand = randsave;
	FBCflags = devstatus = WRITEMICRO;
	for (i=0; i<lim; i++) {
		FBCmicroslice((i>>12)&3,i);
		FBCmicrocode(i) = rand;
		rand = rand*4 + rand + 17625;
	}
	rand = randsave;
	FBCflags = devstatus = READMICRO;
	for (i=0; i<lim; i++) {
		FBCmicroslice((i>>12)&3,i);
		if ((i&0x3000)==0x3000) mask = 0xff;
		else mask = 0xffff;
		if (( (got = FBCmicrocode(i))
			- (val = rand)		) & mask) {
		    if (ucerr("address",slice,i,val,got)=='q') return(err);
		    err++;
		}
		rand = rand*4 + rand + 17625;
	}
	randsave = rand;
	return(err);
   }


rewrite()	/* write and verify microcode */
{
	writestore();
	verifystore();
}

writestore()
{
  register i;
  register wd;
  register short mask;
  short slice,got;

  FBCflags = devstatus = WRITEMICRO;

	for (i=0; i<0x4000; i++) {
		wd = (i>>12) &3;
		FBCmicroslice(wd,i);
		FBCmicrocode(i) = _ucode[i&0xfff][wd];
	}
	FBCflags = devstatus = RUNMODE;
}

int verifystore()
{
  register i;
  register wd;
  register short mask;
  short slice,got;
  int errs = 0;

#ifdef GF2
	FBCflags = devstatus = READMICRO;
	mask = 0xffff;
	for (i=0; i<0x4000; i++) {
		wd = (i>>12) &3;
		FBCmicroslice(wd,i);
		if (wd==3) mask = 0xff;
		if ( (got = FBCmicrocode(i)&mask)
			!= _ucode[i&0xfff][wd]       ) {
			printf("readback error: adr=%x got %x\n",
				i,got);
			if (++errs > 20) return(errs);
		}
	}
	if (Verbose) printf("OK\n");
	FBCflags = devstatus = RUNMODE;
	return(0);
#endif
}

char ucerr(typ,slice,adr,expd,got)	/* returns response */
	char *typ;
	short slice;
	short adr;
	unsigned short expd,got;
{
	register char c;
	register i;

	i = ((slice<<9)+adr)&0xfff;
	slice = slice%4;
	printf("\n%s error, slice=%d, adr=%03x exp'd %04x got %04x",
		typ,slice,i,expd,got);
	c = getchar();
#ifdef PM1
	if (c==3) restart();
#endif
	return(c);
}

testwrite()
{
	int i;

  if ((i=Micro_Write(_ucode,0,4096)) >=0)
	printf("error at %x\n",i);
}


screrr(type,adr,wanted)
	char *type;
	short adr;
	short wanted;
{
	if (++ramerrs < 16)
		printf("%s error at adr=%x, wanted %x  got %x\n",
			type,adr,wanted,FBCdata);
}
