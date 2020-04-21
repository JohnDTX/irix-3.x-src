/*
 * IP2 software process control block
 *
 * $Source: /d2/3.7/src/sys/ipII/RCS/pcb.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:30:56 $
 */

struct	pcb {
	struct  pte *pcb_p0br; 	/* seg 0 base register */
	int	pcb_p0lr; 	/* seg 0 length register and astlevel */
	struct  pte *pcb_p1br; 	/* seg 1 base register */
	int	pcb_p1lr; 	/* seg 1 length register and pme */
	int	pcb_szpt; 	/* number of pages of user page table */
	char	pcb_faketrap;	/* process is getting a fake trace trap */
	long	pcb_aaddr;	/* access address on address errors */

	/*
	 * SGI fpa floating point state
	 */
	char	pcb_fpinuse;		/* fp unit in use */
	char	pcb_fpsaved;		/* fp unit state saved */
	struct {
		unsigned char	f_fpor,	/* option register	*/
				f_fper,	/* error register	*/
				f_fpmr,	/* mask register	*/
				f_fpcr;	/* condition register	*/
		unsigned long	f_fpregs[ 16 ][ 2 ];
	} pcb_fps;
};

/* more vax compatibility junk */
#define	aston()
#define	astoff()

#define	AST_NONE	0x00000000	/* ast level */
#define	AST_USER	0x00000000	/* ast for user mode */

#define	AST_CLR		0x00000000
#define	PME_CLR		0x00000000
