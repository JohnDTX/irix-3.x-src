/* iccmd.c --  console command processor for IC testing
/*		includes device-dependent .h file */

#include "/usr/sun/include/pcmap.h"
#include "/usr/sun/include/m68000.h"

#include "fbc/fbcld.h"
#include "gfdev.h"
#include "dcdev.h"

#define NULL 0

extern char line[];	/* command line buffer */
extern short ix;	/* command line index */
extern short expecting_output;
extern short intcount;
extern short intcmd;
extern short devstatus;	/* copy of currently written status reg */
extern short GEstatus;
char cmd,which,how;
extern short	firsttime,errorct;
extern unsigned short errorlog[][3];

unsigned short indata;	/* data from input file  - set by getindata()  */
short num;	/* current ucode addr */
short val;	/* field designator for dostore */
short low,high;	/* limits for block store */
char *pd = '\0';	/* ptr to next input char */

char *field[NFIELDS][16];	/* text prompts for ucode fields */

char *cats[NFIELDS];		/* names of fields */

unsigned short *outfile;
extern short outx, evenreceived;
extern short waitcount;

unsigned short *pgearray;

/*--------------------------------- */

devcmd(repeat)
   short repeat;
{
  short i;

  cmd = '\n';
  do				/* main command loop */
    {
	ix=0;
	getcmdlin(1);
	how='n';	/* default */

		   /* CMD RECOGNIZER  */
	if (line[ix]=='\n') {num++; how='.';}
	else cmd = line[ix++];

		/* CMD INTERPRETER */
	switch (cmd) {
	  case 'I':
	  case 'i': setup(); break;
	  case 'P':
	  case 'p': passtest(GERESET3); break;
	  case 'F':
	  case 'f': passtest(GEDEBUG); filltest(GEDEBUG); break;
	  case 'V':
	  case 'v': verbtoggle(); break;
	  case 'M':
	  case 'm': singletest(1); break;
	  case 'C': singletest(5); break;
	  case 'S':
 	  case 's': singletest(0xb); break;
	  case 'H':
	  case 'h': carefultest(); break;
	  case 'N':
	  case 'n': singletest(getnum()); break;
	  case 'R':
	  case 'r': reptest(getnum()); break;
	  case 'D': printmac(getnum()); break;
	  case 'W':
	  case 'w': waitcount = getnum(); break;
	  case '?': printf("\ntype 1st letter of:\n");
		    printf("   Handshake test\n");
		    printf("   Passthru test\n");
		    printf("   Fifo test\n");
		    printf("   Matrix mult test\n");
		    printf("   Clipper test\n");
		    printf("   Scaler test\n");
		    printf("   Numbered (1..12) chip test\n");
		    printf("   Repeat (1..12) chip test\n");
		    printf("   Initialize FBC\n");
		    printf("   Verbose mode toggle\n");
		    break;
	  case '\n':
	  case 0:
	  case 'q': break;
	  default : printf("Command error: '%c'?\n",cmd);
	  }

    } while ((cmd != 'q') && repeat) ;
  cmd = '\n';
}


illcmd()
{
  printf("Illegal command %c%c\n",cmd,which);
}
