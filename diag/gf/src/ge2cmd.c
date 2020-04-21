/* ge2cmd.c
 *
 *	Geometry Engine (rev 2) pipeline commands and tests
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
extern char prompt;
extern short devstatus;	/* copy of currently written status reg */
extern short GEstatus;
extern char cmd,which,how;
extern char intcmd;
extern short interrupts;
extern unsigned short *pintdata;
extern short expecting_interrupt;
unsigned short *outfile;
static testlim = 2000;
static testmode = 0;	/* 0= test each GE; 1= test partic. GE as each */
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
extern unsigned short testgso[];
extern unsigned short test10sc1[];
extern unsigned short test10gso[];

extern unsigned short testpass[];		/* testpass.c	*/
extern unsigned short drawtests[][256];		/* drawtests.c	*/
extern unsigned short expect[][64];
extern unsigned short bustests[][256];		/* bustests.c	*/
extern short initdone;
extern short fastest;

unsigned short *pgearray;
unsigned short *testin[] = 
	{0,	testgsi,	testmm1,	testmm2,	testmm3,
		testmm4,	testcl1,	testcl2,	testcl3,
		testcl4,	testcl5,	testcl6,	testsc1,
		testgso
	};
unsigned short *testout[] = 
	{0,	testmm1,	testmm2,	testmm3,	testmm4,
		testcl1,	testcl2,	testcl3,	testcl4,
		testcl5,	testcl6,	testsc1,	testgso,
	0};

unsigned short *test10in[] = 
	{0,	testgsi,	testmm1,	testmm2,	testmm3,
		testcl5,	testmm4,	testcl1,	testcl2,
		testcl3,	testcl4,	testcl4,	test10sc1
	};
unsigned short *test10out[] = 
	{0,	testmm1,	testmm2,	testmm3,	testmm4,
		testcl6,	testcl1,	testcl2,	testcl3,
		testcl4,	testcl5,	test10sc1,	test10gso,
	0};

extern short outx, evenreceived;

unsigned short masterconfig[] = {
	0,	/* dummy */
	0x09,	0x0a,	0x0b,	0x0c,	0x10,	0x11,	0x12,	0x13,	0x14,
	0x15,	0x20,	0x21,	1
};

unsigned short masterconfig10[] = {
	0,	/* dummy */
	0x09,	0x0a,	0x0b,	0x0c,	0x15,	0x10,	0x11,	0x12,	0x13,
	0x14,	0x20,	0x21,	1
};

unsigned short useconfig[] = {
	0,	/* default config stuck in just in case (it couldn't hoit) */
	0xb09,	0xa0a,	0x90b,	0x80c,	0x710,	0x611,	0x512,	0x413,	0x314,
	0x215,	0x120,	0x21,  1
};

extern short GEmask,GEfound;

short delaycount = 1000;
short printwd = 0;

/*--------------------------------- */

