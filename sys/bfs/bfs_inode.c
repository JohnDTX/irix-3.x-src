/*
 * $Source: /d2/3.7/src/sys/bfs/RCS/bfs_inode.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:26:17 $
 */
#include "bfs.h"
#if NBFS > 0

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/user.h"
#include "../h/file.h"
#include "../h/buf.h"
#include "../h/mount.h"
#include "../com/pipe_inode.h"
#include "../bfs/bfs_inode.h"
#include "../bfs/fblk.h"
#include "../bfs/filsys.h"
#include "../bfs/ino.h"

/*
 * Instantiate a private bell inode structure for ip from ftype and
 * the disk address vector dap.
 */
static int
bell_inode(mode, dap, ip)
	register ushort mode;
	register char *dap;
	register struct inode *ip;
{
	register short naddr;
	register char *p1, *p2;
	
	switch (mode & IFMT) {
	case IFIFO:
		if (! pipe_create(ip)) {	/* alloc a pipe inode */
			return EFBIG;
		}
		ip->i_size = 0;
		break;
	case IFBLK:
	case IFCHR:
		if ((ip->i_fsptr = calloc(1, sizeof(struct com_inode)))
		    == NULL) {
			return EFBIG;
		}
		p1 = (char *) &ip->i_rdev;	/* unpack device name XXX */
		p2 = dap + 1;
		*p1++ = *p2++;
		*p1++ = *p2++;
		ip->i_size = 0;
		break;
	default:
		ASSERT(mode != 0);
		if ((ip->i_fsptr = calloc(1, sizeof(struct bell_inode)))
		    == NULL) {
			return EFBIG;
		}
		p1 = (char *) bell_fsptr(ip)->bi_addr;
		p2 = dap;
		for (naddr = NADDR; naddr > 0; --naddr) {
			*p1++ = 0;
			*p1++ = *p2++;
			*p1++ = *p2++;
			*p1++ = *p2++;
		}
		break;
	}
	com_fsptr(ip)->ci_mode = mode;
	return 0;
}

bell_idestroy(ip)
	register struct inode *ip;
{
	free(ip->i_fsptr);
	ip->i_fsptr = NULL;
}

/*
 * Create in ip a bell inode having attributes mode and nlink.
 */
struct inode *
bell_icreate(ip, mode, nlink, rdev)
	register struct inode *ip;
	register ushort mode;
	short nlink;
	dev_t rdev;
{
	register char *p1, *p2;
	char addr[NADDR * 3];

	ASSERT(ip->i_ftype == 0 && ip->i_nlink == 0);
	if (ip->i_fsptr != NULL) {
		/*
		 * The unlinked inode which is being reused was cached
		 * with its representation intact.
		 */
		ASSERT(ip->i_count == 1);
		bell_idestroy(ip);
	}
	p1 = (char *) addr;	/* pack device name XXX */
	if ((mode & IFMT) == IFBLK || (mode & IFMT) == IFCHR) {
		p2 = (char *) &rdev;
		*p1++ = 0;
		*p1++ = *p2++;
		*p1++ = *p2++;
	}
	bzero((caddr_t) p1, (addr + sizeof addr) - p1);
	u.u_error = bell_inode(mode, addr, ip);
	if (u.u_error != 0) {
		ip->i_flag |= IERROR;
		iput(ip);
		return NULL;
	}
	return icreate(ip, mode & IFMT, nlink, rdev);
}

/*
 * bell_iread:
 *	- read in from the disk a bell inode
 */
