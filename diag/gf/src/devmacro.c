/* devmacro -- command line input with nested macros */

#include "m68000.h"
#include "pcmap.h"

#include "fbcld.h"

#define NLEVELS 16

extern char line[];
extern short ix;
extern char prompt;
extern short intoccurred;

struct macro {
	char body[BODSIZ];
	char tag;
	}	macs[NMACS];
short macstarted = 0;	/* mac definition in progress */
short macx = 0;		/* index for next new macro */
short maccalled;	/* index of macro called */
short callstarted = 0;	/* macro call in progress */
char linbuf[200];	/* buffer for multi-line macro input */
char *lbx = linbuf;
char *mpstack[NLEVELS];		/* stack of ptrs to next char in macro */
int mreptstack[NLEVELS];	/* stack of repeat counts */
char *mbodystack[NLEVELS];	/* stack of ptrs to 1st char for macro rept*/
char *mp;		/* ptr to next char in current body */


getcmdlin(fromkbd)
    short fromkbd;
{
  if (callstarted) callmacro();  /* if not at top level, get next macroline */
  else
    {
	if (fromkbd) {
	    putchar(prompt);
	    getlin(prompt);
	}
	while (line[ix] == '/' || macstarted)  /* handle lines of definition*/
	  {
	    if (line[ix] == '/')
		{
		  ++ix;
		  if (!macstarted) savemacro();
		  else savename();
		}
	    else savemacro();
	    ix = 0;
	    getlin(',');
	  }
    }
  while (line[ix] == 'c') startcall();  /* at any level, test for call */
}

startcall()	/* recognize tag in line buffer, stack previous macro,	*/
{		/* and fetch first line of called macro			*/
  ++ix;					/* point to tag		*/
  if ((maccalled = matchtag()) <0 )
	printf("macro %c undefined\n",line[ix]);
  else
     {
	++ix;
	mreptstack[++callstarted] = getnum();	/* stack rept count */
	if (mreptstack[callstarted]==0) mreptstack[callstarted] = 1;
	mpstack[callstarted] = mp;	/* stack ptr to rest of cur. body */
	mbodystack[callstarted] = mp = macs[maccalled].body;
					/* point to new body */
	callmacro();		/* get first line of new macro */
     }
}


callmacro()	/* unsave next line of macro called */
{
  ix=0;
  while ((line[ix++] = *mp++) != '\n') ;
  line[ix] = 0;
  ix = 0;
/*printf("line:%s:*mp:%c:rept:%d\n",line,*mp,mreptstack[callstarted]);*/
  if (!*mp)		/* end of body --  */
     {
	if ((--mreptstack[callstarted])==0)
		mp = mpstack[callstarted--];  /* repts done - unstack */
	else mp = mbodystack[callstarted];	/* else repeat current level*/
     }
  if (nwgetchar() == 003) {	/* check for break char */
	printf("^C\n");
	callstarted = 0;
	ix = 0;
	line[ix++] = '\n';
	line[ix] = 0;
  }
}


savemacro()	/* buffers one line of macro */
{
  short tx;

  tx = ix;
  if (!macstarted) {  lbx = linbuf; ++macstarted; }
  while ( *lbx++ = line[tx++] ) ;
  --lbx;		/* overwrite EOF next time */
  putchar(',');		/* prompt next line */
}


savename()
{
  short dest;
  char *mx;

  if ((dest = matchtag() ) <0 )	/* if new name */
    {
	dest = macx++;	/* new tag, new index */
	macs[dest].tag = line[ix];	/* save new tag */
    }
  mx = macs[dest].body;  lbx = linbuf;	/* init ptrs */
  while (*mx++ = *lbx++);	/* copy new body */
  --macstarted;
  putchar('!');
}

matchtag()
{
  short i;
  short s = -1;

  for (i=0; i<NMACS; ++i)
    if (macs[i].tag == line[ix]) s=i;
  return(s);
}

printmac(i)
   short i;
{
	register j;
	register char *cp;

	if (i==0)
		for (j=0; j<macx; j++) {
			if ((j%3)==0) putchar('\n');
			printf("  %x  '%c'        ",j,macs[j].tag);
		}

	printf("\n\n");
	for (cp=macs[i].body; *cp; cp++) {
		if (*cp=='\n') printf("   ");
		else putchar(*cp);
	}
	printf("\n\\%c\n",macs[i].tag);
}
