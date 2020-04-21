/*
 * Inode/data allocation/de-allocation for the efs.
 *
 * $Source: /d2/3.7/src/sys/efs/RCS/efs_alloc.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:27:50 $
 */
#include "efs.h"
#if NEFS > 0

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/user.h"
#include "../h/buf.h"
#include "../h/fstyp.h"
#include "../h/mount.h"
#include "../efs/efs_inode.h"
#include "../efs/efs_fs.h"
#include "../efs/efs_sb.h"
#include "../efs/efs_ino.h"

/*
 * efs_alloc:
 *	- allocate len worth of space on fs for inode ip
 *	- if we can't find len's worth, then return in ex
 *	  the amount we did find.
 *	- fill in ex with the new information
 *	- return 0 if there is no space, 1 otherwise
 *	- musthave is true if we must have a piece of desiredlen
 * XXX	this needs work!
 */
efs_alloc(ip, ex, desiredlen, musthave, wex)
	struct inode *ip;
	extent *ex;
	register int desiredlen;
	int musthave;
	struct bmapval *wex;
{
	register struct cg *cg;
	register struct cgdmap *cgd;
	register struct efs *fs;
	register int i, j;
	register int last;
	register struct cgdmap *savecgd;
	struct cg *lastcg;
	struct buf *bp;
	int isfirstcgd;

#ifdef	ASSERT
	if (desiredlen == 0)
		panic("efs_alloc desired len is zero");
#endif
	fs = getfs(ip->i_mntdev);
	if (fs->fs_corrupted || (fs->fs_tfree <= 0))
		goto no_more_room;

	lastcg = &fs->fs_cgs[fs->fs_ncg];
	isfirstcgd = 0;
	if (fs->fs_diskfull)
		goto slow_search;

	/*
	 * Search each cylinder group, starting at the inodes for
	 * an extent of the right length.
	 * XXX change this to sweep outward from the inodes cg until
	 * XXX all cg's are scanned instead of just forward.
	 */
	cg = &fs->fs_cgs[EFS_ITOCG(fs, ip->i_number)];
	for (i = fs->fs_ncg; --i >= 0; cg++) {
		if (cg >= lastcg)
			cg = &fs->fs_cgs[0];
		/*
		 * If cg hasn't any data information, build up some
		 * more summary information from the bitmap.
		 * XXX We only allocate out of summary info right now.
		 * XXX this could be very bad.
		 */
		if (cg->cg_dfree < desiredlen)
			continue;
		if (cg->cg_nextd >= cg->cg_lastd) {
#ifdef	ASSERT
			efs_builddsum(fs, cg, desiredlen >> 1, 1);
#else
			efs_builddsum(fs, cg, desiredlen >> 1, 0);
#endif
			if (cg->cg_nextd >= cg->cg_lastd)
				continue;
		}
		/*
		 * Now loop through this cylinders summary info, looking for
		 * an extent big enough for our needs.
		 * XXX add a cg->cg_largestsum
		 */
		cgd = &cg->cg_dsum[cg->cg_nextd];
		savecgd = NULL;
		last = cg->cg_lastd;
		for (j = cg->cg_nextd; j < last; j++, cgd++) {
			if (cgd->d_length == 0)		/* already used */
				continue;
			if (cgd->d_length >= desiredlen) {
				isfirstcgd = 1;
				goto found;
			}
			/*
			 * Remember this cgd if its larger than the
			 * previously saved cgd.
			 */
			if ((savecgd == NULL) ||
			    (savecgd->d_length < cgd->d_length))
				savecgd = cgd;
		}
		/*
		 * If we can stand to get a smaller piece, see if the largest
		 * cgd we found is at least 50% of what we want.  If so,
		 * go ahead and use it.
		 */
		if (!musthave && savecgd &&
			      (savecgd->d_length >= (desiredlen >> 1))) {
			cgd = savecgd;
			goto short_find;
		}
	}
	fs->fs_diskfull = 1;
	if (musthave)
		goto no_more_room;

