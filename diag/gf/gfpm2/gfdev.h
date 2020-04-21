/* gfdev.h  -- definitions for Geometry System and Frame Buffer Control
 *		Multibus interfaces
 *	define  GFALPHA 	for alpha rev. GE and FBC (wire-wrap) boards.
 *	define  GFBETA		for GF1 PC board.
 */



#ifndef GFMBM
#ifdef GFALPHA
#define GFMBMEMLOWADR	0x100000		/* was in pcmap.h */
#define GFMBM(x)	(GFMBMEMLOWADR + (x))
#endif
#ifdef GFBETA
#define GFIOLOWADR	0xf70000		/* was in pcmap.h */
#define GFMBM(x)	(GFIOLOWADR + (x))
#endif
#endif GFMBM

#ifndef GEPORT
#include "pm1.h"
#endif

/*================================================================*/
/* board addressing	  					  */
/*================================================================*/

#ifdef GFALPHA
#define GEDEV		(4<<14)
#define FBCDEV		(1<<15)
#define FBCOFFSET	13		/* shift const for FBC dev decodes */
#endif

#ifdef GFBETA
#define GEDEV		(1<<12)		/* bits A12 - A19 are board decode */
#define FBCDEV		(1<<12)
#define FBCOFFSET	10
#endif

/*================================================================*/
	/* on-board devices */
/*================================================================*/

#define READPIXEL	0	/* also FBCLRINT */

#ifdef GFALPHA
#define GECTL		(1<<12)
#define GEDATA		0
#define FBCTL		(2<<13)
#define FBDATA		(3<<13)
#define FBUCODE		(1<<13)
#define UCODE(x)	((x)<<3)
#endif

#ifdef GFBETA
#define GECTL		(3<<10)
/* GEDATA				/*undefined*/
#define FBCTL		(1<<10)
#define FBDATA		(2<<10)
#endif

/*================================================================*/
/* addressing constants	*/
/*================================================================*/

#ifdef GFALPHA
#define GEdata		*(unsigned short*)(GFMBM(GEDEV|GEDATA))
#else
#define GEdata		GEPORT
#endif

#define GEflags		*(unsigned short*)(GFMBM(GEDEV|GECTL))
#define FBCflags	*(unsigned short*)(GFMBM(FBCDEV|FBCTL))
#define FBCdata		*(unsigned short*)(GFMBM(FBCDEV|FBDATA))
#define FBCpixel	*(unsigned short*)(GFMBM(FBCDEV|READPIXEL))

#define FBCclrint	FBCpixel = 1	/* statement to clear FBC interrupt */

#ifdef GFALPHA
#define FBCmicro(x,y)	*(unsigned short*)(GFMBM(FBCDEV|FBUCODE|UCODE(x)|(y+y)))
#define DEVmicro(x,y)	*(unsigned short*)(GFMBM(FBCDEV|FBUCODE|UCODE(x)|(y+y)))
#endif

/* temporary ? */
#define DEVflags	FBCflags
#define DEVdata		FBCdata
#define DEVpixel	FBCpixel
#define DEVclrint	FBCclrint

/*================================================================*/
/* GEdata --  flags read	*/
/*================================================================*/

#ifdef GFALPHA
#define INREADY_BIT	0x1	/* input fifo ready */
#define EMPTYINT_BIT	0x2	/* input fifo empty interrupt */
#define FIFOINT_BIT	0x4	/* input fifo full interrupt  */
#define TRAPINT_BIT	0x8	/* GE instruction trap interrupt  */
#define GETRAP_BIT(n)	(1<<(n+3))	/* GE instruction trap bits */
					/* GE's are numbered 1-10   */
#endif
#ifdef GFBETA
#define LOWATER_BIT	0x1	/* input fifo low-water  */
#define TRAPINT_BIT	0x2	/* GE instruction trap interrupt  */
#define FIFOINT_BIT	0x4	/* input fifo interrupt (hi/lo water)  */
#define GETRAP_BIT(n)	(1<<(n+2))	/* GE instruction trap bits
					/* GE's are numbered 1-12;
					/* note inversion
					 */
#define HIWATER_BIT	0x8000	/* input fifo high-water  */
#endif

/*================================================================*/
/* GE flags written	*/
/*================================================================*/

#ifdef GFALPHA
#define GERESET_BIT_		1
#define ENABINPINT_BIT		8	/* enable input fifo full interrupts*/
#define ENABEMPTYINT_BIT	0x10	/* enable input empty enterrupts  */
#define ENABTRAPINT_BIT		0x20	/* enable GE trap interrupts	*/
#define ENABTRAPINT_BIT_	0
#define GERESET1		0	/* standard reset sequence	*/
#define GERESET2		0xf
#define GERESET3		0xf	/* ("full" interrupt enabled)	*/
#define GEDEBUG			0xf
#endif

