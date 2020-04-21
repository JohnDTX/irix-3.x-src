/* devload.c -- loader/console for devices --
		parent processor and frame buffer control   */

/*	device-independent code; inserts device-dependent .h file  */

/*#include "relocate.h"*/
#include "m68000.h"
#ifndef PM3
#  include "pcmap.h"
#  include "vectors.h"
#  define MOUSE  *(unsigned short *)0xfce000
#else
#  define u_short	unsigned short
#  include "cpureg.h"
#  define MOUSE M_BUT
#endif

#include "reentrant.h"
#include "fbcld.h"
#include "gfdev.h"

#define MAXOUT 2000
#define MAXERR 2000

extern char line[];		/* command input line buffer */
extern short ix;		/* index into ditto  */
extern unsigned short *outfile;
extern short interrupts;
extern short expectingtest;	/* gl2initall.c */

char prompt='!';
short errorlog[MAXERR][3];		
short errorct;
short interror;		/* interrupt routine error */
short intoccurred;		/* interrupt occurred */
short tokenint_occurred;
short expecting_interrupt;
short expecting_output;
char intcmd;		/* interrupt command */
char resp;		/* response char	*/
short intcount;
short intdata;			/* for ram test */
short intbuf[4097];		/* ditto */
short *pintdata;		/* ptr to expected interrupt data */
short devstatus;		/* global saved flags written */
short GEstatus;
short	firsttime;		/* pipeline sychronization hack. */
short	firstrun;		/* ditto. */
short evenreceived;
short randsave = 27988;
unsigned short savedata;
short flagstatus;	/* state of hostflag  */
short outx;
short fastest;		/* whether pgearray is invalid */
long vectors[256];	/* 68020 exception vectors moved here */
long savevbr;		/* saved vector basereg */

int charhandler();


main()
{
	extern GEServiceSub();
	extern loader_interrupt();
	extern dummy_int();
	int i;
	int good = 1;

/*RELOCATE;*/

interruptinit();
#ifdef PM3
vectorinit();
#else
/* IRQ5Vect = (int)dummy_int;*/		/* used before paul's charhandler */
IRQ4Vect = (int)GEServiceSub;
IRQ3Vect = (int)loader_interrupt;
#endif

#ifdef PM1
serialhandler(0,charhandler);
#endif
buzz(1000);

header();

devinit();
initmacros();
#ifdef GFALPHA
remap();
#endif
devcmd(1);	/* start interacting */
intlevel(7);
}


dummyfn()
{
/* this is to intercept & turn off character interrupts */
label(dummy_int);
	printf("resetting...\n");
	linereset(0);
	linedisarmrx(0);
	linedisarmtx(0);
	asm("	rte");
}

#ifdef PM3
reentrant(stray)
{}
#endif

reentrant(GEServiceSub)
{
int i;
extern unsigned short *pgearray;

#ifdef GF2
  if (!(FBCflags & TOKEN_BIT_)) {
	tokenint_occurred = 1;
	GEflags = GEstatus | ENABTOKENINT_BIT_;
	GEflags = GEstatus;
  }
#endif GF2

#ifdef GFALPHA
  if (GEflags & FIFOINT_BIT)
    {
    while (GEflags & FIFOINT_BIT)
	{
	    GEflags = GEstatus & ~ENABINPINT_BIT;  /* clear interrupt f.f. */
	    GEflags = GEstatus;	/* restore previous flags	*/
	}
   if (expecting_output) putchar('#');
    }
#else	/* GFBETA */
  if (GEflags & FIFOINT_BIT)
    {
	if (interrupts==0) {
		printf("GE fifo int  : ");
		printgflags();
		getchar();
		GEflags = GERESET3 | ENABFIFOINT_BIT_;
	}
	else {
		i = 100000;
		while (GEflags & HIWATER_BIT) {
			if (!(FBCflags & INTERRUPT_BIT_))
				loader_interrupt();
			if (--i<=0) {
				printf("  FIFO hung- new cmd level  ");
				devcmd(1);
			}
		}
		GEflags = GEstatus | ENABFIFOINT_BIT_;
		GEflags = GEstatus;
	}
    }
#endif GFBETA

  else if (GEflags & TRAPINT_BIT)
    {
	printf("GE trap   : ");
	printgflags();
#ifndef ICTEST
	if (!fastest) {
		for (i=0; i<19; i++) --pgearray;
		for (i=0; i<19; i++) {
			 printf("\n    %04x (%d)",*pgearray,*pgearray);
			 pgearray++;
		}
		getchar();
	}
#endif ICTEST
#if GFBETA | DEVEL | GF2
	GEflags = GEstatus = GEstatus | ENABTRAPINT_BIT_;
#endif
	interror = 999;
    }
  else
    {
	if (!tokenint_occurred) {
		printf("bad interrupt on GE level. GE=%x FBC=%x   ?",GEflags,
							FBCflags);
		interror = 999;
#ifdef PM2
		i = MOUSE;	/* read to clear button interrupt */
#endif
#ifndef ICTEST
		getchar();
#endif
	}
    }
}

