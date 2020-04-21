/*
 * Filesystem level operations for the extent filesystem.
 *
 * $Source: /d2/3.7/src/sys/efs/RCS/efs_mount.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:27:59 $
 */
#include "efs.h"
#if NEFS > 0

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/user.h"
#include "../h/file.h"
#include "../h/buf.h"
#include "../h/fstyp.h"
#include "../h/inode.h"
#include "../h/mount.h"
#include "../h/statfs.h"
#include "../com/com_pncc.h"
#include "../efs/efs_fs.h"
#include "../efs/efs_ino.h"
#include "../efs/efs_sb.h"
#undef	DEBUG

short	efs_fstyp = 0;		/* efs with traditional directories */

efs_init()
{
	efs_fstyp = findfstyp(efs_init);
}

int
efs_mount(devip, mp, readonly)
	struct inode *devip;
	struct mount *mp;
	int readonly;
{
	return efs_mountany(devip, mp, readonly, efs_fstyp, EFS_MAGIC);
}

/* define these so that they are patchable */
long	efs_inopchunk = 0;		/* # of inodes per inode chunk */
long	efs_minfree = 0;		/* minimum bb's free for file creat */
long	efs_mindirfree = 0;		/* min bb's free for dir creat */

long	efs_checksum();
long	efs_wildchecksum = 0L;		/* patchable wildcard checksum */

/*
 * efs_mountany:
 *	- mount an efs or efs mark 2 filesystem
 *	- prepare the fs for usage
 */