struct inode *
bell_iread(ip)
	register struct inode *ip;
{
	register struct buf *bp;
	register struct dinode *dp;

	ip->i_flag |= IFLUX;
	bp = bread(ip->i_dev,
		   FsLTOP(ip->i_dev, FsITOD(ip->i_dev, ip->i_number)),
		   BTOBB(FsBSIZE(ip->i_dev)));
	if (bp->b_flags & B_ERROR) {
		goto error;
	}
	dp = bp->b_un.b_dino;
	dp += FsITOO(ip->i_dev, ip->i_number);
#ifdef	ASSERT
	if (ip->i_fsptr)
		panic("bell_iread dup i_fsptr");
#endif

	/*
	 * Copy inode type and generation number from disk.
	 */
	ip->i_ftype = dp->di_mode & IFMT;
	ip->i_gen = (long) dp->di_gen;	/* XXX di_gen too short */

	/*
	 * If the inode is linked, fill in public and common private
	 * state from the disk image.  If the inode is unlinked, its
	 * disk image, apart from the generation number, is stale.
	 * The state set up below will be filled into an unlinked inode
	 * by bell_icreate().
	 */
	if (ip->i_ftype == 0) {
		ip->i_nlink = 0;	/* force to zero */
	} else {
		register struct com_inode *ci;

		ASSERT(dp->di_mode != 0);
		u.u_error = bell_inode(dp->di_mode, dp->di_addr, ip);
		if (u.u_error != 0) {
			goto error;
		}
		/* set common private state */
		ci = com_fsptr(ip);
		ci->ci_mode = dp->di_mode & ~ISVTX;
		ci->ci_atime = dp->di_atime;
		ci->ci_mtime = dp->di_mtime;
		ci->ci_ctime = dp->di_ctime;
	
		ip->i_nlink = dp->di_nlink;
		ip->i_uid = dp->di_uid;
		ip->i_gid = dp->di_gid;
		ip->i_size = dp->di_size;
	}

	brelse(bp);
	ip->i_flag &= ~IFLUX;
	return (ip);

error:
	brelse(bp);
	ip->i_flag |= IERROR;
	iput(ip);
	return (NULL);
}

/*
 * bell_iupdat:
 *	- write out the given inode to the disk
 */
bell_iupdat(ip, ta, tm)
	register struct inode *ip;
	time_t *ta, *tm;
{
	register struct buf *bp;
	register struct dinode *dp;
	register short naddr;
	register char *p1, *p2;
	register struct com_inode *ci;

	bp = bread(ip->i_dev,
		   FsLTOP(ip->i_dev, FsITOD(ip->i_dev, ip->i_number)),
		   BTOBB(FsBSIZE(ip->i_dev)));
	if (bp->b_flags & B_ERROR) {
		ip->i_flag |= IERROR;
		brelse(bp);
		return;
	}
	dp = bp->b_un.b_dino + FsITOO(ip->i_dev, ip->i_number);
	dp->di_nlink = ip->i_nlink;
	dp->di_uid = ip->i_uid;
	dp->di_gid = ip->i_gid;
	dp->di_size = ip->i_size;
	dp->di_gen = (char) ip->i_gen;	/* XXX di_gen too short */

	switch (ip->i_ftype) {
	case IFCHR:
	case IFBLK:
		p1 = (char *) dp->di_addr;	/* pack device name XXX */
		p2 = (char *) &ip->i_rdev;
		*p1++ = 0;
		*p1++ = *p2++;
		*p1++ = *p2++;
		bzero((caddr_t) p1, NDADDR - 3);
		/* FALL THROUGH */
	case IFIFO:
		dp->di_size = 0;
		break;
	case 0:
		/*
		 * This inode has just been unlinked and freed.
		 */
		bzero((caddr_t) dp->di_addr, sizeof dp->di_addr);
		break;
	default:
		p1 = (char *) dp->di_addr;
		p2 = (char *) bell_fsptr(ip)->bi_addr;
		naddr = NADDR;
		do {
			if(*p2++ != 0)
				printf("iaddress > 2^24\n");
			*p1++ = *p2++;
			*p1++ = *p2++;
			*p1++ = *p2++;
		} while (--naddr > 0);
	}

	/*
	 * Update mode, time stamps, and generation number.
	 */
	ci = com_fsptr(ip);
	ASSERT(!(ip->i_nlink > 0 && ci->ci_mode == 0));
	dp->di_mode = ci->ci_mode & ~ISVTX;
	if (ip->i_flag & IACC)
		dp->di_atime = ci->ci_atime = *ta;
	if (ip->i_flag & IUPD) 
		dp->di_mtime = ci->ci_mtime = *tm;
	if (ip->i_flag & ICHG) 
		dp->di_ctime = ci->ci_ctime = time;

	if (ip->i_flag & ISYN)
		bwrite(bp);
	else
		bdwrite(bp);
	ip->i_flag &= ~(IACC|IUPD|ICHG|ISYN);
}

/*
 * Truncate an inode to zero length, freeing all its blocks.
 */
bell_itrunc(ip)
	struct inode *ip;
{
	bell_setsize(ip, (off_t) 0);
}

/*
 * Free all the disk blocks associated with the specified inode structure.
 * The blocks of the file are removed in reverse order. This FILO
 * algorithm will tend to maintain
 * a contiguous free list much longer than FIFO.
 */
