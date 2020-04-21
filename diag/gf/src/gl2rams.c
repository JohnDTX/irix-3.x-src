/* gl2rams.c   ---  fbc ram tests for GL2 microcode
 *
 * define
 *	GFALPHA:	reflects ALPHA FBC and newest diagnostic microcode
 *	GF2:		reflects new GF and microcode
 */

#include "gfdev.h"
#include "gl2cmds.h"

#define HITSTACKPTR	4	/* scratch param */
#define FBCsend(d)	{ FBCdata = (d); \
			  FBCflags = CYCINDEBUG; \
			  FBCflags = RUNDEBUG; }

extern unsigned short _ucode[][4];
extern short devstatus;
extern short GEstatus;
extern short expecting_interrupt;
extern char intcmd;
extern short intdata;
extern short intbuf[];
extern short randsave;
extern char line[];
extern short ix;
extern short intcount;
extern char ucerr();

rams()
{
	register short num;

switch (line[ix++]) {
   case '?':
	     printf("   microcode\n   scratch ram\n   looping scratch ram\n");
		printf("   read micro word [<n>][0]\n");
		printf("   write micro word [<n>][0] with <n>\n");
		printf("   dump scratch from <n>\n   2903 reg dump\n");
		printf("   verify microcode\n");
		break;
   case 'd':	dumpscratch(getnum());
		break;
   case 'm':	num = getnum();		/* no. of locs for random test */
		if (num==0) num = 0x4000;	/* default (4 slices)	*/
		ramtest(num);
		break;
   case 's':	scratchtest();
		break;
   case 'l':	scratchinfwrt(scratchsetup(),getnum());
		break;
   case '2':	dump2903();
		break;
   case 'r':	FBCflags = devstatus = READMICRO;
#ifdef GF2
		num = getnum();
		FBCmicroslice((num>>12)&3,num);
		printf("%x\n",FBCmicrocode(num));
#else
		printf("%x\n",FBCmicro(getnum(),0));
#endif
		break;
   case 'w':	FBCflags = devstatus = WRITEMICRO;
#ifdef GF2
		num = getnum();
		FBCmicroslice((num>>12)&3,num);
		FBCmicrocode(num) = (short)getnum();
#else
		FBCmicro(getnum(),0) = (short)getnum();
#endif
		break;
   case 'v':	verifystore();
		break;
   case 't':	testwrite();
		break;
   default:	printf("which?\n");
   }
}


scratchtest()
{
	register short size;
	register i;

	if ((size = scratchsetup())==0) return(0);
						/* get scratch size  */
	printf("testing %d words\n",size);
	scratchloop(size,0x5555,0xaaaa);
	scratchloop(size,0xaaaa,0x5555);
	scratchrand(size);
	hardinit();
	printf("scratch test done.\n");
}


scratchinfwrt(siz,dat)
	register short siz,dat;
{
	register i;

    while (1) {
	if ((expecting_interrupt = scratchsetup())==0) return(0);
	for (i=0; i < siz; i++) {	/* write loop */
		FBCsend(dat);
	}
    }
}


scratchloop(siz,even,odd)
	short siz,even,odd;
{
	register i;
	char resp;

	if ((expecting_interrupt = scratchsetup())==0) return(0);

    for (i=0; i < siz; )	{	/* write loop */
	FBCsend(even);
	i++;
	FBCsend(odd);
	i++;
    }
    for (intcount = 0; intcount < siz; intcount+=16) {	/* read loop */
	FBCsend(FBCdumpram);
	FBCsend(intcount);
	buzz(50);		/* wait for interrupt to work */
    }
    for (i=0; i<siz;i++) {
	if (intbuf[i] != even) {
		printf("data error, word %d: %04x  exp'd %04x  (c/q)?",
			i,intbuf[i],even);
		resp = getchar();putchar('\n');
		if (resp=='q') return(0);
	}
	if (intbuf[++i] != odd) {
		printf("data error, word %d: %04x  exp'd %04x  (c/q)?",
			i,intbuf[i],odd);
		resp = getchar();putchar('\n');
		if (resp=='q') return(0);
	}
    }

}


