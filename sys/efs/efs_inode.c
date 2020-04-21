/*
 * $Source: /d2/3.7/src/sys/efs/RCS/efs_inode.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:27:56 $
 */
#include "efs.h"
#if NEFS > 0

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/user.h"
#include "../h/buf.h"
#include "../h/mount.h"
#include "../com/pipe_inode.h"
#include "../efs/efs_inode.h"
#include "../efs/efs_fs.h"
#include "../efs/efs_sb.h"
#include "../efs/efs_ino.h"

/*
 * efs_itobp:
 *	- convert an inumber into a disk buffer containing the inode
 *	- use inode chunking, parameterized by fs_inopchunk to do the i/o
 */
struct buf *
efs_itobp(fs, inum, dinopp)
	register struct efs *fs;
	register ino_t inum;
	struct efs_dinode **dinopp;
{
	register ino_t lowi;
	register daddr_t bn;
	register int len;
	register struct buf *bp;
	register struct cg *cg;

	cg = &fs->fs_cgs[EFS_ITOCG(fs, inum)];

	/*
	 * Set lowi to the inode which is at the base of the inode chunk
	 * that contains inum.  To do this, we make inum
	 * relative to the cg, round it down to the nearest fs_inopchunk,
	 * then make the resulting inumber un-relative.
	 */
	lowi = (((inum - cg->cg_firsti) / fs->fs_inopchunk) *
		fs->fs_inopchunk) + cg->cg_firsti;

	bn = EFS_ITOBB(fs, lowi);
	len = fs->fs_inopchunkbb;
	if (bn + fs->fs_inopchunkbb > cg->cg_firstdbn)
		len = cg->cg_firstdbn - bn;
#ifdef	OS_ASSERT
	if ((bn < cg->cg_firstbn) || (bn + len > cg->cg_firstdbn)) {
		printf("efs_itobp: bn=%d inumber=%d inopchunk=%d firsti=%d\n",
				   bn, inum, fs->fs_inopchunk,
				   cg->cg_firsti);
		debug("efs_itobp");
	}
#endif	/* ASSERT */
	bp = bread(fs->fs_dev, bn, len);
	if (bp->b_flags & B_ERROR) {
		brelse(bp);
		return (NULL);
	}
	*dinopp = bp->b_un.b_edino + (inum - lowi);
	return (bp);
}

/*
 * Free ip's private data and clear ip->i_fsptr.  This is done when the
 * inode is being recycled in the inode cache and when an unlinked, cached
 * inode is reallocated and reshaped by efs_icreate.
 */
efs_idestroy(ip)
	register struct inode *ip;
{
	register struct efs_inode *iip;

	switch (ip->i_ftype) {
	  case IFREG:
	  case IFDIR:
	  case IFLNK:
		iip = efs_fsptr(ip);
		ASSERT(iip->ii_com.ci_magic == EFS_IMAGIC);
		if (iip->ii_numindirs > 0) {
			ASSERT(iip->ii_indir != NULL);
			free((char *) iip->ii_indir);
		}
		break;
	}
	free((char *) ip->i_fsptr);
	ip->i_fsptr = NULL;
}

/*
 * Instantiate private inode state for ftype in ip.  Use the addressing
 * information in *dap unless mode's format is IFIFO (named pipes use the
 * incore pipe filesystem inode structure).  Return an error number.
 */
