/* gepa.c
 *
 *	Geometry Accelearator test
 */

#include "pcmap.h"
#include "m68000.h"

#include "fbcld.h"
#include "gfdev.h"

#ifdef DEVEL
#define WAIT	0xf3
#define REQ	0xd3
#else
#define WAIT	0x73
#define REQ	0x63
#endif

#define SGEsetup	register short *GEaddr = (short *)&GEdata; \
			register _ct
#define SGE(x)		{ \
			  *GEaddr = x; \
			 }
#define SGEdata(x)	{ \
			  for (_ct=0x100000; _ct--;) \
				if (!(GEflags & HIWATER_BIT)) break; \
			  if (_ct<=0) {printf("hung\n"); return(0);} \
			  *GEaddr = x; \
			 }

#include "../geofdef.h"

extern char line[];	/* command line buffer */
extern short ix;	/* command line index */
extern short devstatus;	/* copy of currently written status reg */
extern short GEstatus;
extern unsigned short *outfile;
extern short delaycount;
extern unsigned short testpass[];
extern unsigned short headin0[];
extern unsigned short headin1[];
extern unsigned short headin2[];
extern unsigned short headin3[];
extern unsigned short headin4[];
extern unsigned short headin5[];
extern unsigned short headin6[];
extern unsigned short headin7[];
extern unsigned short headin8[];
extern unsigned short headin9[];
extern unsigned short headout0[];
extern unsigned short headout1[];
extern unsigned short headout2[];
extern unsigned short headout3[];
extern unsigned short headout4[];
extern unsigned short headout5[];
extern unsigned short headout6[];
extern unsigned short headout7[];
extern unsigned short headout8[];
extern unsigned short headout9[];
extern unsigned short tailin0[];
extern unsigned short tailin0[];
extern unsigned short tailin1[];
extern unsigned short tailin2[];
extern unsigned short tailin3[];
extern unsigned short tailin4[];
extern unsigned short tailout0[];
extern unsigned short tailout1[];
extern unsigned short tailout2[];
extern unsigned short tailout3[];
extern unsigned short tailout4[];

extern short initdone;
extern short fastest;
extern unsigned short *pgearray;

unsigned short *gain[] = {
	testpass,   headin0,	headin1,	headin2,	headin3,
		headin4,	headin5,	headin6,	headin7,
		headin8,	headin9,	tailin0,	tailin1,
		tailin2,	tailin3,	tailin4
	};
unsigned short *gaout[] = {
	testpass,  headout0,	headout1,	headout2,	headout3,
		headout4,	headout5,	headout6,	headout7,
		headout8,	headout9,	tailout0,	tailout1,
		tailout2,	tailout3,	tailout4
	};

extern short outx, evenreceived;

extern unsigned short masterconfig[];
extern unsigned short masterconfig10[];
unsigned short passconfig[] = {
	0, 0x38,  0x38,  0x38,  0x38,  0x38,  0x38,  0x38, 
	 0x38,  0x38,  0x38,  0x38,  0x38
	};

extern short GEmask,GEfound;
short *foo;
short headfifo = 1;
short headcfp = 1;
short tailfifo= 1;
short tailcfp = 1;
short fillfifo;

/*--------------------------------- */

gepa()
{
	SGEsetup;
	short num;
	register	unsigned short i;
	register	unsigned short *pvec;

	fillfifo = 0;
	switch(line[ix]) {
	    case '?':
		gahelp();
		return(0);
	    case 'r':
		gareset();
		return(0);
	    case 'h':
		++ix;
		switch(line[ix++]) {
		    case 'f': headfifo = getnum(); break;
		    case 'c': fudge(&headcfp); break;
		    default: gahelp(); return(0);
		}
		return(0);
	    case 't':
		++ix;
		switch(line[ix++]) {
		    case 'f': fudge(&tailfifo); break;
		    case 'c': fudge(&tailcfp); break;
		    default: gahelp(); return(0);
		}
		return(0);
	    case 'F':
		fillfifo = 1;
		++ix;
	}

	if (!initdone) {
		printf("init first!\n");
		return(0);
	}
	num = getnum();		/* which test ? */
	pgearray = gain[num];
	outfile = gaout[num];		/* set up ptr to comparison file */

	GEflags = GERESET1;		/* reset pipe */
	buzz(200);
	GEflags = GERESET3;
	buzz(100);

	if (headfifo) SGE(0x3a02);			/* configure pipe */
	if (headcfp) {
		if (num==0) {SGE((short)((GEfound-1)<<8) + 1);}
							/* fifo-only test */
		else {SGE((short)((GEfound-1)<<8) + *pgearray++);}
	}
	else if (num!=0) ++pgearray;
	justconfigure(passconfig);
	if (tailfifo) SGE(0x3a02);
	if (tailcfp) {
		if (num==0) {SGE(0xff01);} 
		else {SGE(0xff00 + *pgearray++);}
	}
	else if (num!=0) ++pgearray;

	pvec = pgearray;
	fastest = 0;
	if (fillfifo) {
		FBCflags = devstatus = devstatus | SUBSTIN_BIT;
		for (i=0; i<70; i++) SGE(8);
		outx = -70;
		while (!(GEflags & HIWATER_BIT)) {
			SGE(*pvec++);
		}
		buzz(1000);
		FBCflags = devstatus = devstatus & ~SUBSTIN_BIT;
		i = GEOF;
		fastest = 1;
		while (*pvec != i)
			*GEaddr = *pvec++;	/* note - no hiwater test */
		fastest = 0;
	}
	else {
		i = GEOF;
		outx = 0;
		while (*pvec != i)
			SGEdata(*pvec++)
	}
}

gahelp()
{
   short num,i;

   printf("[head,tail] [fifo,cfp] [0,1]\n");
   printf("Fifo filled first\n");
   printf("<n> test run:\n");
   for (i=0; i < (sizeof(gaout)/sizeof(foo)); i++) {
	for (num = 0, outfile = gaout[i]; outfile[num++] != GEOF; ) ;
	printf("   test %d: %d words\n",i,num);
   }
}


configpassers(num)
	short num;	/* no. of chips */
{
	register i;

	if (num==0) {
		justconfigure(passconfig);	/* according to found */
		return(0);
	}
	for (i=0; i<num; i++)
		GEdata = 0xff38;
}

forcemask(num)
	short num;
{
	register i;

	GEmask = getnum();
	for (i=0x4000,GEfound=0; i!=0; i>>=1)
	if (GEmask&i) ++GEfound;
}


fudge(flag)
	register short *flag;
{
	register short num = getnum();

	if (num) num = 1;
	if (*flag==0 && num==1) {
		*flag = 1;
		++GEfound;
	}
	else if (*flag==1 && num==0) {
		*flag = 0;
		--GEfound;
	}
}


gareset()
{
	headfifo = headcfp = tailfifo = tailcfp = 1;
}

