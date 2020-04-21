#ifndef GFDEVDEF
#define GFDEVDEF

/*--------------------------------------------------------------------------*/
/*									    */
/*			    PROPRIETARY INFORMATION			    */
/*									    */
/*  	These  coded instructions, statements,  and computer programs	    */
/*	contain unpublished proprietary information and are protected	    */
/*	by Federal copyright law.  They may not be disclosed to third	    */
/*	parties or copied or duplicated in any form without the prior	    */
/*	written consent of Silicon Graphics, Inc.			    */
/*									    */
/*--------------------------------------------------------------------------*/


/* gfdev.h  -- definitions for GF1 board
 *		Multibus interfaces
 *	Define PM2		for PM2's default multibus mapping
 *	Define PM1		for PM1's default multibus mapping
 *	Define MBIOBASE		externally for Unix or other environment.
 *				Default is EPROM standalone environment.
 */

#ifdef PM1
#   define GEPORT	*(unsigned short *)0x400000	/* write-only */
#   define GFMBM(x)	(0x1F0000 + (x))
#   ifdef KERNEL
#       define gewait()	asm(".word 0x3039, 0x001F, 0x1C00, 0x6BF8")
#   endif
#endif PM1

#ifdef PM2
#   define GEPORT		*(unsigned short *)0xFD4000	/* write-only */
#   ifdef V /* v kernel on pm2 */
#       define GFMBM(x)		(0x30000 + (x))
#       define gewait()		asm(".word 0x3039, 0x0003, 0x1C00, 0x6BF8")
#   else V /* unix stuff on pm2 */
#       ifdef KERNEL
#           define GFMBM(x)	(0xEEE000 + (x))
#           define gewait()	asm(".word 0x3039, 0x00EE, 0xFC00, 0x6BF8")
#       else
#           define GFMBM(x)	(0xEF4000 + (x))
#       endif KERNEL
#   endif V
#   ifdef STANDALONE
#	undef GFMBM
#	define GFMBM(x)		(0xF70000 + (x))
#   endif STANDALONE
#endif PM2

#ifdef PM3
#   define GFMBM(x)		(0x50000000 + (x))
#endif PM3
#ifdef IP2
#   define GFMBM(x)		(0x50000000 + (x))
#endif IP2 

/*================================================================*/
/* board addressing	  					  */
/*================================================================*/

#define GEDEV		(1<<12)		/* bits A12 - A19 are board decode */
#define FBCDEV		(1<<12)
#define FBCOFFSET	10

/*================================================================*/
	/* on-board devices */
/*================================================================*/

#define READPIXEL	0	/* also FBCLRINT */

#define GECTL		(3<<10)
/* GEDATA				/*undefined*/
#define FBCTL		(1<<10)
#define FBDATA		(2<<10)

/*================================================================*/
/* addressing constants	*/
/*================================================================*/

#define GEdata		GEPORT

#define GEflags		*(unsigned short*)(GFMBM(GEDEV|GECTL))
#define FBCflags	*(unsigned short*)(GFMBM(FBCDEV|FBCTL))
#define FBCdata		*(unsigned short*)(GFMBM(FBCDEV|FBDATA))
#define FBCpixel	*(unsigned short*)(GFMBM(FBCDEV|READPIXEL))

#define FBCclrint	FBCpixel = 1	/* statement to clear FBC interrupt */
#define	GEOUT(x)	GEdata = (unsigned short)(x)

/* temporary ? */
#define DEVflags	FBCflags
#define DEVdata		FBCdata
#define DEVpixel	FBCpixel
#define DEVclrint	FBCclrint

/*================================================================*/
/* GEdata --  flags read	*/
/*================================================================*/

#define LOWATER_BIT	0x1	/* input fifo low-water  */
#define TRAPINT_BIT	0x2	/* GE instruction trap interrupt  */
#define FIFOINT_BIT	0x4	/* input fifo interrupt (hi/lo water)  */
#define GETRAP_BIT(n)	(1<<(n+2))	/* GE instruction trap bits
					/* GE's are numbered 1-12;
					/* note inversion
					 */
#define HIWATER_BIT	0x8000	/* input fifo high-water  */

/*================================================================*/
/* GE flags written	*/
/*================================================================*/

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

/*================================================================*/
/* FBC flags read  (trailing _ indicates low-active)  */
/*================================================================*/

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

/*================================================================*/
/* FBC flags written */
/*================================================================*/

#define RUNMODE		1	/* MAINTSEL0 */
#define SUBSTDI		2	/* MAINTSEL1 */
#define HOSTFLAG	4	/* MAINTSEL2 */
#define MAINT_BIT	8
#define FORCEREQ_BIT_	0x10	/* supply request to FBC (SUBSTIN_BIT set) */
#define FORCEACK_BIT_	0x20	/* supply ack to FBC when SUBSTOUT_BIT set */
#define SUBSTIN_BIT	0x40
#define SUBSTOUT_BIT	0x80

/*================================================================*/
	/* FBC flag combinations */
/*================================================================*/

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

/*================================================================*/
	/* version-independent enables & tests */
/*================================================================*/

	/* to these macros pass the rest of the flags desired */
	/* note: default GE flags used by FBCxxxvert() !!!	*/

#define MakeGFStatus(ge,fbc,new) \
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


#endif GFDEVDEF
