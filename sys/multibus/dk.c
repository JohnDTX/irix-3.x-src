/*
 * Routines and data structures common to all disk drivers.
 *
 * $Source: /d2/3.7/src/sys/multibus/RCS/dk.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:31:17 $
 */

#include "../h/param.h"
#include "../h/buf.h"
#include "../h/iobuf.h"
#include "../h/dklabel.h"
#include "../h/dk.h"
#include "../h/dkerror.h"
#include "../h/dktype.h"

char *
prdtype(type, dkt, ntypes)
	register u_long type, ntypes;
	register struct dk_type *dkt;
{
	static char buf[40];

	while (ntypes--) {
		if (dkt->d_type == type)
			return (dkt->d_name);
		dkt++;
	}
	sprintf(buf, "unknown disk type (%d)", type);
	return (buf);
}

/*
 * dk_prname:
 *	- print out a disk name during attach time
 */
dk_prname(dl)
	register struct disk_label *dl;
{
	if (dl)
		printf("(%s Name: %s) ",
			prdtype(dl->d_type, dk_dtypes, NTYPES), dl->d_name);
	else
		printf("(**NO LABEL**) ");
}

/*
 * dk_rangecheck:
 *	- check range of request against file system bounds, boudning
 *	  against end, or returning an error when past end
 *	- compute the disk cylinder for disksort
 *	- return 1 if an error occurs
 */
dk_rangecheck(bp, fs, blksize, blkshift)
	register struct buf *bp;
	register struct disk_map *fs;
	register int blksize, blkshift;
{
#ifdef	notdef
	if ((unsigned)bp->b_blkno > fs->d_size) {
		berror(bp);
		return (1);
	}
	if (bp->b_blkno == fs->d_size) {
		bp->b_resid = bp->b_bcount;
		bp->b_error = 0;
		iodone(bp);
		return (1);
	}
	if (bp->b_blkno + ((bp->b_bcount+(blksize-1)) >> blkshift) >
			fs->d_size) {
		u_int oldbcnt;

		oldbcnt = bp->b_bcount;
		bp->b_bcount = (fs->d_size - bp->b_blkno) << blkshift;
		bp->b_resid = oldbcnt - bp->b_bcount;
	}
	return (0);
#else
	/*
	 * Check for exact eof
	 */
	if (bp->b_blkno == fs->d_size) {
		bp->b_resid = bp->b_bcount;
		bp->b_error = 0;
		iodone(bp);
		return (1);
	}

	/*
	 * Check for crossing fs limit
	 */
	if (bp->b_blkno + ((bp->b_bcount + (blksize-1)) >> blkshift) >
			fs->d_size) {
		berror(bp);
		return (1);
	}
	return (0);
#endif
}

/*
 * dkerror:
 *	- search the array of dkerror structures of length ndke for the
 *	  given error code.  return the message string
 *
 * XXX	- won't properly deal with multiple interruptible invalid errors
 */
char *
dkerror(error, dke, ndke)
	register short error;
	register struct dkerror *dke;
	register int ndke;
{
	static char buf[40];

	while (ndke--) {
		if (dke->dke_errornum == error)
			return (dke->dke_msg);
		dke++;
	}

	sprintf(buf, "unknown error, code 0x%x", error);
	return (buf);
}
