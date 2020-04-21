/*
 * Filesystem switch configuration.
 *
 * $Source: /d2/3.7/src/sys/sys/RCS/fs_conf.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:35:12 $
 */

#include "../h/types.h"
#include "../h/conf.h"
#include "../h/fsid.h"
#include "../h/fstyp.h"
#include "bfs.h"
#include "efs.h"
#include "nfs.h"
#include "tcp.h"

int	fsnull();
int	fszero();	/* SGI enhancement: returns 0 */
int	fsstray();
long	fslstray();
struct	inode *fsinstray();
int	*fsistray();

/*
 * Generic filesystem switch operations.  These are common to all conventional
 * disk filesystems.
 */
int	com_init(), com_iput(), com_iupdat(), com_openi(), com_closei(),
	com_readi(), com_writei(), com_statf(), com_access(), com_namei(),
	com_statfs(), com_setattr(), com_fcntl(), com_ioctl(), com_idestroy();
long	com_notify();
struct	inode *com_getinode();

#if NBFS > 0
int	bell_init(), bell_iupdat(), bell_itrunc(), bell_mount(),
	bell_umount(), bell_update(), bell_statfs(), bell_bmap(),
	bell_idestroy(), bell_ifree(), bell_setsize();
struct	inode *bell_iread(), *bell_ialloc();
time_t	bell_gettime();
int	bell_updatetime();
#endif

#if NEFS > 0
int	efs_init(), efs_iupdat(), efs_itrunc(), efs_mount(),
	efs_umount(), efs_update(), efs_statfs(), efs_bmap(),
	efs_idestroy(), efs_ifree(), efs_setsize();
struct	inode *efs_iread(), *efs_ialloc();
time_t	efs_gettime();
int	efs_updatetime();
#endif

#if NBFS > 0 || NEFS > 0
/* bell and efs use the same directory code */
int	bell_getdents(), bell_lookup(), bell_enter(), bell_remove(),
	bell_dirinit(), bell_isempty();
#endif

#if NTCP > 0
/* notice that the socket pseudo-filesystem uses some of the pipe functions */
int	soc_init(), soc_readi(), soc_writei(), soc_statf(), soc_closei(),
	soc_statfs(), soc_fcntl(), soc_ioctl();
#endif

#if NNFS > 0
int	nfs_init(), nfs_iput(), nfs_iupdat(), nfs_readi(),
	nfs_writei(), nfs_itrunc(), nfs_statf(), nfs_namei(),
	nfs_umount(), nfs_openi(), nfs_closei(), nfs_statfs(),
	nfs_access(), nfs_getdents(), nfs_setattr(), nfs_fcntl(),
	nfs_ioctl(), nfs_setsize();
struct	inode *nfs_iread();
#endif

struct	fstypsw fstypsw[] = {
/*
	{ init, iput, iread, unused, iupdat,
	  readi, writei, itrunc, statf, namei,
	  mount, umount, getinode, openi, closei,
	  update, statfs, access, getdents, allocmap,
	  freemap, readmap, setattr, notify, fcntl,
	  fsinfo, ioctl, dirlookup, direnter, dirremove,
	  dirinit, dirisempty, bmap, ialloc, idestroy,
	  ifree, setsize, gettime, updatetime, }
 */

