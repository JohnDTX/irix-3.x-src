/*      @(#)nfs_clnt.h 1.1 85/05/30 SMI      */

#ifndef IPPROTO_IP		/* ouch! XXX */
# ifdef SVR3
#  include "netinet/in.h"
# else
#  include "../netinet/in.h"
# endif
#endif

/*
 * vfs pointer to mount info
 */
#ifdef SVR3
# define vftomi(vfsp)	((struct mntinfo *)((vfsp)->m_bufp))
#else
# define vftomi(vfsp)	((struct mntinfo *)((vfsp)->m_fs))
#endif

/*
 * vnode pointer to mount info
 */
#define	vtomi(vp)	vftomi((vp)->i_mntdev)

/*
 * NFS vnode to server's block size
 */
#define	vtoblksz(vp)	(vtomi(vp)->mi_bsize)

/*
 * NFS private data per mounted file system
 */
struct mntinfo {
	struct sockaddr_in mi_addr;	/* server's address */
	struct inode	*mi_rootvp;	/* root inode */
	u_int		 mi_hard : 1;	/* hard or soft mount */
	u_int		 mi_printed : 1;/* not responding message printed */
	u_int		 mi_refct : 30;	/* active vnodes for this vfs */
	long		 mi_tsize;	/* preferred transfer size (bytes) */
	long		 mi_stsize;	/* server's max transfer size (bytes) */
	long		 mi_bsize;	/* server's disk block size */
	int		 mi_mntno;	/* kludge to set client rdev for stat*/
	int		 mi_timeo;	/* inital timeout in 10th sec */
	int		 mi_retrans;	/* times to retry request */
	char		 mi_hostname[MAXHOSTNAMELEN];
					/* hostname for statfs */
};

/*
 * enum to specifiy cache flushing action when file data is stale
 */
enum staleflush	{NOFLUSH, SFLUSH};
