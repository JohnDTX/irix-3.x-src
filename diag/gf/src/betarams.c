/* rams.c   ---  fbc ram tests for beta microcode
 *	added "rr" and "rw" for  microstore word read/write
 */

#include "gfdev.h"

#define FBCtestram	0x36

#define FBCsend(d)	FBCdata = (d); \
			GFdisabvert(GERESET3|ENABTRAPINT_BIT_,CYCINDEBUG); \
			GFdisabvert(GERESET3|ENABTRAPINT_BIT_,RUNDEBUG);

extern short devstatus;
extern short expecting_interrupt;
extern char intcmd;
extern short intdata;
extern short randsave;
extern char line[];
extern short ix;
extern short intcount;

rams()
{
switch (line[ix++]) {
   case '?':  printf("   microcode\n   scratch ram\n   looping scratch ram\n");
#ifndef GFBETA
		printf("   read micro word <n>\n   write micro word <n> with <n>\n");
#endif
		printf("   dump scratch from <n>\n   2903 reg dump\n");
	      break;
   case 'd':  dumpscratch(getnum());
	      break;
   case 'm':  ramtest();
	      break;
   case 's':  scratchtest();
	      break;
   case 'l':  scratchinfwrt();
	      break;
   case '2':  dump2903();
	      break;
#ifndef GFBETA
   case 'r':  FBCflags = devstatus = READMICRO;
		printf("%x\n",FBCmicro(getnum(),0));
		break;
   case 'w':  FBCflags = devstatus = WRITEMICRO;
		FBCmicro(getnum(),0) = (short)getnum();
		break;
#endif
   default:   printf("which?\n");
   }
}


scratchtest()
{
    short i;

i = scratchsetup();	/* get scratch size  */
intcmd = 'r';
expecting_interrupt = i;	/* possible errors */
scratchloop(i,0x5555,0xaaaa);
expecting_interrupt = i;
scratchloop(i,0xaaaa,0x5555);
expecting_interrupt = i;
scratchrand(i);
printf("scratch test done\n");
expecting_interrupt = 0;
}


scratchinfwrt()
{
	short i;

i = scratchsetup();
FBCdata = 0x3e;		/* dblfeed	*/
expecting_interrupt = 1;	/* to dismiss introductory interrupt  */
intcmd = '?';			/* just ignore the interrupt */
devstatus = RUNDEBUG|HOSTFLAG;
GFdisabvert(GERESET3|ENABTRAPINT_BIT_,CYCINDEBUG|HOSTFLAG);
GFdisabvert(GERESET3|ENABTRAPINT_BIT_,RUNDEBUG|HOSTFLAG);
GFdisabvert(GERESET3|ENABTRAPINT_BIT_,CYCOUTDEBUG|HOSTFLAG);
				/* clear output interface */

while (1)
    {
	FBCdata = 0;
	GFdisabvert(GERESET3|ENABTRAPINT_BIT_,CYCINDEBUG|HOSTFLAG);
	GFdisabvert(GERESET3|ENABTRAPINT_BIT_,RUNDEBUG|HOSTFLAG);
	FBCdata = 0xffff;
	GFdisabvert(GERESET3|ENABTRAPINT_BIT_,CYCINDEBUG|HOSTFLAG);
	GFdisabvert(GERESET3|ENABTRAPINT_BIT_,RUNDEBUG|HOSTFLAG);
    }
}


scratchloop(siz,even,odd)
	short siz,even,odd;
{
	short times;

GFdisabvert(GERESET3|ENABTRAPINT_BIT_,RUNDEBUG);
FBCsend(FBCtestram);		/* ramtest command code */
if (FBCdata < 0x200)  {
	printf("microcode command failure\n");
	return(0);
    }
FBCsend(FBCtestram);		/* extra because of debug mode */
FBCsend(siz);

for (times = 0; times < 2; times++)
  for (intcount=0; intcount < siz; )	/* read/write loop */
    {
	intdata = even;
	FBCsend(even);
	intcount++;
	intdata = odd;
	FBCsend(odd);
	intcount++;
	if (!intcmd) break;
    }
}


