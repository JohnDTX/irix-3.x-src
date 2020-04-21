/*
 * fs_bio.c
 */
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/user.h"
#include "../h/buf.h"
#include "../h/inode.h
#include "../h/fstyp.h"
#include "../h/mount.h"
#include "../h/iobuf.h"
#include "../h/conf.h"
#include "../h/map.h"
#include "../h/cmap.h"
#include "../vm/vm.h"
#include "../h/fs.h"
#include "../h/proc.h"			/* for lint */
#include "machine/pte.h"
#include "machine/cpureg.h"

#define	b_start		b_proc

/*
 * Compute number of pages consumed by a buffer which starts at virtual
 * address "va" of "len" bytes.
 */
#define	PAGES(va, len)	btoc((((long) (va)) & PGOFSET) + (len))

struct pte *bio_pt;
struct map *bio_memmap;

unsigned char *bio_bbcnt;
#define	BBCNT_WANTED	0x80
#define	BBCNT_WAIT	0x40
#define	BBCNT_COUNT	0x0F

struct	{
	ulong	bwrites;	/* # of bwrites ever done */
	ulong	bdsyncs;	/* total number of bdsync's ever done */
	ulong	bdsync_writes;	/* # of writes done in bdsync */
	ulong	bflushs;	/* total number of bflush's ever done */
	ulong	bflush_writes;	/* # of disk writes done with bflush */
	ulong	binvals;	/* calls to binval */
	ulong	binval_writes;	/* # of buffers written in binval */
	ulong	getblks;	/* calls to getblk */
	ulong	getblk_writes;	/* # of buffers written in get(e)blk */
	ulong	pages;		/* # of pages in use in cache */
	ulong	maxpages;	/* maximum # of pages ever used */
	ulong	bbs;		/* # of bb's in use in cache */
	ulong	maxbbs;		/* maximum # of bb's ever used */
	ulong	memmaps;	/* # of rmalloc's of bio_memmap */
	ulong	memmap_fails;	/* # of failed rmalloc's of bio_memmap */
	ulong	memmap_dups;	/* # of physical memory alloc collisions */
	ulong	kmap_pages;	/* pages of Usrptmap used */
	ulong	dma_pages;	/* pages of multibus map used */
} bio_meter;

/*
 * The following several routines allocate and free
 * buffers with various side effects.  In general the
 * arguments to an allocate routine are a device and
 * a block number, and the value is a pointer to
 * the buffer header; the buffer is marked "busy"
 * so that no one else can touch it.  If the block was
 * already in core, no I/O need be done; if it is
 * already busy, the process waits until it becomes free.
 * The following routines allocate a buffer:
 *	getblk
 *	bread
 *	breada
 * Eventually the buffer must be released, possibly with the
 * side effect of writing it out, by using one of
 *	bwrite
 *	bdwrite
 *	bawrite
 *	brelse
 */

/*
 * count and flag for outstanding async writes
 */
int basyncnt, basynwait;

/*
 * Fast access to buffers in cache by hashing.  A given block number is
 * rounded down to a larger sized block.  We loop via the length
 * of the extent over each larger block
 */
#define	bhash(d, b) \
	((struct buf *)&hbuf[(short) (((int)(d)+(int)(b)) & (NHBUF-1)) ])

#define	NHBUF		128
struct	hbuf {
	long	b_flags;
	struct	buf *b_forw;
	struct	buf *b_back;
};
struct	hbuf hbuf[NHBUF];
long	bio_bbsperbucketshift;
long	bio_bbsperbucketminusone;
long	bio_bbsperlbminusone;

/*
 * Macro to compute the number of buckets for searching for a given
 * buffer and the starting bucket to look in.  This accounts for the
 * fact that we have to look both to the left and right of the extent
 * in case of overlap from either side.  The buckets are always twice
 * the size of the efs logical block size.
 */
#define	COMPUTE_BUCKET_STUFF(blkno, len) { \
	bucketbn = (blkno); \
	if (bucketbn >= bio_bbsperlbminusone) { \
		bucketbn -= bio_bbsperlbminusone; \
		buckets = (bucketbn & bio_bbsperbucketminusone) + \
			bio_bbsperlbminusone; \
	} else \
		buckets = (blkno); \
	buckets = (buckets + (len) + bio_bbsperbucketminusone) >> \
		bio_bbsperbucketshift; \
	bucketbn >>= bio_bbsperbucketshift; \
}

#ifdef	NOTDEF
#define	COMPUTE_BUCKET_STUFF(bn, len) { \
	bucketbn = (blkno); \
	if (bucketbn >= bio_maxcachelen) { \
		bucketbn -= (bio_maxcachelen - 1); \
		search_len = (bucketbn & bio_bbsperbucketminusone) + \
				bio_maxcachelen - 1; \
	} else \
		search_len = (blkno); \
	buckets = (search_len + (len) + bio_bbsperbucketminusone) >> \
		bio_bbsperbucketshift; \
	bucketbn >>= bio_bbsperbucketshift; \
}
#endif	NOTDEF

/*
 * Unlink a buffer from the available list and mark it busy.
 * (internal interface).  notavailspl is the same as notavail, but
 * it assumes that the processor is already at spl6.
 */
#define	notavail(bp) { \
	register s; \
\
	s = spl6(); \
	bremfree(bp); \
	bp->b_flags |= B_BUSY; \
	bfreelist.b_bcount--; \
	splx(s); \
}
#define	notavailspl(bp) { \
	bremfree(bp); \
	bp->b_flags |= B_BUSY; \
	bfreelist.b_bcount--; \
}