static int
efs_inode(mode, numextents, dap, ip)
	register ushort mode;
	register int numextents;
	register union di_addr *dap;
	register struct inode *ip;
{
	register int nb;
	register struct efs_inode *iip;

	switch (mode & IFMT) {
	  case IFIFO:
		if (! pipe_create(ip)) {	/* alloc a pipe inode */
			return EFBIG;
		}
		ip->i_size = 0;
		break;
	  case IFCHR:
	  case IFBLK:
		if ((ip->i_fsptr = calloc(1, sizeof(struct com_inode)))
		    == NULL) {
			return EFBIG;
		}
		ip->i_rdev = dap->di_dev;
		ip->i_size = 0;
		com_fsptr(ip)->ci_magic = EFS_IMAGIC;
		break;
	  default:
		ASSERT(mode != 0);
		/*
		 * Check validity on number of extents.  If it's ridiculous,
		 * don't let the user reference it.
		 */
		if ((numextents < 0) || (numextents > EFS_MAXEXTENTS)) {
			return EIO;
		}
		/*
		 * Allocate memory for header of efs_inode area plus room for
		 * as many extents as the file has.  Subtract off the size of
		 * one extent since struct efs_inode already contains
		 * one extent in it.
		 */
		nb = numextents * sizeof(extent);
		if ((ip->i_fsptr =
		    calloc(1, sizeof(struct efs_inode) + nb - sizeof(extent)))
		    == NULL) {
			return EFBIG;
		}
		iip = efs_fsptr(ip);
		iip->ii_com.ci_magic = EFS_IMAGIC;

		/*
		 * Get extent information into efs_inode structure
		 */
		iip->ii_numextents = numextents;

		/*
		 * Because we calloc the fsptr memory, we don't need to
		 * zero these fields:
		 *
		 * iip->ii_numindirs = 0;
		 * iip->ii_flags = 0;
		 */
		if (numextents > EFS_DIRECTEXTENTS) {
			register extent *iex;

			/*
			 * Point ii_indir at the indirect extents in bp,
			 * call efs_readindir() to count and get them,
			 * and then malloc and copy them.
			 */
			iip->ii_indir = dap->di_extents;
			if (efs_readindir(ip)) {
				return EFBIG;
			}
			nb = iip->ii_numindirs * sizeof(extent);
			if ((iex = (extent *) malloc(nb)) == NULL)
				return EFBIG;
			bcopy((caddr_t) iip->ii_indir, (caddr_t) iex, nb);
			iip->ii_indir = iex;
		} else {
			/*
			 * Copy direct extents into the efs_inode
			 * struct's flex array.
			 */
			bcopy((caddr_t) dap->di_extents,
			      (caddr_t) iip->ii_extents, nb);
		}
		break;
	}
	com_fsptr(ip)->ci_mode = mode;
	return 0;
}

/*
 * Create a new inode in ip.  This routine is called from efs_ialloc
 * when a free inode has been found.  It instantiates fs-private state
 * appropriate to mode and calls icreate() to fill in public state and
 * update the disk inode.
 */
struct inode *
efs_icreate(ip, mode, nlink, rdev)
	register struct inode *ip;
	register ushort mode;
	short nlink;
	dev_t rdev;
{
	union di_addr au;	/* addressing union */

	ASSERT(ip->i_ftype == 0 && ip->i_nlink == 0);
	if (ip->i_fsptr != NULL) {
		/*
		 * The unlinked inode which is being reused was cached
		 * with its representation intact.  If its private state was
		 * a struct efs_inode, then we assume that it was truncated
		 * properly and therefore has no indirect extent vector.
		 */
		ASSERT(ip->i_count == 1);
		efs_idestroy(ip);
	}
	au.di_dev = rdev;
	u.u_error = efs_inode(mode, 0, &au, ip);
	if (u.u_error != 0) {
		ip->i_flag |= IERROR;
		iput(ip);
		return NULL;
	}
	return icreate(ip, mode & IFMT, nlink, rdev);
}

/*
 * efs_iread:
 *	- read in from the disk an efs inode
 */
