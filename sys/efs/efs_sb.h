#ifndef	_efs_sb_
#define	_efs_sb_
/*
 * Structure of the extent filesystem superblock
 * XXX split into #ifdef KERNEL incore struct and external superblock struct
 *
 * $Source: /d2/3.7/src/sys/efs/RCS/efs_sb.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:28:01 $
 */

/*
 * Software summary structure per cylinder group.  When a filesystem is
 * mounted, the system builds a cylinder group table for containing summary
 * allocation information.
 */
#define	CG_DSUMS	20
struct	cg {
	daddr_t	cg_firstbn;		/* first bn in cg */
	daddr_t	cg_firstdbn;		/* first data block bn */
	ino_t	cg_firsti;		/* first inode # in cg */
	ino_t	cg_lasti;		/* last inode # in cg, inclusive */

	/*
	 * Inode allocation summary information.  We keep a simple rotor
	 * which tracks the lowest free inode for a given cg.
	 */
	char	cg_fulli;		/* inode area is full (none free) */
	ino_t	cg_lowi;		/* lowest inode # that's free */

	/*
	 * Data allocation summary information.  When the cg is scanned,
	 * we build up information about the total number of free data
	 * blocks, as well as a map of the first CG_DSUMS worth of
	 * data regions.
	 */
	short	cg_dfree;		/* count of free data blocks */
	daddr_t	cg_firstdfree;		/* first free data bn */
	short	cg_nextd;		/* current index into cg_dsums */
	short	cg_lastd;		/* last index in cg_dsums filled */
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
	char	fs_fmod;	/* filesystem has been modified */
	char	fs_corrupted;	/* fs is corrupted; no more write's */
	dev_t	fs_dev;		/* device fs is mounted on */

	short	fs_inopchunk;	/* # of inodes in an inode chunk */
	daddr_t	fs_inopchunkbb;	/* # of bb's in an inode chunk, rounded up */
	short	fs_minfree;	/* min # of free blocks for file placement */
	short	fs_mindirfree;	/* min # of free blocks for dir placement */
	ino_t	fs_ipcg;	/* # of inodes per cg */
	ino_t	fs_lastinum;	/* last inum in fs */
	char	fs_diskfull;	/* had to do a brute search for data blocks */
	char	*fs_dmap;	/* bit map for data allocation */

	/*
	 * An array of cg structs is managed here.  During mounting
	 * the size of this structure plus the number of cg structs
	 * that will be needed, minus 1 (because of the one defined below),
	 * is dynamically allocated.  This makes it easy to do quick looking
	 * through the cg structs for allocation/de-allocation.
	 */
	struct	cg fs_cgs[1];	/* actually, there are more here */
};
#define	EFS_MAGIC	0x041755L

#define	EFS2_MAGIC	0x041756L	/* +1 */

#ifdef KERNEL
#define	getfs(mp)	((struct efs *) (mp)->m_fs)
#endif

#endif	/* _efs_sb_ */
