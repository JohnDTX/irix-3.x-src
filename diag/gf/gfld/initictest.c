/* initictest.c  --- IC test low-level routines
 */

#ifdef GFALPHA
#define SUBSTBPCCODE_BIT_ 0
#define RUNSUBST	0
#endif

#include "/usr/sun/include/m68000.h"
#include "gfdev.h"
#include "dcdev.h"
#include "../../geofdef.h"

extern char line[];
extern short ix, devstatus,which;
extern char intcmd;
extern short outx, evenreceived, expecting_output, expecting_interrupt;
extern short errorct, firsttime, interror;
extern unsigned short errorlog[][3];
extern short *outfile;
extern short testpass[];
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
extern unsigned short testgso[];

unsigned short configs[] = {
	0,
	0xff09,
	0xff0a,
	0xff0b,
	0xff0c,
	0xff10,
	0xff11,
	0xff12,
	0xff13,
	0xff14,
	0xff15,
	0xff20,
	0xff21
};
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
	testcl4,
	testcl5,
	testcl6,
	testsc1,
	testgso};
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
	testcl5,
	testcl6,
	testsc1,
	testgso,
	0};

short *infile;
short verbose = 1;
char response;
short waitcount = 0x40;

#define MAXOUT 2000
#define SendSetup	register short *GEaddr = (short *)&GEdata; \
			register short waitct
#define SendWord(x)	{ short _ct=waitcount; \
			  *GEaddr = x; \
			  while (--_ct >0); \
			  while (!(FBCflags & INREQ_BIT_) ) ; }


setup()
{
	short i;

    i = hardinit();
    if ( (i!= 0x7ff) && (i!= 0xfff) )  {	/* check for scratch size */
	putchar(7);
	printf("can't reset: returned %x\n",i);
	return(0);
    }
    if (!(FBCflags & GEREQ_BIT_))  {	/* check for spurious request */
	putchar(7);
	printf("reqout not clear\n");
	return(0);
    }
/* set up FBC for test mode */
    
    intcmd = 'f';
    interror = 0;
    expecting_interrupt = 1;
    FBCdata = 0x3e;	/* DBLFEED command */
    FBCflags = 0x67;	/* interrupt should happen here */
    FBCflags = 0x35;
    if (FBCdata < 0x300) {
	putchar(7);
	printf("test setup failed\n");
	return(0);
    }
    expecting_output = 1;
    firsttime = 1;
    outx = 0;
    evenreceived = 1;
    errorct = 0;
    return(1);
}


passtest(flags)
    short flags;	/* enable interrupts or not */
{
	SendSetup;

    if (!carefultest()) return(0);	/* first try sending thru anything */

    if (!setup()) return(0);	/* always reset the FBC */
    GEflags = flags;

/* configure the pipe */
/* just the DUT for starters */

    SendWord(0xff38);

/* send the test vector */

    runtestvector(testpass,testpass);
}


filltest(flags)		/* for testing fifo */
    short flags;	/* enable interrupts or not */
{
	SendSetup;
	register short *rinf = testpass;
	short i;

    if (!carefultest()) return(0);	/* first try sending thru anything */

    if (!setup()) return(0);	/* always reset the FBC */
    GEflags = flags;

    SendWord(0x3f00);			/* configure fifo */
    FBCflags = 0x75;			/* set the FBC flag block */
    outfile = testpass;
    for (i=0; i<65; i++) testsend(*rinf++);	/* send the test vector */
    FBCflags = 0x35;			/* unblock */
    for (i=0; i<1000; i++) ;		/* wait til FBC gets all words */
    report();
}


singletest(n)
    short n;
{
	SendSetup;

    if (!setup()) return(0);
    GEflags = GERESET3;

    if ( (n<=0) || n>12) {
	printf("bad chip number.\n");
	return(0);
    }
    SendWord(configs[n]);
    runtestvector(testin[n],testout[n]);
}


reptest(n)
    short n;
{
	SendSetup;

    if ( (n<=0) || n>12) {
	printf("bad chip number.\n");
	return(0);
    }

    response = 'y';
    while ( (interror != 999) && (response=='y') ) {
	if (!setup()) return(0);
	GEflags = GERESET3;
	SendWord(configs[n]);
	runtestvector(testin[n],testout[n]);
    }
}


carefultest()
{
    short i;

    if (!setup()) return(0);
    if (testsend(0xff38)==0)	/* set up as passer */
	{
		printf("   - config word\n");
		return(0);
	}
    for (i=0; i<32; i++)
	    if (testsend(8)==0)		/* carefully send 8's */
		{
			printf("   - dataword %d\n",i+1);
			return(0);
		}
    return(1);
}


runtestvector(inf,outf)
	short *inf,*outf;
{
	SendSetup;
	register short *rinfile;

    rinfile = inf;
    outfile = outf;
    while (*rinfile != GEOF) SendWord(*rinfile++)

    report();		/* feed back and test results */
}


report()
{
    short i,save;

/* wait for last thing to clear the GE pipe */
    for (i=1000; i; --i) ;

/* feed back the outputs */

   FBCflags = devstatus = 0x63;
   FBCflags = 0x73;
   FBCflags = 0x63;

/* wait for everything to feed back */

    save = -1;
    while (save != outx)
	{
	    save = outx;
	    for (i=100; i; --i) ;
	}

    if (outx >= MAXOUT) {
	intlevel(7);
	printf("\007too many outputs\n");
	return(0);
    }

/* report the errors */

    printf("%d errors in %d words.\n",errorct>>1,outx);
    if (verbose)  {
	if (errorct > 0)
	    printf("Press 'y' for each logline desired.\n");
	i = 0;
	while (errorct-- > 0 && ((response=getchar()) == 'y'))
	    {
		printf("item %d bad: %x (%d) expected, %x (%d) received\n",
			errorlog[i][0],errorlog[i][1],errorlog[i][1],
			errorlog[i][2],errorlog[i][2]);
		i++;
	    }
    }
}


hardinit()	 /* returns scratch ram size (beta) */
{
	short i;

intlevel(7);
GEflags = GERESET1 ;
FBCflags = STARTDEV;
#ifdef HYBRID
GE0flags = 0;
for (i=0; ++i<100;)
GE0flags = 0xf;
#endif
FBCflags = STARTDEV & ~FORCEREQ_BIT_;
FBCflags = STARTDEV & ~FORCEACK_BIT_;
FBCflags = STARTDEV;
FBCdata = 0;
FBCclrint;		/* execute microinstruc. 0x3ff	*/
FBCflags = RUNDEBUG;
for (i=0; ++i<20;) FBCclrint;
FBCflags = READOUT;
i = FBCdata;
FBCflags = RUNDEBUG;
intlevel(2);
GEflags = GEDEBUG;	/* no interrupts enabled yet */

return(i);
}


testsend(x)
    short x;
{
    SendSetup;

    GEdata = x;
    waitct = 0x100;
    while ( !(FBCflags & INREQ_BIT_) && (--waitct) ) ;
    if (waitct==0) {
	printf("\007no input acknowledge\n");
	return(0);
    }
    return(1);
}

verbtoggle()
{
    verbose = 1-verbose;
}
