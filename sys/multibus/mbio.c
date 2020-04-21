/*
 * $Source: /d2/3.7/src/sys/multibus/RCS/mbio.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:31:29 $
 */
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/map.h"
#include "../h/buf.h"
#include "../vm/vm.h"
#include "../multibus/mbvar.h"
#include "machine/cpureg.h"
#include "machine/pte.h"
#include "machine/cx.h"
#include "../debug/debug.h"

/*
 * The PM2 & IP2 cpus contain a page map which translates multibus virtual
 * addresses into onboard physical addresses.  This page map contains
 * pages of the same size as the cpu page map, but is only used when the
 * low 1Meg of the multibus address space is refrenced.  The upper 1 Meg
 * is used to access this page map.  It is sparsely distributed.
 *
 * To set a map entry in the PM2, you first have to point a pte at the
 * correct multibus address and then change the multibus map.  The IP2
 * has a segment that allows direct access of the multibus address space.
 *
 * The PM2 does not allow the lower 64K (0x10) pages to be accessed.
 * This area is reserved for the mailbox interrupt.  The IP2 does not
 * have this restriction.
 *
 * It is the resposibility of drivers to map buffers into multibus memory 
 * space if they want to perform DMA.
 */

/* this is set when we can't find multibus map space */
char	mbmapwant;
int	mballocated;

/*
 * we can map 1Mb (maybe minus a bit) worth of multibus space onto physical
 * memory, in page size chunks; MBMAPSPACE is this amount, in pages.
 *
 * MBMAPSIZE, on the other hand, is the number of entries in mbmap, the
 * data structure used to allocate the map space.
 *
 * Note: MBMAP_BAD must not be zero. The rmalloc code can't deal with addresses
 * 	 equal to zero 
 */
#ifdef PM2
#define	ONEMEG	0x100000	/* multibus addr of map registers */
#define	MBMAP_BAD	16	/* low 16 pages aren't usable */
#endif
#ifdef IP2
#define	MBMAP_BAD	1	/* low 16 pages are usable (thanks kurt) */
#endif

#define	MBMAP_SPACE	256
#define	MBMAP_SIZE	128	/* probably too big */

struct	map mbmap[MBMAP_SIZE];

/*
 * init_mbmap:
 *	- setup the mbmap resources
 *	- map all values to invalid page frames
 */
init_mbmap()
{
	register int i;
#ifdef	PM2
	struct pte apte;
#endif
#ifdef	IP2
	register u_short *mbpte;
#endif

	for (i = MBMAP_BAD; i < MBMAP_SPACE; i++) {
		if ( i == 0x7F )	/* skip WUB		*/
			continue;
#ifdef PM2
		*(long *)&apte = (btoc(ONEMEG) + i) | PG_AS_MBRAM |
			PG_KW | PG_V;
		setpte(MBUS_VBASE, KCX, &apte);
		*(u_short *)MBUS_VBASE = 0xFFFF;
#endif
#ifdef IP2
		mbpte = (u_short *)( SEG_MBMEM + ONEMEG + ctob( i ) );
		*mbpte = 0xFFFF;
#endif
	}

	/*
	 * Free multibus map resources into mbmap.  Note that mbmap page
	 * 7F is used for the WUB page, and thus we don't free it.
	 *
	 * XXX make 7F a #define somewhere
	 */
	rminit(mbmap, (long)0x7F - MBMAP_BAD, (long)(MBMAP_BAD),
		      "mbmap", MBMAP_SIZE);
	rmfree(mbmap, (long)0x80, (long)0x80);
}

/*
 * mballoc:
 *	- get map space from mbmap, waiting if necessary
 */
short
mballoc(npages, canwait)
	short npages;
{
	short mbuspage;
	int s;

	s = spl6();
	while ((mbuspage = rmalloc(mbmap, (long)npages)) == 0) {
		if (!canwait)
			return (-1);
		if (bio_uncache_dma(PGTOBB(npages)))
			continue;
		mbmapwant = 1;
		sleep((caddr_t)&mbmapwant, PRIBIO);
	}
	mballocated += npages;
	splx(s);
	return (mbuspage);
}

/*
 * mbmapkget:
 *	- find space for the given region in the multibus map space
 *	- this is used for dma regions in the kernel address space
 *	- return the multibus address of the region
 */