scratchrand(siz)
	short siz;
{
	register i;
	register short rand;
	char resp;

/* perform scratchsetup first */
	if ((expecting_interrupt = scratchsetup())==0) return(0);

    rand = randsave;
    for (i=0; i < siz; i++) {	/* write loop */
	rand = rand*4 + rand + 17625;
	FBCsend(rand);
    }

    for (intcount = 0; intcount < siz; intcount+=16) { /* read loop */
	FBCsend(FBCdumpram);
	FBCsend(intcount);
	buzz(50);
    }

    rand = randsave;
    for (i=0; i<siz;i++) {	/* test loop */
	rand = rand*4 + rand + 17625;
	if (intbuf[i] != rand) {
		printf("addr error, word %d: %04x  exp'd %04x  (c/q)?",
			i,intbuf[i],rand);
		resp = getchar(); putchar('\n');
		if (resp=='q') return(0);
	}
    }
    randsave = rand;
}


scratchsetup()
{
   register short i;
   register j;

	i = hardinit();
/*	i = 2047;	/* force scratchsize (temporary) */
	i++;		/* i is now 2<<n  */

	GFdisabvert(GERESET3|ENABTRAPINT_BIT_,RUNDEBUG);
	devstatus = RUNDEBUG;

	intcmd = 'r';
	putchar('<');
	FBCsend(FBCloadram);		/* command code for testing */
	if ((FBCdata <0x200) || (FBCdata >0x500))  {
		printf("microcode setup failure: loc %x\n",FBCdata);
		return(0);
	}
	FBCsend(0); 	/* starting scratch adr */
	FBCsend(i);	/* no. wds */
	return(i);
}


dumpscratch(n)
	int n;
{
	char response;

    FBCsend(8);
    if (FBCdata != 0x40) {
	printf("FBC not ready!");
	return(0);
    }
    intcmd = 'D';
    do {
	expecting_interrupt = 1;
	FBCsend(FBCdumpram);
	FBCsend(intcount=(short)n);
	while (expecting_interrupt) ;	/* interrupt routine does the rest */
	printf("more? (y/n) ");
	response = getchar();
	putchar('\n');
	n += 16;
    } while (response == 'y') ;
    FBCflags = devstatus;
}

dump2903()
{
    if (FBCdata != 0x40)
	printf("FBC not ready!");
    FBCsend(FBCsaveregs);
    intcmd = 'D';
    expecting_interrupt = 1;
    FBCsend(FBCdumpram);
#ifdef GF2
    FBCsend(intcount=96);
#else
    FBCsend(intcount=81);	/* save area */
#endif
    while (expecting_interrupt) ;	/* interrupt routine does the rest */
    FBCflags = devstatus;
}

/*=====================================================================*/
/*		MICROCODE STUFF						*/

ramtest(lim)	/* microcode ram test */
	short lim;
{
	printf("micro test: ");
	testdata(0,0);
	testdata(0xffff,0xffff);
	testdata(0x5555,0xaaaa);
	testdata(0xaaaa,0x5555);
	testadr(lim);
	printf("done\n");
	GEflags = GEstatus;
}


testdata(eval,oval)	/* microcode ram: data test */
short eval,oval;
   {
	short i,got,val,err=0;
	short slice = 0;
	register short mask = 0xffff;

#ifdef GFALPHA
	FBCflags = devstatus = WRITEMICRO;
	for (i=0; i<1024; i+=2)
	   { FBCmicro(i,0) = eval; FBCmicro(i+1,0) = oval;}
	FBCflags = devstatus = READMICRO;
	for (i=0; i<1024; i++)
	   if ((got = FBCmicro(i,0)) != (val = i&1 ? oval : eval)) {
		if (ucerr("data",slice,i,val,got)=='q') return(0);
		err++;
	   }
#endif
#ifdef GF2
	FBCflags = devstatus = WRITEMICRO;
	for (slice=0; slice<32; slice++) {
		FBCmicroslice(slice%4,slice<<9);
		for (i=0; i<512; i+=2) {
			FBCmicrocode(i) = eval;
			FBCmicrocode(i+1) = oval;
		}
	}
	FBCflags = devstatus = READMICRO;
	for (slice=0; slice<32; slice++) {
		FBCmicroslice(slice%4,slice<<9);
		if ((slice%4)==3) mask = 0xff;
		else mask = 0xffff;
		for (i=0; i<512; i++)
		   if (( (got = FBCmicrocode(i)&mask)
			- (val = i&1 ? oval : eval)   ) & mask ) {
			if (ucerr("data",slice,i,val,got)=='q') return(0);
			err++;
		   }
	}
#endif
	return(err);
   }


