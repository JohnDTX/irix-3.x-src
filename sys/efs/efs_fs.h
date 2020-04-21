#ifndef	_efs_fs_
#define	_efs_fs_
/*
 * An extent filesystem is composed of some number of basic blocks.  A basic
 * block is composed of one disk sector (512 bytes).  The term
 * "bb" is used as short hand for a basic block, whatever size it is.
 *
 * The following is the physical layout of an efs:
 *	1. Basic block 0 is unused.
 *	2. Basic block 1 is the efs superblock.
 *	3. Basic block 2 begins the bitmap.  The bitmap covers
 *	   Basic blocks 2 through 2 + BTOD(fs->fs_bmsize) - 1, inclusively.
 *	4. Between the end of the bitmap and the basic block fs->fs_firstcg
 *	   are some number of unused basic blocks.  fs->fs_firstcg specifies
 *	   the start of the first cylinder group, in basic blocks.
 *	5. Beginning at fs->fs_firstcg are fs->fs_ncg's worth of cylinder
 *	   groups.  The cylinder group size is fs->fs_cgfsize basic blocks.
 *	   Each cylinder group contains fs->fs_cgisize worth of inode basic
 *	   blocks.  The remaining basic blocks in the cylinder group are
 *	   data blocks.
 *	6. At the end of the filesystem, just past the end of the last
 *	   cylinder group are some number of trailing unused disk sectors.
 *
 * How the layout is parameterized in the superblock:
 *	1. The size of the filesystem in basic blocks, excluding the trailing
 *	   unused basic blocks after the last cylinder group, is contained in
 *	   fs->fs_size.
 *	2.
 * Caveats:
 *	1. Trying to change the parameterization of the basic block size
 *	   probably won't work.
 */

/*
 * Locations of the efs superblock and bitmap.
 */
#define	EFS_SUPERBB	((daddr_t)1)		/* bb # of the superblock */
#define	EFS_BITMAPBB	((daddr_t)2)		/* bb # of the bitmap */
#define	EFS_SUPERBOFF	BBTOB(EFS_SUPERBB)	/* superblock byte offset */
#define	EFS_BITMAPBOFF	BBTOB(EFS_BITMAPBB)	/* bitmap byte offset */

/*
 * Inode parameters.
 */
/* number of inodes per bb */
#define	EFS_INOPBB	(1 << EFS_INOPBBSHIFT)
#define	EFS_INOPBBSHIFT	(BBSHIFT - EFS_EFSINOSHIFT)
#define	EFS_INOPBBMASK	(EFS_INOPBB - 1)

/*
 * This macro initializes the computable fields of the efs superblock so
 * as to insure that the macros that use these fields will work.  It's other
 * purpose is to allow the macros to use the computable fields so that they
 * are simpler.  This macro should be executed, whenever a program wishes
 * to manipulate the filesystem, before actually manipulating the filesystem.
 */
#define	EFS_SETUP_SUPERB(fs) \
	{ \
		(fs)->fs_ipcg = EFS_COMPUTE_IPCG(fs); \
	}

/*
 * Compute the number of inodes-per-cylinder-group (IPCG) and the number
 * of inodes-per-basic-block (INOPBB).
 */
#define	EFS_COMPUTE_IPCG(fs) \
	((short) ((fs)->fs_cgisize << EFS_INOPBBSHIFT))

/*
 * Layout macros.  These macro provide easy access to the layout by
 * translating between sectors, basic blocks, and inode numbers.
 * WARNING: The macro EFS_SETUP_SUPERB must be executed before most
 * of these macros!
 */

/* inode number to bb, relative to cylinder group */
#define	EFS_ITOCGBB(fs, i) \
	((daddr_t) (((i) >> EFS_INOPBBSHIFT) % (fs)->fs_cgisize))

/* inode number to offset from bb base */
#define	EFS_ITOO(fs, i) \
	((short) ((i) & EFS_INOPBBMASK))

/* inode number to cylinder group */
#define	EFS_ITOCG(fs, i) \
	((short) ((i) / (fs)->fs_ipcg))

/* inode number to cylinder group inode number offset */
#define	EFS_ITOCGOFF(fs, i) \
	((short) ((i) % (fs)->fs_ipcg))

/* inode number to disk bb number */
#define	EFS_ITOBB(fs, i) \
	((daddr_t) ((fs)->fs_firstcg + \
		    (EFS_ITOCG(fs, i) * (fs)->fs_cgfsize) + \
		    EFS_ITOCGBB(fs, i)))

/* bb to cylinder group number */
#define	EFS_BBTOCG(fs, bb) \
	((short) ((bb - (fs)->fs_firstcg) / (fs)->fs_cgfsize))

/* cylinder group number to disk bb of base of cg */
#define	EFS_CGIMIN(fs, cg) \
	((daddr_t) ((fs)->fs_firstcg + (cg) * (fs)->fs_cgfsize))

/* inode number to base inode number in its chunk */
#define	EFS_ITOCHUNKI(fs, cg, inum) \
	(((((inum) - (cg)->cg_firsti) / (fs)->fs_inopchunk) * \
	  (fs)->fs_inopchunk) + (cg)->cg_firsti)

/*
 * Allocation parameters.  EFS_MINFREE is based on the unix typical
 * file size - less than 1k.  EFS_MINDIRFREE is based on a guess as
 * to the typical number of those less than 1k files per directory.
 * EFS_ICHUNKSIZE is just a good number for the given hardware.
 */
#define	EFS_MINFREEBB		2
#define	EFS_MINDIRFREEBB	(10 * 2)
#define	EFS_INOPCHUNK		(NBPG / sizeof(struct efs_dinode))

#ifdef	KERNEL
/* XXX tunable parameters */
extern	long efs_lbsize;		/* logical block size */
extern	long efs_lbshift;		/* logical block shift */
extern	long efs_bbsperlb;		/* # of bb's per logical block */
extern	long efs_lbtobbshift;		/* convert logical block to bb, log2 */
extern	long efs_inopchunk;		/* # of inodes per inode chunk */
extern	long efs_minfree;		/* minimum bb's free for file creat */
extern	long efs_mindirfree;		/* min bb's free for dir creat */
#endif	KERNEL

#endif	/* _efs_fs_ */
