/* quiettest.c
 */

#define PREAMBLE	70

#include "gfdev.h"

extern short GEfound,devstatus;
extern short GEstatus;
extern unsigned short testgsi[];
extern unsigned short testgso[];
extern unsigned short test10gso[];

unsigned short *infile,*outfile;
extern unsigned short *pgearray;
int filect;

quiettest()
{
	short num;
/* some stuff from testall()
 *	test whether GE's installed
 */
	GEflags = GERESET1;	/* attempt to reset GEs */
	if ((GEflags & 0x7ff8)!=0) {
		qinitscreen();
		printf("GE(s) missing:\n");
		printgflags();
		goto bad;
	}
	hardinit();
	GFdisabvert(GERESET3,RUNMODE);
	findge();
	configureGE(0,0);		/* all installed chips configured */

	if (!(FBCflags & GEREQ_BIT_)) {	/* check for spurious request */
		qinitscreen();
		printf("GEREQFBC not clear\n");
		goto bad;
	}

/* FBC input byte-swap test */

    GEdata = 0x1108;
    buzz(1000);
    FBCflags = 8;
    if (FBCdata != 0x1108) {
	qinitscreen();
	printf("pipeline or FBC inrjust error\n");
	goto bad;
    }
    FBCflags = 0xc;
    if (FBCdata != 0x0811) {
	qinitscreen();
	printf("FBC inljust error\n");
	goto bad;
    }
    setup(7);
    if (GEfound!=12 && GEfound!=10) {
	qinitscreen();
	printf("not enough GE's -- skipping test\n");
	goto bad;
    }
    configureGE(0,0);
    pgearray = testgsi;
    outfile = (GEfound==10) ? test10gso : testgso;
    Bsend(PREAMBLE);
    buzz(1000);

/* now feed it back (pollng) */

    FBCflags = devstatus = 0x63;
    FBCflags = 0x73;
    FBCflags = 0x63;

    for (filect=0; filect<PREAMBLE+PREAMBLE; filect++) {
	while ( FBCflags & INTERRUPT_BIT_ ) ;
	FBCclrint;				/* ignore PREAMBLE */
    }
    filect = 0;
    while ( !(FBCflags & INTERRUPT_BIT_) && qfeedback() ) ++filect;

    qinitscreen();
    printf("\n\nTests done. (%d vector words)\n",filect);
    return(1);

bad: getchar();
     return(0);
}

qfeedback()		/* like getfeedback() */
{
    short read1,read2;

    while (FBCflags & INTERRUPT_BIT_) ;
    FBCflags = READOUTRUN;
    read1 = FBCdata;
    FBCflags = devstatus;
    FBCclrint;
    while (FBCflags & INTERRUPT_BIT_) ; /* wait for interrupt */
    FBCflags = READOUTRUN;
    if ( (read2=FBCdata) != read1) {
	qinitscreen();
	printf("feedback error: %x <> %x\n",read1,read2);
    }
    FBCflags = devstatus;
    FBCclrint;
    if (read1 != *outfile) {
	qinitscreen();
	printf("Error on wd. %d:  %x should be %x ", filect,
		read1, *outfile);
	goto bad;
    }
    ++outfile;
    return(1);

bad: getchar();
     return(0);
}

spl7()
{_spl7();}

splx(x)
    register x;
{_splx(x);}

qinitscreen()
{
    initscreen();
    buzz(20000);
}
