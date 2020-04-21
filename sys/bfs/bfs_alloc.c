/*
 * Inode and data block allocation and freeing for the bell filesystem.
 *
 * $Source: /d2/3.7/src/sys/bfs/RCS/bfs_alloc.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:26:15 $
 */
#include "bfs.h"
#if NBFS > 0

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/buf.h"
#include "../h/file.h"
#include "../h/user.h"
#include "../h/mount.h"
#include "../bfs/bfs_inode.h"
#include "../bfs/fblk.h"
#include "../bfs/filsys.h"
#include "../bfs/ino.h"

/*
 * Check that a block number is in the range between the I list
 * and the size of the device.
 * This is used mainly to check that a
 * garbage file system has not been mounted.
 *
 * bad block on dev x/y -- not in range
 */
static
badblock(fp, bn, dev)
	register struct filsys *fp;
	daddr_t bn;
	dev_t dev;
{
	if (bn < fp->s_isize || bn >= fp->s_fsize) {
		prdev("bad block", dev);
		return (1);
	}
	return (0);
}

/*
 * alloc will obtain the next available free disk block from the free list
 * of the specified device.
 * The super block has up to NICFREE remembered free blocks;
 * the last of these is read to obtain NICFREE more . . .
 *
 * no space on dev x/y -- when the free list is exhausted.
 */
struct buf *
bell_alloc(ip, abn)
	struct inode *ip;
	daddr_t *abn;
{
	register struct filsys *fp;
	register daddr_t bno;
	register struct buf *bp;
	dev_t dev;

	fp = getfs(ip->i_mntdev);
	dev = ip->i_dev;
	while(fp->s_flock)
		(void) sleep((caddr_t)&fp->s_flock, PINOD);
	do {
		if (fp->s_nfree <= 0)
			goto nospace;
		bno = fp->s_free[--fp->s_nfree];
		if (bno == 0)
			goto nospace;
	} while (badblock(fp, bno, dev));

	if (fp->s_nfree <= 0) {
		fp->s_flock++;
		bp = bread(dev, FsLTOP(dev, bno), BTOBB(FsBSIZE(dev)));
		if (u.u_error == 0) {
			fp->s_nfree = (bp->b_un.b_fblk)->df_nfree;
			bcopy((caddr_t)(bp->b_un.b_fblk)->df_free,
			    (caddr_t)fp->s_free, sizeof(fp->s_free));
		}
		brelse(bp);
		/*
		 * Prevent "dups in free"
		 */
		bp = getblk(dev, SUPERB, BTOBB(sizeof(struct filsys)));
		bcopy((caddr_t)fp, bp->b_un.b_addr, sizeof(struct filsys));
		fp->s_fmod = 0;
		fp->s_time = time;
		bdwrite(bp);		/* XXX was bwrite */
		fp->s_flock = 0;
		wakeup((caddr_t)&fp->s_flock);
	}
	if (fp->s_nfree <= 0 || fp->s_nfree > NICFREE) {
		prdev("Bad free count", dev);
		goto nospace;
	}
	bp = getblk(dev, FsLTOP(dev, bno), BTOBB(FsBSIZE(dev)));
	clrbuf(bp);
	if (fp->s_tfree)
		fp->s_tfree--;
	fp->s_fmod = 1;
	*abn = bno;
	return (bp);

nospace:
	fp->s_nfree = 0;
	fp->s_tfree = 0;
	prdev("no space", dev);
	delay(hz<<2);
	u.u_error = ENOSPC;
	return (NULL);
}

/*
 * place the specified disk block back on the free list of the
 * specified device on which ip resides.
 */