scratchrand(siz)
	short siz;
{
	short i,times,rand;

GFdisabvert(GERESET3|ENABTRAPINT_BIT_,RUNDEBUG);
FBCsend(FBCtestram);		/* ramtest command code */
FBCsend(FBCtestram);		/* extra because of debug mode */
FBCsend(siz);

for (times = 0; times < 2; times++)
    {
	rand = randsave;
	for (intcount=0; intcount<siz; ++intcount)	/* read/write loop */
	    {
		rand = rand*4 + rand + 17623;
		intdata = rand;
		FBCsend(rand);
		if (!intcmd) break;
	    }
    }
randsave = rand;
}


testadr()
   {
	short i,got,val,rand,err=0;

#ifndef GFBETA
	rand = randsave;
	FBCflags = devstatus = WRITEMICRO;
	for (i=0; i<1024; i++)
	   {
		FBCmicro(i,0) = rand;
		rand = rand*4 + rand + 17623;
	   }
	rand = randsave;
	FBCflags = devstatus = READMICRO;
	for (i=0; i<1024; i++)
	   {
		if ((got = FBCmicro(i,0)) != (val = rand) )
		   {
			printf("\naddress error, adr=%x exp'd %x rec'd %x",
				i,val,got);
			err++;
		   }
		rand = rand*4 + rand + 17623;
	   }
	randsave = rand;
#endif
	return(err);
   }


ramtest()
{
   short i;

for (i=0; i<8; i++)
  {
	testdata(0,0);
	testdata(0x0fff,0x0fff);
	testdata(0x555,0x0aaa);
	testdata(0x0aaa,0x555);
	testadr();
   }
printf("micro test done\n");
}


scratchsetup()
{
   short i;

#ifdef DEVEL
	i = 2047;
#else
	i = hardinit();
	if (i==1) i = 2047;	/* for alpha board */
#endif
	printf("mem size = %04x\n",i++);	/* i is now 2<<n  */
	GFdisabvert(GERESET3|ENABTRAPINT_BIT_,RUNDEBUG);
	devstatus = RUNDEBUG;
	return(i);
}


dumpscratch(n)
	int n;
{
	char response;

    if (FBCdata != 0x40)
	printf("FBC not ready!");
    intcmd = 'D';
    do {
	expecting_interrupt = 1;
	FBCsend(0x3a);
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
    FBCsend(0x22);	/* saveregs */
    intcmd = 'D';
    expecting_interrupt = 1;
    FBCsend(0x3a);
    FBCsend(intcount=81);	/* save area */
    while (expecting_interrupt) ;	/* interrupt routine does the rest */
    FBCflags = devstatus;
}


testdata(eval,oval)	/* microcode ram test */
short eval,oval;
   {
	short i,got,val,err=0;

#ifndef GFBETA
	FBCflags = devstatus = WRITEMICRO;
	for (i=0; i<1024; i+=2)
	   { FBCmicro(i,0) = eval; FBCmicro(i+1,0) = oval;}
	FBCflags = devstatus = READMICRO;
	for (i=0; i<1024; i++)
	   if ((got = FBCmicro(i,0)) != (val = i&1 ? oval : eval))
		{printf("\ndata error, adr=%x expected %x received %x",i,val,got);
		err++;
		}
#endif
	return(err);
   }


rewrite()
{
  short i,wd;

#ifndef GFBETA
  FBCflags = devstatus = WRITEMICRO;
  for (i=0; i<1024; i++)
    for (wd=0; wd<4; wd++)
	FBCmicro(i,wd) = ucode[i][wd];
  cycle_input();
  cycle_output();
  printf("OK\n");
  FBCflags = devstatus = RUNMODE | ENABVERTINT_BIT_;	/* alpha only! */
#else
  printf("no ram, silly!\n");
#endif
}
