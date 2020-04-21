/*
* $Source: /d2/3.7/src/stand/include/RCS/buf.h,v $
* $Revision: 1.1 $
* $Date: 89/03/27 17:13:34 $
*/
/*
 * Virtual buffer header definitions.
 *
 * The header for buffers in the buffer pool and otherwise used
 * to describe a block i/o request is given here.  The routines
 * which manipulate these things are given in bio.c.
 *
 * Each buffer in the pool is usually doubly linked into 2 lists:
 * hashed into a chain by <dev,blkno> so it can be located in the cache,
 * and (usually) on (one of several) queues.  These lists are circular and
 * doubly linked for easy removal.
 *
 * There are currently three queues for buffers:
 *	one for buffers which must be kept permanently (super blocks)
 * 	one for buffers containing ``useful'' information (the cache)
 *	one for buffers containing ``non-useful'' information
 *		(and empty buffers, pushed onto the front)
 * The latter two queues contain the buffers which are available for
 * reallocation, are kept in lru order.  When not on one of these queues,
 * the buffers are ``checked out'' to drivers which use the available list
 * pointers to keep track of them in their i/o active queues.
 *
 * $Source: /d2/3.7/src/stand/include/RCS/buf.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:13:34 $
 */
struct	buf {
	long	b_flags;		/* see defines below */
	struct	buf *b_forw;		/* headed by d_tab of conf.c */
	struct	buf *b_back;		/*  "  */
	struct	buf *av_forw;		/* position on free list, */
	struct	buf *av_back;		/*     if not BUSY*/
	dev_t	b_dev;			/* major+minor device name */
	daddr_t	b_blkno;		/* cached block # */

	union {
	    caddr_t b_addr;		/* low order core address */
	    struct direct *b_direct;	/* directory entry */
	    int	*b_words;		/* words for clearing */

	    /* bell fs stuff */
	    struct filsys *b_filsys;	/* bell superblocks */
	    struct dinode *b_dino;	/* disk inode */
	    struct fblk *b_fblk;	/* fblk */
	    daddr_t *b_daddr;		/* indirect block */

	    /* extent fs stuff */
	    struct efs *b_efs;		/* efs superblock */
	    struct efs_dinode *b_edino;	/* disk inode */
	    bitmap *b_bitmap;		/* bitmap blocks */
	    struct extent *b_exts;	/* indirect extents */
	} b_un;
	u_int	b_bcount;		/* io transfer count */
	caddr_t	b_iobase;		/* base of i/o */
	long	b_iobn;			/* bn of i/o */
	short	b_maplen;		/* mapped length (B_MAPPED) */
	long	b_mapaddr;		/* mapped address (B_MAPPED) */

	struct	pte *b_pte;		/* pte space for this buffer */
	u_int	b_resid;		/* words not transferred after error */
	short	b_length;		/* length of buffer in basic blocks */
	char	b_error;		/* returned after I/O */

	/* disk addressing information */
	short	b_cyl;			/* cylinder number */
	u_char	b_head;			/* head number */
	u_char	b_sector;		/* sector number */

	/* machine dependent fields */
	short	b_physaddr;		/* address info */
	short	b_physlen;		/* len of b_physaddr region in pages */

/*XXX*/	short	b_pfcent;		/* center page when swapping cluster */
};

/* b_flags */
#define	B_WRITE		0x0000000	/* non-read pseudo-flag */
#define	B_READ		0x0000001	/* read when I/O occurs */
#define	B_DONE		0x0000002	/* transaction finished */
#define	B_ERROR		0x0000004	/* transaction aborted */
#define	B_BUSY		0x0000008	/* not on av_forw/back list */
#define	B_PHYS		0x0000010	/* physical IO */
#define	B_MAPPED	0x0000020	/* buffer has been vmapped */
#define	B_WANTED	0x0000040	/* issue wakeup when BUSY goes off */
#define	B_AGE		0x0000080	/* delayed write for correct aging */
#define	B_ASYNC		0x0000100	/* don't wait for I/O completion */
#define	B_DELWRI	0x0000200	/* write at exit of avail list */
#define	B_TAPE		0x0000400	/* this is a magtape (no bdwrite) */
#define	B_UAREA		0x0000800	/* add u-area to a swap operation */
#define	B_PAGET		0x0001000	/* page in/out of page table space */
#define	B_DIRTY		0x0002000	/* dirty page to be pushed out async */
#define	B_PGIN		0x0004000	/* pagein op, so swap() can count it */
#define	B_CACHE		0x0008000	/* did bread find us in the cache ? */
#define	B_INVAL		0x0010000	/* does not contain valid info  */
#define	B_OPEN		0x0020000	/* open routine called */
#define	B_CALL		0x0040000	/* XXX call swdone in iodone */
#define	B_EXTEND	0x0080000	/* buffer being extended */

#ifdef	KERNEL

struct	buf bfreelist;			/* head of available list */
struct	buf *buf, *swbuf;		/* the buffer pools themselves */
short	nbuf, nswbuf;			/* number of various types of bufs */
struct	buf bswlist;			/* head of free swap header list */
struct	buf *bclnlist;			/* head of cleaned page list */
struct	pte *bio_pagetable;		/* page table for cache memory */
struct	map *bio_map;			/* map for allocating bio_pagetable */

struct	buf *bell_alloc();
struct	buf *bread();
struct	buf *breada();
struct	buf *getblk();
struct	buf *geteblk();
struct	buf *efs_bitpos();
dev_t	getmajor();

#endif
