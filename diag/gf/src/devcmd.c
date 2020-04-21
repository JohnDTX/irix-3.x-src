/* devcmd.c --  console command processor */
/*		includes device-dependent .h file */

#include "pcmap.h"
#include "m68000.h"

#include "fbcld.h"
#include "gfdev.h"

#define NULL 0

extern char line[];	/* command line buffer */
extern short ix;	/* command line index */
extern short devstatus;	/* copy of currently written status reg */
extern short version;
extern short GEmask, GEfound;

char cmd,which,how;
unsigned short indata;	/* data from input file  - set by getindata()  */
short num;	/* current ucode addr */
short val;	/* field designator for dostore */
short low,high;	/* limits for block store */
char *pd = '\0';	/* ptr to next input char */

char *field[NFIELDS][NENTRIES]; 	/* text prompts for ucode fields */

char *cats[NFIELDS];		/* names of fields */

/*--------------------------------- */

devcmd(repeat)
   short repeat;
{
  register short i;
  register short j;
  register char c;  

  cmd = '\n';
  do {			/* main command loop */
	ix=0;
	getcmdlin(1);
	how='n';	/* default */

		   /* CMD RECOGNIZER  */
	if (line[ix]=='\n' && cmd=='p') {num++; how='.';}
	else cmd = line[ix++];

		/* CMD INTERPRETER */
	switch (cmd) {
	  case 'o':
	  case 'p':
	  case 's': printstore(); break;
	  case 'g': ge(); break;
	  case 'd': dismiss(getnum()); break;
	  case 'b': bpctest(); break;
	  case 'r': rams(); break;
	  case 'f': fifotest(); break;
	  case 'i': initall(line[ix]); break;
#ifdef GF2
	  case 'I': inittables(); break;
	  case 'w': breakclear('a'); break;	/* does rewrite() */
	  case 'W': rewrite();
		    breaksetall();
		    break; /* preserves Bp's */
#else
	  case 'w': rewrite(); break;
#endif GF2
	  case 't': testall(); break;
	  case 'v': testvec(); break;
#ifdef GF2
	  case 'B': breakset(line[ix]); break;
	  case 'U': c = line[ix++];
		    breakclear(c,getnum());
		    break;
	  case 'M': forcemask(getnum());
		    break;
#endif
#ifdef PM2
	  case 'Q': quiettest(); break;
#endif
	  case 'S': setupfortest(line[ix]); break;
	  case 'C': printmac(getnum()); break;
	  case '?': cmdhelp();
		    break;
#if GFBETA || GF2
	  case 'G': gedraw(); break;
#endif
	  case 'V': version = getnum(); break;
#ifdef GF2
	  case 'T': tokentest(); break;
	  case 'm': monselect(); break;
#endif
	  case '\n':
	  case 0:
	  case 'q': break;
	  default : printf("Command error: '%c'?\n",cmd);
	  }

    } while ((cmd != 'q') && repeat) ;
  cmd = '\n';
}


printstore()
{
		   /* WHICH RECOGNIZER */
switch (line[ix]) {
  case '?': which = 'Q'; break;
  case 'm':
  case 'i':
  case 'o':
  case 'b':
  case 'c':
  case 'p':
  case 'f': which = line[ix++];
  }
		/* HOW RECOGNIZER */
switch (line[ix]) {
  case '\n': if (which != 'm') how = 'n'; break;
  case 't':
  case '?':
  case '.': how = line[ix++];
  }
		/* WHICH INTERPRETER */
switch (which) {
  case 'm':  dom(); break;
  case 'f':  dof(); break;
  case 'i':  doi(); break;
  case 'o':  doo(); break;
  case 'c':  doc(); break;
  case 'b':  dob(); break;
  case 'p':  dop(); break;
  default:   dohelp();
  }
}

dohelp()
{
if (cmd != 'o')
    {
	printf("   flag reg [text] [<n>]\n   i/o reg (DI bus)\n");
	printf("   output reg\n   <CR> previous 'which'\n   microcode\n");
    }
switch (cmd) {
  case 'p': printf("      [text],  next loc.\n");
	    printf("      [text].[+-][<n>] rel. to current loc.\n");
  case 'o': printf("      [text]<n>\n");
	    break;
  case 's': printf("      <0..3> word no. <value>\n      t<field name> <value>\n");
if (cmd == 'p')
    {
	printf("   pixel\n   command out\n   output reg\n");
    }
	    printf("   block of microcode\n");
	    printf("      <word no> <value> <lowlimit> <highlimit>\n");
	    printf("      t<fieldname> <value> <lowlimit> <highimit>\n");
  }
}


dom()
{
  switch (cmd) {
    case 'o':
    case 'p': printm(); break;
    case 's': storem(); break;
    }
}

