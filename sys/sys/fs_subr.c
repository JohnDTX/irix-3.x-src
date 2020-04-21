/*
 * Miscellaneous fs system calls and subroutines
 *
 * $Source: /d2/3.7/src/sys/sys/RCS/fs_subr.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:35:18 $
 */
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/user.h"
#include "../h/file.h"
#include "../h/fstyp.h"
#include "../h/inode.h"
#include "../h/mount.h"

/*
 * The sys-pipe entry.
 * Allocate an inode on the root device.  Allocate 2 file structures.
 * Put it all together with flags - see com/com_pipe.c
 */
pipe()
{
	register struct inode *ip;
	register struct file *rf, *wf;
	int r;

	/* get an inode on the device associated with pipefstyp */
	ASSERT(0 < pipefstyp && pipefstyp < nfstyp);
	ip = (*fstypsw[pipefstyp].fs_getinode)(fsinfo[pipefstyp].fs_pipe,
		FSG_PIPE, NODEV);
	if (ip == NULL)
		return;

	/*
	 * Now allocate two file descriptors for each end of
	 * the pipe.
	 */
	rf = falloc(ip, FREAD);
	if (rf == NULL)
		goto out;
	ASSERT(ip->i_count == 1);
	r = u.u_rval1;
	wf = falloc(ip, FWRITE);
	if (wf == NULL) {
		rf->f_count = 0;
		rf->f_next = ffreelist;
		ffreelist = rf;
		u.u_ofile[r] = NULL;
		goto out;
	}
	iuse(ip);
	u.u_rval2 = u.u_rval1;
	u.u_rval1 = r;

	iunlock(ip);
	return;

out:
	iput(ip);
}

#undef	NOISY
#ifdef	NOISY
# include "../h/proc.h"
#endif

/*
 * sbgettime:
 *	- get time from root superblock
 */
sbgettime()
{
	register struct mount *rootmp;

	rootmp = rootdir->i_mntdev;
	time = FS_GETTIME(rootmp);
}

/*
 * update is the internal name of 'sync'. It goes through the disk
 * queues to initiate sandbagged IO; goes through the I nodes to write
 * modified nodes; and it goes through the mount table to initiate modified
 * super blocks.
 */
update()
{
	register struct inode *ip;
	register struct mount *mp;
	static char updlock;

	if (updlock)
		return;
	updlock = 1;
#ifdef	NOISY
{
	int s;
	s = spl7();
	printf("[SYNC %d]", u.u_procp->p_pid);
	splx(s);
}
#endif

	/*
	 * First, force superblocks to be updated with the
	 * current time.
	 */
	mp = mount;
	while (mp) {
		FS_UPDATE(mp);
		mp = mp->m_next;
	}

	/*
	 * Secondly, find any dirty inodes and push them out to disk
	 */
	for (ip = &inode[0]; ip < inodeNINODE; ip++) {
		if (ip->i_count && ((ip->i_flag & ILOCKED) == 0) &&
		    (ip->i_flag & (IACC|IUPD|ICHG))) {
			ip->i_flag |= ILOCKED;
			ip->i_count++;
			FS_IUPDAT(ip, &time, &time);
			iput(ip);
		}
	}

	/*
	 * Lastly, cause any delayed write traffic to be forced to
	 * disk.
	 */
	bflush(NODEV);
	updlock = 0;
}
