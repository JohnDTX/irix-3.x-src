/* initall.c  --- hardware initialization
 */

#include "m68000.h"
#include "gfdev.h"
#include "dcdev.h"
#define INTER2
#include "uctest.h"

extern char line[];
extern short ix, devstatus;
extern short GEstatus;
extern short delaycount;
#ifndef GFALPHA
extern short GEfound, GEmask;
#endif
short initdone = 0;	/* whether pipe init done */
short version = 0;	/* microcode version no. */
short interrupts = 0;	/* whether FIFO interrupts enabled */

initall(how)
	char how;
{
   register short i;

   interruptinit();
   GEstatus = GEDEBUG;
   devstatus = RUNDEBUG;	/* defaults */
   if (how!='?') i = hardinit();
   buzz(100);

   switch(how) {
   case '?':	inithelp();
		return(0);
   case 'b':
		GEstatus = GEDEBUG & ~SUBSTBPCCODE_BIT_;
		devstatus = RUNSUBST;
		delaycount = 0x420;
		mapinit();
		break;
   case 'f':	mapinit();
   case 'd':
   case 'D':
   case 's':
		delaycount = 0x420;
		break;
   case 'r':
		GEstatus = GERESET3;
				/* careful! trap interrupts turned on	*/
#ifdef GFALPHA
		devstatus = RUNMODE | ENABVERTINT_BIT_;
#else
		devstatus = RUNMODE;
#endif
		delaycount = 1;
		mapinit();
#ifndef GFALPHA
		findge();
		printf("found: %d   mask: %04x\n",
			GEfound,(unsigned short)GEmask);
#endif
		i = hardinit();
		initdone = 1;
		break;
#ifdef GFBETA
    case 'i':
		mapinit();
    case 'I':
		GEstatus = GERESET3 & ~ENABFIFOINT_BIT_;
		devstatus = RUNMODE;
		delaycount = 1;
		findge();
		printf("found: %d   mask: %04x\n",
			GEfound,(unsigned short)GEmask);
		initdone = 1;
		interrupts = 1;
		i = hardinit();
		break;
#endif
#ifdef GF2
    case 't':	inittest();
		return(0);
#endif
   case 'a':	initall('r');	/* note recursion */
		configureGE(13,0,0);	/* passthru */
		GEdata = 8;
		GEdata = 1;	/* illegal FBC command to gobble inputs */
		break;
   default:
		GEstatus = GERESET3;	/* like 'r' but no findge or mapinit*/
#ifdef GFALPHA
		devstatus = RUNMODE;
#else
		devstatus = RUNMODE | ENABVERTINT_BIT_;
#endif
		delaycount = 1;
		findge();
		printf("found: %d   mask: %04x\n",
			GEfound,(unsigned short)GEmask);
		initdone = 1;
   }

   printf("scratchsize: %04x    version: %d.%d\n",i,version>>8,version&0xff);
#ifdef GF2
   if ((FBCdata == 0x40) && (how=='r')) inittables();
#endif
   GFdisabvert(GEstatus,devstatus);
   intlevel(2);
}


mapinit()	/* init color map */
{
   register short mapindex=0;
   short i;

#define ColorMap(red,grn,blu) DCMapColor(mapindex,(red),(grn),(blu));++mapindex

DCflags = DCBUSOP;
ColorMap(0,0,0);	/* black, white, primaries, secondaries into map */
ColorMap(-1,-1,-1);
ColorMap(-1,0,0);
ColorMap(0,-1,0);
ColorMap(0,0,-1);
ColorMap(-1,-1,0);
ColorMap(0,-1,-1);
ColorMap(-1,0,-1);
ColorMap(-1,0,0);
for (mapindex = 8; mapindex++ <255;)	/* maps 9 - 255 */
	DCMapColor(mapindex,mapindex,mapindex,mapindex);
DCMapColor(255,-1,0,0);	
DCflags = DCMULTIMAP;
#ifdef UC4
*UCRAddr = 0;
#endif
}


hardinit()	 /* returns scratch ram size (beta) ; sets version */
{
	short i;

intlevel(7);
GEflags = GERESET1;
FBCflags = STARTDEV;
FBCflags = STARTDEV & ~FORCEREQ_BIT_;
FBCflags = STARTDEV & ~FORCEACK_BIT_;
FBCflags = STARTDEV;
FBCdata = 0;
FBCclrint;		/* execute instruc. 3ff */
FBCflags = RUNDEBUG;
for (i=0; ++i<20;) FBCclrint;
FBCflags = READOUT;
i = FBCdata;
FBCflags = RUNDEBUG;
intlevel(2);
GEflags = GEDEBUG;
version = 9;
return(i);		/* return scratch size */
}


inittest()
{
	UCTEST_INIT
}

inithelp()
{
		printf("   abort! try to unhang\n");
		printf("   run\n   bpc test\n   standalone\n");
		printf("   dummy FIFOs\n");
		printf("   framebuf (BPC) present\n");
		printf("   interrupts enabled\n");
		printf("   Interrupts enabled, no BPC\n");
#ifdef GF2
		printf("   test FBC_INIT\n");
#endif
		printf("   <CR> GE testing\n");
}
