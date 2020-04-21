/*
 * Common switch shareable subroutines.
 *
 * $Source: /d2/3.7/src/sys/com/RCS/com_inode.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:26:40 $
 */
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/conf.h"
#include "../h/fstyp.h"
#include "../h/inode.h"
#include "../h/mount.h"
#include "../h/nami.h"
#include "../h/buf.h"
#include "../h/file.h"
#include "../h/stat.h"
#include "../com/com_inode.h"
#include "../com/pipe_inode.h"

short	com_fstyp;		/* common fs's filesystem type */
short	pipe_files;		/* # of pipes/clones in use */
struct	mount com_mount;	/* fake mount point for getinode */

/*
 * Find a filesystem's index, given its init procedure name.  Return
 * 0 if not found.
 */
findfstyp(init)
	int (*init)();
{
	register int i;

	for (i = 0; i < nfstyp; i++) {
		if (fstypsw[i].fs_init == init)
			return (i);
	}
	return (0);
}

/*
 * Init the "common" filesystem.
 */
com_init()
{
	com_fstyp = findfstyp(com_init);
	ASSERT(com_fstyp);
	pipefstyp = com_fstyp;
	fsinfo[pipefstyp].fs_pipe = &com_mount;
	com_mount.m_dev = (dev_t)(-2);		/* XXX uglyyyyyyyyyy */
	com_mount.m_fstyp = com_fstyp;
	pncc_init();
}

#ifdef NOTDEF
com_stray()
{
	/* oops */
	printf("stray fs switch call");
}
#endif

/*
 * Initialize a new inode
 */
com_isetup(ip, mode)
	register struct inode *ip;
	ushort mode;
{
	register struct com_inode *ci;

	/*
	 * Init common private data
	 */
	ci = com_fsptr(ip);
	ci->ci_mode = mode;
	ci->ci_ctime = time;
	ci->ci_atime = time;
	ci->ci_mtime = time;

	/*
	 * Init public data
	 */
	ip->i_ftype = mode & IFMT;
}

/*
 * Release an inode.  If the inode is being recycled, release its
 * private storage.  If ip has no links truncate and free it, updating disk
 * NOTE: filesystems which use the common filesystem, must have cacheable
 * inodes to use com_iput().
 */
com_iput(ip)
	register struct inode *ip;
{
	register struct com_inode *ci;

	if (ip->i_fsptr == 0) {
		/*
		 * Inode has no private state.  Usually this means an inode
		 * was read in with mode=0 or nlinks=0, and then iput().
		 * It can also happen if the inode is read in an some sort
		 * of i/o error occurs, or the inode is deemed improper
		 * by the real filesystem.
		 */
		return;
	}

	if (ip->i_fstyp == com_fstyp) {
		/*
		 * The "common" filesystem is only used for incore pipes and
		 * cloned inodes.  It runs with no icache enabled, thus the
		 * semantics for iput are different.
		 */
		ASSERT(ip->i_count == 1);
		ip->i_flag &= ~(IACC|IUPD|ICHG);
		pipe_files--;
		FS_IDESTROY(ip);
		return;
	}

	/*
	 * Rest of com_iput is for shared cacheable filesystems.
	 */
	if (ip->i_count == 0) {
		/*
		 * Inode is being recylced to be used in some other
		 * manner.  Free private data and return.
		 */
		FS_IDESTROY(ip);
		return;
	}

	/*
	 * Last use is going away.  Set true size, free if unlinked.
	 */
	ASSERT(ip->i_count == 1);
	ci = com_fsptr(ip);
	if (ip->i_nlink > 0) {
		FS_SETSIZE(ip, ip->i_size);
	} else if ((ip->i_ftype != IFIFO) || (ip->i_dev != NODEV)) {
		/*
		 * Increment this inode's generation number.  Unlink
		 * inode by clearing its mode.
		 */
		ip->i_gen++;
		FS_ITRUNC(ip);
		ip->i_flag |= IUPD|ICHG;
		ip->i_ftype = 0;
		ci->ci_mode = 0;
		FS_IFREE(ip);
	}

	/* ip is closed, so reset its readahead detector */
	ci->ci_lastbn = 0;
	ci->ci_lastlen = 0;

	/* update the inode only if necessary */
	if (ip->i_flag & (IACC|IUPD|ICHG))
		FS_IUPDAT(ip, &time, &time);
}

/*
 * Return inode stat info
 */
com_statf(ip, st)
	register struct inode *ip;
	register struct stat *st;
{
	register struct com_inode *ci;

	ci = com_fsptr(ip);
	ASSERT(ip->i_ftype);
	ASSERT(ci);
	st->st_mode = ci->ci_mode;
	st->st_atime = ci->ci_atime;
	st->st_mtime = ci->ci_mtime;
	st->st_ctime = ci->ci_ctime;
}

/*
 * Update a common inode.  Only used to update cloned or pipe inodes.
 * This procedure is not to be shared with other filesystems.
 */
com_iupdat(ip, ta, tm)
	register struct inode *ip;
	time_t *ta, *tm;
{
	register struct com_inode *ci;

	ci = com_fsptr(ip);
	if (ip->i_flag & IACC)
		ci->ci_atime = *ta;
	if (ip->i_flag & IUPD)
		ci->ci_mtime = *tm;
	if (ip->i_flag & ICHG)
		ci->ci_ctime = time;
	ip->i_flag &= ~(IACC|IUPD|ICHG|ISYN);
}

/*
 * Destroy internal representation of inode.  Only used for incore pipes
 * and clones.
 */
com_idestroy(ip)
	register struct inode *ip;
{
	ASSERT(com_fsptr(ip) != NULL );
	ASSERT((com_fsptr(ip)->ci_magic == COM_MAGIC) || (com_fsptr(ip)->ci_magic == PIPE_MAGIC));
	free(ip->i_fsptr);
	ip->i_fsptr = 0;
}