/*
 * Pick up the device's error number and pass it to the user;
 * if there is an error but the number is 0 set a generalized code.
 */
#define	geterror(bp) {\
	if (bp->b_flags&B_ERROR)\
		if ((u.u_error = bp->b_error)==0)\
			u.u_error = EIO;\
}

/*
 * Insq/Remq for the buffer hash lists.
 */
#define	bremhash(bp) { \
	(bp)->b_back->b_forw = (bp)->b_forw; \
	(bp)->b_forw->b_back = (bp)->b_back; \
}
#define	binshash(bp, dp) { \
	(bp)->b_forw = (dp)->b_forw; \
	(bp)->b_back = (dp); \
	(dp)->b_forw->b_back = (bp); \
	(dp)->b_forw = (bp); \
}

/*
 * Insq/Remq for the buffer free lists.
 */
#define	bremfree(bp) { \
	(bp)->av_back->av_forw = (bp)->av_forw; \
	(bp)->av_forw->av_back = (bp)->av_back; \
}

/* insert at head/tail of given freelist */
#define	binsheadfree(bp) { \
	bfreelist.av_forw->av_back = (bp); \
	(bp)->av_forw = bfreelist.av_forw; \
	bfreelist.av_forw = (bp); \
	(bp)->av_back = &bfreelist; \
}
#define	binstailfree(bp) { \
	bfreelist.av_back->av_forw = (bp); \
	(bp)->av_back = bfreelist.av_back; \
	bfreelist.av_back = (bp); \
	(bp)->av_forw = &bfreelist; \
}

/*
 * This is a flag indicating when the bio cache needs bio pagetable resources
 * for allocating a buffer.
 */
static	char bio_mem_wanted;

/*
 * Initialize the buffer I/O system by freeing
 * all buffers and setting all device hash buffer lists to empty.
 * Setup the list of swap buffers also.
 */
init_bio()
{
	register struct buf *bp, *dp;
	register short i;

	dp = &bfreelist;
	dp->b_forw = dp; dp->b_back = dp;
	dp->av_forw = dp; dp->av_back = dp;
	for (i = 0, bp = buf; i < nbuf; i++, bp++) {
		bp->b_dev = NODEV;
		binshash(bp, dp);
		bp->b_flags = B_BUSY | B_INVAL;
		brelse(bp);
	}

	/* setup swap buffer headers */
	bswlist.av_forw = bp = swbuf;
	for (i = 0; i < nswbuf - 1; i++, bp++)
		bp->av_forw = bp + 1;
	bp->av_forw = NULL;

	/* initialize hash buffer headers */
	for (i = 0; i < NHBUF; i++)
		hbuf[i].b_forw = hbuf[i].b_back = (struct buf *)&hbuf[i];

	/*
	 * Setup hashing parameters.  Each disk block is scaled into a
	 * larger block, whose length is twice the length of an efs
	 * logical block.  Maximum overlap of buckets is thus reduced
	 * to about 3 (in the worst case).
	 */
	bio_bbsperbucketshift = efs_lbtobbshift + 1;
	bio_bbsperbucketminusone = (1 << bio_bbsperbucketshift) - 1;
	bio_bbsperlbminusone = efs_bbsperlb - 1;
}

/*
 * Read in (if necessary) the block and return a buffer pointer.
 */
struct buf *
bread(dev, blkno, len)
	register dev_t dev;
	daddr_t blkno;
	int len;
{
	register struct buf *bp;

	bp = getblk(dev, blkno, len);
	if (bp->b_flags & B_DONE)
		return (bp);
	bp->b_flags |= B_READ;
	if (!(bp->b_flags & B_DMA))
		bio_dma_alloc(bp);
	(*bdevsw[(short) bmajor(dev)].d_strategy)(bp);
	u.u_ior++;
	iowait(bp);
	return (bp);
}

/*
 * Read in the block, like bread, but also start I/O on the
 * read-ahead block (which is not allocated to the caller)
 * This is only for the old filesystem.
 */
struct buf *
breada(dev, blkno, len, rablkno, ralen)
	register dev_t dev;
	register daddr_t blkno;
	register int len;
	register daddr_t rablkno;
	int ralen;
{
	register struct buf *bp, *rabp;

	bp = NULL;
	if (!incore(dev, blkno, len)) {
		bp = getblk(dev, blkno, len);
		if ((bp->b_flags & B_DONE) == 0) {
			bp->b_flags |= B_READ;
			if (!(bp->b_flags & B_DMA))
				bio_dma_alloc(bp);
			(*bdevsw[(short) bmajor(dev)].d_strategy)(bp);
			u.u_ior++;
		}
	}
	if (rablkno && (bfreelist.b_bcount > 1) &&
		    !incore(dev, rablkno, ralen)) {
		rabp = getblk(dev, rablkno, ralen);
		if (rabp->b_flags & B_DONE)
			brelse(rabp);
		else {
			rabp->b_flags |= B_READ | B_ASYNC;
			if (!(rabp->b_flags & B_DMA))
				bio_dma_alloc(rabp);
			(*bdevsw[(short) bmajor(dev)].d_strategy)(rabp);
			u.u_ior++;
		}
	}
	if (bp == NULL)
		return (bread(dev, blkno, len));
	iowait(bp);
	return (bp);
}

