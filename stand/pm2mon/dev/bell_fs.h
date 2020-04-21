#define	NICFREE	50
#define	NICINOD 100
/*
 * Structure of the super-block
 */
struct	filsys
{
	ushort	s_isize;	/* size in blocks of i-list */
	daddr_t	s_fsize;	/* size in blocks of entire volume */
	short	s_nfree;	/* number of addresses in s_free */
	daddr_t	s_free[NICFREE];	/* free block list */
	short	s_ninode;	/* number of i-nodes in s_inode */
	ino_t	s_inode[NICINOD];	/* free i-node list */
	char	s_flock;	/* lock during free list manipulation */
	char	s_ilock;	/* lock during i-list manipulation */
	char  	s_fmod; 	/* super block modified flag */
	char	s_ronly;	/* mounted read-only flag */
	time_t	s_time; 	/* last super block update */
	short	s_dinfo[4];	/* device information */
	daddr_t	s_tfree;	/* total free blocks*/
	ino_t	s_tinode;	/* total free inodes */
	char	s_fname[6];	/* file system name */
	char	s_fpack[6];	/* file system pack name */
	long	s_fill[14];	/* ADJUST to make sizeof filsys be 512 */
	ino_t	s_lasti;	/* start place for circular search */
	ino_t	s_nbehind;	/* est # free inodes before s_lasti */
	long	s_magic;	/* magic number to indicate new file system */
	long	s_type;		/* type of new file system */
};

#define	FsMAGIC	0xfd187e20	/* s_magic number */

#define	Fs1b	1	/* 512 byte block */
#define	Fs2b	2	/* 1024 byte block */
#define	Fs4b	4	/* 2048 byte block */

# define SUPERBLOCK	1
# define INODEBLOCK	(SUPERBLOCK+1)

# define NDIRECT	10
# define NIADDRS	(NADDR-NDIRECT)
