/* testall.c
 *
 *	"canned" gf1 board test
 */

#include "m68000.h"
#include "gfdev.h"
#include "../geofdef.h"

extern char line[];
extern short ix, devstatus;
extern short expecting_interrupt, intcmd, interror;
extern short GEstatus;
unsigned short *infile,*outfile;
int filect;
extern unsigned short testgsi[];
extern unsigned short testgso[];
extern unsigned short test10gso[];
extern short delaycount;
extern short GEfound, GEmask;
extern short version;


testall()
{
    short i;

    for (i=0; i<20; i++) {
	initall('s');
	if (FBCdata == 0x40) break;
    }
    if (i==20) printf("reset error\n");

    for (i=0; i<10; i++) scratchtest();

#ifndef GFALPHA
/* test Multibus reg */
    FBCflags = 0xfb;
    for (i=1; i!=0; i<<=1) {
	FBCdata = i;
	if (FBCdata != i)
		printf("multibus r/w error: %04x should be %04x\n",
			FBCdata,(unsigned short)i);
    }

    FBCflags = READCODERUN;
    if (FBCdata & BPCACK_BIT)  {		/* if BPC ACK floating... */
	printf("no frame buffer\n");
	return(0);
    }

/* frame buffer present: draw pictures */

    initall('f');			/* perform "if" */
    delaycount = 0x420;
    ix=1;
    strcpy(line,"gm0");
    ge();
    buzz(10000);
    ix=1;
    strcpy(line,"gm2");
    ge();
    buzz(10000);
    ix=1;
    strcpy(line,"gm3");
    ge();
    ix=1;
    strcpy(line,"gm1");
    ge();
    buzz(40000);

/* test whether GE's (nulls OK) installed */

    GEflags = GERESET1;	/* attempt to reset GEs	*/
    if ((GEflags & 0x7ff8)!=0) {
	printf("GE(s) missing:\n");
	printgflags();
	return(0);	/* if any trap floating, quit	*/
    }

    hardinit();
    GFdisabvert(GERESET3,RUNMODE);
    findge();
    printf("found: %d   mask: %04x\n",GEfound,GEmask);
    configureGE(0,0);		/* all installed chips configured */

if (!(FBCflags & GEREQ_BIT_))	/* check for spurious request */
	printf("GEREQFBC not clear\n");

/* FBC input byte-swap test */

    GEdata = 0x1108;
    buzz(1000);
    FBCflags = 8;
    if (FBCdata != 0x1108)
	printf("pipeline or FBC inrjust error\n");
    FBCflags = 0xc;
    if (FBCdata != 0x0811) printf("FBC inljust error\n");

/* test pipeline & fifos */

    setup(7);
    testfifo(0);

/* now send the test vector */

    if (GEfound!=12 && GEfound!=10) {
	printf("not enough GE's -- skipping test\n");
	hardinit();
	return(0);
    }
    setup(7);
    configureGE(0,0);
    infile = testgsi;
    outfile = (GEfound==10) ? test10gso : testgso;
    filect = 0;
    while ( (*infile != GEOF) && gesend(*infile++) ) ++filect;
    if (*infile != GEOF) printf("    Error at word %d\n",filect);
    buzz(1000);

/* now feed it back (pollng) */

    filect = 0;
    FBCflags = devstatus = 0x63;
    FBCflags = 0x73;
    FBCflags = 0x63;

    while ( !(FBCflags & INTERRUPT_BIT_) && getfeedback() ) ++filect;

    printf("\n\nTests done. (%d vector words)\n",filect);
    hardinit();
#endif GFALPHA
}


getfeedback()	/* get results for 1 GEs output */
{
    short read1,read2;

    while (FBCflags & INTERRUPT_BIT_) ; /* wait for interrupt */
    FBCflags = READOUTRUN;
    read1 = FBCdata;
    FBCflags = devstatus;
    FBCclrint;
    while (FBCflags & INTERRUPT_BIT_) ; /* wait for interrupt */
    FBCflags = READOUTRUN;
    if ( (read2=FBCdata) != read1)
	printf("feedback error: %x <> %x\n",read1,read2);
    FBCflags = devstatus;
    FBCclrint;
    if (read1 != *outfile) {
	printf("Error on wd. %d:  %x should be %x\n", filect,
		read1, *outfile);
    }
    ++outfile;
    return(1);
}


buzz(n)
    int n;
{
	for ( ; n--; ) ;
}


setupfortest(pipe)
    char pipe;
{
	line[ix] = 'q';
	fifotest();			/* fq */
	initall(pipe);			/* ir or id */
	setup(2);			/* S  */
	strcpy(&line[ix],"o0-0-");
	fifotest();			/* fo0-0 */
	GEflags = 0x3f;
	buzz(100);
	GEflags = 0x26;
	delaycount = 1;
}


setup(level)
	short level;	/* ending interrupt level */
{		/* perform pipe test setup */
    register char c;

    hardinit();
    GFdisabvert(GERESET3,RUNMODE);
    GEstatus = GERESET3;
    intlevel(7);
    FBCdata = 0x3e;
    if (version>7)
	{
	    FBCflags = 0x63;
	    buzz(50);
	}
    FBCflags = 0x67;
    FBCclrint;
    FBCflags = devstatus = 0x35;
    if (FBCdata < 0x300)
	 printf("test setup failed\n");
    if (level!=7) intlevel(2);
}
