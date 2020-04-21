/*
 * Kernel memory allocator.
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/sys/sys/RCS/kern_chunk.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:35:21 $
 */
#include "../h/param.h"
#include "../h/cmap.h"
#include "../vm/vm.h"
#include "machine/pte.h"

/*
 * This code manages a pool of chunks with the following attributes:
 *	1. Chunks come from physical memory, and when enough contiguous
 *	   chunks are freed, the physical memory is freed.
 *	2. The memory which is allocated is not directly kernel addressable.
 *	   For system will kernel virtual limits, this allows allocation
 *	   larger than the kernel limit.
 *	3. Chunks can be allocated on page boundaries, allowing page
 *	   flipping gimmicks.
 *	4. SOMEDAY The allocation of kernel virtual space is done at two ends:
 *		- one end is for dynamic short lifetime allocation
 *		- the other end is for static long lifetime allocation
 */

/*
 * By default, the memory which is allocated here is only physical memory
 * and is not assumed to be kernel addressable.
 */
#define	CHUNKSHIFT	9			/* 512 bytes, minimum */
#define	CHUNKSIZE	(1<<CHUNKSHIFT)
#define	MAXCHUNKSIZE	(128 * 1024)		/* arbitrary */
#define	CPPG		(NBPG / CHUNKSIZE)	/* chunks per page */

#if (NBPG > CHUNKSIZE)
#define	CPPG_SHIFT	(PGSHIFT - CHUNKSHIFT)
#else
#define	CPPG_SHIFT	(CHUNKSHIFT - PGSHIFT)	/* Huh? */
#endif

/* chunk address to chunk offset in page */
#define	CHUNK_TO_OFFSET(c)	((c) & (CPPG - 1))

/* number of chunks to bytes */
#define	CHUNKS_TO_BYTES(c)	((c) << CHUNKSHIFT)

/* byte to chunk address*/
#define	BYTE_TO_CHUNK(b)	((b) >> CHUNKSHIFT)

/* bytes to number of chunks */
#define	BYTES_TO_CHUNKS(b)	(((b) + (CHUNKSIZE - 1)) >> CHUNKSHIFT)

/* chunk to page address */
#define	CHUNK_TO_PAGE(c)	((c) >> CPPG_SHIFT)

/* chunks to number of pages */
#define	CHUNKS_TO_PAGES(c)	(((c) + (CPPG - 1)) >> CPPG_SHIFT)

/* page to chunk */
#define	PAGE_TO_CHUNK(p)	((p) << CPPG_SHIFT)

/*
 * For fast coalescing, a bitmap is used.  A one in the bitmap means
 * that the chunk has been allocated, a zero means that the chunk is
 * free.
 */
int	chunk_lowfree;			/* lowest free chunk # */
char	*chunk_bitmap;			/* these are allocated in machdep.c */
struct	pte *chunk_pt;			/* these are allocated in machdep.c */
int	chunk_maxchunks;		/* these are allocated in machdep.c */
int	chunk_size = CHUNKSIZE;		/* used by machdep.c */

#ifdef	OS_METER
struct {
	long	outofmems;		/* # of times couldn't get memory */
	long	getmems;
	long	scans;			/* # of bits scanned during getmem */
	long	pages_inuse;
	long	woulduse;		/* pages we would have used */
	long	chunks_inuse;
} chunk_meter;
#endif

/*
 * Release any pages that become fully free
 */
static void
tossmem(addr, len)
	int addr, len;
{
	register int pages;
	register struct pte *pte;
	register int page;

	ASSERT(addr + len <= chunk_maxchunks);
	page = CHUNK_TO_PAGE(addr);
	pages = CHUNKS_TO_PAGES(len + CHUNK_TO_OFFSET(addr));
	pte = &chunk_pt[page];
	while (--pages >= 0) {
		ASSERT(pte->pg_v);
		ASSERT(pte->pg_pfnum);
		if (bftstclr(chunk_bitmap, PAGE_TO_CHUNK(page++), CPPG) == CPPG) {
			/*
			 * Page is fully free.  Release the memory.
			 */
			(void) memfree(pte, 1, 1);
			initpte(pte, 0, 0);		/* zero out */
			METER(chunk_meter.pages_inuse--);
		}
		pte++;
	}
}