long
mbmapkget(addr, size)
	caddr_t addr;
	register long size;
{
	register short mbuspage;
	register int npages;
	register long vaddr;
	int offset;
	struct pte x;
#ifdef	PM2
	struct pte apte;
#endif
#ifdef	IP2
	register u_short *mbpte;
#endif

	vaddr = (long) addr;

	/* allocate needed space from mbmap */
	offset = (long)vaddr & PGOFSET;
	npages = btoc(size + offset);
	mbuspage = mballoc(npages, 1);
	addr = ptob(mbuspage) + offset;

	/* map each successive page */
	while (--npages >= 0) {
		getpte(vaddr, KCX, &x);
#ifdef PM2
		*(long *)&apte = (btoc(ONEMEG) + mbuspage) | PG_AS_MBRAM |
					PG_KW | PG_V;
		setpte(MBUS_VBASE, KCX, &apte);
		*(u_short *)MBUS_VBASE = x.pg_pfnum;
#endif
#ifdef IP2
		mbpte = (u_short *)( SEG_MBMEM + ONEMEG + ctob( mbuspage ) );
		*mbpte = x.pg_pfnum;
#endif
		mbuspage++;
		vaddr += NBPG;
	}
	return ((long)addr);
}

/*
 * mbmapkput:
 *	- release the multibus map space used by the given buffer
 *	- free's up space used by the kernel for dma
 */
mbmapkput(mbva, size)
	register long mbva;
	register long size;
{
	int s;
	
	/*
	 * Convert multibus virtual address and size into multibus page
	 * address and npages, respectively.
	 */
	size = btoc(size + (mbva & PGOFSET));
	mbva = btop(mbva);

	/* free up resources */
	s = spl6();
	mballocated -= size;
	rmfree(mbmap, size, mbva);
	if (mbmapwant) {
		mbmapwant = 0;
		wakeup((caddr_t)&mbmapwant);
	}
	splx(s);
}

/*
 * mbmapalloc:
 *	- internal procedure which takes a pte, a byte offset from the
 *	  beginning of a page, and the number of bytes to map, and
 *	  returns a machine dependent "pointer", which can be used by
 *	  dma devices to address the given memory
 * XXX maybe we can get rid of this - change mbmapkget to read info
 * XXX out of hardware page map, instead of assuming a one to one
 * XXX mapping - then we won't need this code
 */
caddr_t
mbmapalloc(pte, offset, len)
	register struct pte *pte;
	int offset, len;
{
	register short npages;
	register short mbuspage;
	register caddr_t iobase;

	ASSERT(offset < NBPG);
	ASSERT(len);
	ASSERT(pte);

	/* allocate needed space from mbmap */
	npages = btoc(len + offset);
	mbuspage = mballoc(npages, 1);
	iobase = ptob(mbuspage) + offset;

	/* map each successive page */
	while (--npages >= 0) {
#ifdef	PM2
		struct pte apte;

		*(long *)&apte = (btoc(ONEMEG) + mbuspage) | PG_AS_MBRAM |
					PG_KW | PG_V;
		setpte(MBUS_VBASE, KCX, &apte);
		*(u_short *)MBUS_VBASE = pte->pg_pfnum;
#endif
#ifdef IP2
		register u_short *mbpte;

		mbpte = (u_short *)( SEG_MBMEM + ONEMEG + ctob( mbuspage ) );
		*mbpte = pte->pg_pfnum;
		ASSERT(pte->pg_pfnum);
#endif
		mbuspage++;
		pte++;
	}
	return (iobase);
}

/*
 * Set a multibus mapping register to point to the physical memory that
 * pte points to.  Return the multibus virtual address of the page, taking
 * into account the offset into the page expressed in kva.
 */
caddr_t
mbset(pf, mbuspage, kva)
	int pf;
	int mbuspage;
	long kva;
{
#ifdef	PM2
	struct pte apte;
	int s;

	*(long *)&apte = (btoc(ONEMEG) + mbuspage) |
		PG_AS_MBRAM | PG_KW | PG_V;
	s = spl6();
	setpte(MBUS_VBASE, KCX, &apte);
	*(u_short *)MBUS_VBASE = pf;
	splx(s);
#endif
#ifdef	IP2
	register u_short *mbpte;

	mbpte = (u_short *)(SEG_MBMEM + ONEMEG + ctob(mbuspage));
	*mbpte = pf;
#endif

	ASSERT(pf);
	return (ptob(mbuspage) + (kva & PGOFSET));
}
