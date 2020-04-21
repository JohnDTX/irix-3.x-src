#ifndef	__com_inode__
#define	__com_inode__
/*
 * Common inode support.  Filesystems which share a similar set of
 * semantics and information can also share procedures if they
 * use this structure.
 *
 * $Source: /d2/3.7/src/sys/com/RCS/com_inode.h,v $
 * $Date: 89/03/27 17:26:41 $
 * $Revision: 1.1 $
 */

/*
 * Common inode information.  For filesystems using this, this structure
 * must be the first element in the private inode date (pointed at by
 * i_fsptr).
 */
struct com_inode {
	long	ci_magic;	/* magic number */
	time_t	ci_atime;	/* time last accessed */
	time_t	ci_mtime;	/* time last modified */
	time_t	ci_ctime;	/* time created or changed */
	ushort	ci_mode;	/* protection mode */
	short	ci_lastlen;	/* last length read */
	daddr_t	ci_lastbn;	/* last logical block read */
};

/* convert inode fs pointer to structure pointer */
#define	com_fsptr(ip)	((struct com_inode *) (ip)->i_fsptr)

#define	COM_MAGIC	0x12345678	/* identifies "com" filesystem */
#define	COM_ROOTINO	((ino_t) 2)	/* XXX should be in fsinfo[] */

#ifdef KERNEL
extern short	com_fstyp;
#endif

#endif	/* __com_inode__ */
