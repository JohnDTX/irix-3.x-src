/*
 * $Source: /d2/3.7/src/sys/efs/RCS/efs_bmap.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:27:52 $
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

/* convert byte to logical block, truncated and vice-versa */
#define	BTOLB(off)	(((off) + efs_lbsize - 1) >> efs_lbshift)
#define	BTOLBT(off)	((off) >> efs_lbshift)
#define	LBTOB(lb)	((lb) << efs_lbshift)

/* convert logical blocks to bb's */
#define	LBTOBB(lb)	((lb) << efs_lbtobbshift)

/* convert bb to locial block, truncated */
#define	BBTOLBT(bb)	((bb) >> efs_lbtobbshift)

/* convert bb to locial block */
#define	BBSPERLB	(1 << efs_lbtobbshift)
#define	BBTOLB(bb)	(((bb) + BBSPERLB - 1) >> efs_lbtobbshift)

/*
 * efs_bmap:
 *	- translate a logical block # into an extent
 *	- if writing, allocate an extent if we extend the file
 * XXX	- needs alot of rework to support small user i/o
 */
efs_bmap(ip, rw, iex, rex)
	struct inode *ip;
	int rw;
	register struct bmapval *iex;
	struct bmapval *rex;
{
	register struct user *up;
	register extent *ex;
	register int i;
	register int pos;
	register int newpos;
	register int len;
	register int newposbb;
	register struct efs_inode *iip;
	int roff;
	int sz;
	int rem;
	int lbpos;
	struct bmapval wex;
	short justgrew;
	int j;

	ASSERT(ip->i_ftype==IFREG || ip->i_ftype==IFDIR || ip->i_ftype==IFLNK);
	iip = efs_fsptr(ip);
	ASSERT(iip->ii_com.ci_magic == EFS_IMAGIC);
	rex->length = 0;
	up = &u;
	up->u_pbdev = ip->i_dev;

	/*
	 * newpos is the logical block position of the users i/o request.
	 * newposbb is the bb position of the users i/o request. 
	 */
	newpos = BTOLBT(up->u_offset);
	newposbb = BTOBBT(up->u_offset);

start_over:
	justgrew = 0;
	/*
	 * Do a binary search through the extents based on the file size
	 * and the extent offsets.
	 * XXX Using holes in files will mess this code up something fierce
	 */
	if (iip->ii_numextents) {
		ex = &iip->ii_extents[iip->ii_numextents - 1];
		if (newposbb < ex->ex_offset + ex->ex_length) {
			j = iip->ii_numextents >> 1;
			i = j;
			for (;;) {
				ex = &iip->ii_extents[i];
				pos = ex->ex_offset;
				len = ex->ex_length;
				/*
				 * See if the extent overlaps the start of the
				 * request.  If so, use the extent, otherwise
				 * keep looping.
				 */
				if ((pos <= newposbb) && (pos + len > newposbb))
					goto gotit;
				/*
				 * Figure out next spot to look.
				 */
				if ((j >>= 1) == 0)
					j = 1;		/* make sure we move */ 
				if (pos < newposbb)
					i += j;		/* advance */
				else
					i -= j;		/* retreat */
				if ((i < 0) || (i >= iip->ii_numextents))
					break;
			}
		} else {
			pos = ex->ex_offset;
			len = ex->ex_length;
		}
	} else {
		pos = 0;
		len = 0;
	}
	/*
	 * Unable to find the offset into the file.  If reading, then
	 * EOF has been reached.
	 */
	if (rw == B_READ) {
		up->u_pbsize = 0;
		iex->length = 0;
		return (0);
	}
	/*
	 * Inode is being written, and needs to grow.  First, just
	 * try to extend the last extent in the file.  If this works,
	 * try to find our location in the file again...Compute in
	 * rem the number of bb's to allocate to extend the file out
	 * to the end of the current logical block.
	 */
	rem = efs_bbsperlb - (newposbb & (BBSPERLB - 1));
	if (iip->ii_numextents && efs_extend(ip, rem, &wex)) {
		ex = &iip->ii_extents[iip->ii_numextents - 1];
		ip->i_flag |= IUPD | ICHG | IEXT;
		iip->ii_flags |= II_TRUNC;
	} else {
		/*
		 * Couldn't extend the last extent.  Need to allocate a
		 * new extent to hold the new allocation.  If this fails,
		 * give up the ghost.
		 */
		if (i = efs_expand(ip)) {
			return (i);
		}
		iip = efs_fsptr(ip);

		/* note that things changed */
		ip->i_flag |= IUPD | ICHG | IEXT;

		/*
		 * We now have room for the new extent.  Allocate one.
		 */
		ex = &iip->ii_extents[iip->ii_numextents];
		if (efs_alloc(ip, ex, rem, 0, &wex) == 0) {
			efs_error(getfs(ip->i_mntdev), 0);
			return (ENOSPC);
		}
		iip->ii_numextents++;
		iip->ii_flags |= II_TRUNC;
		ex->ex_offset = pos + len;
		ex->ex_magic = 0;
	}

