/*
 * Filesystem level operations for the bell filesystem.
 *
 * $Source: /d2/3.7/src/sys/bfs/RCS/bfs_mount.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:26:19 $
 */
#include "bfs.h"
#if NBFS > 0

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/user.h"
#include "../h/buf.h"
#include "../h/file.h"
#include "../h/fstyp.h"
#include "../h/inode.h"
#include "../h/mount.h"
#include "../h/statfs.h"
#include "../com/com_pncc.h"
#include "../bfs/filsys.h"

short	bell_fstyp;

bell_init()
{
	bell_fstyp = findfstyp(bell_init);
}

/*
 * bell_mount:
 *	- attempt to mount a bell filesystem
 *	- prepare the fs for usage
 */
int
bell_mount(devip, mp, readonly)
	struct inode *devip;
	struct mount *mp;
	int readonly;
{
	register dev_t dev;
	register struct filsys *fs;
	struct buf *bp;

	/*
	 * Open device.
	 */
	dev = devip->i_rdev;
	(*bdevsw[major(dev)].d_open)(dev, readonly ? FREAD : FREAD|FWRITE);
	if (u.u_error != 0)
		return (0);

	/*
	 * Set everything except m_fs, m_mount, m_inodp, and m_next;
	 * m_fs and m_mount are set below.
	 */
	mp->m_dev = dev;
	mp->m_fstyp = bell_fstyp;
	mp->m_bsize = BBSIZE;
	mp->m_flags = readonly ? MRDONLY : 0;

	/* read in super block of bell filesystem */
	bp = bread(mp->m_dev, SUPERB, BTOBB(sizeof(struct filsys)));
	if (bp->b_flags & B_ERROR)
		goto bad;
		
	/* check magic number to insure this is a bell filesystem */
	fs = bp->b_un.b_filsys;
	if (fs->s_magic != FsMAGIC)
		goto bad;
	if ((mp->m_fs = malloc(sizeof(struct filsys))) == NULL)
		goto bad;
	bcopy((caddr_t)fs, mp->m_fs, sizeof(struct filsys));
	brelse(bp);

	fs = getfs(mp);
	fs->s_ilock = 0;
	fs->s_flock = 0;
	fs->s_ninode = 0;
	fs->s_inode[0] = 0;
	fs->s_ronly = rdonlyfs(mp);

	/*
	 * Get the root inode and flag it as root in a
	 * filesystem-independent way.
	 */
	mp->m_mount = iget(mp, ROOTINO);
	if (mp->m_mount == NULL) {
		goto bad;
	}
	mp->m_mount->i_flag |= IISROOT;
	iunlock(mp->m_mount);
	return (1);
bad:
	brelse(bp);
	return (0);
}

/*
 * bell_umount:
 *	- un-mount a bell filesystem
 *	- do things which the 5.3 filesystem switch considers to be
 *	  filesystem-dependent.
 */
bell_umount(mp)
	struct mount *mp;
{
	register dev_t dev;
	register struct inode *rootip;

	dev = mp->m_dev;
	pncc_purgedev(dev);
	xumount(mp);

	if (iflush(mp) < 0) {
		u.u_error = EBUSY;
		return;
	}

	rootip = mp->m_mount;
	ASSERT(rootip != NULL);
	iunuse(rootip);
	iuncache(rootip);

	bflush(dev);
	bdwait();
	binval(dev);

	bell_update(mp);
	(*bdevsw[major(dev)].d_close)(dev, 0);
	if (mp->m_fs)
		free(mp->m_fs);
}

/*
 * bell_update:
 *	- update all incore information attached to the given filesystem
 *	- all this does is flush out the superblock, if its dirty
 */
bell_update(mp)
	struct mount *mp;
{
	register struct filsys *fs;
	struct buf *bp;

	fs = getfs(mp);
	/*
	 * If superblock isn't dirty, or its in use for inode or data block
	 * allocation or its read only, don't update it.
	 */
	if ((fs->s_fmod == 0) || (fs->s_ilock != 0) ||
	    (fs->s_flock != 0) || (fs->s_ronly != 0))
		return;
	/*
	 * Get a buffer from the volume and write out the updated block.
	 */
	bp = getblk(mp->m_dev, SUPERB, BTOBB(sizeof(struct filsys)));
	if (bp->b_flags & B_ERROR)
		brelse(bp);
	else {
		fs->s_time = time;
		fs->s_fmod = 0;
		bcopy((caddr_t)fs, (caddr_t)bp->b_un.b_filsys,
		      sizeof(struct filsys));
		bwrite(bp);
	}
}

/*
 * bell_updatetime:
 *	- update the time in the root superblock
 */
bell_updatetime(mp)
	struct mount *mp;
{
	register struct filsys *fs;

	fs = getfs(mp);
	fs->s_time = time;
	fs->s_fmod = 1;
}

/*
 * bell_gettime:
 *	- get time out of root superblock
 */
time_t
bell_gettime(mp)
	struct mount *mp;
{
	return (getfs(mp)->s_time);
}

/*
 * bell_statfs:
 *	- do a statfs() on the given filesystem, returning the fs type, basic
 *	  block size, number of total and free inodes, total and free data
 *	  blocks, filesystem name, and the volume name
 */
/* ARGSUSED */
bell_statfs(ip, sfsp, ufstyp)
	struct inode *ip;
	register struct statfs *sfsp;
	register short ufstyp;
{
	register struct filsys *fs;
	struct filsys filsys;

	if (ufstyp == 0) {
		fs = getfs(ip->i_mntdev);
	} else {
		ASSERT(ufstyp == bell_fstyp);

		/*
		 * File system is not mounted.  The inode pointer refers
		 * to a device, not necessarily in our filesystem, from
		 * which the superblock must be read.
		 */
		if (ip->i_ftype != IFBLK) {
			/* character devices not supported for now */
			u.u_error = EINVAL;
			return;
		}
		fs = &filsys;
		u.u_offset = SUPERBOFF;
		u.u_base = (caddr_t) fs;
		u.u_count = sizeof *fs;
		u.u_segflg = 1;		/* UIOSEG_KERNEL */
		FS_READI(ip);
		if (u.u_error != 0) {
			return;
		}
		if (fs->s_magic != FsMAGIC) {
			u.u_error = EINVAL;
			return;
		}
	}
	sfsp->f_fstyp = bell_fstyp;
	/*
	 * XXX dev parameter in Fs*() macros is bogus if ufstyp != 0
	 * XXX and ip->i_fstyp != bell_fstyp, but macros don't use it
	 * XXX currently
	 */
	sfsp->f_bsize = BBSIZE;	/* XXX want FsBSIZE(ip->i_dev) too */
	sfsp->f_frsize = 0L;
	sfsp->f_blocks = FsLTOP(ip->i_dev, fs->s_fsize - fs->s_isize);
	sfsp->f_bfree = FsLTOP(ip->i_dev, fs->s_tfree);
	sfsp->f_files = fs->s_isize << INOSHIFT;
	sfsp->f_ffree = fs->s_tinode;
	bcopy(fs->s_fname, sfsp->f_fname, sizeof(sfsp->f_fname));
	bcopy(fs->s_fpack, sfsp->f_fpack, sizeof(sfsp->f_fpack));
}

#endif
