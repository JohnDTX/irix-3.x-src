/*
 * $Source: /d2/3.7/src/stand/lib/dev/RCS/dk.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:14:24 $
 */
/*
 * Routines and data structures common to all disk drivers.
 *
 */

#include "types.h"
#include "sys/param.h"
#include "buf.h"
#include "sys/iobuf.h"
#include "sys/dklabel.h"
#include "sys/dk.h"
#include "sys/dkerror.h"
#include "dprintf.h"

/* disk type structure for pretty printout in driver attach routines */
char *dk_types[] = {
	"Atasi 3046", "Vertex V170", "Fujitsu 2312", "Fujitsu 2351A",
	"Maxtor 1085", "CDC Wren II", "Vertex V185", "Hitachi DK-5118",
	"Maxtor 1140", "Memorex 514", "Fujitsu 2243", "Vertex V130",
};

/*
 * dk_prname:
 *	- print out a disk name during attach time
 */
dk_prname(dl)
	register struct disk_label *dl;
{
	if (dl)
		dprintf(("(%s Name: %s) ", dk_types[dl->d_type], dl->d_name));
	else
		dprintf(("(**NO LABEL**) "));
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
	if ((unsigned)bp->b_iobn > fs->d_size) {
		return (1);
	}
	if (bp->b_iobn == fs->d_size) {
		bp->b_resid = bp->b_bcount;
		bp->b_error = 0;
		iodone(bp);
		return (1);
	}
	if (bp->b_iobn + ((bp->b_bcount+(blksize-1)) >> blkshift) >
			fs->d_size) {
		u_int oldbcnt;

		oldbcnt = bp->b_bcount;
		bp->b_bcount = (fs->d_size - bp->b_iobn) << blkshift;
		bp->b_resid = oldbcnt - bp->b_bcount;
	}
	return (0);
}

/* Must not be static for PROM case! */
char _dkerror_buf[40];

/*
 * dkerror:
 *	- search the array of dkerror structures of length ndke for the
 *	  given error code.  return the message string
 *
 * XXX	- won't properly deal with multiple interruptible invalid errors
 */
char *
dkerror(error, dke)
	register u_char error;
	register struct dkerror *dke;
{

	while (dke->dke_msg) {
		if (dke->dke_errornum == error)
			return (dke->dke_msg);
		dke++;
	}

	sprintf(_dkerror_buf, "unknown error, code 0x%x", error);
	return (_dkerror_buf);
}
