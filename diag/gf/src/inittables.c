/* inittables.c
 */

#include "gf2.h"
#include "gl2cmds.h"

#define FBCsend(d)	FBCdata = (d); \
			FBCflags = CYCINDEBUG; \
			FBCflags = RUNDEBUG;

extern short swizzletab[];
extern unsigned short leftmask[];
extern unsigned short dividetab[];
extern short devstatus;

inittables()
{
	FBCflags = RUNDEBUG;

	sendtab(swizzletab,SWIZZLETAB,256);
	sendtab(leftmask,MASKTAB,32);
	sendtab(dividetab,DIVIDETAB,2048);

	FBCsend(8);
	FBCflags = devstatus;
}


sendtab(tab,adr,nwds)
	register unsigned short *tab;
	short adr,nwds;
{
	register i;

	FBCsend(FBCloadram);
	FBCsend(adr);
	FBCsend(nwds);
	for (i=0; i<nwds; i++) {
		FBCsend(*tab++)
	}
}
