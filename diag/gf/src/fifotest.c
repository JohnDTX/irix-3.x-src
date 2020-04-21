/* fifotest	--- fifo / file stuff
 */

#include "pcmap.h"
#include "m68000.h"
#ifdef _FBC
#include "fbc.h"
#include "GEsystem.h"
#include "dcdev.h"
#endif

#ifdef _GF1
#include "fbcld.h"
#include "gfdev.h"
#include "dcdev.h"
#endif

extern char line[];	/* command line buffer */
extern short ix;	/* command line index */
extern short expecting_output;
extern short intcount;
extern short devstatus;	/* copy of currently written status reg */
extern short GEstatus;
extern char cmd,which,how;
extern short	firsttime,errorct;
extern unsigned short errorlog[][3];
extern unsigned short *outfile, *pgearray;
extern short outx, evenreceived;
extern short testpass[];

short fifoconfig[] = {
	0x38,
	0x38,
	0x38,
	0x38,
	0x38,
	0x38,
	0x38,
	0x38,
	0x38,
	0x38,
	0x38,
	0x38,
	0x38
};

unsigned short fidat[170];
short oldtestno;


fifotest()
{
short i,num;

  switch(line[ix++]) {
    case '?':
	printf("   test fifo <testno> (after 'cs')\n");
	printf("   output g.e. errors <verb> <skip>\n      <verb> =\n");
	printf("      0 non-print\n");
	printf("      1 print total errors\n      2 interactive\n");
	printf("   quit testing\n");
	break;

    case 't':  testfifo(getnum()); break;

    case 'o':
	    expecting_output = 1;
	    firsttime = 1;
	    if (num=getnum())
		printf("%d errors in %d words.\n",errorct,outx);
	    if (num>1)
	      {
		if (errorct > 0)
		    printf("Press 'y' for each logline desired.\n");
		i = 0;
		while (errorct-- > 0 && (getchar() == 'y'))
		    {
			printf("item %d bad: %x (%d) expected, %x (%d) received\n",
				errorlog[i][0],errorlog[i][1],errorlog[i][1],
				errorlog[i][2],errorlog[i][2]);
			i++;
		    }
	      }
	    outx = -(getnum());
	    evenreceived = 1;
	    errorct = 0;
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
