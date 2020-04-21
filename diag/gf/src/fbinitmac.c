/* fbinitmac.c  --  initialize standard macros for frame buffer contol
		 console */

#include "fbcld.h"

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
  if (macx) return(0);	/* already initialized... exit */

  for (i=0; i<NMACS; ++i) macs[i].body[0] = 'U';
			/* all previously unclaimed macros Undefined */

  ninits = 0;

/*******   MACRO DEFINITIONS GO HERE   *****/

  NEXTTAG = 'i';		/* cycle input handshake for subst. input */
#ifdef GFALPHA
  NEXTMAC = "sfd3\nsff3\n\0";
#else
  NEXTMAC = "sf63\nsf73\n\0";
#endif

  NEXTTAG = 's';		/* set up for GE pipeline tests */
#ifdef GFALPHA
  NEXTMAC = "gs0\ngs7\nsi3e\nsfd7\nsf85\nfo\n\0";
#else
  NEXTMAC = "S\n\0";
#endif

  NEXTTAG = 'O';		/* time-waster	*/
  NEXTMAC = "om\n\0";

  NEXTTAG = 'p';		/* vector test of whole pipe	*/
  NEXTMAC = "gC\ngp\nc#\n\0";

  NEXTTAG = 'P';		/* passthru test */
  NEXTMAC = "gC\ngP\nc#\n\0";

  NEXTTAG = 'f';			/* FIFO test */
  NEXTMAC = "ft\nc#\ngi\n\0";

  NEXTTAG = '#';		/* skeleton for GE tests */
#ifdef GFBETA
  NEXTMAC = "cO40\nci\nsf63\nfo2\nsff7\ngs2f\ngs26\nsf35\nfo\n\0";
#else
#if DEVEL || GF2
  NEXTMAC = "fo1\n\0";
#else
  NEXTMAC = "cO40\nci\nsfd3\nfo2\nsff7\ngs0\ngs7\nsf85\nfo\n\0";
#endif DEVEL
#endif GFBETA

  NEXTTAG = '$';
#ifdef GFBETA
  NEXTMAC = "cO40\nci\nsf63\nfo1\nsff7\ngs2f\ngs26\nsf35\nfo\n\0";
#else
#if DEVEL || GF2
  NEXTMAC = "fo0\n\0";
#else
  NEXTMAC = "cO40\nci\nsfd3\nfo1\nsff7\ngs0\ngs7\nsf85\nfo\n\0";
#endif DEVEL
#endif


  NEXTTAG = '1';		/* test of GE 1 */
  NEXTMAC = "gC1\nge1\nc$\n\0";

  NEXTTAG = '2';
  NEXTMAC = "gC2\nge2\nc$\n\0";

  NEXTTAG = '3';
  NEXTMAC = "gC3\nge3\nc$\n\0";

  NEXTTAG = '4';
  NEXTMAC = "gC4\nge4\nc$\n\0";

  NEXTTAG = '5';
  NEXTMAC = "gC5\nge5\nc$\n\0";

  NEXTTAG = '6';
  NEXTMAC = "gC6\nge6\nc$\n\0";

  NEXTTAG = '7';
  NEXTMAC = "gC7\nge7\nc$\n\0";

  NEXTTAG = '8';
  NEXTMAC = "gC8\nge8\nc$\n\0";

  NEXTTAG = '9';
  NEXTMAC = "gC9\nge9\nc$\n\0";

  NEXTTAG = 'a';
  NEXTMAC = "gCa\ngea\nc$\n\0";

#ifndef GFALPHA
  NEXTTAG = 'b';
  NEXTMAC = "gCb\ngeb\nc$\n\0";

  NEXTTAG = 'c';
  NEXTMAC = "gCc\ngec\nc$\n\0";
#endif

  NEXTTAG = 't';	/* call after gT<n> to test chip <n>	*/
  NEXTMAC = "gw1\nc1\nc2\nc3\nc4\nc5\nc6\nc7\nc8\nc9\nca\ncb\ncc\n\0";

  NEXTTAG = 'd';	/* drawing test */
#ifdef GF2
  NEXTMAC = "fq\nir\ngT\ngC\ngw1\ngt\ncR80\n\0";
#else
  NEXTMAC = "fq\nir\ngC\ngw1\ngt5\ncR100\n\0";
#endif

  NEXTTAG = 'R';
#ifdef GF2
  NEXTMAC = "gt1-7\n\0";
#else
  NEXTMAC = "gt6\ngt2\ngt7\ngt2\ngt8\ngt2\n\0";
#endif

  NEXTTAG = 'T';
#ifdef GF2
  NEXTMAC = "gw1\ngB1\nfb\ngB2\nfb\ngB3\nfb\ngB4\nfb\ngB5\nfb\ngB6\nfb\ngB7\nfb\ngB8\nfb\ngB9\nfb\ngBa\nfb\ngBb\nfb\ngBc\nfb\n\0";
#else
  NEXTMAC = "gw1\ngB1\nc$\ngB2\nc$\ngB3\nc$\ngB4\nc$\ngB5\nc$\ngB6\nc$\ngB7\nc$\ngB8\nc$\ngB9\nc$\ngBa\nc$\ngBb\nc$\ngBc\nc$\n\0";
#endif

#ifdef GF2
  NEXTTAG = 'g';
  NEXTMAC = "gw1\nga1\nc$\nga2\nc$\nga3\nc$\nga4\nc$\nga5\nc$\nga6\nc$\nga7\nc$\nga8\nc$\nga9\nc$\ngaa\nc$\ngab\nc$\ngac\nc$\ngad\nc$\ngae\nc$\ngaf\nc$\n";

  NEXTTAG = 'G';
  NEXTMAC = "gw1\ngaF1\nfb\ngaF2\nfb\ngaF3\nfb\ngaF4\nfb\ngaF5\nfb\ngaF6\nfb\ngaF7\nfb\ngaF8\nfb\ngaF9\nfb\ngaFa\nfb\ngaFb\nfb\ngaFc\nfb\ngaFd\nfb\ngaFe\nfb\ngaFf\nfb\n";
#endif
/******************************************/

  for (i=0; i<ninits; ++i)
    {
	ii=0; ip=ist[i];
	while (macs[i].body[ii++] = *ip++) ;
    }

macx = ninits;
}