static int
efs_mountany(devip, mp, readonly, fstyp, magic)
	struct inode *devip;
	register struct mount *mp;
	int readonly;
	short fstyp;
	long magic;
{
	register struct efs *fs;
	register struct cg *cg;
	register dev_t dev;
	register int i;
	register ino_t inum;
	struct buf *bp;
	daddr_t bn;

	/*
	 * Open device
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
	mp->m_fstyp = fstyp;
	mp->m_bsize = efs_lbsize;	/* XXX */
	mp->m_flags = readonly ? MRDONLY : 0;

	/*
	 * Read in the efs super block.  Once block is read in, check it
	 * for being a valid efs in a mountable state.  Root filesystems
	 * get to skip this check, since you have to at least mount
	 * root to run fsck.
	 */
	bp = bread(mp->m_dev, SUPERB, BTOBB(sizeof(struct efs)));
	if (bp->b_flags & B_ERROR)
		goto out;
	fs = bp->b_un.b_efs;
	if (fs->fs_magic != magic)
		goto out;
	if (dev != rootdev) {
		if (fs->fs_checksum != efs_wildchecksum
		    && fs->fs_checksum != efs_checksum(fs)) {
			uprintf("filesystem checksum is messed up");
			goto badbad;
		}
		if (fs->fs_dirty) {
			uprintf("filesystem is dirty");
badbad:
			uprintf(", run fsck(1M) first\n");
			goto out;
		}
		/*
		 * Give superblock a rudimentry legitimacy check.  If
		 * the filesystem size is messed up, or some of the
		 * sizing parameters are negative, or if the bit map
		 * size is too large (more than 2 million sectors worth)
		 */
		i = fs->fs_firstcg + fs->fs_cgfsize * fs->fs_ncg;
		if ((fs->fs_size != i) || (fs->fs_heads < 0) ||
		    (fs->fs_ncg < 0) || (fs->fs_sectors < 0) ||
		    (fs->fs_bmsize < 0) ||
		    (fs->fs_bmsize > 2*1024*1024 / BITSPERBYTE) ||
		    (fs->fs_tinode < 0) ||
		    (fs->fs_tinode >
			   fs->fs_cgisize * EFS_INOPBB * fs->fs_ncg) ||
		    (fs->fs_tfree < 0) ||
		    (fs->fs_tfree >
			  (fs->fs_cgfsize - fs->fs_cgisize) * fs->fs_ncg)) {
			uprintf("filesystem superblock messed up\n");
			goto out;
		}
	}

	/*
	 * Allocate memory for the filesystem structure, as well as the
	 * cylinder group information.  Clear out the entire fs structure
	 * including the cg structs.  Then copy in from the disk buffer
	 * the volume information.
	 */
	i = sizeof(struct efs) + (fs->fs_ncg - 1) * sizeof(struct cg);
	if ((mp->m_fs = calloc(1, i)) == NULL)
		goto out;
	bcopy((caddr_t)fs, mp->m_fs, sizeof(struct efs) - sizeof(struct cg));
	fs = getfs(mp);

	/*
	 * Allocate memory for the inode allocation bitmap and the
	 * data allocation bitmap.  Read in data bitmap.
	 * Compute and fill in the computed portions of the filesystem.
	 */
	fs->fs_ipcg = EFS_INOPBB * fs->fs_cgisize;
	if ((fs->fs_dmap = malloc((int)fs->fs_bmsize)) == NULL)
		goto out;
	brelse(bp);
	bp = bread(mp->m_dev, EFS_BITMAPBB, (int) BTOBB(fs->fs_bmsize));
	if (bp->b_flags & B_ERROR)
		goto out;
	bcopy(bp->b_un.b_addr, fs->fs_dmap, (int)fs->fs_bmsize);
	bp->b_flags |= B_INVAL;		/* throw away */
	brelse(bp);
	fs->fs_corrupted = 0;
	fs->fs_fmod = 0;
	fs->fs_dirty = 0;
	fs->fs_readonly = rdonlyfs(mp);
	fs->fs_diskfull = 0;
	fs->fs_dev = mp->m_dev;
	fs->fs_lastinum = fs->fs_ipcg * fs->fs_ncg - 1;
	fs->fs_inopchunk = EFS_INOPCHUNK;
	fs->fs_minfree = EFS_MINFREEBB;
	fs->fs_mindirfree = EFS_MINDIRFREEBB;
	if (efs_inopchunk)
		fs->fs_inopchunk = efs_inopchunk;
	if (efs_minfree)
		fs->fs_minfree = efs_minfree;
	if (efs_mindirfree)
		fs->fs_mindirfree = efs_mindirfree;
	/* make sure that fs->fs_inopchunk is on a bb boundary */
	fs->fs_inopchunk = ((fs->fs_inopchunk + EFS_INOPBB - 1) >>
				EFS_INOPBBSHIFT) << EFS_INOPBBSHIFT;
	fs->fs_inopchunkbb = EFS_ITOCGBB(fs, fs->fs_inopchunk + EFS_INOPBB - 1);
#ifdef	DEBUG
printf("inopchunk=%d inopchunkbb=%d minfree=%d mindirfree=%d\n",
		     fs->fs_inopchunk, fs->fs_inopchunkbb,
		     fs->fs_minfree, fs->fs_mindirfree);
#endif

	/*
	 * Initialize summary information for the cylinder groups.
	 */
	inum = 0;
	bn = fs->fs_firstcg;
	cg = &fs->fs_cgs[0];
	for (i = fs->fs_ncg; --i >= 0; cg++) {
		cg->cg_firsti = inum;
		cg->cg_lasti = inum + fs->fs_ipcg - 1;
		cg->cg_fulli = 0;
		cg->cg_lowi = cg->cg_firsti;
		cg->cg_firstbn = bn;
		cg->cg_firstdbn = bn + fs->fs_cgisize;
		cg->cg_firstdfree = cg->cg_firstdbn;
		efs_builddsum(fs, cg, (int) BTOBB(efs_lbsize), 1);
		inum += fs->fs_ipcg;
		bn += fs->fs_cgfsize;
	}

	/*
	 * Get the root inode and flag it as root in a
	 * filesystem-independent way.
	 */
	mp->m_mount = iget(mp, ROOTINO);
	if (mp->m_mount == NULL) {
		goto out;
	}
	mp->m_mount->i_flag |= IISROOT;
	iunlock(mp->m_mount);
	return (1);

out:
	brelse(bp);
	efs_freefs(mp);
	return (0);
}

/*
 * efs_umount:
 *	- umount the given efs
 *	- release the root inode and uncache pathname components
 *	- update the incore bitmap information to disk
 *	- close the filesystem's device
 */
efs_umount(mp)
	register struct mount *mp;
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

	efs_update(mp);
	(*bdevsw[major(dev)].d_close)(dev, 0);
	efs_freefs(mp);
}

/*
 * efs_freefs:
 *	- release any dynamic memory attached to an efs
 */
efs_freefs(mp)
	register struct mount *mp;
{
	register struct efs *fs;

	if (mp->m_fs) {
		fs = getfs(mp);
		if (fs->fs_dmap)
			free(fs->fs_dmap);	/* free up data map */
		free(mp->m_fs);
	}
}