reentrant(loader_interrupt)
  {
	register i;

	intoccurred++;
	interror = 0;
	if (FBCflags & INTERRUPT_BIT_) interror++;
	if (expecting_interrupt) {
	    switch (intcmd) {
		case 'g' : GFdisabvert(GEDEBUG,RUNMODE);
			devstatus = RUNMODE;
			break;
		case 'd' : GFdisabvert(GEDEBUG,RUNDEBUG);
			devstatus = RUNDEBUG;
			break;
		case 'r' : FBCclrint;		/* skip one interrupt */
			for (i=0; i<16; i++) {
				intbuf[intcount+i] = FBCdata;
				FBCclrint;
			}
			expecting_interrupt -= 15;  /* decs 1 more at end */
			break;
		case 'D' : 	/* dumpscratch */
			GFdisabvert(GEstatus,READOUTRUN);
			if (FBCdata != 0xc)
				printf("wrong interrupt!\n");
			for (i=0; i<16; i++) {
				GFdisabvert(GEstatus,RUNDEBUG);
				FBCclrint;
				if ((i%4)==0) printf("\n%x",intcount+i);
				GFdisabvert(GEstatus,READOUTRUN);
				printf("    %04x",FBCdata);
			}
			FBCflags = devstatus;
			putchar('\n');
			break;
		case 'x' :	/* expected by "gt" test */
			FBCflags = READOUTRUN;
			if (FBCdata != *pintdata)
				printf("bad feedback: wanted %x, got %x\n",
					*pintdata,FBCdata);
			++pintdata;
			FBCflags = devstatus;
		}
	    expecting_interrupt--;
	}

#ifdef NOTUSED
	else if (expectingtest)  {
		GFdisabvert(GEstatus,READOUTRUN);
		if (intcount != FBCdata) { 
			printf("fbc:%d 68000:%d\n",FBCdata,intcount);
		}
		intcount = FBCdata + 1;
		FBCflags = devstatus;
	}
#endif
	else if (expecting_output)
	    {
		if (intcmd=='a')	/* abort feedback results */
		    {
			while (!(FBCflags & INTERRUPT_BIT_)) {
				FBCclrint;
				buzz(5);
			}
			intcmd = expecting_output = 0;
		    }
		else	/* not aborting */
#if DEVEL || GF2
		    {
			/* first check for correct interrupt code */
			FBCclrint;
			intcount = FBCdata-2-expecting_output;
						/* get no. items to come */
			FBCclrint;
			errorct = 0;
			for (i=0; i<=intcount-1; i++) {
			    if ((outx>=0) && (outfile[outx] != FBCdata)) {
				++errorct;
				if (intcmd==0) continue;
				printf("item %d: rcv'd %x  exp'd %x  ('y' for more)",
					i+1,FBCdata,outfile[outx]);
				if (getchar()!='y') outx = -20000;
				putchar('\n');
			    }
			    ++outx;
			    FBCclrint;
			}
			printf("%d errors in %d words.\n",errorct,intcount);
			while (!(FBCflags & INTERRUPT_BIT_))
				FBCclrint;
			intcmd = 'a';
			expecting_output = 0;	/* signal done */
		    }
#else
		    {
			GFdisabvert(GEstatus,READOUTRUN);
			if (firstrun) { firsttime = firstrun = 0; }
			if (firsttime) firsttime = 0;
			if ((outfile[outx] != FBCdata) && outx>=0)
			    {
				errorlog[errorct][0] = outx;
				errorlog[errorct][1] = outfile[outx];
				errorlog[errorct][2] = FBCdata;
				if (errorct<MAXERR) errorct++;
			    }
			if (evenreceived && intcmd=='p')
				printf("%d:%x   ",outx,FBCdata);
			evenreceived = !evenreceived;
			if((evenreceived) && outx<MAXOUT) outx++;
			FBCflags = devstatus;
		    }
#endif DEVEL
	}	      /* end expecting_output */
	else {
	    if (!(FBCflags & NEWVERT_BIT_))
		printf("Vertical or strobe interrupt");
	    printf("\nInterrupt: cmd/printall/Print/quit/CR to cont.? ");
	    ix=0;
	    getlin(prompt);			/* get interrupt command */
	    switch(intcmd = line[0])  {
		case '\n': break;
		case 'c' : prompt = '>';
			   devcmd(1);
			   prompt = '!';
			   break;
		case 'q' :
#ifdef PM1
			 FBCflags = devstatus = 0xff;
			 restart();
#else
			expecting_output = 1;
			intcmd = 'a';
			break;
#endif
		case 'g' : GFdisabvert(GEDEBUG,RUNMODE);
			   devstatus = RUNMODE;
			   GEstatus = GEDEBUG;
			   ++expecting_interrupt;
			   break;
		case 'd' : 
			   while (!(FBCflags & INTERRUPT_BIT_)) FBCclrint;
			   hardinit();
			   devstatus = RUNDEBUG;
			   GEstatus  = GEDEBUG;
			   GFdisabvert(GEDEBUG,RUNDEBUG);
			   expecting_interrupt = 0;
			   expecting_output = 0;
			   break;
		case 'p' : while (!(FBCflags & INTERRUPT_BIT_)) {
				GFdisabvert(GEDEBUG,READOUTRUN);
				printf("%04x ",FBCdata);
				FBCflags = devstatus;	/* restore */
				FBCclrint;
			   }
			   putchar('\n');
			   break;
		case 'P' : GFdisabvert(GEDEBUG,READOUTRUN);
			   printf("%04x\n",FBCdata);
			   FBCflags = devstatus;	/* restore */
		} /* case */
	    } /* else */
	FBCclrint;	/* dismiss interrupt */
	FBCflags = devstatus;
  }

charhandler(charin)
    int charin;
{
    putchar('?');
#ifdef PM1
    if (charin == 3) restart();
#endif
}


cycle_input()
  {
	FBCflags = devstatus & ~FORCEREQ_BIT_ ;
	FBCflags = devstatus;
   }

cycle_output()
   {
	FBCflags = devstatus & ~FORCEACK_BIT_ ;
	FBCflags = devstatus;
   }


interruptinit()
{
errorct = 0;
interror = 0;		/* interrupt routine error */
intoccurred = 0;		/* interrupt occurred */
expecting_interrupt = 0;
expecting_output = 0;
expectingtest = 0;
intcount = 0;
firstrun = 1;
evenreceived = 1;
interrupts = 0;
fastest = 0;
intcmd = 0;
}

#ifdef PM3
vectorinit()
{
    register i;
    register long *lp;

    lp = (long *)getvbr();
    savevbr = getvbr();
    for (i=0; i<256; i++)
	vectors[i] = lp[i];	/* copy prom to array */
    setvbr(vectors);
    vectors[67] = (long)loader_interrupt;
    vectors[68] = (long)GEServiceSub;
    vectors[83] = (long)stray;
    *STATUS_REG |= ST_EINTR;
}
#endif
