/* testall.c
 *
 *	"canned" gf2 board test
 *	testmask:
 *	bit 0:  Multibus test
 *	bit 1:  microcode ram
 *	bit 2:  scratch ram test
 *	bit 3:  GF-UC4 interface
 *	bit 4:  drawing tests
 *	bit 5:  GE pipeline test
 */

#define STARTPAT	1	/* long xfers work on IP2 */

#include "gums.h"
#include "dcdev.h"
#include "geofdef.h"
#include "gl2cmds.h"

short GEstatus;
short devstatus;
short Verbose;
unsigned short *infile,*outfile;
int filect;
short delaycount;
short printwd = 0;
extern short GEfound, GEmask;
extern unsigned short bustests[][256];
extern short dc_dcr;

unsigned short ctab[] = {
	0,	0x38,	0x38,	0x38,	0x38,	0x38,	0x38,
	0x38,	0x38,	0x38,	0x38,	0x38,	0x38,	8
};

/* dummy declarations */
char line[2];
short ix;
int getnum() {}
interruptinit() {}
configureGE() {}
inittables() {}

#include "gf2init.c"

long testall(verbose,testmask)
	int verbose,testmask;
{
    short i;
    long errors = 0;
    short pat;

    Verbose = verbose;

/* test Multibus reg */

    header();
    if (testmask & 1) {
	if (verbose) printf("\nMultibus: ");
	FBCflags = 0xfb;
	for (i=1; i!=0; i<<=1) {
	    FBCdata = i;
	    if (FBCdata != i) {
		printf("multibus r/w error: %04x should be %04x\n",
			FBCdata,(unsigned short)i);
		++errors;
	    }
	}
    }

/* test micro ram */

    DCflags = dc_dcr;
    if (testmask & 2) {
	if (verbose) printf(" micro ram: ");
	if (errors += ramtest(0x4000))
	    return(errors);
	if (verbose) printf("done");
    }
    if ((testmask & ~(3))==0)
	return(errors);
    breakcheck();
    writestore();

/* initialization */

    if (verbose) printf("\ninit: ");
    if (initall('S'))
	return(++errors);
    DCflags = dc_dcr;

/* test scratch ram */

    if (testmask & 4) 
	errors += scratchtest();
    breakcheck();

    if (testmask & 8) {
	if (FBCflags & BPCACK_BIT)  {		/* if BPC ACK floating... */
	    if (verbose) printf("\nNo frame buffer\n");
	    return(errors);
	}

/* frame buffer present: test interface */

	if (verbose) printf("BPC i/f: ");
	errors += bpciftest();		/* "bt" */
    }

/* draw pictures */

    if (testmask & 0x10) {
	if (verbose) printf("draw: ");
	if (initall('F'))			/* perform "if" */
	    return(++errors);
	DCflags = dc_dcr;
	if (gevec(0)) return(++errors);
	buzz(10000);
	if (gevec(2)) return(++errors);
	if (gevec(3)) return(++errors);
	if (gevec(1)) return(++errors);
	if (FBCdata != 0x40) {
	    printf("microcode failure\n");
	    return(++errors);
	}
	breakcheck();
    }


/* test whether GE's (nulls OK) installed */

    if (testmask & 0x20) {
	GEflags = GERESET1;	/* attempt to reset GEs	*/
	if ((GEflags & 0x7ff8)!=0) {
	    printf("GE(s) or passer(s) missing:");
	    printgflags();
	    return(++errors);		/* if any trap floating, quit	*/
	}

	hardinit();
	GFdisabvert(GERESET3,RUNMODE);
	findge(0);
	if (verbose) printf("found: %d   mask: %04x  ",GEfound,GEmask);
	if (GEfound == 0) {
	    if (verbose) printf("assuming passers in.  ");
	}
	else if (GEfound < 10) {
		printf("Not enough GE's\n");
		return(errors += 10-GEfound);
	}
	GFdisabvert(GERESET3,RUNMODE);
	if (GEfound) smartconfigure(ctab);
					/* all installed chips configured */
	buzz(10000);

	if (!(FBCflags & GEREQ_BIT_)) {	/* check for spurious request */
	    printf("GEREQFBC not clear\n");
	    return(++errors);
	}

/* FBC input byte-swap test */

	if (verbose) printf("byteswap: ");
	for (i = 0x80; i > 0; i >>= 1) {
	    hardinit();
	    GFdisabvert(GERESET3,RUNMODE);
	    if (GEfound) smartconfigure(ctab);
	    pat = (i<<8) | 8;
	    GEdata = pat;
	    buzz(1000);
	    FBCflags = 8;
	    if (FBCdata != pat) {
		printf("pipeline or FBC inrjust - wanted %x got %x\n",
			pat,FBCdata);
		++errors;
	    }
	    FBCflags = 0xc;
	    pat = 0x800 | i;
	    if (FBCdata != pat) {
		printf("FBC inljust - wanted %x got %x\n",pat,FBCdata);
		++errors;
	    }
	}
	if (verbose) printf("feedback:");
	hardinit();
	if (GEfound) {
	    register *LGE = (unsigned long *)&GEdata;
	    register long dat;
	    long rddat;

	    smartconfigure(ctab);
	    GFdisabvert(GERUNMODE,RUNMODE);	/* FBC ints disabled */
	    GEdata = 8;
	    GEdata = FBCfeedback;
	    for (dat=STARTPAT; dat!=0; dat<<=1) {
		GEdata = 0x108;
		*LGE = dat;
	    }
	    GEdata = EOF1;
	    GEdata = EOF2;
	    GEdata = EOF3;
	    buzz(1000);
	    if (FBCflags & INTERRUPT_BIT_) {
		printf("no feedback\n");
		return(++errors);
	    }
	    discard();
	    discard();
	    for (dat=STARTPAT; dat!=0; dat<<=1) {
		discard();	/* throw out 108 */
		rddat = FBCdata;
		discard();
		rddat = (rddat<<16) + FBCdata;
		discard();
		if (rddat != dat) {
		  printf("bad long data: wanted %08x got %08x\n",dat,rddat);
		  ++errors;
		}
	    }
	    hardinit();
	}
    }

/* recheck micro ram */

    if (errors += verifystore())
	 return(errors);
    if (verbose) printf("Tests done.\n");
    hardinit();
    return(0);
}


buzz(n)
    int n;
{
#ifdef PM3
	n += n<<1;
#endif
	while (n-- >0) ;
}


int gevec(vecno)
    short vecno;
{
	register unsigned short *vp;
	register j;
	fsetup;

	vp = bustests[vecno];
	while (*vp != GEOF) {
		fshort(*vp++);
		buzz(1000);
		for (j=100000; j-- > 0 ; )
			if (FBCflags & GET_BIT) break;
		if (j <= 0) {
			printf("FBC hung\n");
			return(1);
		}
	}
	return(0);
}

printgflags()
{
    register short flg = GEflags;
    register short i;

    if (flg & LOWATER_BIT) printf("LOWATER  ");
    if (flg & HIWATER_BIT) printf("HIWATER  ");
    if (flg & FIFOINT_BIT) printf("FIFOINT  ");
    if (flg & TRAPINT_BIT)
	printf("TRAP: ");
    flg = flg>>3;
    for (i=1; i<13; i++) {
	if (flg & 1) printf("%d ",i);
	flg = flg>>1;
    }
    putchar('\n');
}

gesend(n)
    short (n);
{
	GEdata = n;
}

discard()
{
    int i;	/* time waster */

    FBCclrint;
    i = 1;
}
