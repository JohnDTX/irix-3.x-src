#ifndef __INODE__
#define __INODE__
/*
 * The I node is the focus of all file activity in unix. There is a unique
 * inode allocated for each active file, each current directory, each
 * mounted-on file, text file, and the root. An inode is 'named'
 * by its dev/inumber pair.  Data, from mode on, is read in from
 * permanent inode on volume.
 *
 * $Source: /d2/3.7/src/sys/h/RCS/inode.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:29:36 $
 */

#ifndef	KERNEL
#include <sys/ino.h>
#include <sys/efs_ino.h>
#endif

struct ilinks {
	struct inode	*il_hforw;	/* hash links */
	struct inode	*il_hback;
	struct inode	*il_fforw;	/* freelist links */
	struct inode	*il_fback;
};

struct inode {
	struct ilinks	i_links;	/* cache linkage */
	short		i_flag;
	cnt_t		i_count;	/* reference count */
	dev_t		i_dev;		/* device where inode resides */
	ino_t		i_number;	/* number, 1-to-1 with dev address */
	dev_t		i_rdev;		/* underlying device if IFBLK/IFCHR */
	long		*i_filocks;	/* pointer to filock structure list */

	short		i_fstyp;	/* file system type */
	struct fstypsw	*i_fstypp;	/* pointer to file system switch */
	struct mount	*i_mntdev;	/* pointer to mount info */
	union i_u {
		struct mount *i_mton;	/* ptr to mount table entry which */
					/* is "mounted on" this inode */
		struct stdata *i_sp;	/* associated stream */
	} i_un;
	struct rcvd	*i_rcvd;	/* receive descriptor */

	ushort		i_ftype;	/* file type = IFDIR, IFREG, etc. */
	short		i_nlink;    	/* number of links to file */
	ushort		i_uid;      	/* owner's user id */
	ushort		i_gid;      	/* owner's group id */
	off_t		i_size;     	/* number of bytes in file */
	char		*i_fsptr;	/* private per fs type data */
	long		i_gen;		/* generation number */
};

#define	i_hforw	i_links.il_hforw
#define	i_hback	i_links.il_hback
#define	i_fforw	i_links.il_fforw
#define	i_fback	i_links.il_fback
#define	i_sptr	i_un.i_sp
#define	i_mnton	i_un.i_mton

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
#define	IXSAVED	0x0800		/* inode has text vm but it's unused */
#define	IISROOT	0x1000		/* inode is a root inode of an fs */
#define IVALID	0x2000		/* inode is valid and free */
#define	IERROR	0x4000		/* inode i/o had an error */

/* modes */
#define	IFMT	0170000		/* type of file */
#define		IFIFO	0010000	/* fifo special */
#define		IFCHR	0020000	/* character special */
#define		IFDIR	0040000	/* directory */
#define		IFBLK	0060000	/* block special */
#define		IFREG	0100000	/* regular */
#define		IFLNK	0120000	/* symbolic link */
#define	ISUID	04000		/* set user id on execution */
#define	ISGID	02000		/* set group id on execution */
#define ISVTX	01000		/* shared text (IGNORED) */
#define	IREAD	0400		/* read, write, execute permissions */
#define	IWRITE	0200
#define	IEXEC	0100

/* the following are from 5.3 */
#define	IOBJEXEC	0x010	/* execute as an object file */
				/* i.e., 410, 411, 413 */
#define IMNDLCK		0x001	/* mandatory locking set */
#define	ICDEXEC		0x020	/* cd permission */
#define	IDIRSRCH	ICDEXEC	/* a better name */
#define	PERMMSK		0x1ff	/* Nine permission bits: rwxrwxrwx */
#define	MODEMSK		0xfff	/* PERMMSK bits plus SUID, SGID, and SVTX */

#ifdef KERNEL

/* note a use or unuse of an inode, without locking */
#define	iuse(ip)	((ip)->i_count++)
int	iunuse();

/* hold an inode which is already in core */
#define IHOLD(ip) { \
	(ip)->i_count++; \
	ILOCK(ip); \
}

/* release an inode which has other references */
#define	IRELE(ip) { \
	--(ip)->i_count; \
	IUNLOCK(ip); \
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

/* generic inode/fs procedures */
struct	inode *getinode();
struct	inode *iget();
struct	inode *icreate();
struct	inode *owner();

/*
 * globals
 */
extern struct inode	*inode, *inodeNINODE;
extern short		ninode;
extern struct inode	*rootdir;
extern struct ilinks	ifreepool;

#endif	/* KERNEL */

#endif	/* __INODE__ */