/*
 * See if the block is associated with some buffer.
 * Allow mis-shaped buffers to match even though it will cause a delay
 * later (to keep breada from waiting now).
 */
incore(dev, blkno, len)
	register dev_t dev;
	register daddr_t blkno;
	register int len;
{
	register struct buf *bp;
	register struct buf *dp;
	register daddr_t endblkno;
	register int bucketbn;
	register int buckets;

	COMPUTE_BUCKET_STUFF(blkno, len);
	endblkno = blkno + len;
	while (buckets--) {
		dp = bhash(dev, bucketbn);
		for (bp = dp->b_forw; bp != dp; bp = bp->b_forw) {
			if ((bp->b_dev == dev) && !(bp->b_flags & B_INVAL)) {
				/*
				 * See if our request completely misses the
				 * given buffer.  If so, keep looking.
				 */
				if ((endblkno <= bp->b_blkno) ||
				    (blkno >= bp->b_blkno + bp->b_length))
					continue;
				return (1);
			}
		}
		bucketbn++;
	}
	return (0);
}

/*
 * Write the buffer, waiting for completion.
 * Then release the buffer.
 */
bwrite(bp)
	register struct buf *bp;
{
	register int flag;

	flag = bp->b_flags;
	bp->b_flags &= ~(B_READ | B_DONE | B_ERROR | B_DELWRI);
	ASSERT(bp->b_bcount == BBTOB(bp->b_length));
	if (!(bp->b_flags & B_DMA))
		bio_dma_alloc(bp);
#ifdef	OS_DEBUG
	if (dbgtest(DBG_WRITELOCK)) {
		/*
		 * Gobble up writes if write lock is on
		 */
		iodone(bp);
		iprintf("bwrite: ignoring write of dev=%x bn=%d len=%d\n",
				 bp->b_dev, bp->b_blkno, bp->b_bcount);
	} else
#endif
	bio_meter.bwrites++;
	(*bdevsw[(short) bmajor(bp->b_dev)].d_strategy)(bp);
	u.u_iow++;
	if ((flag & B_ASYNC) == 0) {
		iowait(bp);
		brelse(bp);
	} else {
		basyncnt++;
		if (flag & B_DELWRI)
			bp->b_flags |= B_AGE;
		else
			geterror(bp);
	}
}

/*
 * Release the buffer, marking it so that if it is grabbed
 * for another purpose it will be written out before being
 * given up (e.g. when writing a partial block where it is
 * assumed that another write for the same block will soon follow).
 * This can't be done for magtape, since writes must be done
 * in the same order as requested.
 */
bdwrite(bp)
	register struct buf *bp;
{
	bp->b_flags |= B_DELWRI | B_DONE;
	bp->b_resid = 0;
	brelse(bp);
}

/*
 * Release the buffer, start I/O on it, but don't wait for completion.
 */
bawrite(bp)
	register struct buf *bp;
{
#ifdef NOTDEF
	if (bfreelist.b_bcount > 4)	/* XXX dubious hack */
#endif
	bp->b_flags |= B_ASYNC;
	bwrite(bp);
}

/*
 * release the buffer, with no I/O implied.
 */
brelse(bp)
	register struct buf *bp;
{
	register int s;
	extern char mbmapwant;

	if (bp->b_flags & B_WANTED)
		wakeup((caddr_t)bp);
	if (bfreelist.b_flags & B_WANTED) {
		bfreelist.b_flags &= ~B_WANTED;
		wakeup((caddr_t)&bfreelist);
	}
	if (bp->b_flags & B_ERROR) {
		bp->b_flags |= B_INVAL;
		bp->b_dev = NODEV;		/* no association */
	}

	/*
	 * If buffer was used for strange purposes, then just free the
	 * virtual memory used.  For normal buffers, we cache the dma
	 * space and kernel map space in case of a quick re-reference.
	 */
	if (bp->b_flags & B_INVAL) {
		/*
		 * If this buffer is btoss'd, or was invalidated for
		 * any other strange reason, clear the delayed write
		 * bit so that it won't get written.
		 */
		bp->b_flags &= ~B_DELWRI;
		bio_freeall(bp);
	}

	/*
	 * Place the buffer on an appropriate free list
	 */
	s = spl6();
	if (bp->b_flags & (B_AGE | B_INVAL)) {
		binsheadfree(bp);
	} else {
		binstailfree(bp);
	}
	bp->b_flags &= ~(B_WANTED|B_BUSY|B_ASYNC|B_BDFLUSH|B_AGE);
	bfreelist.b_bcount++;
	if (bp->b_flags & B_DELWRI) {
		ASSERT((bp->b_flags & B_PHYS) == 0);
		bp->b_start = (struct proc *) lbolt;
	}
	splx(s);

	/*
	 * Check for tight resources (dma resources, kernel map, etc).
	 * If resources are tight, wakeup sleepers so they can free
	 * what they need.
	 */
	if (mbmapwant) {
		mbmapwant = 0;
		wakeup((caddr_t)&mbmapwant);
	}
	if (bio_mem_wanted) {
		bio_mem_wanted = 0;
		wakeup((caddr_t)&bio_mem_wanted);
	}
	if (kmap_wanted) {
		kmap_wanted = 0;
		wakeup((caddr_t)&kmap_wanted);
	}
}

/*
 * Find a buffer which is available for use on the freelist.
 */
