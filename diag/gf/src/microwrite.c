/* microwrite.c
 *
 *	write microcode onto GL2 board
 *	returns -1 if writes correctly
 *	returns failing address otherwise { 0xSWWW : S=slice W=word adr}
 */

#include "gfdev.h"

extern unsigned short ucode[][4];

rewrite()	/* write and verify microcode */
{
  register i;
  register wd;
  register short mask;
  short got;

	FBCflags = WRITEMICRO;

	for (i=0; i<0x4000; i++) {
		wd = (i>>12) &3;
		FBCmicroslice(wd,i);
		FBCmicrocode(i) = ucode[i&0xfff][wd];
	}

	FBCflags = READMICRO;
	mask = 0xffff;
	for (i=0; i<0x4000; i++) {
		wd = (i>>12) &3;
		FBCmicroslice(wd,i);
		if (wd==3) mask = 0xff;
		if ( (got = FBCmicrocode(i)&mask)
			!= ucode[i&0xfff][wd]       ) return(i);
	}
	FBCflags = RUNMODE;
	return(-1);
}