/*
 * Assign physical memory to a given chunk location
 */
static int
assignmem(addr, len, canwait)
	int addr, len;
	int canwait;
{
	register int pages;
	register struct pte *pte;

	pages = CHUNKS_TO_PAGES(len + CHUNK_TO_OFFSET(addr));
	pte = &chunk_pt[CHUNK_TO_PAGE(addr)];
	while (--pages >= 0) {
		if (pte->pg_v == 0) {
			ASSERT(pte->pg_pfnum == 0);
			/* need memory at this addr - allocate a page */
			if (canwait)
				(void) vmemall(pte, 1, (struct proc *)NULL,
						    CSYS);
			else {
				if (memall(pte, 1, (struct proc *)NULL,
						CSYS) == 0) {
					/*
					 * Couldn't get memory.  Free the
					 * memory we did get, and return.
					 */
					bfclr(chunk_bitmap, addr, len);
					tossmem(addr, len);
					return (1);
				}
			}
			pte->pg_v = 1;
			METER(chunk_meter.pages_inuse++);
		}
		pte++;
	}
	return (0);
}

int
kmem_allocmem(len, how, canwait)
	register int len;
	int how;
	int canwait;
{
	register int (*kind)();
	register int addr;
	register int foundlen;
	register int maxlen;
	register int remainder;
	extern int bftstclr(), bftstset();
	int s;

	METER(chunk_meter.getmems++);
	METER(chunk_meter.woulduse += btoc(len));
	ASSERT(len && (len < MAXCHUNKSIZE));

	addr = chunk_lowfree;
	maxlen = chunk_maxchunks - addr;
	len = BYTES_TO_CHUNKS(len);
	kind = bftstclr;

	while (maxlen >= len) {
		ASSERT(addr + maxlen <= chunk_maxchunks);
		if (kind == bftstset) {
			/*
			 * If we are searching for one's, then search the
			 * maximum remaining.
			 */
			remainder = maxlen;
		} else {
			remainder = len;
			if (how) {
				/*
				 * Search for length plus amount to get
				 * aligned.  If we find less, that may be
				 * okay.
				 */
				remainder += CPPG - 1;
			}
		}
		s = spl6();
		foundlen = (*kind)(chunk_bitmap, addr, remainder);
		splx(s);
		METER(chunk_meter.scans += foundlen);
		if ((kind == bftstclr) && (foundlen >= len)) {
			/*
			 * Found a string of zeros that is at least
			 * as long as the piece we are interested in.
			 */
			if (how) {
				remainder = CPPG - CHUNK_TO_OFFSET(addr);
				if (remainder != CPPG) {
					/* data is not aligned */
					if (foundlen - remainder < len) {
						/*
						 * Too bad. Trimming off the
						 * unaligned pieces took too
						 * much of the chunk away.
						 * Keep searching.
						 */
						foundlen = remainder;
						goto next;
					}
					addr += remainder;
				}
			}
			bfset(chunk_bitmap, addr, len);		/* allocate chunks */
			s = spl6();
			if (assignmem(addr, len, canwait)) {
				/*
				 * Oops.  Could not assign memory.  Give up
				 * the ghost.  Note that this can only happen
				 * when canwait==0.
				 */
				METER(chunk_meter.outofmems++);
				splx(s);
iprintf("allocmem: could not allocate memory, %s\n",
		   canwait ? "canwait" : "cannot wait"); 
				return (0);
			}

			METER(chunk_meter.chunks_inuse += len);
			DBG((DBG_KMEM_MEM, "getmem: addr=%d len=%d\n",
					   addr, CHUNKS_TO_BYTES(len)));
			/*
			 * If we just allocated the lowest free chunk,
			 * advance the low water mark.  If end of allocated
			 * region is beyond starting point, advance starting
			 * point.
			 */
			if (addr == chunk_lowfree)
				chunk_lowfree = addr + len;
			splx(s);
			return (addr + 1);
		} else {
			/*
			 * Invert sense of search, since each bit search stops
			 * when it sees the other bit.
			 */
			if (kind == bftstclr)
				kind = bftstset;
			else {
				/*
				 * If the test-for-one's search started at the
				 * low water mark, advance the low water
				 * mark to the end of the one's region.
				 */
				if (addr == chunk_lowfree)
					chunk_lowfree = addr + foundlen;
				kind = bftstclr;
			}
		}
next:
		addr += foundlen;
		maxlen -= foundlen;
	}
iprintf("allocmem: scanned freemap - couldn't find anything to use!\n");
iprintf("bit map=0x%x page table=0x%x chunk_lowfree=%d\n", chunk_bitmap, chunk_pt, chunk_lowfree);
debug("allocmem");
	return (0);
}

