/*
* $Source: /d2/3.7/src/stand/include/RCS/fs.h,v $
* $Revision: 1.1 $
* $Date: 89/03/27 17:13:43 $
*/
/*
 * Structure of the filesystem super-block's
 *
 */

/*
 * Filesystem independent definitions.  This section defines constants
 * common to both filesystems.  These areas are very specifically designed
 * to match on both filesystems to allow the system to automagically
 * figure out the filesystem type during mount'ing
 */
#define	SUPERB	((daddr_t)1)	/* physical block number of the super block */
#define	SUPERBOFF	512	/* byte offset of the super block */

/* first inode number, and root inode number */
#define	FIRSTINO	((ino_t)1)
#define	ROOTINO		((ino_t)2)

/* XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX */
/* XXX where should this stuff go? */

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

/*
 * Structure used by the bmap's to fill in block/length information.
 */
struct	iextent {
	daddr_t	bn;		/* physical block # */
	long	length;		/* length of this extent in bytes */
};

/* XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX */

/*
 * BELL FILESYSTEM DEFINITIONS
 */
#ifdef	KERNEL
#include "../h/filsys.h"
#include "../h/fblk.h"
#else
#include "filsys.h"
#include <sys/fblk.h>
#endif

/*
 * EXTENT FILESYSTEM DEFINITIONS
 */

/*
 * The extent file system is a structure imposed on a contiguous
 * range of blocks.  Its gross layout is as follows (all offsets
 * and sizes are denominated in BBSIZE blocks):
 * 	Block 0 is unused.
 * 	Block 1 is the superblock.
 * 	Blocks [2 .. fs_bmsize) are the free block bitmap.
 * 	Blocks [fs_bmsize .. fs_firstcg) are "runt" data blocks.
 * 	Starting at block fs_firstcg, and every fs_cgsize blocks
 *	thereafter, are	fs_cgisize blocks of inodes.
 * 	All other blocks are data blocks.
 *
 * Note:
 *	The macros are defined ASSUMING that inodes are <= size of a BB
 */
#define	BITMAPOFF	(2 * BBSIZE)	/* byte offset to the bitmap */
#define	BITMAPB		((daddr_t)2)	/* first phys block # of the bitmap */

/*
 * macros for finding an inode
 */
/* cylinder group to inode-base disk block number */
#define	CGIMIN(fs, cg) \
	((fs)->fs_firstcg + (cg) * (fs)->fs_cgfsize)

/* inumber to offset from bb base */
#define	ITOO(fs, i) \
	((i) & EFS_INOPBBMASK)

/* inumber to cg number */
#define	ITOCG(fs, i) \
	((i) / (fs)->fs_ipcg)

/* inumber to disk block number */
#define	ITOD(fs, i) \
	((daddr_t)(CGIMIN(fs, ITOCG(fs, i)) + \
			 (((i) % (fs)->fs_ipcg) >> EFS_INOPBBSHIFT)))
/*			 ((i) >> EFS_INOPBBSHIFT) % (fs)->fs_cgisize)) */

/* bb to cg # */
#define	BBTOCG(fs, bn) \
	((bn - (fs)->fs_firstcg) / (fs)->fs_cgfsize)

/* number of inodes per bb */
#define	EFS_INOPBB	(1 << EFS_INOPBBSHIFT)
#define	EFS_INOPBBSHIFT	(BBSHIFT - EFS_DINODESHIFT)
#define	EFS_INOPBBMASK	(EFS_INOPBB - 1)

/* sizeof(efs_dinode), log2 */
#define	EFS_DINODESHIFT	7

/* number of efs inodes per page */
#define	EFS_INODESPERPAGE	(NBPG / sizeof(struct efs_dinode))

/* maximum extents per file */
#define	EFS_MAXEXTENTS	256

/*
 * Software summary structure per cylinder group.  When a filesystem is
 * mounted, the system builds a cylinder group table for containing summary
 * allocation information.
 */
#define	CG_DSUMS	20
#define	CG_ISUMS	20
struct	cg {
	u_char	cg_wanted;		/* somebody wants cg lock */
	u_char	cg_corrupted;		/* cg is bad; don't use */
	daddr_t	cg_firstbn;		/* first bn in cg */
	daddr_t	cg_firstdbn;		/* first data block bn */
	ino_t	cg_firsti;		/* first inode # in cg */