struct buf *
getnewbuf(len)
	int len;
{
	register struct buf *bp, *dp;
	register int s;

loop:
	s = spl6();
	dp = &bfreelist;
	while (dp->av_forw == dp) {		/* no free blocks */
		dp->b_flags |= B_WANTED;
		sleep((caddr_t)dp, PRIBIO+1);
	}
	bp = dp->av_forw;
	notavailspl(bp);
	splx(s);
	_RANGEOFP(bp, sizeof(*bp), buf, nbuf, "buf");
	if (bp->b_flags & B_DELWRI) {
		bio_meter.getblk_writes++;
		bp->b_flags |= B_ASYNC;
		bwrite(bp);
		goto loop;
	}
	bremhash(bp);
	bp->b_flags = B_BUSY | (bp->b_flags & (B_DMA|B_KMAP));
	if (bp->b_length != len) {
		bio_freeall(bp);
		bio_allocmem(bp, len);
	} else
		bp->b_bcount = BBTOB(len);	/* force, just in case */
	if (!(bp->b_flags & B_KMAP))
		bio_allockmap(bp);
	return (bp);
}

/*
 * Assign a buffer for the given block.  If the appropriate
 * block is already associated, return it; otherwise search
 * for the oldest non-busy buffer and reassign it.
 */
struct buf *
getblk(dev, blkno, len)
	register dev_t dev;
	register daddr_t blkno;
	register int len;
{
	register struct buf *bp;
	register struct buf *dp;
	register daddr_t bucketbn;
	register daddr_t endblkno;
	register int buckets;
#ifdef NOTDEF
	register daddr_t basebucket;
#endif
	int s;

	bio_meter.getblks++;
	endblkno = blkno + len;
loop:
	COMPUTE_BUCKET_STUFF(blkno, len);
#ifdef NOTDEF
	basebucket = bucketbn;
#endif
	while (buckets--) {
		dp = bhash(dev, bucketbn);
		for (bp = dp->b_forw; bp != dp; bp = bp->b_forw) {
			/*
			 * Insure its on the same device and that its valid.
			 */
			if ((bp->b_flags & B_INVAL) || (bp->b_dev != dev))
				continue;
			/*
			 * See if our request completely misses the given
			 * buffer
			 */
			if ((endblkno <= bp->b_blkno) ||
			    (blkno >= bp->b_blkno + bp->b_length))
				continue;
			/*
			 * Found at least a piece of the desired buffer.
			 * Wait for any i/o in progress, then capture
			 * this portion.
			 */
			s = spl6();
			if (bp->b_flags & B_BUSY) {
				bp->b_flags |= B_WANTED;
				(void) sleep((caddr_t)bp, PRIBIO+2);
				splx(s);
				goto loop;
			}
			notavailspl(bp);
			splx(s);
			/*
			 * If buffer is not of the exact same shape as the
			 * request, we need to toss out the buffer.
			 */
			if ((bp->b_blkno != blkno) || (bp->b_length != len)) {
				/*
				 * If its dirty, start a write.  If its not,
				 * invalidate it so it will be tossed
				 * out of the cache.
				 */
				if (bp->b_flags & B_DELWRI) {
					bio_meter.getblk_writes++;
					bp->b_flags |= B_ASYNC;
					bwrite(bp);
					goto loop;
				}
				bp->b_flags |= B_INVAL;
				brelse(bp);
				goto loop;
			}
			if (!(bp->b_flags & B_KMAP))
				bio_allockmap(bp);
			return (bp);
		}
		bucketbn++;
	}
	/*
	 * The blocks requested are not in the cache.  Allocate a new
	 * buffer header, then perform the random book keeping needed to
	 * make the blocks accessable making sure to use the base hash
	 * header already computed for the blkno being cached.
	 */
	bp = getnewbuf(len);
#ifdef NOTDEF
	dp = bhash(dev, basebucket);	/* re-hash */
#else
	dp = bhash(dev, blkno >> bio_bbsperbucketshift);
#endif
	binshash(bp, dp);
	bp->b_dev = dev;
	bp->b_blkno = blkno;
	bp->b_error = 0;
	bp->b_resid = 0;
	ASSERT(bp->b_flags & B_KMAP);
	return (bp);
}

/*
 * get an empty block,
 * not assigned to any particular device
 */
struct buf *
geteblk(len)
	int len;
{
	register struct buf *bp;
	register struct buf *dp;

	bio_meter.getblks++;
	bp = getnewbuf(len);
	dp = &bfreelist;
	bp->b_flags = B_BUSY|B_AGE|B_INVAL | (bp->b_flags & (B_DMA|B_KMAP));
	binshash(bp, dp);
	bp->b_dev = NODEV;
	bp->b_error = 0;
	bp->b_resid = 0;
	bio_dma_free(bp);
	return (bp);
}

/*
 * getdmablk:
 *	- like geteblk, but insure that the buffer has dma resources
 */
struct buf *
getdmablk(len)
	int len;
{
	struct buf *bp;
	
	bp = geteblk(len);
	bio_dma_alloc(bp);
	return (bp);
}

/*
 * btoss:
 *	- remove blocks "blkno" through "blkno + len - 1" from the cache
 *	- only delete exact subsets of (blkno, blkno+len-1)
 *	- this is done to (a) keep the cache clean of detritus from removed
 *	  files and (b) to cancel i/o operations on deleted file data.
 *	  Hopefully files with a short lifetime will never go out to disk!
 */