	/*
	 * Okay.  Being smart didn't work.  Set a flag indicating so, so that
	 * next time we will be stupid first. (duh).  Now do a brute force
	 * search of the cylinder groups looking for the first free spot
	 * we find.  Guaranteed to make slow files.
	 */
slow_search:
	cg = &fs->fs_cgs[0];
	for (i = fs->fs_ncg; --i >= 0; cg++) {
		/*
		 * Generate new summary information, this time being less
		 * picky about what it looks like.  If we succesfully
		 * build summary info, use the first piece we find.
		 * If musthave is set, then we still check to make sure that
		 * the piece we find is big enough.
		 */
		if (cg->cg_nextd >= cg->cg_lastd) {
			cg->cg_firstdfree = cg->cg_firstdbn;
#ifdef	ASSERT
			efs_builddsum(fs, cg, 1, 1);
#else
			efs_builddsum(fs, cg, 1, 0);
#endif
			if (cg->cg_nextd >= cg->cg_lastd)
				continue;
		}
		cgd = &cg->cg_dsum[cg->cg_nextd];
		if (cgd->d_length) {
			if (!musthave || (cgd->d_length >= desiredlen))
				goto short_find;
		}
		/*
		 * Summary info has already been used.  We search the
		 * rest of the summary slots looking for the first
		 * free one.  If musthave is set, then we still check
		 * to make sure the piece is big enough.
		 */
		cgd++;
		cg->cg_nextd++;
		j = cg->cg_nextd;
		last = cg->cg_lastd;
		while (j < last) {
			if (cgd->d_length) {
				if (!musthave || (cgd->d_length >= desiredlen))
					goto short_find;
			}
			j++;
			cgd++;
		}
	}

	/*
	 * Out of space.  Thats the way the partition crumbles.
	 */
no_more_room:
	u.u_error = ENOSPC;
	desiredlen = 0;
	goto out;

	/*
	 * Found a piece to use, but its not the right length.  Truncate
	 * desiredlen if its larger than the summary info length.  It
	 * is possible under fs_diskfull situations to end up with a piece
	 * here which is too big.
	 */
short_find:
	if (cgd->d_length < desiredlen)
		desiredlen = cgd->d_length;

	/*
	 * Found a piece of disk to use.  Lop off the amount being
	 * used from the summary piece.
	 */
found:
#ifdef	ASSERT
	if (desiredlen == 0) {
		printf("efs_alloc botch, desiredlen=0\n");
		printf("cgd=%x cg=%x d_bn=%d d_len=%d\n",
			       cgd, cg, cgd->d_bn, cgd->d_length);
		debug("efs_alloc");
	}
#endif
	ex->ex_bn = last = cgd->d_bn;
	ex->ex_length = desiredlen;
	i = bftstset(fs->fs_dmap, last, desiredlen);
	if (i != desiredlen) {
		uprintf("filesystem corruption during alloc\n");
		goto no_more_room;
	}
	cgd->d_bn += desiredlen;
	if (((cgd->d_length -= desiredlen) == 0) && isfirstcgd)
		cg->cg_nextd++;
	cg->cg_dfree -= desiredlen;
	fs->fs_tfree -= desiredlen;
	fs->fs_fmod = 1;
	bfclr(fs->fs_dmap, last, desiredlen);
	/*
	 * Get extent and clear it out IFF caller doesn't want to
	 * deal with it (wex == NULL), otherwise return info so
	 * caller can see if it needs to clear it.
	 */
	if (wex) {
		wex->bn = last;
		wex->length = desiredlen;
	} else {
		bp = getblk(fs->fs_dev, (daddr_t)last, desiredlen);
		clrbuf(bp);
		bdwrite(bp);
	}
out:
	return (desiredlen);
}

/*
 * efs_extend:
 *	- attempt to extend the given inode by growing the trailing extent
 */
efs_extend(ip, len, wex)
	struct inode *ip;
	int len;
	struct bmapval *wex;
{
	struct efs_inode *iip;

	iip = efs_fsptr(ip);
	ASSERT(iip->ii_com.ci_magic == EFS_IMAGIC);
	return (efs_grow(ip, &iip->ii_extents[iip->ii_numextents - 1],
			     len, wex));
}

/*
 * efs_grow:
 *	- attempt to extend the given extent attached to the given inode
 */
int
efs_grow(ip, ex, len, wex)
	register struct inode *ip;
	register extent *ex;
	register int len;
	struct bmapval *wex;
{
	register int firstbit;
	register struct cg *cg;
	register struct efs *fs;
	register daddr_t bn;
	register int exlen;
	struct buf *bp;

	fs = getfs(ip->i_mntdev);
	if (efs_badextent(fs, ex, ip))
		return (0);

