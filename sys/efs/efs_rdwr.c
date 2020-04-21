/*
 * Read and write - efs filesystem switch implementation.
 *
 * $Source: /d2/3.7/src/sys/efs/RCS/efs_rdwr.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:28:00 $
 */
#include "efs.h"
#include "efs2.h"
#if NEFS > 0 || NEFS2 > 0

#ifndef att
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/user.h"
#include "../h/file.h"
#include "../h/buf.h"
#include "../h/flock.h"
#include "../h/conf.h"
#include "../h/fstyp.h"
#include "../h/inode.h"
#else
# include "sys/types.h"
# include "sys/sysmacros.h"
# include "sys/param.h"
# include "sys/systm.h"
# include "sys/signal.h"
# include "sys/errno.h"
# include "sys/psw.h"
# include "sys/pcb.h"
# include "sys/user.h"
# include "sys/buf.h"
# include "sys/cmn_err.h"
# include "sys/flock.h"
# include "sys/conf.h"
# include "sys/fstyp.h"
# include "sys/inode.h"
# include "sys/fs/bmap.h"
#endif

int	efs_bmap();

/*
 * Read the file corresponding to the inode pointed at by the argument.
 * The actual read arguments are found in the variables:
 *	u_base		core address for destination
 *	u_offset	byte offset in file
 *	u_count		number of bytes to read
 *	u_segflg	read to kernel/user/user I
 */