ge()
{
    SGEsetup;
    short num,lim;
    char resp;
    register	short i;
    register	j;

switch (line[ix++]) {
   case '?': gehelp();
	     break;
#ifdef GF2
   case 'a': gepa();
	     break;
#endif
   case 'j': if (!initdone) { printf("init first!\n"); break; }
		if (testmode) testGE(getnum(),1);
				/* config GE #testmode as chip "num" */
		else {
			i = getnum();
			configureGE(i,getnum(),1);
		}
		break;
   case 'f': printf("read = %04x    written = %04x\n",
		(unsigned short)GEflags,(unsigned short)GEstatus);
	     if (line[ix]=='t') printgflags();
	     break;
   case 's': if (line[ix]=='?') gshelp();
	     else GEflags = GEstatus = getnum();
	     break;
   case 'd': GEdata = getnum(); break;
#ifdef GF2
   case 'E': GETOKEN = getnum(); break;
   case 'V': fbcsend(getnum()); break;
#endif
   case 'v': vectsend(getnum()); break;
   case 'p':
		pgearray = testgsi;
		if ((GEmask&0x1ffe)==0x1ffe) outfile = testgso;
		else if ((GEmask&0x1ffe)==0x1ef6) outfile = test10gso;
		else {printf("wrong no. of GEs\n"); return(0);}
		while (*pgearray != GEOF) SGEdata(*pgearray++)
		break;
		
   case 'P':		/* 128 wds to fill both FIFOs  */
		pgearray = testpass;
		outfile = testpass;
		while (*pgearray != GEOF) SGE(*pgearray++)
		break;
   case 't':		/* allow range spec */
		num = getnum();
		if ((lim = getnum())<num) lim = num;
		intcmd = 'x';
		for (; num<=lim; num++) {
			pgearray = drawtests[num];
			pintdata = expect[num];
#ifdef GF2
			expecting_interrupt = *pintdata++;
#else
			expecting_interrupt = 0;
#endif
			while (*pgearray != GEOF) SGEdata(*pgearray++);
		}
		if (expecting_interrupt) {
			buzz(100);	/* give em a chance */
			if (expecting_interrupt)
			    printf("%d interrupts missing\n",
					expecting_interrupt);
		}
		expecting_interrupt = 0;
		break;
   case 'w': delaycount = (line[ix] == 'r') ?
		 ((delaycount<<2) + delaycount + 17623) & 0x3ff
		 : getnum();
	     break;
   case 'm': pgearray = bustests[getnum()];
		FBCflags = WAIT;
		while (*pgearray != GEOF)
		    {
			FBCdata = *pgearray++;
			FBCflags = REQ;
			FBCflags = WAIT;
			buzz(delaycount);	/* wait a while */
#ifdef GF2
			buzz(50);
			for (j=100000; j-- > 0 ; )
				if (FBCflags & GET_BIT) break;
			if (j <= 0) {
				printf("FBC hung\n");
				break;
			}
#endif
		    }
		break;
   case 'i':	GEflags = GEstatus = GERESET1;	/* just reset GE/FIFO pipe */
		buzz(100);
	 	GEflags = GEstatus = GERESET3;
		outx = 0;
		intcmd = 0;
		break;
   case 'C':
	if (!initdone) { printf("init first!\n"); break; }
	if (testmode) testGE(getnum(),0);
				/* config GE #testmode as chip "num" */
	else {
		i = getnum();
		configureGE(i,getnum(),0);
	}
	break;
    case 'D':	printwd = 1-printwd;
		intcmd = 'p';
		break;
    case 'T': num = getnum();
	      testmode = 0;
	      if (num && !inpipe(num))
			break;
	      testmode = num;
	      prompt = (testmode) ? 'T' : '!';
	      break;
    case 'L': testlim = getnum(); break;

    case 'B':
	if (!initdone)
	    {
		printf("init first!\n");
		return(0);
	    }
	num = getnum();		/* which chip's input?	*/
	if (!inpipe(num)) {
		testGE(13,0);
		GEdata = 0xbad;	/* send something down to tickle microcode */
		return(0);
	}
	pgearray = ((GEmask&0x1ffe)!=0x1ffe) ? test10in[num] : testin[num];
					/* set up ptr to input file	*/
	lim = getnum();		/* which chip's output?	*/
	if (lim < num) lim = num;	/* there's no going back	*/
	if (!inpipe(lim)) return(0);
	outfile = ((GEmask&0x1ffe)!=0x1ffe) ? test10out[lim] : testout[lim];
					/* set up ptr to comparison file */
	if (testmode) testGE(num,0);   /* config GE #testmode as chip "num" */
	else configureGE(num,lim,0);

/* set block; send down some 8's; send test words til HIWATER; unblock;
 *   send rest
 */
	num = getnum();
	if (num==0) num = 70;		/* default: fill tail & GEs */
	Bsend(num);			/* send vector */
	break;

    case 'e':		/* send down a GE test vector */
	if (!initdone)
	    {
		printf("init first!\n");
		return(0);
	    }
	fastest = 1;
	num = getnum();		/* which chip's input?	*/
	if (!inpipe(num)) {
		testGE(13,0);
		GEdata = 0xbad;
		return(0);
	}
	pgearray = ((GEmask&0x1ffe)!=0x1ffe) ? test10in[num] : testin[num];
					/* set up ptr to input file	*/
	lim = getnum();		/* which chip's output?	*/
	if (lim < num) lim = num;	/* there's no going back	*/
	if (!inpipe(lim)) return(0);
	outfile = ((GEmask&0x1ffe)!=0x1ffe) ? test10out[lim] : testout[lim];
					/* set up ptr to comparison file */
	i = 0;
	while ((++i<testlim) && (*pgearray != GEOF)) SGEdata(*pgearray++);
	break;

    case 'x':
	num = getnum();
	pgearray = ((GEmask&0x1ffe)!=0x1ffe) ? test10in[num] : testin[num];
	resp = 'y'; i = 0;
	while (resp == 'y') {
		printf("%03d: ",i); i += 10;
		for (j=0; j<10; j++) {
			printf(" %04x",*pgearray++);
			if (*pgearray == GEOF) goto quitx;
		}
		printf(" (y/n?)");
		resp = getchar();
		putchar('\n');
	}
quitx:
	break;
    default: gehelp();
	}
}

		   /* configure the whole pipe, with num to lim active */