#ifdef GFBETA
#define GERESET_BIT_		1
#define SUBSTBPCCODE_BIT_	0x2	/* BPC cmd code 0-3 <= DI bus   */
#define ENABFIFOINT_BIT_	0x4	/* enable input fifo interrupts */
#define ENABTRAPINT_BIT_	0x8	/* enable GE trap interrupts	*/
#define STATUSLED_BIT		0x10
#define ENABVERTINT_BIT_	0x20	/* enable vertical retrace ints.*/
	/* for upward compatibility:		*/
#define ENABINPINT_BIT		0x4
#define GERESET1		0x2f	/* standard reset sequence	*/
#define GERESET2		0x26
#define GERESET3		0x26	/* (trap interrupts enabled)	*/
#define GEDEBUG			0x2e	/* (		    disabled)	*/
#endif

/*================================================================*/
/* FBC flags read  (trailing _ indicates low-active)  */
/*================================================================*/

#ifdef GFALPHA
#define VERTINT_BIT_	0x80	/* vertical interval	*/
#define NEWVERT_BIT_	0x40	/* vertical interrupt	*/
#define ENABDOFBD_BIT	0x20	/* enable DO bus (not maint read)  */
#define INTERRUPT_BIT_	0x10
#define BPCACK_BIT_	8
#define FBCREQ_BIT_	4
#define FBCACK_BIT_	2
#define GEREQ_BIT_	1
#endif

#ifdef GFBETA
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
#endif

/*================================================================*/
/* FBC flags written */
/*================================================================*/

#ifdef GFALPHA
#define FLOAT		0
#define RUNMODE		1
#define SUBSTIN		2
#define HOSTFLAG	4
#define MAINT_BIT	8
#define SUBST_BIT	0x10
#define FORCEREQ_BIT_	0x20
#define FORCEACK_BIT_	0x40
#define ENABVERTINT_BIT_ 0x80
#define ENABVERT_SHIFTED_ 0x8000
#endif

#ifdef GFBETA
#define RUNMODE		1	/* MAINTSEL0 */
#define SUBSTDI		2	/* MAINTSEL1 */
#define HOSTFLAG	4	/* MAINTSEL2 */
#define MAINT_BIT	8
#define FORCEREQ_BIT_	0x10	/* supply request to FBC (SUBSTIN_BIT set) */
#define FORCEACK_BIT_	0x20	/* supply ack to FBC when SUBSTOUT_BIT set */
#define SUBSTIN_BIT	0x40
#define SUBSTOUT_BIT	0x80
#endif

/*================================================================*/
	/* FBC flag combinations */
/*================================================================*/

#ifdef GFALPHA
#define FLOAT		0
#define READOUTRUN	0x82	/* spy on output reg w/o disturbing runmode */
#define READOUT		0x82
#define RUNDEBUG	0xF3	/* run substituted  - debug mode */
#define RUNDEBUGF	0xF7	/* debug mode w/ host flag  */
#define READCODERUN	0xF6	/* spy on fbccode/BPC bus  */
#define READMICRO	0xFA
#define WRITEMICRO	0xFB
#define CYCINDEBUG	0xD3	/* use in debug mode */
#define CYCOUTDEBUG	0xB3	/* use in debug mode */
#define CYCFLOATBLOCKED	0x93
#define STARTDEV	0x90	/* start machine in debug mode */
#endif

#ifdef GFBETA
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
#endif

/*================================================================*/
	/* version-independent enables & tests */
/*================================================================*/

	/* to these macros pass the rest of the flags desired */
	/* note: default GE flags used by FBCxxxvert() !!!	*/

#define MakeGFStatus(ge,fbc,new) \
		new = ((fbc)<<8) | (ge) ;
#ifdef GFALPHA
#define Enabvert(stat) \
		{ stat &= ~ENABVERT_SHIFTED_; FBCflags = (stat)>>8; }
#define Disabvert(stat) \
		{ stat |= ENABVERT_SHIFTED_; FBCflags = (stat)>>8; }
#define FBCenabvert(fl)		FBCflags = (fl) & ~ENABVERTINT_BIT_
#define FBCdisabvert(fl)	FBCflags = (fl) | ENABVERTINT_BIT_
#define GFenabvert(geflags,fbcflags) \
		FBCenabvert(fbcflags); GEflags = (geflags)
#define GFdisabvert(geflags,fbcflags) \
		FBCdisabvert(fbcflags); GEflags = (geflags)
#endif

#ifdef GFBETA
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
#endif
