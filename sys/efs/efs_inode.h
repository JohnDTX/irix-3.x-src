#ifndef __efs_inode__
#define	__efs_inode__
/*
 * Variations on the in-core efs inode structure.
 *
 * $Source: /d2/3.7/src/sys/efs/RCS/efs_inode.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:27:58 $
 */
#ifdef KERNEL
#include "../h/inode.h"
#include "../com/com_inode.h"
#include "../efs/efs_ino.h"

/*
 * Version of a regular, directory, or symbolic link extent inode as it appears
 * in memory.  This is a template for the actual data structure which contains
 * ii_numextents worth of ii_extents.
 */
struct	efs_inode {
	struct	com_inode ii_com;	/* mode and times */
	short	ii_flags;		/* random private flags */
	short	ii_numextents;		/* number of extents */
	short	ii_numindirs;		/* number of indirect extents */
	long	ii_indirbytes;		/* bytes in indir extents */
	extent	*ii_indir;		/* malloc'd indirect extent info */
	extent	ii_extents[1];		/* actually, more of these */
};
#define	ii_mode		ii_com.ci_mode
#define	ii_atime	ii_com.ci_atime
#define	ii_mtime	ii_com.ci_mtime
#define	ii_ctime	ii_com.ci_ctime

#define	II_TRUNC	0x0001		/* inode needs truncation */

#define	efs_fsptr(ip)	((struct efs_inode *) (ip)->i_fsptr)
#define	EFS_IMAGIC	0x56374696

struct inode	*efs_icreate(/* ip, mode, nlink, rdev */);

#endif	/* KERNEL */
#endif	/* __efs_inode__ */
