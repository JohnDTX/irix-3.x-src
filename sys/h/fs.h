#ifndef __fs__
#define __fs__
/*
 * Structure of the filesystem super-block's
 *
 * $Source: /d2/3.7/src/sys/h/RCS/fs.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:29:31 $
 */

/*
 * Filesystem independent definitions.  This section defines constants
 * common to all filesystems.  These areas are very specifically designed
 * to match on all filesystems to allow the system to automagically
 * figure out the filesystem type during mounting
 */

#ifdef	KERNEL
/*
 * Include here ONLY those peculiar filesystem definitions needed by the
 * generic kernel code.
 */
#include "../efs/efs_fs.h"

/*
 * Size of block device i/o is parameterized here.
 * Currently the system supports 1k i/o, for backwards compatability.
 * Doesn't seem to make much sense to change this behaviour.
 */
#define	BLKDEV_IOSHIFT		10
#define	BLKDEV_IOSIZE		(1<<BLKDEV_IOSHIFT)

/*
 * Convert a byte offset into an offset into a logical block for
 * a block device
 */
#define	BLKDEV_OFF(off)		((off) & (BLKDEV_IOSIZE - 1))

/*
 * Convert a byte offset into a block device logical block #
 */
#define	BLKDEV_LBN(off)		((off) >> BLKDEV_IOSHIFT)

/*
 * Number of bb's per block device block
 */
#define	BLKDEV_BB		BTOBB(BLKDEV_IOSIZE)

#else	/* !KERNEL */

/*
 * Include here the definitions for all possible types of filesystems
 */
#include <sys/filsys.h>
#include <sys/fblk.h>
#include <sys/efs_fs.h>
#include <sys/efs_sb.h>

/* first inode number */
#define	FIRSTINO	((ino_t)1)
#endif	/* KERNEL */
#endif	/* __fs__ */