struct inode *
efs_iread(ip)
	register struct inode *ip;
{
	register struct efs *fs;
	register struct buf *bp;
	register struct efs_dinode *dp;
	auto struct efs_dinode *adp;

	ASSERT(ip->i_count == 1 && ip->i_fsptr == NULL);
	ip->i_flag |= IFLUX;
	fs = getfs(ip->i_mntdev);
#ifdef	OS_ASSERT
	if (ip->i_number > fs->fs_lastinum) {
		printf("ip=%x i_number=%d\n", ip, ip->i_number);
		debug("efs_iread");
		ip->i_flag |= IERROR;
		iput(ip);
		return (NULL);
	}
#endif	/* ASSERT */

	/*
	 * Convert inumber to buffer according to efs_itobp() rules.
	 * Returned in adp is the pointer to the efs_dinode in the
	 * buffer.
	 */
	bp = efs_itobp(fs, ip->i_number, &adp);
	if (bp == NULL) {
		u.u_error = EIO;
		goto error;
	}
	dp = adp;

	/*
	 * Set inode type and generation number from disk.
	 */
	ip->i_ftype = (dp->di_mode & IFMT);
	ip->i_gen = dp->di_gen;

	/*
	 * Set private and remaining public state unless unlinked.
	 */
	if (ip->i_ftype == 0) {
		ip->i_nlink = 0;	/* force to zero */
	} else {
		register struct com_inode *ci;

		ASSERT(dp->di_mode != 0);
		u.u_error =
		    efs_inode(dp->di_mode, dp->di_numextents, &dp->di_u, ip);
		if (u.u_error != 0) {
			goto error;
		}
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
	ip->i_flag |= IERROR;
	iput(ip);
	return (NULL);
}

/*
 * efs_readindir:
 *	- read indirect extents from the filesystem for the given inode
 *	- return non-zero on error, zero if everythings ok
 */
int
efs_readindir(ip)
	struct inode *ip;
{
	register int i, ni, amount, nb;
	register struct buf *bp;
	register extent *iex, *ex;
	register struct efs_inode *iip;

	iip = efs_fsptr(ip);
	iex = &iip->ii_indir[0];
	ex = &iip->ii_extents[0];

	/*
	 * Here we check and make sure that the number of indirect extents
	 * is set to something reasonable.  Older versions of the kernel
	 * only supported one indirect extent, and thus the field used
	 * above probably contained a zero.
	 */
	ni = iex->ex_offset;
	if ((ni == 0) || (ni > EFS_DIRECTEXTENTS))
		ni = 1;			/* fix up old kernel mess */

	/*
	 * For each indirect extent, bread in the data and copy it into
	 * the incore extent region.  Note that after this loop completes,
	 * every indirect extent will have been examined and ii_indirbytes
	 * will contain the total number of bytes of indirect extent space
	 * allocated.
	 */
	nb = iip->ii_numextents * sizeof(extent);
	iip->ii_indirbytes = 0;
	for (i = 0; i < ni; i++, iex++) {
		if (iex->ex_length == 0) {
			/*
			 * Indirect extent information in the file is messed
			 * up.  Make the file zero length so that the user
			 * can remove it.
			 */
			goto badfile;
		}
		bp = bread(ip->i_dev, (daddr_t)iex->ex_bn, (int)iex->ex_length);
		if (bp->b_flags & B_ERROR) {
			brelse(bp);
			return (1);
		}
		amount = BBTOB(iex->ex_length);
		iip->ii_indirbytes += amount;
		if (amount > nb)
			amount = nb;
		bcopy((caddr_t)bp->b_un.b_exts, (caddr_t)ex, amount);
		brelse(bp);
		ex = (extent *) ((long)ex + amount);
		nb -= amount;
	}
	iip->ii_numindirs = ni;
	ASSERT((iip->ii_indirbytes % BBSIZE) == 0);

	/*
	 * If there are extra bytes left over, then something screwed up.
	 * Make the inode unusable forcing the user to fsck the filesystem.
	 */
	if (nb) {
		/*
		 * Inode has more extents than the indirect extents provide
		 * for.  Truncate the number of extents down to the amount
		 * provided for.  This way the user can at least recover
		 * part of the files contents.
		 */
		iip->ii_numextents -= (nb / sizeof(extent));
		ASSERT(iip->ii_indirbytes >= iip->ii_numextents * sizeof(extent));
		iip->ii_flags |= II_TRUNC;
		goto badout;
	}
	return(0);

badfile:
	iip->ii_numindirs = 0;
	iip->ii_indirbytes = 0;
	iip->ii_numextents = 0;
	free((char *) iip->ii_indir);
	iip->ii_indir = 0;
	ip->i_fsptr = realloc((char *) iip,
			      sizeof(struct efs_inode) - sizeof(extent));
	ip->i_size = 0;

badout:
	ip->i_flag |= IUPD | ICHG;
	printf("indirect inode corruption on dev %x ino %d\n",
			 ip->i_dev, ip->i_number);
	return (0);
}

/*
 * efs_iupdat:
 *	- write out the given inode to the disk
 */
efs_iupdat(ip, ta, tm)
	register struct inode *ip;
	time_t *ta, *tm;
{
	register struct efs_dinode *dp;
	register struct efs_inode *iip;
	register struct com_inode *ci;
	register struct buf *bp;
	register struct efs *fs;
	auto struct efs_dinode *adp;

	fs = getfs(ip->i_mntdev);
#ifdef	OS_ASSERT
	if (ip->i_number > fs->fs_lastinum) {
		uprintf("inode number %d out of range\n", ip->i_number);
		fs->fs_corrupted++;
		ip->i_flag |= IERROR;
		return;
	}
#endif	/* ASSERT */

	/*
	 * Convert inumber to buffer according to efs_itobp() rules.
	 * Returned in adp is the pointer to the efs_dinode in the
	 * buffer.
	 */
	bp = efs_itobp(fs, ip->i_number, &adp);
	if (bp == NULL) {
		ip->i_flag |= IERROR;
		u.u_error = EIO;
		return;
	}
	dp = adp;

	dp->di_nlink = ip->i_nlink;
	dp->di_uid = ip->i_uid;
	dp->di_gid = ip->i_gid;
	dp->di_size = ip->i_size;
	dp->di_gen = ip->i_gen;

	/*
	 * We only update the extents if the file is NOT a named pipe.
	 * Named pipes have no addressing info, just time stamps.
	 */
	switch (ip->i_ftype) {
	  case IFCHR:
	  case IFBLK:
		dp->di_u.di_dev = ip->i_rdev;
		ASSERT(com_fsptr(ip)->ci_magic == EFS_IMAGIC);
		dp->di_numextents = 0;
		dp->di_size = 0;
		break;
	  case IFIFO:
		/*
		 * Clear out unused portions of inode for these inode types.
		 * This repairs damage, if any existed.
		 */
		ASSERT(com_fsptr(ip)->ci_magic == PIPE_MAGIC);
		dp->di_numextents = 0;
		dp->di_size = 0;
		break;
	  case 0:
		/*
		 * This inode has just been unlinked and freed.
		 */
		bzero((caddr_t) &dp->di_u, sizeof dp->di_u);
		break;
	  default: {
		register int nb, ne;	/* byte and disk inode extent counts */

		/*
		 * Copy out extents into inode if they fit inside the inode.
		 * Otherwise write the extents to a seperate location on
		 * the disk.  Because the system may have pre-extended the
		 * inode we truncate the last extent to fit within the
		 * files i_size, to keep the on-disk inode consistent.
		 * Also, fill the disk inode's extent array, zeroing the
		 * unused part.
		 */
		iip = efs_fsptr(ip);
		ASSERT(iip->ii_com.ci_magic == EFS_IMAGIC);
		ne = iip->ii_numextents;
		dp->di_numextents = ne;
		if (ne > EFS_DIRECTEXTENTS) {
			ne = iip->ii_numindirs;
			nb = ne * sizeof(extent);
			bcopy((caddr_t) &iip->ii_indir[0],
			      (caddr_t) &dp->di_u.di_extents[0], nb);
			dp->di_u.di_extents[0].ex_offset = ne;
			efs_writeindir(ip);
		} else {
			nb = ne * sizeof(extent);
			bcopy((caddr_t) &iip->ii_extents[0],
			      (caddr_t) &dp->di_u.di_extents[0], nb);
			/*
			 * See if the last extent in the copy needs to
			 * be trimmed.
			 */
			if (ne) {
				register extent *copy;

				copy = &dp->di_u.di_extents[ne-1];
				if (copy->ex_offset + copy->ex_length >
						  BTOBB(ip->i_size)) {
					copy->ex_length = BTOBB(ip->i_size) -
						copy->ex_offset;
				}
			}
		}
		/* clear out unused extents in disk inode */
		if (EFS_DIRECTEXTENTS * sizeof(extent) - nb > 0)
			bzero((caddr_t) &dp->di_u.di_extents[ne],
			      EFS_DIRECTEXTENTS * sizeof(extent) - nb);
		break;
	  }
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
 * efs_writeindir:
 *	- write to disk the indirect extents for inode ip
 */
efs_writeindir(ip)
	register struct inode *ip;
{
	register extent *iex, *ex;
	register struct buf *bp;
	register struct efs_inode *iip;
	register int ni, nb, amount;

	iip = efs_fsptr(ip);

	iex = &iip->ii_indir[0];
	ex = &iip->ii_extents[0];
	nb = iip->ii_numextents * sizeof(extent);
	for (ni = iip->ii_numindirs; --ni >= 0; iex++) {
		bp = getblk(ip->i_dev, (daddr_t)iex->ex_bn,
				       (int)iex->ex_length);
		amount = BBTOB(iex->ex_length);
		if (amount > nb)
			amount = nb;
		bcopy((caddr_t)ex, bp->b_un.b_addr, amount);
		/*
		 * See if last extent needs to be trimmed.
		 */
		if ((ni == 0) && amount) {
			register extent *copy;

			copy = (extent *) (bp->b_un.b_addr + amount -
							   sizeof(extent));
			if (copy->ex_offset + copy->ex_length >
					    BTOBB(ip->i_size)) {
				copy->ex_length = BTOBB(ip->i_size) -
					copy->ex_offset;
			}
		}
		if (ip->i_flag & ISYN)
			bwrite(bp);
		else
			bdwrite(bp);
		ex = (extent *) ((long)ex + amount);
		nb -= amount;
	}

	if (nb < 0)
		prdev("inode indirect corruption", ip->i_dev);
}

/*
 * Truncate an inode to zero length, freeing all its blocks
 */
efs_itrunc(ip)
	struct inode *ip;
{
	register int i;

	i = ip->i_ftype;
	if ((i == IFREG) || (i == IFDIR) || (i == IFLNK)) {
		efs_fsptr(ip)->ii_flags |= II_TRUNC;
		efs_dotrunc(ip, (off_t) 0);
	}
}

/*
 * Same is efs_itrunc, except truncate to a given size
 */
efs_setsize(ip, newsize)
	struct inode *ip;
	off_t newsize;
{
	register int i;

	i = ip->i_ftype;
	if (((efs_fsptr(ip)->ii_flags & II_TRUNC) ||
	     (newsize != ip->i_size)) &&
	    ((i == IFREG) || (i == IFDIR) || (i == IFLNK))) {
		efs_fsptr(ip)->ii_flags |= II_TRUNC;
		efs_dotrunc(ip, (off_t) newsize);
	}
}

/*
 * Free extra disk blocks associated with the inode structure
 */
efs_dotrunc(ip, newsize)
	register struct inode *ip;
	off_t newsize;
{
	register int i;
	register struct efs_inode *iip;
	register extent *ex;
	register int newsizebb;
	register int numextents;
	register int offset;
	int newlen;
	int oldlen;
	extent tempex;
	int numindirs;

	/*
	 * There are three reasons not to truncate a file:
	 *	(1) the file is not the sort to be truncated
	 *	(2) "truncation" would lengthen the file
	 *	(3) the inode doesn't need truncation, because it hasn't
	 *	    been grown.
	 */
	i = ip->i_ftype;
	iip = efs_fsptr(ip);
	ASSERT(iip->ii_com.ci_magic == EFS_IMAGIC);
	if ((!(i == IFREG || i == IFDIR || i == IFLNK)) ||
	    (newsize > ip->i_size) ||
	    (!(iip->ii_flags & II_TRUNC)))
		return;

	/*
	 * Loop through each extent in the file, release those that are
	 * past the new end of file.
	 */
	tempex.ex_offset = 0;			/* loop invariant */
	numextents = iip->ii_numextents;
	newsizebb = BTOBB(newsize);		/* round up */
	ex = &iip->ii_extents[0]; 
	for (i = numextents; --i >= 0; ex++) {
		if (ex->ex_offset >= newsizebb) {
			/*
			 * Free entire extent
			 */
			efs_free(ip, ex);
			iip->ii_numextents--;
		} else
		if (ex->ex_offset + ex->ex_length > newsizebb) {
			/*
			 * Free partial extent
			 */
			newlen = newsizebb - ex->ex_offset;
			tempex.ex_bn = ex->ex_bn + newlen;
			tempex.ex_length = ex->ex_length - newlen;
			ex->ex_length = newlen;
			efs_free(ip, &tempex);
		}
	}

	/*
	 * Loop through the indirect extents, if any and reduce those also.
	 * We have to handle the case where the file shrinks from indirect
	 * extents back in to direct extents.  When the dust settles,
	 * realloc the direct and indirect extent memory so as to avoid
	 * wastage.
	 */
	numindirs = iip->ii_numindirs;
	if (numindirs) {
		if (iip->ii_numextents > EFS_DIRECTEXTENTS)
			newsizebb = BTOBB(iip->ii_numextents * sizeof(extent));
		else {
			/*
			 * File is losing its indirect extents.  Truncate the
			 * indirect extents away
			 */
			newsizebb = 0;
		}
		ex = &iip->ii_indir[0];
		offset = 0;
		for (i = numindirs; --i >= 0; ex++) {
			oldlen = ex->ex_length;
			if (offset >= newsizebb) {
				/*
				 * Free entire indirect extent.
				 */
				iip->ii_indirbytes -= BBTOB(oldlen);
				iip->ii_numindirs--;
				efs_free(ip, ex);
			} else
			if (offset + oldlen > newsizebb) {
				/*
				 * Free partial indirect extent.
				 */
				newlen = newsizebb - offset;
				tempex.ex_bn = ex->ex_bn + newlen;
				tempex.ex_length = oldlen - newlen;
				ex->ex_length = newlen;
				iip->ii_indirbytes -= BBTOB(tempex.ex_length);
				efs_free(ip, &tempex);
			}
			offset += oldlen;
		}
	} else {
		ASSERT(iip->ii_indir == 0);
		ASSERT(iip->ii_indirbytes == 0);
	}

	/*
	 * Adjust the memory allocated for the indirect extents.
	 */
	if (iip->ii_numindirs == 0) {
		if (numindirs != 0) {
			/*
			 * Throw away memory for old indirect extent handling
			 */
			free((char *) iip->ii_indir);
			iip->ii_indir = 0;
			iip->ii_indirbytes = 0;
		}
	} else {
		ASSERT(iip->ii_numextents > EFS_DIRECTEXTENTS);
		ASSERT(iip->ii_indirbytes >= iip->ii_numextents * sizeof(extent));
		iip->ii_indir = (extent *) realloc((char *) iip->ii_indir,
					   iip->ii_numindirs * sizeof(extent));
	}
	/*
	 * Adjust the memory allocated for the direct extents
	 */
	ip->i_fsptr = realloc((char *) iip,
			      sizeof(struct efs_inode) - sizeof(extent) +
			      iip->ii_numextents * sizeof(extent));

	/* lastly, update size and inode flags if file changed */
	ip->i_size = newsize;
	ip->i_flag |= IUPD | ICHG;
	efs_fsptr(ip)->ii_flags &= ~II_TRUNC;
}

#endif
