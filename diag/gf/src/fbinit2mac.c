/* fbinit2mac.c  --  initialize standard macros for frame buffer contol
		 console */

#ifdef _FBC
#include "fbc.h"
#endif
#ifdef _GF1
#include "fbcld.h"
#endif

#define NEXTMAC ist[ninits++]
#define NEXTTAG  macs[ninits].tag
extern struct macro {
	char body[BODSIZ];
	char tag;
	}	 macs[];
extern short macx, macstarted, callstarted;
char *ist[NMACS];
short ninits;

initmacros()
{
  short i,ii;
  char *ip;

/* initialize state of macro handler */
  macstarted = 0;
  callstarted = 0;

  if (macx) return(0);
  for (i=0; i<NMACS; ++i) macs[i].body[0] = 'U';
		/* all Undefined */

  ninits = 0;

/*******   MACRO DEFINITIONS GO HERE   *****/

  NEXTTAG = 'i';		/* cycle input handshake for subst. input */
  NEXTMAC = "sf63\nsf73\n\0";

  NEXTTAG = 's';		/* set up for GE pipeline tests */
  NEXTMAC = "fq\nir\nsi3e\nxs\nsf63\nsf35\nfo\ngs3f\ngs26\n\0";

  NEXTTAG = 'O';		/* time-waster	*/
  NEXTMAC = "o\n\0";

  NEXTTAG = 'P';		/* stopping passthru test */
  NEXTMAC = "gdff38\ngP\nc#\n\0";

  NEXTTAG = 'p';		/* passthru test */
  NEXTMAC = "gdff38\ngP\nc$\n\0";

  NEXTTAG = 'g';		/* stopping scaler test */
  NEXTMAC = "gdff20\ngb\nc#\n\0";

  NEXTTAG = 'F';			/* FIFO full test */
#ifdef GFALPHA
  NEXTMAC = "gC\nsff5\ngP\ngo\nsf85\nc#\n\0";
#else
  NEXTMAC = "gC\nsf55\ngP\ngo\nsf5\nc#\n\0";
#endif

  NEXTTAG = 'f';		/* FIFO test */
  NEXTMAC = "gd3f00\ngP\nc#\n\0";

  NEXTTAG = '#';		/* skeleton for GE tests */
  NEXTMAC = "cO40\nci\nsf63\nfo2\nsff7\ngs2f\ngs26\nsf35\nfo\n\0";

  NEXTTAG = '$';		/* no GE resets!! */
  NEXTMAC = "cO40\nci\nsf63\nfo1\nsff7\nsf35\nfo0\n\0";


  NEXTTAG = '1';		/* test of GE 1 */
  NEXTMAC = "gdff09\ng1\nc$\n\0";

  NEXTTAG = '2';
  NEXTMAC = "gdff0a\ng2\nc$\n\0";

  NEXTTAG = '3';
  NEXTMAC = "gdff0b\ng3\nc$\n\0";

  NEXTTAG = '4';
  NEXTMAC = "gdff0c\ng4\nc$\n\0";

  NEXTTAG = '5';
  NEXTMAC = "gdff10\ng5\nc$\n\0";

  NEXTTAG = '6';
  NEXTMAC = "gdff11\ng6\nc$\n\0";

  NEXTTAG = '7';
  NEXTMAC = "gdff12\ng7\nc$\n\0";

  NEXTTAG = '8';
  NEXTMAC = "gdff13\ng8\nc$\n\0";

  NEXTTAG = '9';
  NEXTMAC = "gdff14\ng9\nc$\n\0";

  NEXTTAG = 'a';
  NEXTMAC = "gdff15\nga\nc$\n\0";

  NEXTTAG = 'b';
  NEXTMAC = "gdff20\ngb\nc$\n\0";

  NEXTTAG = 'c';
  NEXTMAC = "gdff21\ngc\nc$\n\0";

/******************************************/

  for (i=0; i<ninits; ++i)
    {
	ii=0; ip=ist[i];
	while (macs[i].body[ii++] = *ip++) ;
    }

macx = ninits;
}