void
btoss(dev, blkno, len)
	register dev_t dev;
	register daddr_t blkno;
	int len;
{
	register struct buf *bp, *dp;
	register int s;
	register daddr_t endblkno;

	endblkno = blkno + len;			/* just past end */
	dp = &bfreelist;
	s = spl6();

loop:
	for (bp = dp->av_forw; bp != dp; bp = bp->av_forw) {
		if ((bp->b_dev == dev) &&
		    ((bp->b_flags & (B_BUSY | B_INVAL)) == 0) &&
		    ((bp->b_blkno >= blkno) &&
		     (bp->b_blkno + bp->b_length <= endblkno))) {
			/*
			 * Buffer is wholly contained within the
			 * freed blocks.  Invalidate it.
			 */
			notavailspl(bp);
			bp->b_flags |= B_INVAL;
			brelse(bp);
			/*
			 * Since we just messed up the freelist linkage,
			 * start over at the top (UGH).
			 */
			goto loop;
		}
	}
	splx(s);
}

/*
 * Wait for I/O completion on the buffer; return errors
 * to the user.
 */
iowait(bp)
	register struct buf *bp;
{
	register int s;

	s = spl6();
	while ((bp->b_flags & B_DONE)==0)
		(void) sleep((caddr_t)bp, PRIBIO);
	splx(s);
	geterror(bp);
}

/*
 * Mark I/O complete on a buffer, release it if I/O is asynchronous,
 * and wake up anyone waiting for it.
 */
iodone(bp)
	register struct buf *bp;
{
	ASSERT(!(bp->b_flags & B_DONE));
	if (bp->b_flags & B_PHYS) {
		bio_dma_free(bp);
	}
	bp->b_flags |= B_DONE;
	if (bp->b_flags & B_CALL) {
		bp->b_flags &= ~B_CALL;
		swdone(bp);
		return;
	}
	if (bp->b_flags&B_ASYNC) {
		if (!(bp->b_flags & B_READ))
			basyncnt--;
		if (basyncnt == 0 && basynwait) {
			basynwait = 0;
			wakeup((caddr_t)&basyncnt);
		}
		brelse(bp);
	} else {
		bp->b_flags &= ~B_WANTED;
		wakeup((caddr_t)bp);
	}
}

/*
 * Zero the core associated with a buffer.
 */
clrbuf(bp)
	register struct buf *bp;
{
	_RANGEOFP(bp, sizeof(*bp), buf, nbuf, "buf");
	if (!(bp->b_flags & B_KMAP))
		bio_allockmap(bp);
	bzeroBBS(bp->b_un.b_addr, bp->b_length);
	bp->b_resid = 0;
}

/*
 * make sure all write-behind blocks on dev (or NODEV for all)
 * are flushed out.
 */
bflush(dev)
	register dev_t dev;
{
	register struct buf *bp, *dp;
	register int s;

	bio_meter.bflushs++;
loop:
	dp = &bfreelist;
	s = spl6();
	for (bp = dp->av_forw; bp != dp; bp = bp->av_forw) {
		if ((bp->b_flags & B_DELWRI) &&
		    ((dev == NODEV) || (dev == bp->b_dev))) {
			bio_meter.bflush_writes++;
			bp->b_flags |= B_ASYNC;
			notavailspl(bp);
			splx(s);
			bwrite(bp);
			/*
			 * Since we wrote something, the buffer list is likely
			 * to have changed shape.  Start over at the top.
			 */
			goto loop;
		}
	}
	splx(s);
}

/*
 * Flush out stale delayed write's.  This is called periodically by
 * a kernel process to keep the buffer cache moving to the disk.
 * We are called once a second.  We **can** sleep.
 */
#define	SYNCRATE	(2*60)		/* 2 minutes */
#define	BWRITES_PER_SEC	10
#define	AUTOUP		(5*HZ)		/* write out in 5 seconds */
#define	ROOTSYNCRATE	3		/* every 3rd SYNCRATE update root */
bdsync()
{
	static ulong lastBwrites;
	static short syncRate;
	static struct buf *rotor;
	static short rootLastWritten;
	register long remainder; 
	register struct buf *bp;
	register short buffersLeft;
	int s;

	if (--syncRate <= 0) {
		syncRate = SYNCRATE;
		if (--rootLastWritten <= 0) {
			/*
			 * Guarantee that the root superblock gets written
			 * out at least every ROOTSYNCRATE.
			 */
			rootLastWritten = ROOTSYNCRATE;
			FS_UPDATETIME(rootdir->i_mntdev);
		}
		update();
		goto out;
	}
	bio_meter.bdsyncs++;

	/*
	 * Compute the number of writes we are allowed to do, reducing
	 * our amount by the number that were written in the previous
	 * second.  Thus, if more than BWRITES_PER_SEC writes were done
	 * in the last second, reaminder will be less than zero,
	 * indicating no more writes remaining.
	 */
	remainder = BWRITES_PER_SEC - (bio_meter.bwrites - lastBwrites);

	/*
	 * Using a rotor, walk through the buffer array looking for dirty
	 * buffers than can be fiddled with.  Because we are running as
	 * a regular process, nothing can make a buffer change state on us
	 * during our scan (except for making one go free that was busy).
	 */
	buffersLeft = nbuf;
	bp = rotor;
	while ((remainder > 0) && buffersLeft) {
		if ((bp == NULL) || (bp >= &buf[nbuf]))
			bp = &buf[0];
		s = spl6();
		if ((bp->b_flags & B_DELWRI) &&
		    ((bp->b_flags & (B_BUSY|B_INVAL)) == 0) &&
		    ((lbolt - (long) bp->b_start) >= AUTOUP)) {
			bio_meter.bdsync_writes++;
			bp->b_flags |= B_ASYNC|B_BDFLUSH;
			notavailspl(bp);
			splx(s);
			bwrite(bp);
			remainder--;
		} else
			splx(s);
		bp++;
		buffersLeft--;
	}
	rotor = bp;

out:
	lastBwrites = bio_meter.bwrites;
}

