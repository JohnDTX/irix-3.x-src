/*	vm_swp.c	6.2	83/09/09	*/

#include "machine/pte.h"

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/buf.h"
#include "../h/conf.h"
#include "../h/proc.h"
#include "../vm/vm.h"
#include "../h/map.h"
#include "../h/sysinfo.h"

/*
 * Swap IO headers -
 * They contain the necessary information for the swap I/O.
 * At any given time, a swap header can be in three
 * different lists. When free it is in the free list, 
 * when allocated and the I/O queued, it is on the swap 
 * device list, and finally, if the operation was a dirty 
 * page push, when the I/O completes, it is inserted 
 * in a list of cleaned pages to be processed by the pageout daemon.
 */
struct	buf *swbuf;

/*
 * getswbuf:
 *	- get a swap buffer for doing raw i/o on
 */
struct buf *
getswbuf()
{
	register struct buf *bp;
	int s;

	s = spl6();
	while (bswlist.av_forw == NULL) {
		bswlist.b_flags |= B_WANTED;
		sleep((caddr_t)&bswlist, PSWP+1);
	}
	bp = bswlist.av_forw;
	bswlist.av_forw = bp->av_forw;
	splx(s);
	return (bp);
}

/*
 * putswbuf:
 *	- put a swap buffer back into the free list
 *	- wakeup processes waiting for a buffer
 */
putswbuf(bp)
	register struct buf *bp;
{
	int s;

	s = spl6();
	bp->b_flags &= ~(B_BUSY|B_WANTED|B_PHYS|B_PAGET|B_UAREA|B_DIRTY);
	bp->av_forw = bswlist.av_forw;
	bswlist.av_forw = bp;
	if (bswlist.b_flags & B_WANTED) {
		bswlist.b_flags &= ~B_WANTED;
		wakeup((caddr_t)&bswlist);
		if (pager_sleeping)
			wakeup((caddr_t)&proc[2]);
	}
	splx(s);
}

/*
 * swap I/O -
 *
 * If the flag indicates a dirty page push initiated
 * by the pageout daemon, we map the page into the i th
 * virtual page of process 2 (the daemon itself) where i is
 * the index of the swap header that has been allocated.
 * We simply initialize the header and queue the I/O but
 * do not wait for completion. When the I/O completes,
 * iodone() will link the header to a list of cleaned
 * pages to be processed by the pageout daemon.
 */
swap(p, dblkno, addr, nbytes, rdflg, flag, dev, pfcent)
	struct proc *p;
	swblk_t dblkno;
	caddr_t addr;
	int nbytes, rdflg, flag;
	dev_t dev;
	u_int pfcent;
{
	register struct buf *bp;
	register u_int c;
	int p2dp;
	register struct pte *dpte, *vpte;
	extern swdone();

#ifdef	OS_DEBUG
	if ((dev == rootdev) ||
	    ((major(dev) == major(rootdev)) &&
	     (minor(dev) == minor(rootdev)))) {
		printf("swap: dev=%x == rootdev!\n", dev);
		panic("swap");
	}
#endif
	bp = getswbuf();
	bp->b_flags = B_BUSY | B_PHYS | rdflg | flag;
	if ((bp->b_flags & (B_DIRTY|B_PGIN)) == 0)
		if (rdflg == B_READ)
			sum.v_pswpin += btoc(nbytes);
		else
			sum.v_pswpout += btoc(nbytes);
	bp->b_proc = p;
	if (flag & B_DIRTY) {
		p2dp = (bp - swbuf) * KLMAX;
#ifdef	OS_DEBUG
		if ((p2dp < 0) || (p2dp + btop(nbytes) > proc[2].p_dsize)) {
			printf("swap: out of range of page table\n");
			printf("bp=%x swbuf=%x p2dp=%d nbytes=%d\n",
				      bp, swbuf, p2dp, nbytes);
			panic("swap");
		}
#endif
		dpte = dptopte(&proc[2], p2dp);
		vpte = vtopte(p, btop(addr));
		for (c = 0; c < nbytes; c += NBPG) {
			if (vpte->pg_pfnum == 0 || vpte->pg_fod)
				panic("swap bad pte");
			*dpte++ = *vpte++;
		}
		bp->b_un.b_addr = (caddr_t)ctob(dptov(&proc[2], p2dp));
		bp->b_flags |= B_CALL;
		bp->b_pfcent = pfcent;
	} else
		bp->b_un.b_addr = addr;

	if (rdflg) {
		sysinfo.swapin++;
		sysinfo.bswapin += btoc(nbytes);
	} else {
		sysinfo.swapout++;
		sysinfo.bswapout += btoc(nbytes);
	}
	while (nbytes > 0) {
		bp->b_bcount = nbytes;
		minphys(bp);
		c = bp->b_bcount;
		bp->b_blkno = dblkno;
		bp->b_dev = dev;
		physstrat(bp, bdevsw[major(dev)].d_strategy, PSWP);
		if (flag & B_DIRTY) {
			if (c < nbytes)
				panic("big push");
			return;
		}
		bp->b_un.b_addr += c;
		bp->b_flags &= ~B_DONE;
		if (bp->b_flags & B_ERROR) {
			if ((flag & (B_UAREA|B_PAGET)) || rdflg == B_WRITE)
				panic("hard IO err in swap");
			swkill(p, (char *)0);
		}
		nbytes -= c;
		dblkno += BTOBB(c);
	}
	putswbuf(bp);
}

