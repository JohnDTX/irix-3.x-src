/*
 * pmII software process control block
 *
 * $Source: /d2/3.7/src/sys/pmII/RCS/pcb.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:33:46 $
 */

#define	NPHYS	4

struct	pcb {
	struct  pte *pcb_p0br; 	/* seg 0 base register */
	int	pcb_p0lr; 	/* seg 0 length register and astlevel */
	struct  pte *pcb_p1br; 	/* seg 1 base register */
	int	pcb_p1lr; 	/* seg 1 length register and pme */
	int	pcb_szpt; 	/* number of pages of user page table */
	char	pcb_faketrap;	/* process is getting a fake trace trap */
	long	pcb_aaddr;	/* access address on address errors */

	/*
	 * Sky floating point state
	 */
	char	pcb_fpinuse;		/* fp unit in use */
	char	pcb_fpsaved;		/* fp unit state saved */
	struct {
		unsigned short f_comreg; /* command register */
		unsigned long  f_reg[8]; /* state registers */
	} pcb_fps;

	/*
	 * Phys regions
	 */
	char	pcb_physused;		/* non-zero if phys region in use */
	struct phys {
		int	p_phladdr;	/* phys logical address */
		int	p_phsize;	/* phys size, in clicks */
		int	p_phpaddr;	/* phys physical address */
	} pcb_phys[NPHYS];
};

/* more vax compatibility junk */
#define	aston()
#define	astoff()

#define	AST_NONE	0x00000000	/* ast level */
#define	AST_USER	0x00000000	/* ast for user mode */

#define	AST_CLR		0x00000000
#define	PME_CLR		0x00000000

