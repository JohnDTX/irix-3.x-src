/*
 * $Source: /d2/3.7/src/sys/ipII/RCS/pte.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:30:59 $
 */
/*
** IP2 page table map entry
**
** There are two major kinds of pte's: those which have ever existed (and are
** thus either now in core or on the swap device), and those which have
** never existed, but which will be filled on demand at first reference.
** There is a structure describing each.  There is also an ancillary
** structure used in page clustering.
*/

#ifndef LOCORE

/*
** a pte in the page map
*/
struct pte
{
	unsigned int
		pg_m      : 1,	/* changed (dirty) bit		*/
		pg_used   : 1,	/* referenced bit		*/
		pg_prot   : 2,	/* protection info		*/
		pg_v	  : 1,	/* page valid (software only)	*/
		pg_fod	  : 1,	/* set if fill on demand (software only) */
			  : 10,
		pg_pfnum  : 16;	/* physical page number		*/
};

struct	fpte {
unsigned int
			  : 2,
		pg_prot   : 2,		/* eventual access control */
		pg_v      : 1,		/* software-only valid bit */
		pg_fod    : 1,		/* is fill on demand */
		pg_zfod   : 1,		/* zfod vs. text fill type */
		          : 25;		/* unused */
};

#endif LOCORE

/*
** protection encodings defined when referenced as part of a (long)
*/
#define	PTE_PROTMSK	0x30000000	/* mask for protection bits only*/
#define	PTE_NOACC	0x00000000	/* no access			*/
#define	PTE_RACC	0x10000000	/* read access only		*/
#define	PTE_SACC	0x20000000	/* system access only		*/
#define	PTE_RWACC	0x30000000	/* read/write access		*/
#define	PTE_REF		0x40000000	/* page has been referenced	*/
#define	PTE_M		0x80000000	/* page has been modified	*/
#define	PTE_PFNUM	0x00001fff	/* mask for page number		*/

#define	PTE_GOODBITS	0xF0001FFF	/* good bits from hardware pte */

/*
** for compatibility
*/
#define	PG_M		PTE_M
#define	PG_PROT		PTE_PROTMSK
#define	PG_NOACC	PTE_NOACC
#define	PG_KR		PTE_RACC
#define	PG_KW		PTE_SACC
/* #define	PG_URKW		PTE_RACC cannot map this one	*/
#define	PG_UWKW		PTE_RWACC
#define	PG_UW		PTE_RWACC
#define	PG_URKR		PTE_RACC

/*
** these bits are located in the unused portions of a pte for software info
** only
*/
#define	PG_V		0x08000000		/* pte is valid		*/
#define	PG_FOD		0x04000000		/* pte is fill on demand*/

/*
** random software bits
*/
#define	PG_FZERO	(NOFILE)
#define	PG_FTEXT	(NOFILE+1)
#define	PG_FMAX		(PG_FTEXT)

/*
 * Pte related macros
 */
#define	dirty(pte) \
	((pte)->pg_m && ((pte)->pg_fod == 0) && (pte)->pg_pfnum)

/*
 * BEWARE THIS DEFINITION WORKS ONLY WITH COUNT OF 1
 */
#define	mapin(pte, v, pfnum, count, prot) \
	*(int *)(pte) = (pfnum) | (prot)

/*
 * Initialize a pte.  Used to save bit fields operations.
 */
#define	initpte(pte, pfnum, bits) \
	*(long *)(pte) = (pfnum) | (bits)

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
