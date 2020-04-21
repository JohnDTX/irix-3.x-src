/*
 * $Source: /d2/3.7/src/sys/bfs/RCS/bfs_bmap.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:26:16 $
 */
#include "bfs.h"
#if NBFS > 0

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/signal.h"
#include "../h/user.h"
#include "../h/buf.h"
#include "../h/fstyp.h"
#include "../bfs/bfs_inode.h"

/*
 * Bmap defines the structure of bell file system storage
 * by returning the physical block number on a device given the
 * inode and the logical block number in a file.
 *
 * When convenient, it also leaves the physical
 * block number of the next block of the file in rex
 * for use in read-ahead.
 */
int
bell_bmap(ip, readflg, ex, rex)
	register struct inode *ip;
	int readflg;
	struct bmapval *ex, *rex;
{
	register struct user *up;
	register int i;
	register dev_t dev;
	register struct bell_inode *bip;
	register daddr_t bn;
	daddr_t nb;
	daddr_t *bap;
	register int raflag;

	up = &u;
	bip = bell_fsptr(ip);
	raflag = 0;
	rex->length = 0;
	{
		register int sz, rem;

		up->u_pbdev = dev = ip->i_dev;
		bn = FsBNO(dev, up->u_offset);
		if (bn < 0)
			return (EFBIG);
		/*
		 * Attempt to detect sequential i/o.  If the last i/o
		 * logical block # we read plus the last length we read
		 * matches the new logical block number, read-ahead
		 * should probably be done.
		 */
		if ((readflg == B_READ) &&
		    (bip->bi_com.ci_lastbn + bip->bi_com.ci_lastlen == bn)) {
			raflag++;
		}
		up->u_pboff = FsBOFF(dev, up->u_offset);
		sz = FsBSIZE(dev) - up->u_pboff;
		if (up->u_count < sz) {
			sz = up->u_count;
			raflag = 0;
		}
		up->u_pbsize = sz;
		/*
		 * Record current logical file position so that next
		 * request can check for read-ahead.
		 */
		bip->bi_com.ci_lastbn = bn;		/* logical block # */
		bip->bi_com.ci_lastlen = 1;		/* 1 logical block */
		if (readflg == B_READ) {
			rem = ip->i_size - up->u_offset;
			if (rem < 0)
				rem = 0;
			if (rem < sz)
				sz = rem;
			if ((up->u_pbsize = sz) == 0) {
				ex->length = 0;
				return (0);
			}
		}
	}

	{
	register struct buf *bp;
	register j, sh;


	/*
	 * blocks 0..NADDR-4 are direct blocks
	 */
	if (bn < NADDR-3) {
		i = bn;
		nb = bip->bi_addr[i];
		if (nb == 0) {
			if (readflg == B_READ) {
				ex->bn = -1;
				return (0);
			}
			if ((bp = bell_alloc(ip, &nb)) == NULL)
				return (up->u_error);
			bdwrite(bp);
			bip->bi_addr[i] = nb;
			ip->i_flag |= IUPD | ICHG;
		}
		if (raflag && (i < NADDR-4)) {
			rex->bn = FsLTOP(dev, bip->bi_addr[i + 1]);
			rex->length = BTOBB(FsBSIZE(dev));
		}
		goto out;
	}

	/*
	 * addresses NADDR-3, NADDR-2, and NADDR-1
	 * have single, double, triple indirect blocks.
	 * the first step is to determine
	 * how many levels of indirection.
	 */
	sh = 0;
	nb = 1;
	bn -= NADDR-3;
	for(j=3; j>0; j--) {
		sh += FsNSHIFT(dev);
		nb <<= FsNSHIFT(dev);
		if (bn < nb)
			break;
		bn -= nb;
	}
	if (j == 0)
		return (EFBIG);

	/*
	 * fetch the address from the inode
	 */
	nb = bip->bi_addr[NADDR - j];
	if (nb == 0) {
		if (readflg == B_READ) {
			ex->bn = -1;
			return (0);
		}
		if ((bp = bell_alloc(ip, &nb)) == NULL)
			return (up->u_error);
		bwrite(bp);
		bip->bi_addr[NADDR - j] = nb;
		ip->i_flag |= IUPD | ICHG;
	}

	/*
	 * fetch through the indirect blocks
	 */
	for(; j<=3; j++) {
		bp = bread(dev, FsLTOP(dev, nb), BTOBB(FsBSIZE(dev)));
		if (up->u_error) {
			brelse(bp);
			return (up->u_error);
		}
		bap = bp->b_un.b_daddr;
		sh -= FsNSHIFT(dev);
		i = (bn>>sh) & FsNMASK(dev);
		nb = bap[i];
		if (nb == 0) {
			register struct buf *nbp;

			if (readflg == B_READ) {
				ex->bn = -1;
				return (0);
			}
			if ((nbp = bell_alloc(ip, &nb)) == NULL) {
				brelse(bp);
				return (up->u_error);
			}
			if (j < 3)
				bwrite(nbp);
			else
				bdwrite(nbp);
			bap[i] = nb;
			bdwrite(bp);
		} else
			brelse(bp);
	}

	/*
	 * calculate read-ahead.
	 */
	if (raflag && (i < FsNINDIR(dev) -1)) {
		rex->bn = FsLTOP(dev, bap[i + 1]);
		rex->length = BTOBB(FsBSIZE(dev));
	}
	}

	/*
	 * Found the block #.  Return it and the
	 * length of the i/o to do to get it.
	 */
out:
	ex->bn = FsLTOP(dev, nb);
	ex->length = BTOBB(FsBSIZE(dev));
	return (0);
}

#endif
