/* testall.c
 *
 *	"canned" gf1 board test
 */

#include "m68000.h"
#include "gfdev.h"
#include "geofdef.h"

extern char line[];
extern short ix, devstatus;
extern short expecting_interrupt, intcmd, interror;
extern short expecting_output;
extern short GEstatus;
unsigned short *infile,*outfile;
int filect;
extern short expectingtest;
extern unsigned short testgsi[];
extern unsigned short testgso[];
extern unsigned short test10gso[];
extern short delaycount;
extern short GEfound, GEmask;
extern short version;
extern short fastest;

#define FBCfeedback	0x25
#define FBCsend(d)	FBCdata = (d); \
			GFdisabvert(GERESET3|ENABTRAPINT_BIT_,CYCINDEBUG); \
			GFdisabvert(GERESET3|ENABTRAPINT_BIT_,RUNDEBUG);

testall()
{
    short i;

/* test Multibus reg */

    printf("Multibus: ");
    FBCflags = 0xfb;
    for (i=1; i!=0; i<<=1) {
	FBCdata = i;
	if (FBCdata != i)
		printf("multibus r/w error: %04x should be %04x\n",
			FBCdata,(unsigned short)i);
    }

/* test micro ram */

    printf("micro ram: ");
    ramtest(0x4000);
    rewrite();

/* initialization */

    printf("init: ");
    i=0;
    while (FBCdata != 0x40) {
	initall('s');
	if (++i < 20)
	    if (FBCdata != 0x40)
		printf("reset error\n");
    }
    for (i=0; i<5; i++) scratchtest();

#ifdef GF2
    if (FBCflags & BPCACK_BIT)  {		/* if BPC ACK floating... */
	printf("no frame buffer\n");
	return(0);
#else
    FBCflags = READCODERUN;
    if (FBCdata & BPCACK_BIT)  {		/* if BPC ACK floating... */
	printf("no frame buffer.\n");
	return(0);
#endif
    }

/* frame buffer present: test interface */

    printf("BPC i/f: ");
    bpciftest();		/* "bt" */

/* draw pictures */

    printf("draw: ");
    initall('f');			/* perform "if" */
#ifdef PM3
    delaycount = 0x840;
#else
    delaycount = 0x420;
#endif
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
    buzz(30000);

/* test whether GE's (nulls OK) installed */

    GEflags = GERESET1;	/* attempt to reset GEs	*/
    if ((GEflags & 0x7ff8)!=0) {
	printf("GE(s) or passer(s) missing:\n");
	printgflags();
	return(0);	/* if any trap floating, quit	*/
    }

    hardinit();
    GFdisabvert(GERESET3,RUNMODE);
    findge(1);
    printf("found: %d   mask: %04x\n",GEfound,GEmask);
    GFdisabvert(GERESET3,RUNMODE);
    configureGE(0,0,0);		/* all installed chips configured */

    buzz(10000);

if (!(FBCflags & GEREQ_BIT_))	/* check for spurious request */
	printf("GEREQFBC not clear\n");

/* FBC input byte-swap test */

    printf("byteswap: ");
    GEdata = 0x1108;
    buzz(1000);
    FBCflags = 8;
    if (FBCdata != 0x1108)
	printf("pipeline or FBC inrjust error - %x\n",FBCdata);
    FBCflags = 0xc;
    if (FBCdata != 0x0811) printf("FBC inljust error\n");

/* test pipeline & fifos */

    if ((GEmask & 0x1ef6) != 0x1ef6) {
	printf("skipping pipe tests.\n");
	hardinit();
	return(0);
    }
    tokentest();
    printf("fifo test: ");
    setup(7);
    testfifo(0);

/* now send the test vector */

    printf("GE test: ");
    setup(7);
    configureGE(0,0,0);
    infile = testgsi;
    outfile = ((GEmask&0x1ffe)==0x1ffe) ? testgso : test10gso;
    filect = 0;
    while ( (*infile != GEOF) && gesend(*infile++) ) ++filect;
    if (*infile != GEOF) printf("    Error at word %d\n",filect);
    buzz(1000);

/* now feed it back (pollng) */

    filect = 0;
    FBCflags = devstatus = RUNDEBUG;
    FBCsend(EOF1);
    FBCsend(EOF2);
    FBCsend(EOF3);
    buzz(50);
    if (FBCflags & INTERRUPT_BIT_) {
	printf("no feedback from GE test");
	getchar();
    }
    FBCclrint;	/* skip int code */
    buzz(1);
    FBCclrint;	/* skip wordcount */
    buzz(1);

    while ( !(FBCflags & INTERRUPT_BIT_) && getfeedback() ) ++filect;

    printf("\n\nTests done. (%d vector words)\n",filect);
    hardinit();
}


getfeedback()	/* get results for 1 GEs output */
{
    short read1;

    while (FBCflags & INTERRUPT_BIT_) ; /* wait for interrupt */
    read1 = FBCdata;
    FBCclrint;
    if (*outfile==GEOF) return(1);
    if (read1 != *outfile) {
	printf("Error on wd. %d:  %x should be %x  (q to quit)", filect,
		read1, *outfile);
	if (getchar() =='q') return(0);
	putchar('\n');
    }
    ++outfile;
    return(1);
}


buzz(n)
    int n;
{
#ifdef PM3
	n <<= 1;
#endif
	while (n-- >0) ;
}


setupfortest(pipe)		/* S command */
    register char pipe;
{
	switch (pipe) {
	    case '?': inithelp(); return(0);
	    case 'D':
	    case 'd': pipe = 's';
	    case 'i':
	    case 'I': break;
	    default:  pipe = 'X'; break;	/* no UC */
	}
	initall(pipe);			/* ir or id */
	setup(2);			/* S  */
	GEflags = GERESET1;
	buzz(100);
	GEflags = GEstatus = GERESET3;
	delaycount = 1;
	fastest = 0;
	gareset();
}

setup(level)
	short level;	/* ending interrupt level */
{		/* perform initial pipe test setup */
    hardinit();
    intlevel(7);
    dosetup();
    if (level!=7) intlevel(2);
}

dosetup()	/* used by fifotest */
{
    expecting_output = 1;
    FBCsend(FBCfeedback);
    if (FBCdata != FBCfeedback)
	 printf("test setup failed: %x\n",FBCdata);
#ifdef GF2
    FBCflags = devstatus = RUNMODE;
#else /* devel */
    FBCflags = devstatus = 0xe1;	/* hardware-dependent! */
#endif
    expectingtest = 1;
}
