/* gfinit.c  --- hardware initialization
 *	DEVELOPMENT VERSION
 *	this should be compiled with -DGFBETA and used to reset the GF board.
 *	Subsequently, any reset commands SHOULD NOT affect the GF's FBC.
 *
 */

#include "m68000.h"
#include <gfdev.h>

extern short GEstatus;


gfinit()	 /* returns scratch ram size (beta) ; sets version */
{
	short i;

	intlevel(7);
	GEflags = GEstatus = GERESET1 ;
	FBCflags = STARTDEV;
	FBCflags = STARTDEV & ~FORCEREQ_BIT_;
	FBCflags = STARTDEV & ~FORCEACK_BIT_;
	FBCflags = STARTDEV;
	FBCdata = 0;
	FBCclrint;		/* execute microinstruc. 0x3ff	*/
	FBCflags = RUNDEBUG;
	for (i=0; ++i<20;) FBCclrint;
	if (FBCdata != 0x40)
		printf("Can't reset FBC on GF1\n");
	GEflags = GEstatus = GEDEBUG;
	FBCflags = RUNMODE;		/* allow data in and out */
}

