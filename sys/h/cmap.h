/*
 * $Source: /d2/3.7/src/sys/h/RCS/cmap.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:29:13 $
 */

/*
 * core map entry
 */
struct cmap
{
unsigned short	c_next,		/* index of next free list entry */
		c_prev,		/* index of previous free list entry */
		c_page,		/* virtual page number in segment */
		c_pfnum,	/* physical page frame # */
		c_iolocks;	/* # of i/o locks on this page */

unsigned short 	c_lock:1,	/* can't be paged out */
		c_want:1,	/* wanted */
		c_intrans:1,	/* intransit bit */
		c_free:1,	/* on the free list */
		c_gone:1,	/* associated page has been released */
		c_ndx:9,	/* index of owner proc or text */
		c_type:2;	/* type CSYS or CTEXT or CSTACK or CDATA */
};

#define	CMHEAD	0

#ifdef	KERNEL
struct	cmap *cmap;
struct	cmap *ecmap;
short	ncmap;
short	firstfree, maxfree;
int	ecmx;			/* cmap index of ecmap */
short	btocmx[];		/* memory board to cmap index translation */
#endif

/* bits defined in c_type */

#define	CSYS		0		/* none of below */
#define	CTEXT		1		/* belongs to shared text segment */
#define	CDATA		2		/* belongs to data segment */
#define	CSTACK		3		/* belongs to stack segment */

#define	pgtocm(pf)	(btocmx[((short)(pf))>>7] + ((short)(pf) & 0x7f))
#define	cmtopg(cmx)	cmap[cmx].c_pfnum
