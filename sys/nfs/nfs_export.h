#ifndef _nfs_export_
#define	_nfs_export_
/*
 * Exported nfs definitions and declarations.
 *
 * $Source: /d2/3.7/src/sys/nfs/RCS/nfs_export.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:33:10 $
 */
#ifdef	KERNEL

typedef fhandle_t;

extern short	nfs_fstyp;	/* nfs filesystem type */
extern dev_t	nfs_major;	/* major for nfs_strategy (conf.c) */
extern int	nfs_mntno;	/* nfs mount counter (m_dev minor) */

int		nfs_mount();	/* mount nfs system call */
int		nfs_svc();	/* nfs server system call */
int		nfs_getfh();	/* get file handle system call */
int		async_daemon();	/* bio daemon system call */
int		exportfs();	/* export served filesytem call */
int		nfs_strategy();	/* bio strategy function */

#else	/* USER */

#ifdef SVR3
# include <sys/fs/nfs.h>
#else
# include <nfs/nfs.h>
#endif

/*
 * NFS system calls.
 */
int	getfh(/* int fd, fhandle_t *fhp */);
int	nfsmount(/* struct nfs_args *ap, char *dir, int ro */);
int	nfssvc(/* void */);
int	async_daemon(/* void */);
int	exportfs(/* char *dir, int rootid, int flags */);

#endif	/* KERNEL/USER */

/*
 * NFS mount arguments
 */
struct nfs_args {
	struct sockaddr_in	*addr;		/* file server address */
	fhandle_t		*fh;		/* File handle to be mounted */
	int			flags;		/* flags */
	int			wsize;		/* write size in bytes */
	int			rsize;		/* read size in bytes */
	int			timeo;		/* initial timeout in .1 secs */
	int			retrans;	/* times to retry send */
	char			*hostname;	/* server's name */
};

/*
 * NFS mount option flags
 */
#define	NFSMNT_SOFT	0x001	/* soft mount (hard is default) */
#define	NFSMNT_WSIZE	0x002	/* set write size */
#define	NFSMNT_RSIZE	0x004	/* set read size */
#define	NFSMNT_TIMEO	0x008	/* set initial timeout */
#define	NFSMNT_RETRANS	0x010	/* set number of request retrys */
#define	NFSMNT_HOSTNAME	0x020	/* set hostname for error printf */

#endif