/*
 * efs_update:
 *	- write out the superblock, if its dirty
 *	- update the time stamp on the superblock
 * XXX	get rid of copy in update by using kernel raw i/o
 */
efs_update(mp)
	struct mount *mp;
{
	register struct efs *fs;
	register struct buf *bp;

	fs = getfs(mp);
	if (!fs->fs_fmod || fs->fs_readonly)
		return;
	fs->fs_fmod = 0;
	fs->fs_time = time;
	fs->fs_checksum = efs_checksum(fs);

	/* update superblock */
	bp = getblk(mp->m_dev, SUPERB, BTOBB(sizeof(struct efs)));
	if (bp->b_flags & B_ERROR)
		brelse(bp);
	else {
		bcopy((caddr_t)fs, bp->b_un.b_addr, sizeof(struct efs));
		bzero((caddr_t)(bp->b_un.b_efs + 1),
		      BBTOB(BTOBB(sizeof(struct efs))) - sizeof(struct efs));
		bwrite(bp);
	}

	/* update bitmap */
	bp = getblk(mp->m_dev, EFS_BITMAPBB, (int) BTOBB(fs->fs_bmsize));
	if (bp->b_flags & B_ERROR)
		brelse(bp);
	else {
		bcopy(fs->fs_dmap, bp->b_un.b_bitmap, (int)fs->fs_bmsize);
		bwrite(bp);
	}
}

/*
 * efs_gettime:
 *	- get time out of root superblock
 */
time_t
efs_gettime(mp)
	struct mount *mp;
{
	return (getfs(mp)->fs_time);
}

/*
 * efs_updatetime:
 *	- update the time in the root superblock
 */
efs_updatetime(mp)
	struct mount *mp;
{
	register struct efs *fs;

	fs = getfs(mp);
	fs->fs_time = time;
	fs->fs_fmod = 1;
}

/*
 * efs_checksum:
 *	- compute the checksum of the read in portion of the filesystem
 *	  structure
 */
long
efs_checksum(fs)
	register struct efs *fs;
{
	register u_short *sp;
	register long checksum;

	checksum = 0;
	sp = (u_short *)fs;
	while (sp < (u_short *)&fs->fs_checksum) {
		checksum ^= *sp++;
		checksum = (checksum << 1) | (checksum < 0);
	}
	if (checksum == efs_wildchecksum)
		checksum = ~checksum;
	return (checksum);
}

/*
 * efs_statfs:
 *	- do a statfs() on the given filesystem, returning the fs type, basic
 *	  block size, number of total and free inodes, total and free data
 *	  blocks, filesystem name, and the volume name
 */
/* ARGSUSED */
efs_statfs(ip, sfsp, ufstyp)
	struct inode *ip;
	register struct statfs *sfsp;
	register short ufstyp;
{
	register struct efs *fs;
	struct efs efs;

	if (ufstyp == 0) {
		fs = getfs(ip->i_mntdev);
	} else {
		register long magic;

		ASSERT(ufstyp == efs_fstyp);
		magic = EFS_MAGIC;

		/*
		 * File system is not mounted.  The inode pointer refers
		 * to a device, not necessariliy in our filesystem, from
		 * which the superblock must be read.
		 */
		if (ip->i_ftype != IFBLK) {
			/* character devices not supported for now */
			u.u_error = EINVAL;
			return;
		}
		fs = &efs;
		u.u_offset = EFS_SUPERBOFF;
		u.u_base = (caddr_t) fs;
		u.u_count = sizeof *fs;
		u.u_segflg = 1;		/* UIOSEG_KERNEL */
		FS_READI(ip);
		if (u.u_error) {
			return;
		}
		if (fs->fs_magic != magic) {
			u.u_error = EINVAL;
			return;
		}
	}
	sfsp->f_fstyp = ufstyp ? ufstyp : ip->i_fstyp;
	sfsp->f_bsize = BBSIZE;	/* XXX want lbsize too */
	sfsp->f_frsize = 0;
	sfsp->f_blocks = (fs->fs_ncg * (fs->fs_cgfsize - fs->fs_cgisize));
	sfsp->f_bfree = fs->fs_tfree;
	sfsp->f_files = fs->fs_ncg * fs->fs_ipcg;
	sfsp->f_ffree = fs->fs_tinode;
	bcopy(fs->fs_fname, sfsp->f_fname, sizeof(sfsp->f_fname));
	bcopy(fs->fs_fpack, sfsp->f_fpack, sizeof(sfsp->f_fpack));
}

#endif
