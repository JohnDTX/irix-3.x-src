/*
 * Portable dma interface for the bio code.
 *
 * $Source: /d2/3.7/src/sys/pmII/RCS/dma.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:33:36 $
 */

#ifdef	KERNEL

/*
 * Init private parts (ahem) of buffer which are for the dma codes use
 * only.
 * XXX as soon as the bio memory allocator is changed, this will need to be
 * XXX updated.
 */
#define	dma_init(bp) \
{ \
	(bp)->b_iobase = 0; \
}

/* length of the dma mapped region, in bytes */
#define	dma_length(bp) (ctob(BTOBB((bp)->b_iolen)))

/* allocate dma space for the given buffer */
#define	dma_alloc(bp)	mbmapget(bp)

/* free dma space for the given buffer */
#define	dma_free(bp)	mbmapput(bp)

/*
 * Check and see if any processes are waiting for dma space.  If they are,
 * then clear the waiting flag and wakeup the sleepers.
 */
#define	dma_wakeup() \
{ \
	if (mbmapwant) { \
		mbmapwant = 0; \
		wakeup((caddr_t)&mbmapwant); \
	} \
}

/*
 * Flag indicating whether or not anybody is waiting for the dma map
 * space.
 */
extern	char mbmapwant;

#endif	KERNEL
