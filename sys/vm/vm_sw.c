/*
 * $Source: /d2/3.7/src/sys/vm/RCS/vm_sw.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:36:05 $
 */
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/conf.h"
#include "../h/user.h"
#include "../h/inode.h"
#include "../h/map.h"
#include "../h/file.h"
#include "../vm/vm.h"

/*
 * Single swapping partition version of swfree()
 */
swfree()
{
	register long blk;
	register swblk_t dvbase;

	(*bdevsw[bmajor(swapdev)].d_open)(swapdev, 1);
	for (dvbase = 0; dvbase < nswap; dvbase += DMMAX) {
		blk = nswap - dvbase;
		if (blk > DMMAX)
			blk = DMMAX;
		if (dvbase == 0) {
			/*
			 * Can't free a block starting at 0 in the swapmap
			 * but need some space for argmap so use 1/2 this
			 * hunk which needs special treatment anyways.
			 */
			rminit(argmap, (long)(blk/2-ctod(1)),
				       swplo + ctod(1),
				       "argmap", ARGMAPSIZE);
			/*
			 * First of all chunks... initialize the swapmap
			 * the second half of the hunk.
			 */
			rminit(swapmap, (long)blk/2, swplo + blk/2,
					"swap", nswapmap);
		} else
			rmfree(swapmap, blk, swplo + dvbase);
	}
}