bell_free(ip, bno)
	struct inode *ip;
	daddr_t bno;
{
	register struct filsys *fp;
	register struct buf *bp;

	fp = getfs(ip->i_mntdev);
	fp->s_fmod = 1;
	while(fp->s_flock)
		(void) sleep((caddr_t)&fp->s_flock, PINOD);
	if (badblock(fp, bno, ip->i_dev))
		return;
	if (fp->s_nfree <= 0) {
		fp->s_nfree = 1;
		fp->s_free[0] = 0;
	}
	if (fp->s_nfree >= NICFREE) {
		fp->s_flock++;
		bp = getblk(ip->i_dev, FsLTOP(ip->i_dev, bno),
				       BTOBB(FsBSIZE(ip->i_dev)));
		(bp->b_un.b_fblk)->df_nfree = fp->s_nfree;
		bcopy((caddr_t)fp->s_free,
		      (caddr_t)(bp->b_un.b_fblk)->df_free,
		      sizeof(fp->s_free));
		fp->s_nfree = 0;
		bwrite(bp);
		fp->s_flock = 0;
		wakeup((caddr_t)&fp->s_flock);
	}
	fp->s_free[fp->s_nfree++] = bno;
	fp->s_tfree++;
	fp->s_fmod = 1;
}

/*
 * Allocate an unused I node on the specified device.
 * Used with file creation.
 * The algorithm keeps up to NICINOD spare I nodes in the
 * super block. When this runs out, a linear search through the
 * I list is instituted to pick up NICINOD more.
 */
struct inode *
bell_ialloc(pip, mode, nlink, rdev)
	struct inode *pip;
	int mode, nlink;
	dev_t rdev;
{
	register struct filsys *fp;
	register struct inode *ip;
	register short i;
	register struct buf *bp;
	register struct dinode *dp;
	register ino_t ino;
	register daddr_t adr;
	dev_t dev;

	fp = getfs(pip->i_mntdev);
	dev = pip->i_dev;

loop:
	while(fp->s_ilock)
		(void) sleep((caddr_t)&fp->s_ilock, PINOD);
	if ((fp->s_ninode > 0) && (ino = fp->s_inode[--fp->s_ninode])) {
		ip = iget(pip->i_mntdev, ino);
		if (ip == NULL)
			return (NULL);
#ifdef	ASSERT
		if (ip->i_mntdev != pip->i_mntdev)
			panic("bell_ialloc: changed mount points");
#endif
		if (ip->i_ftype == 0) {
			if (fp->s_tinode)
				fp->s_tinode--;
			fp->s_fmod = 1;
			return (bell_icreate(ip, mode, nlink, rdev));
		}
		/*
		 * Inode was allocated after all.
		 * Look some more.
		 */
		bell_iupdat(ip, &time, &time);
		iput(ip);
		goto loop;
	}
	fp->s_ilock++;
	fp->s_ninode = NICINOD;
	ino = FsINOS(dev, fp->s_inode[0]);
	for(adr = FsITOD(dev, ino); adr < fp->s_isize; adr++) {
		bp = bread(dev, FsLTOP(dev, adr), BTOBB(FsBSIZE(dev)));
		if (u.u_error) {
			brelse(bp);
			ino += FsINOPB(dev);
			continue;
		}
		dp = bp->b_un.b_dino;
		for(i=0; i<FsINOPB(dev); i++,ino++,dp++) {
			if (fp->s_ninode <= 0)
				break;
			if (dp->di_mode == 0)
				fp->s_inode[--fp->s_ninode] = ino;
		}
		brelse(bp);
		if (fp->s_ninode <= 0)
			break;
	}
	fp->s_ilock = 0;
	wakeup((caddr_t)&fp->s_ilock);
	if (fp->s_ninode > 0) {
		fp->s_inode[fp->s_ninode-1] = 0;
		fp->s_inode[0] = 0;
	}
	if (fp->s_ninode != NICINOD) {
		fp->s_ninode = NICINOD;
		goto loop;
	}
	fp->s_ninode = 0;
	prdev("Out of inodes", dev);
	u.u_error = ENOSPC;
	fp->s_tinode = 0;
	return (NULL);
}

/*
 * Free the specified I node on the specified device.
 * The algorithm stores up to NICINOD I nodes in the super
 * block and throws away any more.
 */
bell_ifree(ip)
	register struct inode *ip;
{
	register struct filsys *fp = getfs(ip->i_mntdev);

	fp->s_tinode++;
	if (fp->s_ilock)
		return;
	fp->s_fmod = 1;
	if (fp->s_ninode >= NICINOD) {
		if (ip->i_number < fp->s_inode[0])
			fp->s_inode[0] = ip->i_number;
		return;
	}
	fp->s_inode[fp->s_ninode++] = ip->i_number;
}

#endif