/*
 * Wait for asynchronous writes to finish.
 */
bdwait()
{
	register int s = spl6();
	while (basyncnt) {
		basynwait = 1;
		sleep((caddr_t)&basyncnt, PRIBIO);
	}
	splx(s);
}

/*
 * Invalidate blocks for a dev after last close.
 */
binval(dev)
	register dev_t dev;
{
	register struct buf *dp;
	register struct buf *bp;
	register short i;

	for (i = 0; i < NHBUF; i++) {
		dp = (struct buf *)&hbuf[i];
		for (bp = dp->b_forw; bp != dp; bp = bp->b_forw) {
			if (bp->b_dev == dev)
				bp->b_flags |= B_INVAL|B_AGE;
		}
	}
}

/*
 * Invalidate blocks associated with dev which are on the freelist.
 * Make sure all write-behind blocks associated with dev are flushed out.
 */
binvalfree(dev)
	register dev_t dev;
{
	register struct buf *bp, *dp;
	int s;

	bio_meter.binvals;
loop:
	dp = &bfreelist;
	s = spl6();
	for (bp = dp->av_forw; bp != dp; bp = bp->av_forw) {
		if (dev == bp->b_dev || dev == NODEV) {
			if (bp->b_flags & B_DELWRI) {
				bio_meter.binval_writes++;
				bp->b_flags |= B_ASYNC;
				notavailspl(bp);
				(void) splx(s);
				bwrite(bp);
				goto loop;
			} else {
				bp->b_flags |= B_INVAL;
				bp->b_dev = NODEV;
			}
		}
	}
	(void) splx(s);
}

/*
 * Get the major device number given a strategy routine address
 */
dev_t
getmajor(strat)
	register int (*strat)();
{
	register struct bdevsw *bdp;
	register short i;

	bdp = &bdevsw[0]; 
	for (i = 0; i < bdevcnt; i++, bdp++)
		if (bdp->d_strategy == strat)
			return ((dev_t)i);
	return (NODEV);
}

/*
 * berror:
 *	- tag a buffer with an error
 */
berror(bp)
	register struct buf *bp;
{
	bp->b_flags |= B_ERROR;
	bp->b_resid = bp->b_bcount;
	bp->b_error = 0;
	iodone(bp);
}

/*
 * bio_freeall:
 *	- free all the resources attached to the given buffer
 *	- this does things in a particular order to guarantee success
 *	  as some of the free operations clear parameters that the
 *	  other resources use
 */
bio_freeall(bp)
	struct buf *bp;
{
	_RANGEOFP(bp, sizeof(*bp), buf, nbuf, "buf");
	bio_dma_free(bp);	/* release dma resources */
	bio_freekmap(bp);	/* release kernelmap resources */
	bio_freemem(bp);	/* release memory resources */
}

/*
 * Allocate len bb's worth of physical memory for the given buffer.
 * It is important that the state of "bp" be self consistent before
 * we sleep, so as to insure that the same disk address does not get
 * cached by an improper getblk.  Thus, we set "b_length" before we
 * have actually try to allocate memory.
 */
bio_allocmem(bp, len)
	register struct buf *bp;
	int len;
{
	register int pages, i;
	register struct pte *pt;
	register unsigned char *ref;
	register long byteOffset;
	register int remainder;
	int pgaddr;
	int s, addr;

	_RANGEOFP(bp, sizeof(*bp), buf, nbuf, "buf");
	ASSERT(len);
	ASSERT((bp->b_flags & (B_KMAP|B_DMA)) == 0);
	if (bp->b_length)
		return;
	bp->b_length = len;
	len = BBTOB(len);
	bp->b_bcount = len;

	/*
	 * Get bio page table space.  If we are out, free some memory
	 * from the buffer cache.  If that doesn't work then go to sleep
	 * waiting for active i/o to release the page table space.
	 */
	bio_meter.memmaps++;
	bio_meter.bbs += bp->b_length;
	if (bio_meter.bbs > bio_meter.maxbbs)
		bio_meter.maxbbs = bio_meter.bbs;
	s = spl6();
	while ((addr = rmalloc(bio_memmap, bp->b_length)) == 0) {
		bio_meter.memmap_fails++;
		if (bio_uncache_mem(1))
			continue;
		bio_mem_wanted = 1;
		(void) sleep((caddr_t)&bio_mem_wanted, PRIBIO);
	}

	/*
	 * Allocate physical pages for each page in the page table
	 * that doesn't already have one.
	 */
	byteOffset = BBTOB(addr - 1);
	pgaddr = btop(byteOffset);
	pages = PAGES(byteOffset, len);
	pt = &bio_pt[pgaddr];
	ref = &bio_bbcnt[pgaddr];
	for (i = pages; --i >= 0; pt++, ref++) {
		/*
		 * Update reference count by the number of bb's we
		 * are going to use in this page.
		 */
		remainder = NBPG - (byteOffset & PGOFSET);
		if (remainder > len)
			remainder = len;
		*ref += BTOBBT(remainder);
		ASSERT((*ref & BBCNT_COUNT) <= BTOBBT(NBPG));
		byteOffset += remainder;
		len -= remainder;

		/* see if page is already valid */
		if (pt->pg_v) {
			ASSERT((*ref & (BBCNT_WANTED|BBCNT_WAIT)) == 0);
			continue;
		}

		/*
		 * Page is not valid.  Need to actually allocate one here.
		 * Because we can go to sleep allocating this page, we mess
		 * with the reference count cell to indicate that it is being
		 * allocated right now, and that some other allocation should
		 * wait until this one completes.
		 */
		if (*ref & BBCNT_WAIT) {
			*ref |= BBCNT_WANTED;
			bio_meter.memmap_dups++;
			(void) sleep(ref, PRIBIO);
			ASSERT(pt->pg_v);
			continue;
		}
		*ref |= BBCNT_WAIT;
		(void) vmemall(pt, 1, (struct proc *)NULL, CSYS);
		pt->pg_v = 1;
		if (*ref & BBCNT_WANTED)
			wakeup(ref);
		*ref &= ~(BBCNT_WANTED | BBCNT_WAIT);
		bio_meter.pages++;
		if (bio_meter.pages > bio_meter.maxpages)
			bio_meter.maxpages = bio_meter.pages;
	}
	ASSERT(len == 0);
	splx(s);
	bp->b_memaddr = addr;
}