	/*
	 * If the piece of extent we found doesn't overlap the region of
	 * the file we are interested in, clear out the piece we did
	 * allocate (insuring zeros for un-written pieces) and start
	 * searching the extent list over again.
	 * XXX This is in lieu of correctly supporting holes
	 */
	justgrew = 1;
	pos = ex->ex_offset;
	len = ex->ex_length;
	if (! ((pos <= newposbb) && (pos + len > newposbb)) ) {
		struct buf *bp;

		bp = getblk(ip->i_dev, wex.bn, (int)wex.length);
		clrbuf(bp);
		bdwrite(bp);
		goto start_over;
	}

	/*
	 * Found extent overlapping u_offset.  Procede to figure out
	 * what kind of i/o to do.
	 */
gotit:
	/*
	 * lbpos is the logical block position of the extent,
	 * rounded up.  If lbpos <= newpos, then this
	 * extent contains the beginning of the logical block
	 * we are interested in doing i/o on.  If not, then
	 * the previous extent contains the beginning piece
	 * of the logical block we are interested in.  Since
	 * we are not manipulating the previous extent, the
	 * users u_offset must land somewhere in the current
	 * extent and thus the remainder of the logical block
	 * should be read/written from the current extent.
	 * GASP.
	 */
	lbpos = BBTOLB(pos);
	if (lbpos <= newpos) {
		roff = LBTOBB(newpos) - pos;
		len -= roff;
		pos += roff;
		if (len > efs_bbsperlb)
			len = efs_bbsperlb;
	} else {
		roff = 0;
		sz = LBTOBB(lbpos) - pos;
		if (len > sz)
			len = sz;
	}
	iex->bn = ex->ex_bn + roff;
	iex->length = len;
	ASSERT(len > 0);
	/*
	 * If the user is reading, and it looks like
	 * the read is sequential from the previous
	 * read, grab another chunk out of the extent
	 * of (hopefully) the same length as the
	 * previous extent.
	 */
	if ((rw == B_READ) &&
	    (iip->ii_com.ci_lastbn + iip->ii_com.ci_lastlen == newposbb)) {
		newposbb += len;
		roff = newposbb - ex->ex_offset;
		len = ex->ex_length - roff;
		if (len > 0) {
			if (len > efs_bbsperlb)
				len = efs_bbsperlb;
			rex->bn = iex->bn + iex->length;
			rex->length = len;
		}
	}
	/*
	 * Now that we have figured out which extent
	 * we are in, figure out where we will begin
	 * i/o, given the size of the extent.  Then,
	 * compute the offset into the extent as
	 * well as the number of bytes to read.
	 * XXX this can be pushed up
	 */
	up->u_pboff = up->u_offset - BBTOB(pos);
	sz = BBTOB(iex->length) - up->u_pboff;
	if (up->u_count < sz)
		sz = up->u_count;
	if (rw == B_READ) {
		rem = ip->i_size - up->u_offset;
		if (rem < 0)
			rem = 0;
		if (rem < sz)
			sz = rem;
		iip->ii_com.ci_lastbn = BTOBBT(up->u_offset);
		iip->ii_com.ci_lastlen = BTOBB(sz);
		if ((up->u_pbsize = sz) == 0) {
			iex->length = 0;
			return (0);
		}
	} else {
		/*
		 * If we just grew the file, check and see if write request
		 * exactly overlaps the newly allocated area.  If so, then
		 * we can avoid clearing the buffer as lfs_rdwr will do a
		 * getblk and overwrite the entire buffer.
		 */
		if (justgrew) {
			if ((wex.bn != iex->bn) ||
			    (wex.length != iex->length) || up->u_pboff ||
			    (sz != BBTOB(iex->length))) {
				struct buf *bp;

				bp = getblk(ip->i_dev, wex.bn, (int)wex.length);
				clrbuf(bp);
				bdwrite(bp);
			}
		}
		up->u_pbsize = sz;
	}
	return (0);
}

/*
 * efs_expand:
 *	- expand the inode
 *	- if the number of extents is going to overflow the direct
 *	  extents, allocate an indirect extent
 *	- if the indirect extent is going to overflow its region,
 *	  grow the indirect region (if possible) otherwise, throw out
 *	  the old indirect region and allocate a new one
 *	XXX improve indirect extent handling, reduce indentation level
 */