bell_setsize(ip, newsize)
	register struct inode *ip;
	off_t newsize;
{
	register struct bell_inode *bip;
	register daddr_t bn;
	register int i;
	register int stop;
	register off_t newsize_fs;

	bip = bell_fsptr(ip);
	i = ip->i_ftype;
	if (!(i == IFREG || i == IFDIR || i == IFLNK))
		return;

	if (ip->i_size <= newsize)
		return;

	/*
	 * Convert size to filesystem blocks
	 */
	newsize_fs = (newsize + FsBSIZE(ip->i_dev) - 1) >> FsBSHIFT(ip->i_dev);

	/*
	 * Loop backwards through file, releasing blocks which are
	 * no longer being used.
	 */
	stop = 0;
	for (i = NADDR; --i >= 0; ) {
		bn = bip->bi_addr[i];
		if (bn == 0)
			continue;
		switch (i) {
		  default:
			if (i < newsize_fs)
				stop = 1;
			break;
		  case NADDR-1:			/* triple indirect */
			stop = bell_tloop(ip, bn, newsize_fs, 2,
					      FsNINDIR(ip->i_dev),
					      FsNINDIR(ip->i_dev));
			break;
		  case NADDR-2:			/* double indirect */
			stop = bell_tloop(ip, bn, newsize_fs, 1, 0,
					 FsNINDIR(ip->i_dev));
			break;
		  case NADDR-3:			/* single indirect */
			stop = bell_tloop(ip, bn, newsize_fs, 0, 0, 0);
			break;
		}
		if (stop)
			break;
		bip->bi_addr[i] = 0;
		bell_free(ip, bn);
		ip->i_flag |= IUPD | ICHG;
	}
	ip->i_size = newsize;
}

bell_tloop(ip, bn, newsize, level, t, d)
	struct inode *ip;
	daddr_t bn;
	off_t newsize;
	int level;
	int t, d;
{
	register daddr_t nb;
	register int i;
	register struct buf *bp;
	register daddr_t *bap;
	int stop;
	off_t size;
	int dirty;

	/*
	 * Check argument position to see if any truncation should
	 * be done at this file position.  If not, return.  If block
	 * # is zero, then entire area can be ignored.
	 */
	size = (t << (FsNSHIFT(ip->i_dev) * 2)) +
		(d << FsNSHIFT(ip->i_dev)) +
		(FsNINDIR(ip->i_dev)) +
		(NADDR - 3);
	if (size < newsize)
		return (1);			/* all done */

	/*
	 * Have to truncate part (or all) of this block indirect.
	 */
	bp = NULL;
	stop = 0;
	dirty = 0;
	for (i = NINDIR; --i >= 0; ) {
		if (bp == NULL) {
			bp = bread(ip->i_dev, FsLTOP(ip->i_dev, bn),
					      BTOBB(FsBSIZE(ip->i_dev)));
			if (bp->b_flags & B_ERROR) {
				brelse(bp);
				return (1);
			}
			bap = bp->b_un.b_daddr;
		}
		nb = bap[i];
		if (nb == 0)
			continue;

		switch (level) {
		  case 0:			/* do indirect blocks */
			/*
			 * Compute location in file (in filesystem blocks).
			 * Clear out indirect blocks.
			 */
			size = (t << (FsNSHIFT(ip->i_dev) * 2)) +
				(d << FsNSHIFT(ip->i_dev)) +
				(i) +
				(NADDR - 3);
			if (size < newsize)
				goto done;
			break;
		  case 1:			/* do single indirects */
			if (dirty)
				bdwrite(bp);
			else
				brelse(bp);
			bp = NULL;
			if (stop = bell_tloop(ip, nb, newsize, 0, t, i))
				goto done;
			goto get_back_bp;
		  case 2:			/* do double indirects */
			if (dirty)
				bdwrite(bp);
			else
				brelse(bp);
			bp = NULL;
			if (stop = bell_tloop(ip, nb, newsize, 1, i, d))
				goto done;
get_back_bp:
			/*
			 * Get back pointer to indirect block, incase it was
			 * released above.  Because we didn't stop, the entire
			 * sub-block was released.  Release pointer to it.
			 */
			if (bp == NULL) {
				bp = bread(ip->i_dev, FsLTOP(ip->i_dev, bn),
						      BTOBB(FsBSIZE(ip->i_dev)));
				if (bp->b_flags & B_ERROR) {
					brelse(bp);
					return (1);
				}
				bap = bp->b_un.b_daddr;
			}
			break;
		}
		bap[i] = 0;
		dirty = 1;
		bell_free(ip, nb);
		ip->i_flag |= IUPD | ICHG;
	}

done:
	if (bp) {
		if (dirty)
			bdwrite(bp);
		else
			brelse(bp);
	}
	return (stop);
}

#endif
