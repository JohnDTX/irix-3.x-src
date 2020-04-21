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
 * $Source: /d2/3.7/src/sys/h/RCS/buf.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:29:11 $
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
	    char *b_bitmap;		/* bitmap blocks */
	    struct extent *b_exts;	/* indirect extents */
	} b_un;
	uint	b_bcount;		/* io transfer count */
	uint	b_resid;		/* words not transferred after error */
	char	b_error;		/* returned after I/O */

	/*
	 * Buffers that are not just headers, but actuall hold data, use
	 * these two fields to manage the memory allocated.  b_memaddr
	 * is the chunk address (see kern_chunk.c) of the buffers memory.
	 * b_length is the length of the buffer, in basic blocks.
	 */
	short	b_memaddr;		/* "address" of buffer's memory */
	ushort	b_length;		/* length of buffer in basic blocks */

	/*
	 * When the paging system pages in or out pages for a process,
	 * b_proc points to the process which is having i/o done for it.
	 * During pageouts, b_pfcent is the center page frame being pushed,
	 * in the given kluster.
	 */
	struct  proc *b_proc;		/* proc doing physical or swap I/O */
	short	b_pfcent;		/* center page when swapping cluster */

	/*
	 * Disk addressing information.  For disk devices, the strategy
	 * routine is responsible for filling these fields for two
	 * other routines: the drivers start routine and the disksort
	 * routine in fs_dsort.c
	 */
	short	b_cyl;			/* cylinder number */
	u_char	b_head;			/* head number */
	u_char	b_sector;		/* sector number */

	/*
	 * Dma fields.  b_iobase represents the virtual address of the
	 * given dma region, from the dma devices point of view.
	 */
	caddr_t	b_iobase;		/* base of i/o */
	long	b_iolen;		/* length of dma region (bytes)  */
#ifdef	OS_ASSERT
	long	b_kmlen;		/* length of kmap region */
#endif
};

/* b_flags */
#define	B_WRITE		0x00000000	/* non-read pseudo-flag */
#define	B_READ		0x00000001	/* read when I/O occurs */
#define	B_DONE		0x00000002	/* transaction finished */
#define	B_ERROR		0x00000004	/* transaction aborted */
#define	B_BUSY		0x00000008	/* not on av_forw/back list */
#define	B_PHYS		0x00000010	/* physical IO */
#define	B_WANTED	0x00000020	/* issue wakeup when BUSY goes off */
#define	B_AGE		0x00000040	/* delayed write for correct aging */
#define	B_ASYNC		0x00000080	/* don't wait for I/O completion */
#define	B_DELWRI	0x00000100	/* write at exit of avail list */
#define	B_TAPE		0x00000200	/* this is a magtape (no bdwrite) */
#define	B_UAREA		0x00000400	/* add u-area to a swap operation */
#define	B_PAGET		0x00000800	/* page in/out of page table space */
#define	B_DIRTY		0x00001000	/* dirty page to be pushed out async */
#define	B_PGIN		0x00002000	/* pagein op, so swap() can count it */
#define	B_INVAL		0x00004000	/* does not contain valid info  */
#define	B_OPEN		0x00008000	/* open routine called */
#define	B_CALL		0x00010000	/* call swdone in iodone */
#define	B_KMAP		0x00020000	/* buffer has kernel map resources */
#define	B_DMA		0x00040000	/* buffer has dma map resources */
#define	B_BDFLUSH	0x00080000	/* buffer flushed by bdsync */

#ifdef	OS_METER
#define	B_CACHE		0x80000000	/* did bread find us in the cache ? */
#endif

#ifdef	KERNEL
struct	buf bfreelist;			/* head of available list */
struct	buf *buf, *swbuf;		/* the buffer pools themselves */
short	nbuf, nswbuf;			/* number of various types of bufs */
struct	buf bswlist;			/* head of free swap header list */
struct	buf *bclnlist;			/* head of cleaned page list */

struct	buf *bell_alloc();
struct	buf *bread();
struct	buf *breada();
struct	buf *getblk();
struct	buf *geteblk();
struct	buf *getdmablk();
dev_t	getmajor();
void	btoss();
#endif