efs_expand(ip)
	register struct inode *ip;
{
	register struct efs_inode *iip;
	register int i;
	register int nb;
	register int numindirs;
	register int numextents;
	extent nindir;

	iip = efs_fsptr(ip);
	ASSERT(iip->ii_com.ci_magic == EFS_IMAGIC);
	numindirs = iip->ii_numindirs;
	numextents = iip->ii_numextents;
	if (numextents >= EFS_MAXEXTENTS)
		return (EFBIG);
	nb = (numextents + 1) * sizeof(extent);
	if (numextents == EFS_DIRECTEXTENTS) {
		/*
		 * Inode is about to start using indirect extents.
		 * Allocate disk space for the extents.  We ASSUME here
		 * that the number of bytes to hold EFS_DIRECTEXTENTS+1
		 * is <= BBSIZE.
		 */
		ASSERT(iip->ii_numindirs == 0 && iip->ii_indir == NULL);
		if ((iip->ii_indir =
		    (extent *) calloc(1, sizeof(extent))) == NULL) {
			return (EFBIG);
		}
		if (efs_alloc(ip, &iip->ii_indir[0], 1, 1,
		    (struct bmapval *)NULL) == 0) {
			free((char *) iip->ii_indir);
			iip->ii_indir = NULL;
			goto no_space;
		}
		ASSERT(iip->ii_indir[0].ex_length == 1);
		iip->ii_indirbytes = BBSIZE;
		iip->ii_numindirs = 1;
	} else
	if ((numextents > EFS_DIRECTEXTENTS) && (iip->ii_indirbytes < nb)) {
	    /*
	     * The indirect area needs to grow.  Attempt to
	     * extend the last indirect extent.  If that fails, allocate
	     * a new one to cover the entire file.  If that fails,
	     * allocate a new indirect extent.  If that fails, we wash
	     * our hands of expanding the file.
	     */
	    if (efs_grow(ip, &iip->ii_indir[numindirs-1], 1,
		  (struct bmapval *)NULL) == 0) {
		/*
		 * Try to allocate an indirect extent that covers the
		 * entire file.
		 */
		if (efs_alloc(ip, &nindir, BTOBB(nb), 1,
		      (struct bmapval *)NULL)) {
		    ASSERT(BTOBB(nb) == nindir.ex_length);
		    /*
		     * We now have an extent which can map the entire
		     * file.  Throw away the old indirect info.
		     */
		    for (i = 0; i < numindirs; i++)
			efs_free(ip, &iip->ii_indir[i]);

		    /*
		     * Shrink the indirect extent vector down to one
		     * extent using realloc.
		     */
		    iip->ii_indir =
			(extent *) realloc((char *) iip->ii_indir,
			    sizeof(extent));
		    nindir.ex_magic = 0;		/* force to zero */
		    nindir.ex_offset = 0;
		    iip->ii_indir[0] = nindir;
		    iip->ii_indirbytes = BBTOB(nindir.ex_length);
		    iip->ii_numindirs = 1;
		} else {
		    /*
		     * We couldn't grow the indirect extent, nor could
		     * we allocate a replacement indirect extent.
		     * Try to allocate another indirect extent.
		     */
		    if (numindirs == EFS_DIRECTEXTENTS) {
			struct mount *mp;

			/*
			 * No more room in inode for indirect extents.
			 */
			mp = ip->i_mntdev;
			prdev("out of indirect extent space", mp->m_dev);
			return (ENOSPC);
		    }
		    /*
		     * Grow the indirect extent vector by one extent.
		     */
		    iip->ii_indir =
			(extent *) realloc((char *) iip->ii_indir,
			    (numindirs+1) * sizeof(extent));
		    if (iip->ii_indir == NULL)
			return (EFBIG);

		    if (efs_alloc(ip, &iip->ii_indir[numindirs], 1, 1,
			  (struct bmapval *)NULL) == 0) {
			iip->ii_indir =
			    (extent *) realloc((char *) iip->ii_indir,
				numindirs * sizeof(extent));
			goto no_space;
		    }
		    ASSERT(iip->ii_indir[numindirs].ex_length == 1);
		    iip->ii_numindirs++;
		    iip->ii_indirbytes += BBSIZE;
		}
	    } else {
		iip->ii_indirbytes += BBSIZE;
	    }
	}

	/*
	 * Allocate space for the new incore information.  Use realloc to
	 * avoid a copy, if it can.  The number of bytes needed for ip's
	 * expanded extents is in nb, so we subtract off the size of the
	 * extent array's first element in struct efs_inode.
	 */
	ip->i_fsptr =
	    realloc(ip->i_fsptr, sizeof(struct efs_inode) + nb
		- sizeof(extent));
	if (ip->i_fsptr == NULL)
		return (EFBIG);
	return (0);

no_space:
	efs_error(getfs(ip->i_mntdev), 0);
	return (ENOSPC);
}

#endif