/*
 * Put a buffer on the clean list after I/O is done.
 * Called from biodone.
 */
swdone(bp)
	register struct buf *bp;
{
	register int s;

	if (bp->b_flags & B_ERROR)
		panic("IO err in push");
	s = spl6();
	bp->av_forw = bclnlist;
	bclnlist = bp;
	cnt.v_pgout++;
	cnt.v_pgpgout += bp->b_bcount / NBPG;
	/*
	 * Always wakeup the pager here, because it needs to run to
	 * clean pages that are done.
	 */
	if ((bswlist.b_flags & B_WANTED) || pager_sleeping)
		wakeup((caddr_t)&proc[2]);
	splx(s);
}

/*
 * If rout == 0 then killed on swap error, else
 * rout is the name of the routine where we ran out of
 * swap space.
 */
swkill(p, rout)
	struct proc *p;
	char *rout;
{
	char *mesg;

	printf("pid %d: ", p->p_pid);
	if (rout)
		printf(mesg = "killed due to no swap space\n");
	else
		printf(mesg = "killed on swap error\n");
	uprintf("sorry, pid %d was %s", p->p_pid, mesg);
	/*
	 * To be sure no looping (e.g. in vmsched trying to
	 * swap out) mark process locked in core (as though
	 * done by user) after killing it so noone will try
	 * to swap it out.
	 */
	psignal(p, SIGKILL);
	p->p_flag |= SULOCK;
}

/*
 * Raw I/O. The arguments are
 *	The strategy routine for the device
 *	A buffer, which will always be a special buffer
 *	  header owned exclusively by the device for this purpose
 *	The device number
 *	Read/write flag
 * Essentially all the work is computing physical addresses and
 * validating them.
 * If the user has the proper access privilidges, the process is
 * marked 'delayed unlock' and the pages involved in the I/O are
 * faulted and locked. After the completion of the I/O, the pages
 * are unlocked.
 * If the driver passes in a null "bp", then the system allocates
 * a swap buffer to do the i/o with.
 */
