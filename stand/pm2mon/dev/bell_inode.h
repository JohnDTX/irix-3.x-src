/* @(#)inode.h	1.4 */
/*
 * The I node is the focus of all
 * file activity in unix. There is a unique
 * inode allocated for each active file,
 * each current directory, each mounted-on
 * file, text file, and the root. An inode is 'named'
 * by its dev/inumber pair. (iget/iget.c)
 * Data, from mode on, is read in
 * from permanent inode on volume.
 */

#define	NADDR	13
#define	NSADDR	(NADDR*sizeof(daddr_t)/sizeof(short))

struct	inode {
	struct inode *i_forw;	/* hash chain forw */
	struct inode *i_back;	/* hash chain back */
	short	i_flag;
	cnt_t	i_count;	/* reference count */
	dev_t	i_dev;		/* device where inode resides */
	ino_t	i_number;	/* i number, 1-to-1 with device address */
	ushort	i_mode;
	short	i_nlink;	/* directory entries */
	ushort	i_uid;		/* owner */
	ushort	i_gid;		/* group of owner */
	off_t	i_size;		/* size of file */
	struct {
		union {
			daddr_t i_a[NADDR];	/* if normal file/directory */
			short	i_f[NSADDR];	/* if fifo's */
			char	*i_r;		/* if remote file */
		} i_p;
		daddr_t	i_l;	/* last logical block read (for read-ahead) */
	} i_blks;
};

/* flags */
#define	ILOCK	00001		/* inode is locked */
#define	IUPD	00002		/* file has been modified */
#define	IACC	00004		/* inode access time to be updated */
#define	IMOUNT	00010		/* inode is mounted on */
#define	IWANT	00020		/* some process waiting on lock */
#define	ITEXT	00040		/* inode is pure text prototype */
#define	ICHG	00100		/* inode has been changed */
#define	ISYN	00200		/* do synchronous write for iupdate */
#define IFLUX	00400		/* inode is in transit */
#define IREMOTE	01000		/* inode is remote */

/* modes */
#define	IFMT	0170000		/* type of file */
#define		IFDIR	0040000	/* directory */
#define		IFCHR	0020000	/* character special */
#define		IFBLK	0060000	/* block special */
#define		IFREG	0100000	/* regular */
#define		IFMPC	0030000	/* multiplexed char special */
#define		IFMPB	0070000	/* multiplexed block special */
#define		IFIFO	0010000	/* fifo special */
#define		IFLNK	0120000	/* symbolic link */
#define	ISUID	04000		/* set user id on execution */
#define	ISGID	02000		/* set group id on execution */
#define ISVTX	01000		/* save swapped text even after use */
#define	IREAD	0400		/* read, write, execute permissions */
#define	IWRITE	0200
#define	IEXEC	0100
#define IRWXRWXRWX	0000777	/* rwx permissions for all */

/* modes which apply only to symbolic links */
#define INLNK	02000		/* network symbolic link */
#define ILNIX	00077		/* mask for logical network number */
#define ISQUASH	04000		/* flag for "squashed" symbolic link */

#define i_rmtinfo	i_blks.i_r

#define	i_addr	i_blks.i_p.i_a
#define	i_lastr	i_blks.i_l
#define	i_rdev	i_blks.i_p.i_a[0]

#define	i_faddr	i_blks.i_p.i_a
#define	NFADDR	10
#define	PIPSIZ	NFADDR*DEV_BSIZE
#define	i_frptr	i_blks.i_p.i_f[NSADDR-5]
#define	i_fwptr	i_blks.i_p.i_f[NSADDR-4]
#define	i_frcnt	i_blks.i_p.i_f[NSADDR-3]
#define	i_fwcnt	i_blks.i_p.i_f[NSADDR-2]
#define	i_fflag	i_blks.i_p.i_f[NSADDR-1]
#define	IFIR	01
#define	IFIW	02

#ifdef	KERNEL
struct	inode *inode;		/* the inode table itself */
struct	inode *inodeNINODE;	/* the end of the inode table */
short	ninode;			/* number of slots in the table */
#endif
