/* gl2fifotest.c	--- fifo / file stuff
 */

#include "pcmap.h"
#include "m68000.h"
#include "fbcld.h"
#include "gfdev.h"
#include "geofdef.h"

#define FBCsend(d)	FBCdata = (d); \
			GFdisabvert(GERESET3|ENABTRAPINT_BIT_,CYCINDEBUG); \
			GFdisabvert(GERESET3|ENABTRAPINT_BIT_,RUNSUBST);

extern char line[];	/* command line buffer */
extern short ix;	/* command line index */
extern short expecting_output;
extern short expectingtest;
extern short intcount;
extern char intcmd;
extern short devstatus;	/* copy of currently written status reg */
extern short GEstatus;
extern char cmd,which,how;
extern short	firsttime,errorct;
extern unsigned short errorlog[][3];
extern unsigned short *outfile, *pgearray;
extern short outx, evenreceived;
extern unsigned short testpass[];

unsigned short fifoconfig[] = {
	0,	0x38,	0x38,	0x38,	0x38,	0x38,	0x38,
	0x38,	0x38,	0x38,	0x38,	0x38,	0x38,	1
};

unsigned short fidat[170];
short oldtestno;


fifotest()
{
	int i;
	short skip = 0;

  switch(line[ix++]) {
    case '?':
	printf("   data <n> send to FBC via multibus\n");
	printf("   fullblown fifo test (after 'S'n");
	printf("   test fifo <1=silent> (after 'S')\n");
	printf("   output g.e. errors <verb> <skip>\n      <verb> =\n");
	printf("      0 print total errors\n      1 interactive\n");
	printf("   better GE test output (see 'o')\n");
	printf("   quit testing\n");
	break;

    case 'd':  FBCsend(getnum()); break;

    case 'f':  fulltest(); break;

    case 't':  testfifo(getnum()); break;

    case 'b':			/* generate test results */
	skip = 1;
    case 'o':			/* entry for non- gB testing */
	intcmd = getnum();	/* get verbose flag */
	if (!skip) outx = -(getnum());
	if (!expectingtest) {
		printf("do 'S' first!\n");
		break;
	}
	expecting_output = 1;
	for (i=0; ++i<2000;) ;
	GFdisabvert(GERESET3|ENABTRAPINT_BIT_,RUNDEBUG);
	do {
		FBCsend(EOF1);	/* for some reason these aren't always seen */
		FBCsend(EOF2);
		FBCsend(EOF3);
		for (i=0; ++i<10000; )
			if (expecting_output==0) break;
		if (expecting_output) expecting_output += 3;
		} while (expecting_output) ;
	dosetup();			/* prepare for next test */
	break;

    case 'q': expecting_output = 0;
	    break;

    default:  illcmd();
    }
}


testfifo(testno)
   short testno;
{
	short i;
	register unsigned short evendat;
	register unsigned short odddat;

/* assume FBC is set up in DBLFEED mode, pipeline has been reset */
/* (use like: cs  ft  c#  ft  c#  ...)   */

/* configure the pipe */

smartconfigure(fifoconfig);

/* set FBC flag block */
FBCflags = RUNDEBUG;

/* check empty/full */
if (GEflags!=LOWATER_BIT) {
	printf("After config: should be LOWATER: GEflags: ");
	printgflags();
}

/* send stuff down pipe till full */
if (testno==0) pgearray = outfile = testpass;
else {
	pgearray = outfile = fidat;
	switch (testno) {
		case 1: evendat = 0; odddat = 0; break;
		case 2: evendat = -1; odddat = -1; break;
		case 3: evendat = 0x5555; odddat = 0xaaaa; break;
		case 4: evendat = 0xaaaa; odddat = 0x5555; break;
	}
	if (oldtestno != testno) {
		for (i=0; i<170; i+=2) {
			fidat[i] = evendat;
			fidat[i+1] = odddat;
		}
		oldtestno = testno;
printf("(%x,%x)",evendat,odddat);
	}
}
for (i=0; i<160 && !(GEflags & HIWATER_BIT); i++)
	GEdata = *pgearray++;

printf("\t%d wds\n",i);

/* clear the block */
FBCflags = devstatus;
buzz(5000);

/* check empty/full */
    if (GEflags != LOWATER_BIT) {
	printf("After unblock: should be LOWATER: GEflags: ");
	printgflags();
    }
}


fulltest()
{
/* run data tests */

	fcheck(0,0);
	fcheck(-1,-1);
	fcheck(0x5555,0xaaaa);
	fcheck(0xaaaa,0x5555);
}


fcheck(even,odd)
	unsigned short even;
	unsigned short odd;
{
#ifdef NOTBROKEN
	register unsigned short *ge = &GEdata;
	register unsigned short *FD = &FBCdata;
	register int wdct;
	int i;

printf("fcheck %x %x: ",even,odd);
	setup(7);
/* configure the pipe */

	smartconfigure(fifoconfig);

/* set FBC flag block */

	FBCflags = RUNDEBUG;

/* send stuff down pipe till full */

	for (wdct = 0; !(GEflags & HIWATER_BIT); wdct++) {
		*ge = even;
		*ge = odd;
	}
printf("%d ",wdct);
	if (wdct < 64)
		printf("filled only %d pairs\n",wdct);

/* clear the block */

	FBCflags = devstatus;
	for (i=0; i<100000; i++)
		if (GEflags & LOWATER_BIT) break;
printf("clear ");
/* check empty/full */

    if (i == 100000) {
	printf("After unblock: never got LOWATER: GEflags: ");
	printgflags();
	return(0);
    }

/* check data */

	*ge = EOF1;
	*ge = EOF2;
	*ge = EOF3;
printf("EOF sent ");
	while (FBCflags & INTERRUPT_BIT_) ;	/* wait for 1st intrrupt*/
	FBCclrint;
	FBCflags = READOUTRUN;
	if (*FD != wdct+wdct+3)
		printf("wrong output count: %d\n",*FD);
	wdct = (*FD-3)>>1;
	FBCflags = devstatus;
	FBCclrint;

	for (i=0; i < wdct; ++i) {
		while (FBCflags & INTERRUPT_BIT_) ;	/* wait for intrrupt*/
		if (*FD != even)
			printf("Wd. %d -- %x should be %x\n",i,*FD,even);
		FBCclrint;
		while (FBCflags & INTERRUPT_BIT_) ;	/* wait for intrrupt*/
		if (*FD != odd)
			printf("Wd. %d -- %x should be %x\n",i,*FD,odd);
		FBCclrint;
	}
	FBCclrint;
	FBCclrint;
	FBCclrint;
printf("did %x %x ",even,odd);getchar();
#endif NOTBROKEN
}