	/*
	 * Insure that we don't extend across an extent, nor that we attempt
	 * to make an extent too large.
	 */
	bn = ex->ex_bn;
	exlen = ex->ex_length;
	cg = &fs->fs_cgs[EFS_BBTOCG(fs, bn)];
	firstbit = bn + exlen;
	if (firstbit + len >= cg->cg_firstbn + fs->fs_cgfsize)
		len = cg->cg_firstbn + fs->fs_cgfsize - firstbit;
	if (exlen + len >= EFS_MAXEXTENTLEN)
		len = EFS_MAXEXTENTLEN - exlen;
	if (len == 0)
		goto out;

	/*
	 * Scan bitmap starting at bb just following last extent, counting
	 * the number of free bb's directly connected to the last extent.
	 * Stop when either len is exhausted or an allocated bb is found.
	 */
	len = bftstset(fs->fs_dmap, firstbit, len);
	if (len) {
		bfclr(fs->fs_dmap, firstbit, len);
		fs->fs_tfree -= len;
		fs->fs_fmod = 1;
		ex->ex_length += len;
#ifdef	ASSERT
		if (cg->cg_dfree - len < 0) {
			printf("cg too small, cg=%d len=%d dfree=%d\n",
				   cg - &fs->fs_cgs[0], len, cg->cg_dfree);
			debug("efs_grow");
		}
#endif
		cg->cg_dfree -= len;
		if (cg->cg_nextd < cg->cg_lastd)
			efs_cgdcheck(cg, (daddr_t)firstbit, len);

		/*
		 * Get block covering new area and clear it IFF caller
		 * doesn't want to deal with it.
		 */
		if (wex) {
			wex->bn = bn + exlen;
			wex->length = len;
		} else {
			bp = getblk(ip->i_dev, bn + exlen, len);
			clrbuf(bp);
			bdwrite(bp);
		}
	}

out:
	return (len);
}

/*
 * efs_free:
 *	- free the given extent
 */
efs_free(ip, ex)
	register struct inode *ip;
	register extent *ex;
{
	register struct efs *fs;
	register struct cg *cg;
	register daddr_t bn;
	register int len;

	/*
	 * Check and see if the filesystem is corrupted.  If it is
	 * or if the extent is somehow messed up, throw it away.
	 */
	fs = getfs(ip->i_mntdev);
	if (fs->fs_corrupted || efs_badextent(fs, ex, ip))
		return;

	len = ex->ex_length;
	bn = ex->ex_bn;
	cg = &fs->fs_cgs[EFS_BBTOCG(fs, bn)];
	cg->cg_nextd = CG_DSUMS;		/* XXX */
	fs->fs_diskfull = 0;
	cg->cg_dfree += len;
	fs->fs_tfree += len;			/* update total free */
	fs->fs_fmod = 1;
	bfset(fs->fs_dmap, (int) ex->ex_bn, (int) ex->ex_length);
	btoss(fs->fs_dev, bn, len);
}

/*
 * efs_goodcg:
 *	- given a cg which presumably has some free inodes on it,
 *	  figure out if this is a good place to put an inode
 */
int
efs_goodcg(fs, cg, mode)
	register struct efs *fs;
	register struct cg *cg;
	int mode;
{
	switch (mode & IFMT) {
	  case IFREG:
	  case IFLNK:
		if (cg->cg_dfree >= fs->fs_minfree)
			return (1);
		break;
	  case IFDIR:
		/*
		 * Insure that there are at least fs_mindirfree blocks in
		 * the cg before placing a directory in the cg.  This tries
		 * to insure that a new directory will have room for its
		 * child inodes and child data files.
		 */
		if (cg->cg_dfree >= fs->fs_mindirfree)
			return (1);
		break;
	  default:
		/*
		 * For files which need no data blocks, don't bother checking
		 * to see if the cg has any.
		 */
		return (1);
	}
	return (0);
}

/*
 * efs_pickcg:
 *	- given the filesystem and the parent inode of an inode being
 *	  created, pick a good cg to place the inode in
 */
