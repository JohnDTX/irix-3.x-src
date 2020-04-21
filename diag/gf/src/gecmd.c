/* gecmd.c
 *
 *	Geometry Engine pipeline commands and tests
 */

#include "pcmap.h"
#include "m68000.h"

#ifdef _GF1
#include "fbcld.h"
#include "gfdev.h"
#ifdef GFBETA

#define WAIT	0x73
#define REQ	0x63
#else
#define WAIT	0xf3
#define REQ	0xd3
#endif

#ifdef GFBETA
#define GEFLAGMASK 0xffff
#else
#define GEFLAGMASK 0x3fff
#endif

#include "../geofdef.h"

#define SINGLETESTS 1

extern char line[];	/* command line buffer */
extern short ix;	/* command line index */

extern short devstatus;	/* copy of currently written status reg */
extern short GEstatus;
extern char cmd,which,how;
unsigned short *outfile;
short verbose;

extern unsigned short testvecin[];
extern unsigned short testvecout[];
extern short testvi,testvo;
extern unsigned short testgsi[];
extern unsigned short testmm1[];
extern unsigned short testmm2[];
extern unsigned short testmm3[];
extern unsigned short testmm4[];
extern unsigned short testcl1[];
extern unsigned short testcl2[];
extern unsigned short testcl3[];
extern unsigned short testcl4[];
extern unsigned short testcl5[];
extern unsigned short testcl6[];
extern unsigned short testsc1[];
extern unsigned short testsc2[];
extern unsigned short testgso[];

extern unsigned short testpass[];
extern unsigned short drawtests[10][256];	/* drawtests.c	*/
extern unsigned short bustests[5][256];		/* bustests.c	*/

unsigned short *pgearray;
unsigned short *testin[] = 
	{0,
	testgsi,
	testmm1,
	testmm2,
	testmm3,
	testmm4,
	testcl1,
	testcl2,
	testcl3,
	testcl6,
	testsc1,
	testsc2};
unsigned short *testout[] = 
	{0,
	testmm1,
	testmm2,
	testmm3,
	testmm4,
	testcl1,
	testcl2,
	testcl3,
	testcl4,
	testsc1,
	testsc2};

extern short outx, evenreceived;

unsigned short geconfig[12] = {
	0,	/* dummy */
	0x809,
	0x70a,
	0x60b,
	0x50c,
	0x410,
	0x311,
	0x212,
	0x113,
	0x20,
	0xff21
};

short delaycount = 1000;

/*--------------------------------- */

ge()
{
short i,num,lim;
register	short	sendingreg;	/* d7 */
register	short	*GEaddr = (short *)&GEdata;	/* a5 */
unsigned short *j;

#ifdef GFALPHA
#define GE0flags GEflags
#endif

#ifndef FIFOINT
#define SGEdata(x)	{ short _ct=delaycount; \
			  *GEaddr = x; \
			  while( !(GE0flags & INREADY_BIT)) ; \
			  while (--_ct) ; }
	/*  note -- this is only relevant to ALPHA GE board. */
	/* note also -- written GE flags have to be changed in pipeline
		test macros to enable FIFO interrupts !
	 */
#else
#define SGEdata(x)	{ *GEaddr = x; }
#endif

switch (line[ix++]) {
   case '?': printf("   flag reg print\n   store flag reg <n>\n");
		printf("   data word send <n>\n   pipeline (507) test\n");
		printf("   PassThru test\n   test picture <n>\n");
		printf("   wait <n> between 'm' test words\n");
		printf("   multibus picture test <n>\n");
		printf("   initialize board\n   Configure pipe [<0..10>] chip isolated\n");
		printf("   overflow fifo test\n");
		printf("   loop PassThrus\n   1..10 isolated chip test\n");
		printf("   vector test\n");
	     break;
   case 'f': printf("%x\n",GEflags & GEFLAGMASK); break;
   case 's': GEflags = getnum(); break;
   case 'd': GEdata = getnum(); break;
   case 'v': pgearray = testvecin;
		outfile = testvecout;
		for (i=0; i<testvi; i++) SGEdata(*pgearray++)
		break;
   case 'p': pgearray = testgsi;
		outfile = testgso;
		while (*pgearray != GEOF) SGEdata(*pgearray++)
		break;
   case 'P':		/* 128 wds to fill both FIFOs  */
		pgearray = testpass;
		outfile = testpass;
		while (*pgearray != GEOF) SGEdata(*pgearray++)
		break;
   case 't': pgearray = drawtests[getnum()];
		while (*pgearray != GEOF) SGEdata(*pgearray++)
		break;
   case 'w': delaycount = getnum();
		break;
   case 'm': pgearray = bustests[getnum()];
		FBCflags = WAIT;
		while (*pgearray != GEOF)
		    {
			FBCdata = *pgearray++;
			FBCflags = REQ;
			FBCflags = WAIT;
			for (i=0; i<delaycount; i++) ;	/* wait a while */
		    }
		break;
   case 'i': 	GEstatus = GERESET3;
		FBCdisabvert(RUNMODE);
		devstatus = RUNMODE | 0x100;	/* fake disabvert indicator */
		outx = 0;
		break;
   case 'C': num = getnum();	/* 0 for default, 1-10 for specific chip */
		lim = getnum();   /* ditto, for last chip to configure	*/
		if (lim < num) lim = num;
		for (i=1; i<11; ++i)
		    {
			if ((i >= num && i <= lim) || num==0)
				gesend(geconfig[i]);
			else gesend(geconfig[i] & 0xff00 | 0x38);
		    }
		break;
    case 'D':   pgearray = drawtests[getnum()];
		while (*pgearray != GEOF) printf(" %04x ",*pgearray++);
		break;
    case 'o':
	GEdata = 8;	/* send down a word to fill the FIFOs	*/
	if (!(GE0flags & INREADY_BIT))
		printf("OK -- 2 errors follow:\n");
	break;
    case 'l': while (1) { SGEdata(8); SGEdata(0xfff7) }

    default:  --ix;
	num = getnum();		/* which chip's input?	*/
	if (num < 1 || num > 10) {illcmd(); return(0);}
	pgearray = testin[num];	/* set up ptr to input file	*/
	lim = getnum();		/* which chip's output?	*/
	if (lim < num) lim = num;	/* there's no going back	*/
	outfile = testout[lim];		/* set up ptr to comparison file */
	while (*pgearray != GEOF) SGEdata(*pgearray++)
   }
}

gesend(num)
   short num;
{
/*printf(" %x ",num);*/
GEdata = num;
}
