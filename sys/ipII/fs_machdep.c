/*
 * Machine dependent portions of efs
 *
 * $Source: /d2/3.7/src/sys/ipII/RCS/fs_machdep.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:30:47 $
 */
#include "../h/param.h"
#include "../h/buf.h"
#include "../multibus/mbvar.h"

/*
 * nbuf and efs_lbshift are patchable.  We set them to zero here to allow
 * this.
 */
short	nbuf = 0;
long	efs_lbshift = 0;

long	efs_lbsize;
long	efs_bbsperlb;
long	efs_lbtobbshift;

/*
 * Based on physical memory available, compute the size of the
 * cache.
 */
void
efs_setparams(totalpages)
	register long totalpages;
{
	register int nnbuf;
	register int bshift;

	if (efs_lbshift == 0) {
		if (totalpages <= btoc(0x200000)) {	/* 2mb machine */
			nnbuf = 50;
			bshift = 13;
		} else
		if (totalpages <= btoc(0x400000)) {	/* 4mb machine */
			nnbuf = 50;
			bshift = 14;
		} else
		if (totalpages <= btoc(0x800000)) {	/* 8mb machine */
			nnbuf = 100;
			bshift = 14;
		} else {				/* BIG machine */
			nnbuf = 200;
			bshift = 14;
		}
		if (nbuf == 0)
			nbuf = nnbuf;
		efs_lbshift = bshift;
	} else {
		if (nbuf == 0)
			nbuf = 50;
	}
	efs_lbsize = 1 << efs_lbshift;
	efs_bbsperlb = BTOBBT(efs_lbsize);
	efs_lbtobbshift = efs_lbshift - BBSHIFT;	/* log2 efs_bbsperlb */
}