/*
 * Release physical memory attached to the argument buffer.
 * This releases bio page table space, and if the pages underneath
 * become free, it releases the physical memory as well.
 */
bio_freemem(bp)
	register struct buf *bp;
{
	register long byteOffset;
	register int pages, i, remainder;
	register unsigned char *ref;
	register struct pte *pt;
	register int len;
	int pgaddr;
	int s;

	_RANGEOFP(bp, sizeof(*bp), buf, nbuf, "buf");
	if (!bp->b_length)
		return;

	/* free page table resources */
	s = spl6();
	bio_meter.bbs -= bp->b_length;
	rmfree(bio_memmap, bp->b_length, bp->b_memaddr);

	/* free physical memory, if reference says to */
	len = BBTOB(bp->b_length);
	byteOffset = BBTOB(bp->b_memaddr - 1);
	pgaddr = btop(byteOffset);
	pages = PAGES(byteOffset, len);
	pt = &bio_pt[pgaddr];
	ref = &bio_bbcnt[pgaddr];
	for (i = pages; --i >= 0; pt++, ref++) {
		remainder = NBPG - (byteOffset & PGOFSET);
		if (remainder > len)
			remainder = len;
		*ref -= BTOBBT(remainder);
		byteOffset += remainder;
		len -= remainder;
		if (*ref == 0) {
			(void) memfree(pt, 1, 1);
			initpte(pt, 0, 0);		/* zero out */
			bio_meter.pages--;
		}
	}
	ASSERT(len == 0);
	if (bio_mem_wanted) {
		bio_mem_wanted = 0;
		wakeup((caddr_t)&bio_mem_wanted);
	}
	splx(s);
	bp->b_length = 0;
	bp->b_bcount = 0;
	bp->b_memaddr = 0;
}

/*
 * Allocate kernel virtual space for the argument buffer.  Buffers by
 * default do not get virtual space, thus allowing the cache to be
 * larger than the machines virtual space.  Use kmap_alloc to get space
 * in the Usrptmap, then validate those pages.  Because the buffer is
 * locked with B_BUSY, we know that only call to bio_allockmap can
 * occur at a time, thus no locking is needed if we have to sleep to
 * get space in the Usrptmap.
 */
bio_allockmap(bp)
	register struct buf *bp;
{
	register int pages;
	register struct pte *pt, *upt;
	register long a;
	struct pte *spt;
	long byteOffset;
	int pgaddr;

	_RANGEOFP(bp, sizeof(*bp), buf, nbuf, "buf");
	if (bp->b_flags & B_KMAP)
		return;
	ASSERT(bp->b_length && bp->b_memaddr);
	ASSERT(bp->b_kmlen == 0);

	byteOffset = BBTOB(bp->b_memaddr - 1);
	pgaddr = btop(byteOffset);
	pages = PAGES(byteOffset, BBTOB(bp->b_length));
	spt = pt = &bio_pt[pgaddr];

	/* allocate kernel map space (Usrptmap) */
	a = kmap_alloc(pages, 1);
	bp->b_un.b_addr = (caddr_t) kmxtob(a) + (byteOffset & PGOFSET);
	bio_meter.kmap_pages += pages;

	/* fill in Usrptmap from bio page table */
	upt = &Usrptmap[a];
	for (a = pages; --a >= 0; )
		*upt++ = *pt++;

	/* fill in kernel mapping registers to allow virtual access */
	ptaccess(spt, (struct pte *) bp->b_un.b_addr, pages);

	bp->b_flags |= B_KMAP;
#ifdef	OS_ASSERT
	bp->b_kmlen = bp->b_length;
#endif
}

/*
 * bio_freekmap:
 *	- un-map the kernel virtual space assigned to this buffer
 */
bio_freekmap(bp)
	register struct buf *bp;
{
	int pages;
	long a;

	_RANGEOFP(bp, sizeof(*bp), buf, nbuf, "buf");
	if (!(bp->b_flags & B_KMAP))
		return;
	ASSERT(bp->b_un.b_addr);
	ASSERT(bp->b_kmlen == bp->b_length);

	/* release kernel map */
	pages = PAGES(bp->b_un.b_addr, BBTOB(bp->b_length));
	a = btokmx((struct pte *) bp->b_un.b_addr);
	kmfree(pages, a);
	bio_meter.kmap_pages -= pages;

	bp->b_flags &= ~B_KMAP;
	bp->b_un.b_addr = 0;
#ifdef	OS_ASSERT
	bp->b_kmlen = 0;
#endif
}