efs_readi(ip)
	register struct inode *ip;
{
	register struct user *up;
	register dev_t dev;

#ifndef att
	if (ip->i_ftype == IFREG
	    && (istatf_fsptr(ip)->i_mode & (ISGID|(IEXEC>>3))) == ISGID) {
#else
	if (efs_access(ip, IMNDLCK) == 0) {
#endif
		/* enforce record locking protocol */
		efs_chklock(ip, FREAD);
		if (u.u_error != 0)
			return;
	}
	up = &u;
	if (up->u_count == 0)
		return;

	/*
	 * Switch out to file type dependent code
	 */
	switch (ip->i_ftype) {
	  case IFCHR:
		dev = ip->i_rdev;
		ip->i_flag |= IACC;
		if (cdevsw[major(dev)].d_str != NULL)
			strread(ip);	/* stream device read */
		else
			(*cdevsw[major(dev)].d_read)(dev);
		break;
	  case IFIFO:
		(*fstypsw[pipefstyp].fs_readi)(ip);
		break;
	  case IFREG:
	  case IFDIR:
	  case IFLNK:
		if (up->u_offset < 0) {
			up->u_error = EINVAL;
			break;
		}
		/* FALL THROUGH */
	  case IFBLK:
#ifndef att
		up->u_error = fs_rdwr(ip, B_READ, efs_bmap);
#else
		up->u_error = efs_rdwr(ip, B_READ, efs_bmap);
#endif
		break;
	  default:
		up->u_error = ENODEV;
		break;
	}
}

/*
 * Write the file corresponding to the inode pointed at by the argument.
 * The actual write arguments are found in the variables:
 *	u_base		core address for source
 *	u_offset	byte offset in file
 *	u_count		number of bytes to write
 *	u_segflg	write to kernel/user/user I
 */
efs_writei(ip)
	register struct inode *ip;
{
	register struct user *up;
	register dev_t dev;

#ifndef att
	if (ip->i_ftype == IFREG
	    && (istatf_fsptr(ip)->i_mode & (ISGID|(IEXEC>>3))) == ISGID) {
#else
	if (efs_access(ip, IMNDLCK) == 0) {
#endif
		/* enforce record locking protocol */
		efs_chklock(ip, FWRITE);
		if (u.u_error != 0)
			return;
	}
	up = &u;
	if (up->u_count == 0)
		return;

	/*
	 * Switch out to file type dependent code
	 */
	switch (ip->i_ftype) {
	  case IFCHR:
		dev = ip->i_rdev;
		ip->i_flag |= IACC;
		if (cdevsw[major(dev)].d_str != NULL)
			strwrite(ip);	/* stream device write */
		else
			(*cdevsw[major(dev)].d_write)(dev);
		break;
	  case IFIFO:
		(*fstypsw[pipefstyp].fs_writei)(ip);
		break;
	  case IFREG:
	  case IFDIR:
	  case IFLNK:
		if (up->u_offset < 0) {
			up->u_error = EINVAL;
			break;
		}
		/* FALL THROUGH */
	  case IFBLK:
#ifndef att
		up->u_error = fs_rdwr(ip, B_WRITE, efs_bmap);
#else
		up->u_error = efs_rdwr(ip, B_WRITE, efs_bmap);
#endif
		break;
	  default:
		up->u_error = ENODEV;
		break;
	}
}

/*
 * Common code for lock enforcement during read and write
 */
efs_chklock(ip, mode)
	register struct inode *ip;
	register int mode;
{
	register int i;
	struct flock bf;

	bf.l_type = (mode & FWRITE) ? F_WRLCK : F_RDLCK;
	bf.l_whence = 0;
	bf.l_start = u.u_offset;
	bf.l_len = u.u_count;
	i = (u.u_fmode & FNDELAY) ? INOFLCK : SLPFLCK|INOFLCK;
	if ((i = reclock(ip, &bf, i, 0, u.u_offset)) || bf.l_type != F_UNLCK)
		u.u_error = (i == 0) ? EAGAIN : i;
}

#ifdef att
/*
 * efs_rdwr:
 *	- local fs read/write, for both efs and sgi bell filesystems
 *	- call fs type dependent bmap to map blocks
 */
int
efs_rdwr(ip, rw, bmap)
	register struct inode *ip;
	int rw;
	int (*bmap)();
{
	register struct user *up;
	register int off, n;
	register int len;
	register daddr_t bn;
	register daddr_t rabn;
	register int ralen;
	register struct buf *bp;
	register dev_t dev;
	register int type;
	struct bmapval ex, rex;

	up = &u;
	/* XXX clean this offset < 0 stuff up */
	if ((rw == B_READ) && (up->u_offset < 0))
		return (EINVAL);

	type = ip->i_ftype;
	do {
		/*
		 * Insure that users write request will not grow the
		 * inode too large.
		 */
		if ((rw == B_WRITE) && (BTOBB(up->u_offset) >= up->u_limit))
			return (EFBIG);
		/*
		 * Call fs type dependent bmap to convert users seek
		 * position into a block,length pair.
		 * XXX get rid of side-effect stuff in user struct.  Have
		 * XXX bmap fill in passed structure with offset (etc).
		 * XXX can speed up fod_pagein if we do
		 * XXX move pboff stuff to here!
		 */
		ex.length = 0;
		rex.length = 0;
		if (type == IFBLK) {
			/*
			 * Convert users i/o offset into a block dev logical
			 * block #, then convert that into a physical block
			 * number.
			 */
			bn = FsLTOP(dev, BLKDEV_LBN(up->u_offset));
			ex.length = len = BLKDEV_BB;
			rabn = bn + len;
			off = BLKDEV_OFF(up->u_offset);
			dev = ip->i_rdev;
			n = BLKDEV_IOSIZE - off;
			/*
			 * Trim number of bytes to be moved to fit within the
			 * users request.  Turn off read-ahead if we do this
			 * as the next user read will probably want the same
			 * block.
			 */
			if (up->u_count < n) {
				n = up->u_count;
				ralen = 0;
			} else
				ralen = BLKDEV_BB;
			/*
			 * See if read ahead can be triggered by checking the
			 * last i/o request and see if this request lines up
			 * with it.
			 */
			if (ralen && ((rw != B_READ) ||
				      (ip->i_lastbn + ip->i_lastlen != bn)))
				ralen = 0;
			ip->i_lastbn = bn;
			ip->i_lastlen = len;
		} else {
			/*
			 * Map users logical file position into a physical
			 * block number and length pair.  If an error
			 * occurs, or end-of-file is reached, bust out
			 * of the loop.
			 */
			up->u_error = (*bmap)(ip, rw, &ex, &rex);
			if (up->u_error || (ex.length == 0))
				break;
			bn = ex.bn;
			len = ex.length;
			off = up->u_pboff;
			dev = up->u_pbdev;
			n = up->u_pbsize;
			rabn = rex.bn;
			ralen = rex.length;
		}

		/*
		 * Get a buffer to read/write the data from/to
		 */
		if (rw == B_READ) {
			/*
			 * If bn is less than zero, then i/o request
			 * is inside a hole in a the file.
			 */
			if (bn < 0) {
				bp = geteblk(len);
				clrbuf(bp);
			} else
			if (rabn && ralen)
				bp = breada(dev, bn, len, rabn, ralen);
			else
				bp = bread(dev, bn, len);
			/*
			 * XXX
			 * if the request was truncated,
			 * stop at the previous logical block.
			 */
			if (bp->b_resid)
				n = 0;
		} else {
			/*
			 * If write request exactly overlaps a block, don't
			 * bother reading it.  Just get a buffer to hold
			 * the new data.
			 */
			if ((off == 0) && (n == BBTOB(ex.length)))
				bp = getblk(dev, bn, len);
			else
				bp = bread(dev, bn, len);
			/*
			 * XXX
			 * if the request was truncated,
			 * report an error.
			 */
			if (bp->b_resid)
				bp->b_flags |= B_ERROR;
		}
		/*
		 * If an error occured doing the i/o, give up
		 * right away.
		 */
		if (bp->b_flags & B_ERROR) {
			brelse(bp);
			return (EIO);			/* XXX */
		}
		iomove(bp->b_un.b_addr + off, (int)n, rw);
		if (rw == B_READ) {
			brelse(bp);
			ip->i_flag |= IACC;
		} else {
			/*
			 * XXX 4.2 checks for i/o completely filling buffer
			 * XXX and toss buffer to the head of the freelist
			 * XXX if so.  Do we want to do this?
			 */
			if ((up->u_fmode & FSYNC) || (type == IFDIR))
				bwrite(bp);
			else
				bdwrite(bp);
			/*
			 * Check and see if write extended the inode.  If so,
			 * update the inode size.  In any case, mark the
			 * inode as updated and changed.
			 */
			if ((type == IFREG) || (type == IFDIR) ||
			    (type == IFLNK)) {
				if (up->u_offset > ip->i_size)
					ip->i_size = up->u_offset;
			}
			ip->i_flag |= IUPD | ICHG;
		}
	} while ((up->u_error == 0) && up->u_count && n);
	return (up->u_error);
}
#endif

#endif
