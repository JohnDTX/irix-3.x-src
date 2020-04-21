# include "toyfs.h"
# include "bell_toyfs.h"

extern USR U;

/*
 * Allocate an unused I node on the specified device.
 * Used with file creation.
 * The algorithm keeps up to NICINOD spare I nodes in the
 * super block. When this runs out, a linear search through the
 * I list is instituted to pick up NICINOD more.
 */
struct bell_dinode *
bell_ialloc(dev, mode, nlink)
	register dev_t dev;
{
# ifdef notdef
	register struct filsys *fp;
	register struct inode *ip;
	register short i;
	register struct buf *bp;
	register struct dinode *dp;
	register ino_t ino;
	register daddr_t adr;

	fp = getfs(dev);
loop:
	while(fp->s_ilock)
		(void) sleep((caddr_t)&fp->s_ilock, PINOD);
	if ((fp->s_ninode > 0) && (ino = fp->s_inode[--fp->s_ninode])) {
		ip = iget(dev, ino);
		if (ip == NULL)
			return(NULL);
		if (ip->i_mode == 0) {
			/* found inode: update now to avoid races */
			ip->i_mode = mode;
			ip->i_nlink = nlink;
			ip->i_flag |= IACC|IUPD|ICHG|ISYN;
			ip->i_uid = u.u_uid;
			ip->i_gid = u.u_gid;
			ip->i_size = 0;
			for (i = NADDR; --i >= 0; )
				ip->i_addr[i] = 0;
			if (fp->s_tinode) fp->s_tinode--;
			fp->s_fmod = 1;
			iupdat(ip, &time, &time);
			return(ip);
		}
		/*
		 * Inode was allocated after all.
		 * Look some more.
		 */
		iupdat(ip, &time, &time);
		iput(ip);
		goto loop;
	}
	fp->s_ilock++;
	fp->s_ninode = NICINOD;
	ino = FsINOS(dev, fp->s_inode[0]);
	for(adr = FsITOD(dev, ino); adr < fp->s_isize; adr++) {
		bp = bread(dev, adr);
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
	return(NULL);
# endif notdef
}

/*
 * Free the specified I node on the specified device.
 * The algorithm stores up to NICINOD I nodes in the super
 * block and throws away any more.
 */
bell_ifree(dev, ino)
	dev_t dev;
	ino_t ino;
{
# ifdef notdef
	register struct filsys *fp;

	fp = getfs(dev);
	fp->s_tinode++;
	if (fp->s_ilock)
		return;
	fp->s_fmod = 1;
	if (fp->s_ninode >= NICINOD) {
		if (ino < fp->s_inode[0])
			fp->s_inode[0] = ino;
		return;
	}
	fp->s_inode[fp->s_ninode++] = ino;
# endif notdef
}