struct cg *
efs_pickcg(fs, pip, mode)
	register struct efs *fs;
	register struct inode *pip;
	int mode;
{
	register int cgs_to_try;
	register struct cg *cg;
	register short cgnum, left, right;

	/*
	 * Search radially outward in either direction for a cg to place
	 * the inode in.  This knows that we have already tried the parent's
	 * cg in efs_ialloc().  Given a cg which may have some inodes in
	 * it, try to see if its a good place to put a new inode based
	 * on external criteria.
	 */
	cgnum = EFS_ITOCG(fs, pip->i_number);
	cgs_to_try = fs->fs_ncg - 1;
	left = cgnum - 1;
	right = cgnum + 1;
	while (cgs_to_try) {
		if (left >= 0) {
			cg = &fs->fs_cgs[left];
			if (!cg->cg_fulli && efs_goodcg(fs, cg, mode))
				return (cg);
			cgs_to_try--;
			left--;
		}
		if (right < fs->fs_ncg) {
			cg = &fs->fs_cgs[right];
			if (!cg->cg_fulli && efs_goodcg(fs, cg, mode))
				return (cg);
			cgs_to_try--;
			right++;
		}
	}

	/*
	 * Okay.  We couldn't find any cylinder groups which had both free
	 * inodes and free data blocks.  Now search for a cylinder group
	 * that has at least one free inode.
	 */
	cg = &fs->fs_cgs[0];
	for (cgs_to_try = fs->fs_ncg; --cgs_to_try >= 0; cg++) {
		if (!cg->cg_fulli)
			return (cg);
	}

	/*
	 * Oh well. No inodes no-where-no-how.
	 */
	return (NULL);
}

/*
 * efs_scanchunk:
 *	- scan the given chunk trying to find a free inode
 *	- ino is assumed here to be at the base of a chunk boundary
 *	- return NULL if an error occurs, or if there are no free inodes
 *	  to be found in the chunk
 *	- return a locked inode pointer if an inode is found free
 *	- the main reason we do this scanning, instead of just doing a
 *	  bunch of iget's is to avoid flushing the inode cache during
 *	  a file allocation
 */
struct inode *
efs_scanchunk(pip, cg, inum)
	struct inode *pip;
	struct cg *cg;
	register ino_t inum;
{
	register int i;
	register struct efs_dinode *dp;
	register struct efs *fs;
	register struct buf *bp;
	register struct inode *ip;
	register daddr_t bn;
	register int len;

	/*
	 * Determine the location of the inode chunk on the disk.
	 */
	fs = getfs(pip->i_mntdev);
	bn = EFS_ITOBB(fs, inum);
	len = fs->fs_inopchunkbb;
	if (bn + fs->fs_inopchunkbb > cg->cg_firstdbn)
		len = cg->cg_firstdbn - bn;
#ifdef	ASSERT
	if ((bn < cg->cg_firstbn) || (bn + len > cg->cg_firstdbn)) {
		printf(
		    "efs_scanchunk: bn=%d inumber=%d inopchunk=%d firsti=%d\n",
				    bn, inum, fs->fs_inopchunk,
				    cg->cg_firsti);
		debug("efs_scanchunk");
	}
#endif

	/*
	 * Start at inum, unless inum is < 2 (i.e., 0 or 1).  We never
	 * check 0 or 1, because those inodes are historically unusable.
	 */
	i = 0;
	if (inum < 2) {
		i = 2;
		inum = 2;
	}
	bp = NULL;
	for (; i < fs->fs_inopchunk; i++, inum++) {
		if (inum > cg->cg_lasti)
			break;
		if (bp == NULL) {
			bp = bread(fs->fs_dev, bn, len);
			if (bp->b_flags & B_ERROR) {
				brelse(bp);
				return (NULL);
			}
		}
		dp = bp->b_un.b_edino + i;
		if (dp->di_mode == 0) {
			/*
			 * We have to release the buffer here, because
			 * iget will need it when it does an iread()
			 */
			brelse(bp);
			bp = NULL;
			ip = iget(pip->i_mntdev, inum);
			if (ip == NULL)
				return (NULL);
			if (ip->i_ftype == 0)
				return (ip);
			efs_iupdat(ip, &time, &time);
			iput(ip);
		}
	}
	if (bp)
		brelse(bp);
	return (NULL);
}

/*
 * efs_scancg:
 *	- scan an entire cg for a free inode, starting at the low point
 *	  kept up to date in cg->cg_lowi.
 *	- if an inode is found free, then a pointer into the inode table
 *	  is returned, and the inode is locked.
 */
