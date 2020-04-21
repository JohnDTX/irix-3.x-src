/*
 * The I node is the focus of all file activity in unix. There is a unique
 * inode allocated for each active file, each current directory, each
 * mounted-on file, text file, and the root. An inode is 'named'
 * by its dev/inumber pair.  Data, from mode on, is read in from
 * permanent inode on volume.
 *
 * $Source: /d2/3.7/src/stand/pm2mon/dev/RCS/efs_inode.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:16:44 $
 */

#define	NADDR	13
#define	NSADDR	(NADDR*sizeof(daddr_t)/sizeof(short))

#define	EFS_DIRECTEXTENTS	12

typedef struct	extent {
unsigned int	ex_magic:8,	/* magic # */
		ex_bn:24,	/* block # */
		ex_length:8,	/* length of this extent, in bb's */
		ex_offset:24;	/* logical bb offset into file */
} extent;

/*
 * This information is common to all inode types
 */
struct	icommon {
	ushort	ic_mode;	/* mode and type of file */
	short	ic_nlink;    	/* number of links to file */
	ushort	ic_uid;      	/* owner's user id */
	ushort	ic_gid;      	/* owner's group id */
	off_t	ic_size;     	/* number of bytes in file */
};

#ifndef	__DINODE__
#define	__DINODE__

/*
 * Old bell style filesystem inode as it appears on disk.  The 40 bytes of
 * addressing information is divided into 13 3 byte address's.  The old
 * style inode is 64 bytes long.
 */
struct	dinode {
	struct	icommon di_common;	/* old stuff */
	char  	di_addr[40];		/* disk block addresses */
	time_t	di_atime;   		/* time last accessed */
	time_t	di_mtime;   		/* time last modified */
	time_t	di_ctime;   		/* time created */
};
/* stuff to make old code work */
#define	di_mode		di_common.ic_mode
#define	di_nlink	di_common.ic_nlink
#define	di_uid		di_common.ic_uid
#define	di_gid		di_common.ic_gid
#define	di_size		di_common.ic_size

#endif

/*
 * Version of bell dinode, as used in core.  This area is allocated per
 * bell inode and contains the converted addresses from the packed 3
 * byte addressing that is present in the dinode.
 */
struct	incore_dinode {
	daddr_t	ii_addr[NADDR];		/* addresses */
};

/*
 * Extent based filesystem inode as it appears on disk.  The efs inode
 * is exactly 128 bytes long.
 */
struct	efs_dinode {
	struct	icommon di_common;
	time_t	di_atime;   	/* time last accessed */
	time_t	di_mtime;   	/* time last modified */
	time_t	di_ctime;   	/* time created */
	time_t	di_etime;	/* time last extended */
	short	di_numextents;	/* # of extents */
	short	di_unused;	/* UNUSED */
	union {
		extent	di_extents[EFS_DIRECTEXTENTS];
		dev_t	di_dev;	/* device for IFCHR/IFBLK */
	} di_u;
};

/*
 * Version of the extent inode as it appears in memory.
 * This is a template for the actual data structure which contains
 * ii_numextents work of ii_extents.
 */
struct	incore_efs_dinode {
	int	ii_numextents;	/* number of extents */
	extent	ii_indir;	/* indirect extent info */
	extent	ii_extents[1];	/* actually, more of these */
};

/*
 * Inode information for pipe's.  Pipes are always allocated from pipedev
 * and thus the i_fs reflects the fs that the data blocks were taken from.
 */
/* PIPSIZ should be configuration dependent/memory size dependent */
#define	PIPSIZ	(10 * 1024)
struct	fspipe {
	short	i_fflag;		/* state flag */
	short	i_frptr;		/* read pointer */
	short	i_frcnt;		/* number of readers count */
	short	i_fwptr;		/* write pointer */
	short	i_fwcnt;		/* number of writers count */
	struct	buf *i_bp;		/* buffer holding data */
	time_t	i_atime;		/* access time */
	time_t	i_mtime;		/* modified time */
	time_t	i_ctime;		/* creation time */
};
#define	IFIR	01
#define	IFIW	02

/*
 * iops defines an interface used to switch out to various class specific
 * operations that are done on inodes.
 */
struct	iops {
	struct	inode *(*iread)();	/* read in an inode from volume */
	int	(*iwrite)();		/* write out an inode to volume */
	int	(*itrunc)();		/* truncate inode */
	struct	inode *(*ialloc)();	/* allocate an inode */
	int	(*ifree)();		/* free an inode */
	int	(*gettimes)();		/* read times from disk */
	int	(*rdwr)();		/* user read/write of inode */
	int	(*bmap)();		/* do logical to physical mapping */
};

struct	inode {
	struct	inode *i_forw;	/* hash chain forw */
	struct	inode *i_back;	/* hash chain back */
	short	i_flag;
	cnt_t	i_count;	/* reference count */
	dev_t	i_dev;		/* device where inode resides */
	ino_t	i_number;	/* i number, 1-to-1 with device address */
	dev_t	i_rdev;		/* underlying device if IFBLK/IFCHR */
	daddr_t	i_lastbn;	/* last bn read (for read-ahead) */
	short	i_lastlen;	/* length of last read (in bb's) */
	short	i_advice;	/* user advice to system */
	u_short	i_shlocks;	/* count of shared locks */
	u_short	i_exlocks;	/* count of exclusive locks */

