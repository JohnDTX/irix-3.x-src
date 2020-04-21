/*
 * Text structure.
 * One allocated per pure procedure on swap device.
 * Manipulated by text.c
 *
 * $Source: /d2/3.7/src/sys/h/RCS/text.h,v $
 * $Date: 89/03/27 17:30:10 $
 * $Revision: 1.1 $
 */

struct	text {
	swblk_t	x_daddr[NXDAD];	/* disk addresses of DMTEXT-page segments */
	swblk_t	x_ptdaddr;	/* disk address of page table */
	size_t	x_size;		/* size (clicks) */
	struct	proc *x_caddr;	/* ptr to linked proc, if loaded */
	struct	inode *x_iptr;	/* inode of prototype */
	short	x_rssize;
	short	x_swrss;
	short	x_cmx;		/* cmap index of first real page (XSAVE only) */
	char	x_count;	/* reference count */
	char	x_ccount;	/* number of loaded references */
	char	x_flag;		/* traced, written flags */
	char	x_slptime;
	short	x_poip;		/* page out in progress count */
};

#define	XTRC	0x01		/* text may be written, exclusive use */
#define	XWRIT	0x02		/* text written into, must swap out */
#define	XLOAD	0x04		/* currently being read from file */
#define	XLOCK	0x08		/* being swapped in or out */
#define	XWANT	0x10		/* wanted for swapping */
#define	XERROR	0x20		/* error in reading text image */
#define	XSAVE	0x40		/* text image is being saved */

#ifdef	KERNEL
struct	text *text, *textNTEXT;
int	ntext;
#endif
