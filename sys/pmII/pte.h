/*
 * pmII page table entry:
 *	- we store the relcation info in the first short, and the
 *	  protection info in the second short
 *	- we don't want to keep the context info up to date in the
 *	  protection part of the page map (as the hardware does), so
 *	  we can use those 8 bits to hold the software pte info
 *
 * There are two major kinds of pte's: those which have ever existed (and are
 * thus either now in core or on the swap device), and those which have
 * never existed, but which will be filled on demand at first reference.
 * There is a structure describing each.  There is also an ancillary
 * structure used in page clustering.
 *
 * $Source: /d2/3.7/src/sys/pmII/RCS/pte.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:33:49 $
 */

#ifndef LOCORE

struct	pte {
unsigned int
		pg_used:1,		/* page has been used */
		pg_m:1,			/* modified bit */
		pg_as:2,		/* address space */
		:1,
		pg_prot:3,		/* access control */
		pg_v:1,			/* software-only valid bit */
		pg_fod:1,		/* is fill on demand (=0) */
		:6,
		pg_pfnum:16;		/* core page frame number or 0 */
};

struct	fpte {
unsigned int
		:5,
		pg_prot:3,		/* eventual access control */
		pg_v:1,			/* software-only valid bit */
		pg_fod:1,		/* is fill on demand */
		pg_zfod:1,		/* zfod vs. text fill type */
		:22;
};

/* XXX */
/* grot for set{u}pte */
struct	grotpte {
	short	pg_protbits;	/* includes cx which must be masked out */
	short	pg_pfnum;
};

#endif

#define	PG_USED		0x80000000
#define	PG_M		0x40000000
#define	PG_PFNUM	0x0000FFFF

#define	PG_FZERO	0x80000000
#ifdef	notdef
#define	PG_FZERO	(NOFILE)
#define	PG_FTEXT	(NOFILE+1)
#define	PG_FMAX		(PG_FTEXT)
#endif

#define	PG_PROT		0x07000000
#define	PG_NOACC	0
#define	PG_KR		0x01000000
#define	PG_KW		0x02000000
#define	PG_URKW		0x04000000
#define	PG_UWKW		0x05000000
#define	PG_UW		PG_UWKW
#define	PG_URKR		PG_URKW

/* these are random software bits */
#define	PG_V		0x00800000
#define	PG_FOD		0x00400000

#define	PG_AS		0x30000000	/* address space of page */
#define PG_AS_MBRAM	0x00000000	/* page points to multibus ram */
#define PG_AS_MBIO	0x10000000	/* page points to multibus i/o */
#define	PG_AS_OBRAM	0x20000000	/* page points to onboard ram */
#define	PG_AS_INVAL	PG_NOACC	/* page points to nowhere */

/*
 * Pte related macros
 */
#define	dirty(pte) \
	((pte)->pg_m && ((pte)->pg_fod == 0) && (pte)->pg_pfnum)

/*
 * BEWARE THIS DEFINITION WORKS ONLY WITH COUNT OF 1
 */
#define	mapin(pte, v, pfnum, count, prot) \
	*(int *)(pte) = (pfnum) | (prot) | PG_AS_OBRAM

/*
 * Initialize a pte.  Used to save bit fields operations.
 */
#define	initpte(pte, pfnum, bits) \
	*(long *)(pte) = (pfnum) | (bits) | PG_AS_OBRAM

#ifndef LOCORE
#ifdef KERNEL
struct	pte *vtopte();

/* utilities defined in locore.s */
extern	struct pte Pushmap[];
extern	struct pte Forkmap[];
extern	struct pte Xswapmap[];
extern	struct pte Xswap2map[];
extern	struct pte Swapmap[];
extern	struct pte usrpt[];		/* vaddr of user page tables */
extern	struct pte Usrptmap[];
#endif
#endif