	char	*i_mount;	/* pointer to mount info */
	u_char	i_class;	/* class of fs inode is mounted on */
	struct	iops *i_ops;	/* pointer to inode ops functions */
	struct	icommon i_common;
	char	*i_data;	/* uncommon stuff */
};
/* stuff to make life easier */
#define	i_mode	i_common.ic_mode
#define	i_nlink	i_common.ic_nlink
#define	i_uid	i_common.ic_uid
#define	i_gid	i_common.ic_gid

/* flags */
#define	ILOCKED	0x0001		/* inode is locked */
#define	IWANT	0x0002		/* some process waiting on lock */
#define	IUPD	0x0004		/* file has been modified */
#define	IACC	0x0008		/* inode access time to be updated */
#define	ICHG	0x0010		/* inode has been changed */
#define	IEXT	0x0020		/* inode has been extended */
#define	ISYN	0x0040		/* do synchronous write for iupdate */
#define	IMOUNT	0x0080		/* inode is mounted on */
#define	ITEXT	0x0100		/* inode is pure text prototype */
#define IFLUX	0x0200		/* inode is in transit */
#define IREMOTE	0x0400		/* inode is remote */
#define	IEFS	0x0800		/* inode lives on extent filesystem */
#define	IPIPE	0x1000		/* inode is a pipe, and lives nowhere */

/* modes */
#define	IFMT	0170000		/* type of file */
#define		IFIFO	0010000	/* fifo special */
#define		IFCHR	0020000	/* character special */
#define		IFDIR	0040000	/* directory */
#define		IFBLK	0060000	/* block special */
#define		IFACT	0070000	/* (NOT USED YET) action file */
#define		IFREG	0100000	/* regular */
#define		IFPAG	0110000	/* (NOT USED YET) pageable regular file */
#define		IFLNK	0120000	/* symbolic link */
#define	ISUID	04000		/* set user id on execution */
#define	ISGID	02000		/* set group id on execution */
#define ISVTX	01000		/* shared text (IGNORED) */
#define	IREAD	0400		/* read, write, execute permissions */
#define	IWRITE	0200
#define	IEXEC	0100
#define IRWXRWXRWX	0000777	/* rwx permissions for all */

/* modes which apply only to symbolic links */
#define INLNK	02000		/* network symbolic link */
#define ILNIX	00077		/* mask for logical network number */
#define ISQUASH	04000		/* flag for "squashed" symbolic link */

#ifdef	KERNEL

/* convert i_data pointer into particular inode information */
#define	BellInode(ip)	((struct incore_dinode *)(((ip)->i_data)->p))
#define	EfsInode(ip)	((struct incore_efs_dinode *)(((ip)->i_data)->p))
#define	Pipe(ip)	((struct fspipe *)(((ip)->i_data)->p))
#define	Vnode(ip)	((struct vnode *)(((ip)->i_data)->p))

/* constants and definitions relating to the use of namei() */
#define	MAXLNKDEPTH	8	/* maximum depth of symbolic links */
#define	LOOKUP	0		/* lookup existing file */
#define	CREATE	1		/* create a new file */
#define	DELETE	2		/* delete an existing file */

/* quick form of iget when inode is already in core */
#define IGET(ip) { \
	ip->i_count++; \
	ILOCK(ip); \
}

#define	ILOCK(ip) { \
	while(ip->i_flag&ILOCKED) { \
		ip->i_flag |= IWANT; \
		(void) sleep((caddr_t)ip, PINOD); \
	} \
	ip->i_flag |= ILOCKED; \
}

#define	IUNLOCK(ip) { \
	ip->i_flag &= ~ILOCKED; \
	if(ip->i_flag&IWANT) { \
		ip->i_flag &= ~IWANT; \
		wakeup((caddr_t)ip); \
	} \
}

int	uchar(), schar();

/* bell fs procedures */
daddr_t	bell_bmap();
daddr_t	bell_getaddr();
struct	inode *bell_ialloc();
struct	inode *bell_iget();

/* generic fs procedures */
struct	inode *ialloc();
struct	inode *iget();
struct	inode *owner();
struct	inode *makenode();
struct	inode *namei();
time_t	sbtime();

/*
 * Inode hashing is used for quick lookup of an inode given its dev/i-number.
 * NHINO is the number of hash bucket headers.
 * ifreelist is the freelist of inode structures for allocation.
 */
#define	NHINO	128	/* must be power of 2 */
#define	ihash(X)	(&hinode[(int)(X) & (NHINO-1)])
struct	hinode {
	struct inode *i_forw;
} hinode[NHINO];

extern	struct inode *ifreelist;
extern	struct inode *inode, *inodeNINODE;
extern	struct inode *rootdir;
extern	short ninode;
extern	struct iops bell_iops, extent_iops;
#endif