/*
 * Release len bytes of memory
 */
void
kmem_putmem(addr, len)
	int addr;
	int len;
{
	int s;

	ASSERT(addr && len);

	addr--;
	METER(chunk_meter.woulduse -= btoc(len));
	len = BYTES_TO_CHUNKS(len);
	METER(chunk_meter.chunks_inuse -= len);

	DBG((DBG_KMEM_MEM, "putmem: addr=%d len=%d\n", addr, len));
	ASSERT(bftstset(chunk_bitmap, addr, len) == len);

	s = spl6();
	if (addr < chunk_lowfree)
		chunk_lowfree = addr;			/* new low water mark */
	bfclr(chunk_bitmap, addr, len);
	tossmem(addr, len);
	splx(s);
}

/*
 * Make the given chunks of memory addressable by the kernel.
 * Return the virtual address of the addressable memory, or NULL if there
 * is no mapping space, and we can't wait.
 */
caddr_t
kmem_allocmap(addr, len, canwait)
	int addr;
	int len;
	int canwait;
{
	register long a;
	register caddr_t result;
	register int pages;
	register int offset;
	register struct pte *ptes;

	ASSERT(addr && len);

	addr--;
	offset = CHUNKS_TO_BYTES(CHUNK_TO_OFFSET(addr));
	pages = btoc(len + offset);
	a = kmap_alloc(pages, canwait);
	result = (caddr_t) kmxtob(a) + offset;
	ptes = &chunk_pt[CHUNK_TO_PAGE(addr)];
	bcopy((caddr_t) ptes, (caddr_t) &Usrptmap[a],
	      sizeof(Usrptmap[a]) * pages);
	ptaccess(ptes, (struct pte *)result, pages);
	DBG((DBG_KMEM_MAP, "getmap: i=%d addr=%x len=%d\n", a, result, len));
	return (result);
}

/*
 * Release mapping resources
 */
void
kmem_putmap(addr, len)
	caddr_t addr;
	int len;
{
	ASSERT(addr && len);
	DBG((DBG_KMEM_MAP, "putmap: i=%d addr=%x len=%d\n",
			   btokmx((struct pte *)addr), addr, len));

	kmfree(btoc(((long)addr & PGOFSET) + len), btokmx((struct pte *)addr));
}

/*
 * Make the given chunks of memory addressable by the dma devices
 * on the system.  Return the dma address of the memory
 */
caddr_t
kmem_getdma(addr, len)
	int addr;
	int len;
{
	extern caddr_t mbmapalloc();
	caddr_t a;

	ASSERT(addr && len);
	addr--;
	a = mbmapalloc(&chunk_pt[CHUNK_TO_PAGE(addr)],
				CHUNKS_TO_BYTES(CHUNK_TO_OFFSET(addr)), len);
	DBG((DBG_KMEM_DMA, "getdma: addr=%x len=%d\n", a, len));
	return (a);
}

/*
 * Release the dmap mapping for the given chunks
 */
void
kmem_putdma(addr, len)
	caddr_t addr;
	int len;
{
	ASSERT(addr && len);
	DBG((DBG_KMEM_DMA, "putdma: addr=%x len=%d\n", addr, len));

	mbmapkput(addr, len);
}

/* XXX compatability stub - remove me */
caddr_t
kmem_getmap(addr, len)
	int addr;
	int len;
{
	return (kmem_allocmap(addr, len, 1));
}

/* XXX old obsolete interface */
int
kmem_getmem(len, how)
	int len;
	int how;
{
	return (kmem_allocmem(len, how, 1));
}