	/*
	 * Inode allocation summary information.  When a cylinder group
	 * is first scanned, we build up the inode allocation bit map
	 * as well as summary information for quick allocation.  cg_isums
	 * contains summary information (which cg_nexti indexes) about
	 * the first CG_ISUMS worth of inodes.  Cg_firstifree is the
	 * inumber of the first free inode in the cg, prior to the filling
	 * of the cg_isums info.
	 */
	long	cg_ifree;		/* count of free inodes */
	ino_t	cg_firstifree;		/* first inode # that's free */
	long	cg_nexti;		/* current index into cg_isums */
	long	cg_lasti;		/* last index in cg_isums filled */
	ino_t	cg_isum[CG_ISUMS];	/* array of free inodes */

	/*
	 * Data allocation summary information.  When the cg is scanned,
	 * we build up information about the total number of free data
	 * blocks, as well as a map of the first CG_DSUMS worth of
	 * data regions.
	 */
	long	cg_dfree;		/* count of free data blocks */
	daddr_t	cg_firstdfree;		/* first free data bn */
	long	cg_nextd;		/* current index into cg_dsums */
	long	cg_lastd;		/* last index in cg_dsums filled */
	struct	cgdmap {
		long	d_bn;		/* block # of extent */
		short	d_length;	/* length of extent */
	} cg_dsum[CG_DSUMS];
};

/* structure of the super-block for the extent filesystem */
struct	efs {
	/*
	 * This portion is read off the volume
	 */
	long	fs_size;	/* size of filesystem, in sectors */
	long	fs_firstcg;	/* bb offset to first cg */
	long	fs_cgfsize;	/* size of cylinder group in bb's */
	short	fs_cgisize;	/* bb's in inodes per cylinder group */
	short	fs_sectors;	/* sectors per track */
	short	fs_heads;	/* heads per cylinder */
	short	fs_ncg;		/* # of groups in filesystem */
	short	fs_dirty;	/* fs needs to be fsck'd */
	time_t	fs_time;	/* last super-block update */
	char	fs_fname[6];	/* file system name */
	char	fs_fpack[6];	/* file system pack name */
	long	fs_magic;	/* magic number */
	long	fs_prealloc;	/* a good filesystem pre-alloc size */
	long	fs_bmsize;	/* size of bitmap in bytes */
	long	fs_tfree;	/* total free data blocks */
	long	fs_tinode;	/* total free inodes */
	char	fs_spare[100];	/* space for expansion */
	long	fs_checksum;	/* checksum of volume portion of fs */

	/*
	 * The remainder is used for in-core manipulation.  During
	 * super-block creation, and possible writing in the root's case,
	 * these fields will be written to disk.  It is assumed when
	 * the super-block is read in that these fields contain trash,
	 * and are accordingly initialized.
	 */
	char	fs_readonly;	/* device is read-only */
	char	fs_fmod;	/* filesystem time has been modified */
	char	fs_lock;	/* lock during data bitmap manipulation */
	char	fs_wanted;	/* somebody wants fs */
	char	fs_corrupted;	/* fs is corrupted; no more write's */
	dev_t	fs_dev;		/* device fs is mounted on */

	handle	*fs_imap;	/* bit map for inode allocation */
	ino_t	fs_ipcg;	/* # of inodes per cg */
	long	fs_imapsize;	/* size of each cg imap */
	u_char	fs_diskfull;	/* had to do a brute search for data blocks */

	/*
	 * An array of cg structs is managed here.  During mounting
	 * the size of this structure plus the number of cg structs
	 * that will be needed, minus 1 (because of the one defined below),
	 * is dynamically allocated.  This makes it easy to do quick looking
	 * through the cg structs for allocation/de-allocation.
	 */
	struct	cg fs_cgs[1];	/* actually, there are more here */
};
#define	EFS_MAGIC	0x041755

/* filesystem class info */
#define	FS_BELL		1		/* bell filesystem */
#define	FS_EXTENT	2		/* extent filesystem */

/* convenience data structure */
union	ufs {
	struct	filsys u_bell;
	struct	efs u_efs;
};

#ifdef	KERNEL

/* handle conversion utilities */
#define	Ufs(handle)	((union ufs *)((handle)->p))
#define	Filsys(handle)	((struct filsys *)((handle)->p))
#define	Efs(handle)	((struct efs *)((handle)->p))

handle	*getfs();

/* tunable parameters */
extern	long efs_maxreadlen, efs_rafactor, efs_alloclen;
#endif