	/* null filesystem type */
	{ fsnull, fsnull, fsinstray, fsnull, fsnull,
	  fsnull, fsnull, fsnull, fsnull, fsnull,
	  fsnull, fsnull, fsinstray, fsnull, fsnull,
	  fsnull, fsnull, fsnull, fsnull, fsstray,
	  fsistray, fsstray, fsnull, (long (*)()) fsnull, fsnull,
	  fsstray, fsnull, fszero, fszero, fszero,
	  fszero, fszero, fszero, fsinstray, fsnull,
	  fsnull, fsnull, (time_t (*)()) fsnull, fsnull, },

#if NBFS > 0
	/* bell filesystem */
	{ bell_init, com_iput, bell_iread, fsnull, bell_iupdat,
	  com_readi, com_writei, bell_itrunc, com_statf, com_namei,
	  bell_mount, bell_umount, fsinstray, com_openi, com_closei,
	  bell_update, bell_statfs, com_access, bell_getdents, fsstray,
	  fsistray, fsstray, com_setattr, com_notify, com_fcntl,
	  fsstray, com_ioctl, bell_lookup, bell_enter, bell_remove,
	  bell_dirinit, bell_isempty, bell_bmap, bell_ialloc, bell_idestroy,
	  bell_ifree, bell_setsize, bell_gettime, bell_updatetime, },
#endif

#if NEFS > 0
	/* extent filesystem */
	{ efs_init, com_iput, efs_iread, fsnull, efs_iupdat,
	  com_readi, com_writei, efs_itrunc, com_statf, com_namei,
	  efs_mount, efs_umount, fsinstray, com_openi, com_closei,
	  efs_update, efs_statfs, com_access, bell_getdents, fsstray,
	  fsistray, fsstray, com_setattr, com_notify, com_fcntl,
	  fsstray, com_ioctl, bell_lookup, bell_enter, bell_remove,
	  bell_dirinit, bell_isempty, efs_bmap, efs_ialloc, efs_idestroy,
	  efs_ifree, efs_setsize, efs_gettime, efs_updatetime, },
#endif

	/* pipe pseudo-filesystem */
	{ com_init, com_iput, fsinstray, fsnull, com_iupdat,
	  com_readi, com_writei, fsstray, com_statf, fsstray,
	  fszero, fsstray, com_getinode, com_openi, com_closei,
	  fsstray, com_statfs, com_access, fsstray, fsstray,
	  fsistray, fsstray, fsstray, com_notify, com_fcntl,
	  fsstray, com_ioctl, fsstray, fsstray, fsstray,
	  fsstray, fsstray, fsstray, fsinstray, com_idestroy,
	  fsstray, fsstray, fslstray, fsstray, },

#if NTCP > 0
	/* socket pseudo-filesystem */
	{ soc_init, fsnull, fsinstray, fsnull, com_iupdat,
	  soc_readi, soc_writei, fsstray, soc_statf, fsstray,
	  fszero, fsstray, fsinstray, fsstray, soc_closei,
	  fsstray, soc_statfs, com_access, fsstray, fsstray,
	  fsistray, fsstray, fsstray, com_notify, soc_fcntl,
	  fsstray, soc_ioctl, fsstray, fsstray, fsstray,
	  fsstray, fsstray, fsstray, fsinstray, fsstray,
	  fsstray, fsstray, fslstray, fsstray, },
#endif

#if NNFS > 0
	{ nfs_init, nfs_iput, nfs_iread, fsnull, nfs_iupdat,
	  nfs_readi, nfs_writei, nfs_itrunc, nfs_statf, nfs_namei,
	  fszero, nfs_umount, fsinstray, nfs_openi, nfs_closei,
	  fsnull, nfs_statfs, nfs_access, nfs_getdents, fsstray,
	  fsistray, fsstray, nfs_setattr, fslstray, nfs_fcntl,
	  fsstray, nfs_ioctl, fsstray, fsstray, fsstray,
	  fsstray, fsstray, fsstray, fsinstray, fsstray,
	  fsstray, nfs_setsize, fslstray, fsstray, },
#endif
};

short	nfstyp = sizeof fstypsw / sizeof fstypsw[0];
short	pipefstyp = 0;

struct	fsinfo fsinfo[] = {
/*	flags		pipe	name		notify	*/
	{ 0L,		0,	"<null fs>",	0L },
#if NBFS > 0
	{ 0L,		0,	FSID_BELL,	0L },
#endif
#if NEFS > 0
	{ 0L,		0,	FSID_EFS,	0L },
#endif
	{ FS_NOICACHE,	0,	FSID_PIPE,	0L },
#if NTCP > 0
	{ FS_NOICACHE,	0,	FSID_SOCKET,	0L },
#endif
#if NNFS > 0
	{ 0L,		0,	FSID_NFS,	0L },
#endif
};