struct inode *
efs_scancg(pip, cg)
	struct inode *pip;
	register struct cg *cg;
{
	register struct efs *fs;
	register struct inode *ip;
	register int nchunks;
	register ino_t lowi, highi;

	fs = getfs(pip->i_mntdev);
	lowi = EFS_ITOCHUNKI(fs, cg, cg->cg_lowi);
	highi = EFS_ITOCHUNKI(fs, cg, cg->cg_lasti) + fs->fs_inopchunk - 1;

	/*
	 * Figure out how many chunks we will need to examine.
	 * Becuase lowi was rounded down to the base of a chunk, and
	 * highi was rounded up, we don't need to do any rounding
	 * here.  Do add one here, because highi is inclusive.
	 */
	nchunks = (highi - lowi + 1) / fs->fs_inopchunk;

	while (nchunks--) {
		if (ip = efs_scanchunk(pip, cg, lowi))
			return (ip);
		lowi += fs->fs_inopchunk;
	}
	cg->cg_fulli = 1;
	return (NULL);
}

/*
 * efs_ialloc:
 *	- allocate an inode on the given filesystem
 */
struct inode *
efs_ialloc(pip, mode, nlink, rdev)
	struct inode *pip;
	int mode, nlink;
	dev_t rdev;
{
	register struct inode *ip = NULL;
	register struct efs *fs;
	register struct cg *cg;
	register ino_t lowinum;

	fs = getfs(pip->i_mntdev);
	if (fs->fs_tinode == 0)
		goto out_of_inodes;

	/*
	 * First try to use an inode that is in the same inode chunk
	 * as the parent inode.
	 */
	cg = &fs->fs_cgs[EFS_ITOCG(fs, pip->i_number)];
	if (!cg->cg_fulli) {
		/*
		 * Figure out low inode number in the chunk that the
		 * parents inode number is in.  If the low allocation
		 * mark for the cylinder group is smaller than the end
		 * of the chunk + 1, then there might be some free inodes
		 * in the chunk.  Otherwise, we know for sure that there
		 * are no free inodes in the chunk, and we don't bother
		 * to scan the chunk.
		 */
		lowinum = EFS_ITOCHUNKI(fs, cg, pip->i_number);
		if (cg->cg_lowi < lowinum + fs->fs_inopchunk) {
			if (ip = efs_scanchunk(pip, cg, lowinum))
				goto found;
		}
	}

	/*
	 * Sigh.  Optimal placement didn't work.  Try to place the inode in
	 * the same cg as the parent.  If the parents cg is full, then
	 * pick a good neighboring cg to use.
	 */
	for (;;) {
		if (cg->cg_fulli) {
			if ((cg = efs_pickcg(fs, pip, mode)) == NULL)
				goto out_of_inodes;
		}
		if ((cg->cg_lowi < cg->cg_firsti) ||
		    (cg->cg_lowi > cg->cg_lasti))
			cg->cg_lowi = cg->cg_firsti;
		if (ip = efs_scancg(pip, cg))
			break;
	}

found:
	/*
	 * We found an inode.  Initialize the inode in a generic manner.
	 */
	cg->cg_lowi = ip->i_number + 1;		/* advance low mark */
	fs->fs_tinode--;
	fs->fs_fmod = 1;
	return (efs_icreate(ip, mode, nlink, rdev));

out_of_inodes:
	/*
	 * >LOOK
	 * I see no inodes here.
	 * >QUIT
	 * Your score is 0 out of a possible 560.
	 */
	fs->fs_tinode = 0;
	fs->fs_fmod = 1;
	efs_error(fs, 1);
	u.u_error = ENOSPC;
	return (NULL);
}

/*
 * efs_ifree:
 *	- free the specified I node on the specified filesystem
 *	- if the inode being free'd is earlier than the current
 *	  lowest numbered inode, then note that in cg_firstifree.
 *	  This attempts to keep inodes packed in the front of the
 *	  cylinder group, so as to improve the inode caching done
 *	  via the buffer cache
 */
efs_ifree(ip)
	register struct inode *ip;
{
	register struct efs *fs;
	register struct cg *cg;

	fs = getfs(ip->i_mntdev);
	if (fs->fs_corrupted)			/* don't bother */
		return;

	fs->fs_fmod = 1;
	fs->fs_tinode++;
	cg = &fs->fs_cgs[EFS_ITOCG(fs, ip->i_number)];
	if (ip->i_number < cg->cg_lowi)
		cg->cg_lowi = ip->i_number;
	cg->cg_fulli = 0;
}

#endif
