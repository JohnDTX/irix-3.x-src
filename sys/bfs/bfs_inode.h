#ifndef __bfs_inode__
#define	__bfs_inode__
/*
 * bfs_inode specification.
 *
 * $Source: /d2/3.7/src/sys/bfs/RCS/bfs_inode.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:26:19 $
 */
#ifdef	KERNEL
#include "../h/inode.h"
#include "../com/com_inode.h"
#include "../bfs/ino.h"

/*
 * Version of bell disk inode, as used in core.  This area is allocated per
 * bell inode and contains the converted addresses from the packed 3 byte
 * addressing that is present in the dinode.
 */
struct	bell_inode {
	struct	com_inode bi_com;	/* common state */
	daddr_t	bi_addr[NADDR];		/* addresses */
};
#define	bi_mode		bi_com.ci_mode
#define	bi_atime	bi_com.ci_atime
#define	bi_mtime	bi_com.ci_mtime
#define	bi_ctime	bi_com.ci_ctime

#define	bell_fsptr(ip)	((struct bell_inode *) (ip)->i_fsptr)

struct inode	*bell_icreate(/* ip, mode, nlink, rdev */);

#endif	/* KERNEL */
#endif	/* __bfs_inode__ */