configureGE(num,lim,justGE)
    short num;		/* 0 for default, 1-10 for specific chip */
    register short lim;
    short justGE;
{
    register short i;
    unsigned short *tab = ((GEmask&0x1ffe)!=0x1ffe) ?
					 masterconfig10 : masterconfig;

	if (num==0) {num=1; lim=12;}
	if (lim < num) lim = num;
	if (!inpipe(num) || !inpipe(lim)) return(0);
	for (i=1; i<13; ++i)
	    {
		if (i >= num && i <= lim)
			useconfig[i] = *(tab+i);
		else useconfig[i] = 0x38;
		if (printwd) printf("%x  ",useconfig[i]);
	    }
	if (printwd) putchar('\n');
	if (justGE) justconfigure(useconfig);
	else smartconfigure(useconfig);
	GEflags = GEstatus;
}


testGE(num,justGE)		/* configure chip determined by testmode  */
    short num,justGE;	/* 0 for default, 1..9,a..c for specific chip */
{
    register short i;
    unsigned short *tab = ((GEmask&0x1ffe)!=0x1ffe) ?
					 masterconfig10 : masterconfig;

	for (i=1; i<13; ++i)
		useconfig[i] = 0x38;
	if (num==0) useconfig[testmode] = *(tab+testmode);
	else if (inpipe(num))
		useconfig[testmode] = *(tab+num);
	else return(0);
	if (justGE) justconfigure(useconfig);
	else smartconfigure(useconfig);
	GEflags = GEstatus;
}


inpipe(chipno)
    short chipno;
{
    char save;

    save = intcmd;
    intcmd = 0;
    if (chipno==13) return(1);		/* allow chip 13 for all-passer gC */
    if (chipno<1 || chipno>12)
	{
	    printf("bad chip no. %d\n",chipno);
	    intcmd = 'a';
	    return(0);
	}
    if (testmode==0 && !((0x2000>>chipno) & GEmask) )
	    {
		printf("chip %d not installed\n",chipno);
		intcmd = 'a';
		return(0);
	    }
    intcmd = save;
    return(1);
}

Bsend(num)	/* zippy test vector sending routine */
    register short num;
{
	SGEsetup;
	register short i;
	register unsigned short *pvec;

	FBCflags = devstatus = devstatus | SUBSTIN_BIT;
	num &= ~1;		/* make it even  */
	for (i=0; i<num; i++) SGE(8);
	outx = -num;
	while (!(GEflags & HIWATER_BIT)) {
		SGE(*pgearray++);
	}
	buzz(1000);
	FBCflags = devstatus = devstatus & ~SUBSTIN_BIT;
	i = GEOF;
	pvec = pgearray;
	fastest = 1;
	while (*pvec != i)
		*GEaddr = *pvec++;	/* note - no hiwater test */
	fastest = 0;
}

gehelp()
{
	register i,num;

	printf("   accelerator test vector <n>\n");
	printf("   Better test of GE 1..c[-1..c] [#prefix wds] test\n");
	printf("   Configure pipe [0..c]  [to 0..c] chip(s) isolated\n");
	printf("   data word send down pipe <n>\n");
	printf("   engine test vector <n>\n");
#ifdef GF2
	printf("   E - data word <n> sent with TOKEN\n");
#endif
	printf("   Display config when gC called (mode toggles)\n");
	printf("   flag reg print\n");
	printf("   initialize pipe\n");
	printf("   just config [<n> chips] as passers (no fifo/ga config)\n");
	printf("   Limit GE test to <n> words in\n");
	printf("   multibus picture test <n>\n");
	printf("   PassThru test vector\n");
	printf("   store flag reg <n>\n");
	printf("   test picture <n>\n");
	printf("   Test vec for specific GE (0 for all, <n> for chip n)\n");
	printf("   wait <n> between 'm' test words\n");
	printf("   1..c[-1..c] send vector for specific chip(s)\n");
	printf("   vector test\n");
	printf("   xamine test input vector <n>\n");
	printf("more...");
	getchar();
	putchar('\n');
	for (i=1; testout[i] != 0; i++) {
	    for (num = 0, outfile = testout[i];
		 outfile[num++] != GEOF; ) ;
	    printf("g%x: %d words\n",i,num-1);
	}
	for (num = 0, outfile = test10gso;
		outfile[num++] != GEOF; ) ;
	printf("10-chip gp: %d words\n",num-1);
}

vectsend(reps)
	register int reps;
{
	SGEsetup;
	register i;
	register done = testvi;
	register unsigned short *vec;

    if (reps==0) reps = 1;
    if (interrupts) {		/* fast version, no checking possible */
	while (reps-- > 0) {
		vec = testvecin;
		i = done-1;
		asm("vvvv:");
		SGE(*vec++);
		asm("	dbf	d5,vvvv");
	}
    }
    else while (reps-- > 0) {
	pgearray = testvecin;
	outfile = testvecout;
	for (i=0; i<done; i++) SGEdata(*pgearray++);
    }
}
