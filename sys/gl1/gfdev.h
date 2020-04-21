/*
 * Definitions for the SGI GF1 board
 */

#ifdef	KERNEL
#include "../gl1/addrs.h"
#else
#include <gl1/addrs.h>
#endif

/* board addressing (i/o space) */
#define GEDEV		(1<<12)		/* bits A12 - A19 are board decode */
#define FBCDEV		(1<<12)
#define FBCOFFSET	10

/* on-board devices */
#define READPIXEL	0		/* also FBCLRINT */
#define GECTL		(3<<10)
#define FBCTL		(1<<10)
#define FBDATA		(2<<10)

/* various registers on the GE board(s) */
#define GEdata		*(unsigned short *)GEPORT
#define GEflags		*(unsigned short *)(GFMBM(GEDEV|GECTL))
#define FBCflags	*(unsigned short *)(GFMBM(FBCDEV|FBCTL))
#define FBCdata		*(unsigned short *)(GFMBM(FBCDEV|FBDATA))
#define FBCpixel	*(unsigned short *)(GFMBM(FBCDEV|READPIXEL))

/* GEflags read bits */
#define LOWATER_BIT	0x1	/* input fifo low-water  */
#define TRAPINT_BIT	0x2	/* GE instruction trap interrupt  */
#define FIFOINT_BIT	0x4	/* input fifo interrupt (hi/lo water)  */
#define GETRAP_BIT(n)	(1<<(n+2))	/* GE instruction trap bits
					/* GE's are numbered 1-12;
					/* note inversion
					 */
#define	GETRAP_BITS	0x7FF8		/* all the ge bits */
#define HIWATER_BIT	0x8000	/* input fifo high-water  */

/* GEflags write bits */
#define GERESET_BIT_		1
#define SUBSTBPCCODE_BIT_	0x2	/* BPC cmd code 0-3 <= DI bus   */
#define ENABFIFOINT_BIT_	0x4	/* enable input fifo interrupts */
#define ENABTRAPINT_BIT_	0x8	/* enable GE trap interrupts	*/
#define STATUSLED_BIT		0x10
#define ENABVERTINT_BIT_	0x20	/* enable vertical retrace ints.*/

/* for upward compatibility */
#define ENABINPINT_BIT		0x4
#define GERESET1		0x2f	/* standard reset sequence	*/
#define GERESET2		0x26
#define GERESET3		0x26	/* (trap interrupts enabled)	*/
#define GEDEBUG			0x2e	/* (		    disabled)	*/

/* FBCflags read bits */
#define NEWVERT_BIT_	0x80	/* vertical interrupt	*/
#define VERTINT_BIT	0x40	/* BPC vertical interval	*/
#define INREQ_BIT_	0x20	/* input stage request to GEs	*/
#define INTERRUPT_BIT_	0x10	/* FBC programmed interrupt  */
#define FOTRAP_BIT_	8	/* output GEPA fifo trap  */
#define FITRAP_BIT_	4	/* input  GEPA fifo trap  */
#define FBCREQ_BIT_	2	/* output request flag */
#define GEREQ_BIT_	1	/* GE request to FBC */

/* the following can be read by performing:
 *	FBCflags = READOUT;
 *	bit = FBCdata & <FLAG>
 */
#define LED_BIT_	0x2000	/* microprogrammed LED indicator	*/
#define FBCACK_BIT	0x4000	/* FBC ACK GE	*/
#define BPCACK_BIT	0x8000	/* BPC ACK FBC	*/

/* FBCflags write bits */
#define RUNMODE		1	/* MAINTSEL0 */
#define SUBSTDI		2	/* MAINTSEL1 */
#define HOSTFLAG	4	/* MAINTSEL2 */
#define MAINT_BIT	8
#define FORCEREQ_BIT_	0x10	/* supply request to FBC (SUBSTIN_BIT set) */
#define FORCEACK_BIT_	0x20	/* supply ack to FBC when SUBSTOUT_BIT set */
#define SUBSTIN_BIT	0x40
#define SUBSTOUT_BIT	0x80

/* useful FBCflags write combintations */
#define READOUT		0xf2	/* read output reg  */
#define READOUTRUN	0x2	/* spy on output reg w/o disturbing runmode */
#define READCODERUN	0x6	/* spy on fbccode/BPC bus */
	/*
	 *	0..7		BPC data 0..7
	 *	8..12		FBC command reg 0..4
	 *	13		LED_BIT_	(see above)
	 *	14		FBCACK_BIT	(see above)
	 *	15		BPCACK_BIT	(see above)
	 */
#define RUNDEBUG	0xF3	/* run, I/O substituted - debug mode	*/
#define RUNSUBST	0x73	/* run, only input substituted		*/
#define RUNSUBSTF	0x75	/* same w/ host flag			*/
#define CYCINDEBUG	0xE3	/* use in debug mode (RUNDEBUG)		*/
#define CYCOUTDEBUG	0xD3	/* use in debug mode (RUNDEBUG)		*/
#define STARTDEV	0xF0	/* start machine in debug mode		*/
#define READMICRO	0xFA

/* more useful combos of the FBCflags */
#define FORCEREQ	(RUNMODE | SUBSTDI | SUBSTIN_BIT)
#define FORCEWAIT	(FORCEREQ | FORCEREQ_BIT_)

/*
 * Various useful macros:
 *	- to these macros pass the rest of the flags desired
 *	- NOTE: default GE flags used by FBCxxxvert()
 */
#define MakeGFStatus(ge, fbc,new) \
		new = ((fbc)<<8) | (ge) ;
#define Enabvert(stat) \
	{ GEflags = stat = (stat) & ~ENABVERTINT_BIT_; }
#define Disabvert(stat) \
	{ GEflags = stat = (stat) | ENABVERTINT_BIT_; }
#define FBCenabvert(fl)		GEflags = GERESET3 & ~ENABVERTINT_BIT_; \
					FBCflags = fl
#define FBCdisabvert(fl)	GEflags = GERESET3 | ENABVERTINT_BIT_; \
					FBCflags = fl
#define GFenabvert(geflags,fbcflags) \
		GEflags = (geflags) & ~ENABVERTINT_BIT_; \
		FBCflags = (fbcflags)
#define GFdisabvert(geflags,fbcflags) \
		GEflags = (geflags) | ENABVERTINT_BIT_; \
		FBCflags = (fbcflags)

#define FBCclrint	FBCpixel = 1	/* statement to clear FBC interrupt */

#define	GEOUT(x)	GEdata = (ushort)(x)