/*
 * bio_dma_alloc:
 *	- called by bio to get dma resources for the given buffer
 *	- this is only used for cooked i/o
 */
bio_dma_alloc(bp)
	register struct buf *bp;
{
	long byteOffset;
	struct pte *pt;
	int pages;
	int len;
	extern caddr_t mbmapalloc();

	ASSERT(!(bp->b_flags & B_PHYS));
	if ((bp->b_flags & B_DMA) == 0) {
		ASSERT(bp->b_memaddr && bp->b_length);
		ASSERT(bp->b_iobase == (caddr_t)NULL);
		ASSERT(bp->b_iolen == 0);

		byteOffset = BBTOB(bp->b_memaddr - 1);
		pt = &bio_pt[btop(byteOffset)];
		len = BBTOB(bp->b_length);
		bp->b_iobase = mbmapalloc(pt, byteOffset & PGOFSET, len);
		pages = PAGES(bp->b_iobase, len);
		bio_meter.dma_pages += pages;

		bp->b_flags |= B_DMA;
		bp->b_iolen = BBTOB(bp->b_length);
	}
}

/*
 * Release dma resources from a buffer.  This is used for both
 * cooked and raw i/o
 */
bio_dma_free(bp)
	register struct buf *bp;
{
	int pages;
	int count;

	if (bp->b_flags & B_DMA) {
		ASSERT(bp->b_length && bp->b_iobase);
		ASSERT(bp->b_iolen);

		/*
		 * Use the same byte count (saved in b_iolen) as was
		 * used when the mbmapalloc was done (in physstrat).
		 */
		count = bp->b_iolen;
		mbmapkput(bp->b_iobase, count);
		pages = PAGES(bp->b_iobase, count);
		if (!(bp->b_flags & B_PHYS))
			bio_meter.dma_pages -= pages;
		bp->b_iobase = (caddr_t)NULL;
		bp->b_flags &= ~B_DMA;
		bp->b_iolen = 0;
	}
}

/*
 * bio_uncache_mem:
 *	- called to free up page frames used by cached buffers
 *	- this is called to either release bio_pagetable resources
 *	  or to release the memory tied up when memory gets tight
 *	- when a buffer is freed, all possible address spaces attached
 *	  to it are freed (dma, kernel map, physical mem)
 *	- "amount" is in bb's
 */
int
bio_uncache_mem(amount)
	register int amount;
{
	register struct buf *bp, *dp;
	register int s;
	register int tossed;

	tossed = 0;
	s = spl6();
	dp = &bfreelist;
	for (bp = dp->av_forw; bp != dp; bp = bp->av_forw) {
		if ((bp->b_flags & (B_DELWRI | B_BUSY | B_INVAL)) ||
		    (bp->b_length == 0))
			continue;
		tossed += bp->b_length;
		bp->b_flags |= B_INVAL;
		bio_freeall(bp);
		if (tossed >= amount)
			break;
	}
	splx(s);
	return (tossed);
}

/*
 * bio_uncache_kmap:
 *	- somebody needs kernelmap resources and there isn't enough
 *	  left to allocate.  Throw out some kernel map resources being used
 *	  by the cache
 *	- "amount" is in bb's.  If its zero, toss out everything
 *	- because the kernel mapping may cross a page boundary, due to the bufs
 *	  memory not beginning on a page boundary, the b_length estimate
 *	  for the amount of kernel map resources free may be low.  The
 *	  effect of this is innocuous.
 */
int
bio_uncache_kmap(amount)
	register long amount;
{
	register struct buf *bp, *dp;
	register int tossed;
	register int s;

	dp = &bfreelist;
	tossed = 0;
	s = spl6();
	for (bp = dp->av_forw; bp != dp; bp = bp->av_forw) {
		if ((bp->b_flags & (B_BUSY | B_INVAL)) ||
		    ((bp->b_flags & B_KMAP) == 0))
			continue;
		tossed += bp->b_length;		/* approximate only */
		bio_freekmap(bp);
		if (amount && (tossed >= amount))
			break;
	}
	splx(s);
	return (tossed);
}

/*
 * bio_uncache_dma:
 *	- somebody needs dma resources and there isn't enough left to
 *	  allocate with.  Throw out some resources being used by the cache
 *	- "amount" is in bb's.  If its zero, toss out everything
 *	- because the dma mapping may cross a page boundary, due to the bufs
 *	  memory not beginning on a page boundary, the b_length estimate
 *	  for the amount of dma map resources free may be low.  The
 *	  effect of this is innocuous.
 */
int
bio_uncache_dma(amount)
	register int amount;
{
	register struct buf *bp, *dp;
	register int tossed;
	register int s;

	dp = &bfreelist;
	tossed = 0;
	s = spl6();
	for (bp = dp->av_forw; bp != dp; bp = bp->av_forw) {
		if ((bp->b_flags & (B_BUSY | B_INVAL)) ||
		    ((bp->b_flags & B_DMA) == 0))
			continue;
		tossed += bp->b_length;		/* approximate only */
		bio_dma_free(bp);
		if (amount && (tossed >= amount))
			break;
	}
	splx(s);
	return (tossed);
}
