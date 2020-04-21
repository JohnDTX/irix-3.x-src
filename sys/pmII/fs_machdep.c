/*
 * Machine dependent portions of efs
 *
 * $Source: /d2/3.7/src/sys/pmII/RCS/fs_machdep.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:33:37 $
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
 * Since GL1 systems were often shipped with only 1.5Mb of
 * memory, trim the cache to a maximum of 200kb.  The parameterisation
 * done here for larger systems is artificial - it represents the desire
 * to provide product differentiation - the newer product runs faster.
 */
#ifdef	GL1
#define	SMALLBUF	12		/* 2^12, that is */
#define	LARGEBUF	13
#else
#define	SMALLBUF	13		/* 2^13, that is */
#define	LARGEBUF	14
#endif

/*
 * Based on physical memory available, compute the size of the
 * cache.
 */
efs_setparams(totalpages)
	register long totalpages;
{
	if (nbuf == 0)
		nbuf = 50;
	if (efs_lbshift == 0) {
		if (totalpages <= btoc(0x200000))
			efs_lbshift = SMALLBUF;
		else
			efs_lbshift = LARGEBUF;
	}
	efs_lbsize = 1 << efs_lbshift;
	efs_bbsperlb = BTOBBT(efs_lbsize);
	efs_lbtobbshift = efs_lbshift - BBSHIFT;	/* log2 efs_bbsperlb */
}