dof()
{
  switch (cmd) {
    case 'p': printflag(); break;
    case 's': storeflag(); break;
    }
}

doi()
{
  switch(cmd) {
    case 'p': printf("%04x\n",FBCdata); break;
    case 's': FBCdata = getnum(); break;
    }
}

doo()
{
  switch(cmd) {
    case 'p': FBCflags = (devstatus&0xf0) | (READOUTRUN & 0xf);
		printf("%04x\n",FBCdata);
		FBCflags = devstatus;
		break;
    default: illcmd();
  }
}

doc()	/* for printing bpc command bits  */
{
  switch(cmd) {
#ifndef GFBETA
     case 'p': printf("%x\n",(FBCdata>>8)&0xf); break;
#else
     case 'p': FBCflags = (devstatus&0xf0) | READCODERUN;
	printf("cmd = %x   bpc = %x   FBCACK = %x   BPCACK = %x\n",
		(FBCdata>>8)&0x1f,FBCdata&0xff,(FBCdata>>14)&1,
		(FBCdata>>15)&1);
	FBCflags = devstatus;
	break;
#endif
     default:  illcmd();
     }
}

dob()
{
  switch(cmd) {
    case 's': storem(); break;
    case 'p': printm(); break;
    }
}

dop()
{
  switch(cmd) {
    case 'p': printf("%04x\n",FBCpixel); break;
    default: illcmd();
    }
}

illcmd()
{
  printf("Illegal command %c%c\n",cmd,which);
}


#ifdef FILESTUFF
getindata(verb)	/* put next numeric input from file into indata  */
   short verb;
{
   char *getinf();		/* gets next string of chars from file */
   short tx = ix;		/* keep place in line */

  do {				/* scan up to next \n in input file */
	if (*pd == '\0')	/* if end of input buffer hit,  */
	   if((pd = getinf(verb)) == NULL) break;
					/* refill buffer from file */
	line[tx++] = *pd;
     } while (*pd++ != '\n');	/* include \n	*/

  indata = getnum();
  return((short)(pd != NULL && indata != 0xbad));
}

fileGErun()
{
short done=0;

   FBCflags = devstatus = 0x81;
   openinf(line+ix);
   pd = '\0';			/* reset input stream  */
   while (!done)
	{
	  if (getindata(0)) GEdata = indata;
	  else {++done; break; }
	}
}

filerun()	/* open designated file, feed into FBC til EOF hit */
{
   short done = 0;
   short i;

   openinf(line+ix);
   pd = '\0';
   while(!done)
 	{
	  FBCflags = 0xf3;		/* debug, no interrupts  */
	  FBCflags = 0xf3;		/* why not	*/
	  if (getindata(0)) FBCdata = indata;
	  else {++done;break;}		/* quit on EOF  */
	  FBCflags = 0xd3;		/* force request	*/
	  FBCflags = 0xd3;
	  for (i=10000; i>0; --i)	/* allow a while for ack  */
	     if (!(FBCflags & FBCACK_BIT_)) break ;
	 				/* exit on ack	*/
	  if (i==0) {
		printf("Timeout\n");
		++done;		/* if counted to 0, give up */
	    }
	}
   FBCflags = devstatus = 0xf3;
}
#endif FILESTUFF

dismiss(num)
	register num;
{
	if (num==0) num = 1;
	while (num-- > 0) FBCclrint;
}

cmdhelp()
{
	printf("\n<cmd> is 1st letter of:\n");
	printf("   bpc op <type> <code> <data>\n");
	printf("   Breakpoint set [no LOADOUT]\n");
	printf("   call macro <name>\n");
	printf("   Callable macro [#n] display\n");
	printf("   dismiss interrupt\n");
	printf("   fifo/file/test <which>\n");
	printf("   geom. engine cmd <which>\n");
	printf("   Geometry engine test spiral\n");
	printf("   init hardware <runmode>\n");
	printf("   Init tables after ir\n");
#ifdef GF2
	printf("   monitor format toggle\n");
#endif
	printf("   Mask of GE's force to <n> after init\n");
	printf("   open microcode loc. <n>\n");
	printf("   print <which> <how>\n");
	printf("   <CR> print next loc.\n");
	printf("   quit this command level\n---more---");
	getchar();
#ifdef PM2
	printf("   Quick GE confidence test\n   ^Re-init screen\n");
#endif
	printf("   ram test <which>\n");
	printf("   store <which> <how>\n");
	printf("   Setup for vector testing ('SD' for dummy FIFOs)\n");
	printf("   test gf board\n");
#ifdef GF2
	printf("   Token flag test\n");
#endif
	printf("   Unset breakpoint(s): Ua = all  Ul <n> loc. n\n");
	printf("   vector testing\n");
	printf("   Version of microcode force to <n>\n");
	printf("   write microcode\n   Write microcode, preserve breakpts\n");
}
