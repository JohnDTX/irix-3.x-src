/* initall.c  --- hardware initialization
 */

#include "m68000.h"
#include "gfdev.h"
#include "dcdev.h"
#ifdef UC4
#include "ucdev.h"
#define INTER3
#include "uc4test.h"
#include "ucscreen.h"
#endif

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
short dcr = DCMULTIMAP;
short expectingtest;


int initall(how)
	char how;
{
   register short i;
   short silent = 0;

   interruptinit();
   expectingtest = 0;
   GEstatus = GEDEBUG;
   devstatus = RUNDEBUG;	/* defaults */
   if (how!='?') i = hardinit();
   buzz(100);

   switch(how) {
   case '?':	inithelp();
		return(0);
   case 'b':
		silent = 1;
		GEstatus = GEDEBUG & ~SUBSTBPCCODE_BIT_;
		devstatus = RUNSUBST;
		delaycount = 0x420;
		mapinit();
		break;
   case 'F':	silent = 1;
   case 'f':	mapinit();
   case 'd':
   case 's':
		delaycount = 0x420;
		break;
   case 'S':	silent = 1;
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
		findge(1);
		printf("found: %d   mask: %04x\n",
				GEfound,(unsigned short)GEmask);
#endif
		i = hardinit();
		initdone = 1;
		break;
#if GFBETA || GF2
    case 'i':
		mapinit();
    case 'I':
		GEstatus = GERESET3 & ~ENABFIFOINT_BIT_;
		devstatus = RUNMODE;
		delaycount = 1;
		findge(1);
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
		findge(1);
		printf("found: %d   mask: %04x\n",
			GEfound,(unsigned short)GEmask);
		initdone = 1;
   }

   if (!silent)
	printf("scratchsize: %04x    version: %d.%d\n",
		i,version>>8,version&0xff);
#ifdef GF2
   if ((FBCdata == 0x40) && (how=='r')) inittables();
#endif
   GFdisabvert(GEstatus,devstatus);
   intlevel(2);
#ifdef GF2
   if ((FBCdata == 0x40) && (i==0xfff) && ((version>>8)==2))
	return(0);	/* no errors */
   return(-1);
#endif
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
DCflags = dcr;
#ifdef UC4
*UCRAddr = 0;
#endif
}


hardinit()	 /* returns scratch ram size (beta) ; sets version */
{
	short save = GEstatus;
	short i;

/* assumes no UC or GE's */

i = FBC_Reset();

GEflags = GEstatus = GEDEBUG;

/* get microcode version by executing a clearhitmode */

if (version == 0)	/* if version not yet set */
    {
	version = Get_Micro_Version();
	if ((version&0xff)==0xff) version = 0x107;
    }
GEstatus = save;	/* restore intended GE flags */
intlevel(2);
return(i);		/* return scratch size */
}


inittest()
{
   printf("reset = %x\n",FBC_Reset());
   printf("version = %x\n",Get_Micro_Version());
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

#ifdef UC4
initscreen()
{
    UC4setup;
    register curint;

    curint = spl7();
    FBC_Reset();
    splx(curint);

    *UCRAddr = UCR_BOARDENAB + UCR_MBENAB;
    LDMODE(UCMODE);
    LDCONFIG(CONFIG);
    LDXS(0);
    LDYS(0);
    LDXE(0x3ff);
    LDYE(0x3ff);
    REQUEST(UC_SETSCRMASKX,0)
    REQUEST(UC_SETSCRMASKY,0)
    REQUEST(UC_SETWEAB,IRISWE)
}
#endif UC4