testadr(lim)	/* microcode adressing test */
	short lim;
   {
	register i;
	short got,val,rand,err=0;
	short slice = 0;
	register short mask;

#ifdef GFALPHA
	rand = randsave;
	FBCflags = devstatus = WRITEMICRO;
	for (i=0; i<1024; i++)
	   {
		FBCmicro(i,0) = rand;
		rand = rand*4 + rand + 17625;
	   }
	rand = randsave;
	FBCflags = devstatus = READMICRO;
	for (i=0; i<1024; i++)
	   {
		if ((got = FBCmicro(i,0)) != (val = rand) )
		   {
			if (ucerr("address",slice,i,val,got)=='q') return(0);
			err++;
		   }
		rand = rand*4 + rand + 17625;
	   }
	randsave = rand;
#endif
#ifdef GF2
	rand = randsave;
	FBCflags = devstatus = WRITEMICRO;
	for (i=0; i<lim; i++) {
		FBCmicroslice((i>>12)&3,i);
		FBCmicrocode(i) = rand;
		rand = rand*4 + rand + 17625;
	}
	rand = randsave;
	FBCflags = devstatus = READMICRO;
	for (i=0; i<lim; i++) {
		FBCmicroslice((i>>12)&3,i);
		if ((i&0x3000)==0x3000) mask = 0xff;
		else mask = 0xffff;
		if (( (got = FBCmicrocode(i))
			- (val = rand)		) & mask) {
		    if (ucerr("address",slice,i,val,got)=='q') return(0);
		    err++;
		}
		rand = rand*4 + rand + 17625;
	}
	randsave = rand;
#endif
	return(err);
   }


rewrite()	/* write and verify microcode */
{
	writestore();
	verifystore();
}

writestore()
{
  register i;
  register wd;
  register short mask;
  short slice,got;

  FBCflags = devstatus = WRITEMICRO;

#ifdef GFALPHA
  for (i=0; i<1024; i++)
    for (wd=0; wd<4; wd++)
	FBCmicro(i,wd) = _ucode[i][wd];
  cycle_input();
  cycle_output();
  printf("OK\n");
  FBCflags = devstatus = RUNMODE | ENABVERTINT_BIT_;	/* alpha only! */
#endif
#ifdef GF2
	for (i=0; i<0x4000; i++) {
		wd = (i>>12) &3;
		FBCmicroslice(wd,i);
		FBCmicrocode(i) = _ucode[i&0xfff][wd];
	}
#endif
	FBCflags = devstatus = RUNMODE;
}

verifystore()
{
  register i;
  register wd;
  register short mask;
  short slice,got;

#ifdef GF2
	FBCflags = devstatus = READMICRO;
	mask = 0xffff;
	for (i=0; i<0x4000; i++) {
		wd = (i>>12) &3;
		FBCmicroslice(wd,i);
		if (wd==3) mask = 0xff;
		if ( (got = FBCmicrocode(i)&mask)
			!= _ucode[i&0xfff][wd]       ) {
			printf("readback error: adr=%x got %x",
				i,got);
			if (getchar()=='q') return(0);
			putchar('\n');
		}
	}
	printf("OK\n");
	FBCflags = devstatus = RUNMODE;
#endif
}

char ucerr(typ,slice,adr,expd,got)	/* returns response */
	char *typ;
	short slice;
	short adr;
	unsigned short expd,got;
{
	register char c;
	register i;

	i = ((slice<<9)+adr)&0xfff;
	slice = slice%4;
	printf("\n%s error, slice=%d, adr=%03x exp'd %04x got %04x",
		typ,slice,i,expd,got);
	c = getchar();
#ifdef PM1
	if (c==3) restart();
#endif
	return(c);
}

testwrite()
{
	int i;

  if ((i=Micro_Write(_ucode,0,4096)) >=0)
	printf("error at %x\n",i);
}
