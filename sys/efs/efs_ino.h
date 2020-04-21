#ifndef	_efs_ino_
#define	_efs_ino_
/*
 * Definitions for the on-volume version of extent filesystem inodes.
 * Inodes consist of some number of extents, as contained in di_numextents.
 * When di_numextents exceeds EFS_DIRECTEXTENTS, then the extents are
 * kept elsewhere.  ``Elsewhere'' is defined as a list of up to
 * EFS_DIRECTEXTENTS worth of indirect extents.  An indirect extent is an
 * extent descriptor (bn,len) which points to the disk blocks holding the
 * actual extents.  For direct extents, the ex_offset field contains the
 * logical offset into the file that the extent covers.  For indirect
 * extents the field di_u.di_extents[0].ex_offset contains the number of
 * indirect extents.
 *
 * Valid extents are determined by their ex_bn, ex_length, ex_offset.
 * If the ex_bn is zero, then the tuple (ex_length, ex_offset) determine
 * a ``hole'' in the file - namely, a section of the file which was never
 * written, and is assumed to contain zeros.  If the ex_bn is non-zero, and
 * if the ex_bn is less than fs_firstcg, or if the ex_bn is greater than or
 * equal to the fs_size then the block # is out of range.  If the ex_length
 * is zero or if the ex_length is larger than EFS_MAXEXTENT (efs_sb.h) than
 * the extent is bad.  If the (ex_offset, ex_length) tuple overlaps any other
 * extent, then the extent is bad.
 *
 * $Source: /d2/3.7/src/sys/efs/RCS/efs_ino.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:27:55 $
 */

/* # of directly mappable extents */
#define	EFS_DIRECTEXTENTS	12

/* maximum extents per inode */
#define	EFS_MAXEXTENTS		2048

/* maximum length of an extent */
#define	EFS_MAXEXTENTLEN	(256 - 8)

/* # of inodes per page */
#define	EFS_INODESPERPAGE	(NBPG / sizeof(struct efs_dinode))

/*
 * An extent.
 */
typedef struct	extent {
unsigned int	ex_magic:8,	/* magic # (MUST BE ZERO) */
		ex_bn:24,	/* basic block # */
		ex_length:8,	/* length of this extent, in bb's */
		ex_offset:24;	/* logical bb offset into file */
} extent;

/*
 * Extent based filesystem inode as it appears on disk.  The efs inode
 * is exactly 128 bytes long.
 */
struct	efs_dinode {
	ushort	di_mode;	/* mode and type of file */
	short	di_nlink;    	/* number of links to file */
	ushort	di_uid;      	/* owner's user id */
	ushort	di_gid;      	/* owner's group id */
	off_t	di_size;     	/* number of bytes in file */
	time_t	di_atime;   	/* time last accessed */
	time_t	di_mtime;   	/* time last modified */
	time_t	di_ctime;   	/* time created */
	long	di_gen;		/* generation number */
	short	di_numextents;	/* # of extents */
	ushort	di_refs;	/* refrence count since last reorg */
	union di_addr {
		extent	di_extents[EFS_DIRECTEXTENTS];
		dev_t	di_dev;	/* device for IFCHR/IFBLK */
	} di_u;
};

/* sizeof(struct efsino), log2 */
#define	EFS_EFSINOSHIFT		7

#endif	/* _efs_ino_ */
