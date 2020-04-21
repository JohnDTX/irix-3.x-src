/* rams.c   ---  fbc ram tests  */

#ifdef _FBC
#include "/usr/iris/new/include/GEsystem.h"
#define FBCtestram	0x25
#include "fbc/fbc.h"
#endif
#ifdef _GF1
#include "gfdev.h"
#define FBCtestram	0x36
#endif

extern short expecting_interrupt;
extern short intcmd;
extern char line[];
extern short ix;
extern short intcount;
extern short devstatus;

rams()
{
switch (line[ix]) {
   case '?':  printf("   microcode\n   scratch ram\n   looping scratch ram\n");
	      break;
   case 'u':
   case 'm':  ramtest();
	      break;
   case 's':  scratchtest();
	      break;
   case 'l':  scratchinfloop();
	      break;
   case 'w':  wrttest();
   default:   printf("which?\n");
   }
}


scratchtest()
{
#ifdef _FBC
intcmd = 'r';
expecting_interrupt = 8192;	/* 4096 errors possible  */
scratchloop(0,0);
expecting_interrupt = 8192;
scratchloop(0xffff,0xffff);
expecting_interrupt = 8192;
scratchloop(0x5555,0xaaaa);
expecting_interrupt = 8192;
scratchloop(0xaaaa,0x5555);
printf("scratch test done\n");
#endif
expecting_interrupt = 0;
}


scratchloop(even,odd)
	short even,odd;
{
	short i,times;

DEVdata = 0x25;		/* ramtest command code */
DEVflags = CYCINDEBUG;
DEVflags = RUNDEBUG;
if (DEVdata != 0x25)
    {
	printf("microcode error\n");
	return(0);
    }
for (times = 0; times < 2; ++times)
  for (intcount=0; intcount<1024; ++intcount)	/* read/write loop */
    {
	DEVdata = even;
	DEVflags = 0xd3;
	DEVflags = 0xf3;
	DEVdata = odd;
	DEVflags = 0xd3;
	DEVflags = 0xf3;
    }
}

scratchinfloop()
{
while (1)
    {
	expecting_interrupt = 1;
	intcmd = 'X';	/* unused  */
	scratchloop(0,0xffff);
	scratchloop(0xffff,0);
    }
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


wrttest()
{
    short i;

while (1)
    {
	DEVflags = devstatus = WRITEMICRO;
	DEVmicro(0,0) = 0x555;
	DEVmicro(1023,0)= 0xaaa;
	DEVflags = devstatus = READMICRO;
	i = DEVmicro(0,0);
	i = DEVmicro(1023,0);
    }
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