physio(strat, bp, dev, rw, mincnt)
	int (*strat)(); 
	register struct buf *bp;
	dev_t dev;
	int rw;
	int (*mincnt)();
{
	register int c;
	register caddr_t a;
	register int s;
	register caddr_t limit;
	register caddr_t dsstart, dsend;
	int using_swbuf;

	if (u.u_count == 0)
		return;

	/* make sure users address range is ok */
	a = u.u_base;
	dsstart = (caddr_t)u.u_loadaddr + ctob(u.u_tsize);
	dsend = dsstart + ctob(u.u_dsize);
	limit = a + u.u_count - 1;

	/* check for address wrap around */
	if (a >= a + u.u_count)
		goto bad;

	/*
	 * Check that transfer is either entirely in the data region,
	 * entirely in the stack region, or is writing the text region
	 */
	if (!(((a >= dsstart) && (limit < dsend)) ||
	      ((a >= (caddr_t)USRSTACK - ctob(u.u_ssize)) &&
	       (limit < (caddr_t)USRSTACK)) ||
	      ((rw != B_READ) && (a >= (caddr_t)u.u_loadaddr) &&
	       (limit < (caddr_t)u.u_loadaddr + ctob(u.u_tsize))))) {
bad:
		u.u_error = EFAULT;
		return;
	}

	syswait.physio++;
	if ( rw )
		sysinfo.phread++;
	else
		sysinfo.phwrite++;

	using_swbuf = (bp == NULL);
	if (using_swbuf)
		bp = getswbuf();
	else {
		s = spl6();
		while (bp->b_flags&B_BUSY) {
			bp->b_flags |= B_WANTED;
			sleep((caddr_t)bp, PRIBIO+1);
		}
		splx(s);
	}

	bp->b_proc = u.u_procp;
	while (u.u_count > 0) {
		a = u.u_base;
		bp->b_un.b_addr = a;		/* for physstrat() */
		bp->b_error = 0;
		bp->b_resid = 0;
		bp->b_flags = B_BUSY | B_PHYS | rw;
		bp->b_dev = dev;
		bp->b_blkno = BTOBBT(u.u_offset);
		bp->b_bcount = u.u_count;
		(*mincnt)(bp);
		c = bp->b_bcount;
		u.u_procp->p_flag |= SPHYSIO;
		vslock(a, c);
		physstrat(bp, strat, PRIBIO);
		vsunlock(a, c, rw);
		u.u_procp->p_flag &= ~SPHYSIO;
		c -= bp->b_resid;
		u.u_base += c;
		u.u_count -= c;
		u.u_offset += c;
		/*
		 * If an error occured, or the i/o requested was truncated
		 * to zero, or if the i/o request was truncated and the
		 * device was a tape, then bust out of the loop.  We
		 * bust out of the loop for tapes because we want to be
		 * able to ask the tape for a record larger than is actually
		 * written on the tape, and get back how much i/o WAS done.
		 */
		if (bp->b_flags & B_ERROR) {
			if ((u.u_error = bp->b_error) == 0)
				u.u_error = EIO;
			break;
		}
		if ((c <= 0) || ((rw & B_TAPE) && bp->b_resid))
			break;
	}
	syswait.physio--;

	/* wakeup process(s) sleeping on the buffer we are using */
	if (using_swbuf)
		putswbuf(bp);
	else {
		s = spl6();
		if (bp->b_flags & B_WANTED)
			wakeup((caddr_t)bp);
		bp->b_flags &= ~(B_BUSY|B_WANTED|B_PHYS);
		splx(s);
	}
}

int
minphys(bp)
	struct buf *bp;
{

	if (bp->b_bcount > MAXPHYS)
		bp->b_bcount = MAXPHYS;
}

/*
 * physck:
 *	- used by callers of physio() to truncate users i/o request to
 *	  fit within a given filesystem
 */
physck(nblocks, rw, blkshift)
	daddr_t nblocks;
	int rw;
	int blkshift;
{
	register unsigned over;
	register off_t upper, limit;
	struct a {
		int	fdes;
		char	*cbuf;
		unsigned count;
	} *uap;

	limit = nblocks << blkshift;
	if (u.u_offset >= limit) {
		if (u.u_offset > limit || rw == B_WRITE)
			u.u_error = ENXIO;
		return(0);
	}
	upper = u.u_offset + u.u_count;
	if (upper > limit) {
		over = upper - limit;
		u.u_count -= over;
		uap = (struct a *)u.u_ap;
		uap->count -= over;
	}
	return(1);
}
