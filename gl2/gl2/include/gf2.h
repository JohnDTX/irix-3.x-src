#ifndef GF2DEF
#define GF2DEF
/**************************************************************************
 *									  *
 * 		 Copyright (C) 1984, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/

/* gf2.h  -- definitions for GF2 board
 *		Multibus interfaces
 *	use switch PM1 / PM2 for proper Multibus mapping.
 *	use switch KERNEL to assign proper GE port address.
 */
#include "addrs.h"

/*================================================================*/
/* board addressing	  					  */
/*================================================================*/

#define GEDEV		0x2000		/* bits A12 - A19 are board decode */
#define FBCDEV		0x2000

/*================================================================*/
	/* on-board devices */
/*================================================================*/

#define READPIXEL	0	/* also FBCLRINT */
#define FBCTL		0x400
#define FBDATA		0x800
#define GECTL		0xC00

/*================================================================*/
/* addressing constants	*/
/*================================================================*/

#define GEdata		GEPORT

#define GEflags		*(unsigned short*)(GFMBM(GEDEV|GECTL))
#define FBCflags	*(unsigned short*)(GFMBM(FBCDEV|FBCTL))
#define FBCdata		*(unsigned short*)(GFMBM(FBCDEV|FBDATA))
#define FBCpixel	*(unsigned short*)(GFMBM(FBCDEV|READPIXEL))
#define FBCucode(n)	*(unsigned short*)(GFMBM(FBCDEV|FBDATA)+(n))
					/* note - n is a word address */

#define FBCclrint	FBCpixel = 1	/* statement to clear FBC interrupt */
#define	GEOUT(x)	GEdata = (unsigned short)(x)

/*==============================================================*/
/* GE flags read						*/
/*==============================================================*/

#define LOWATER_BIT	0x1	/* input fifo low-water  */
#define TRAPINT_BIT	0x2	/* GE instruction trap interrupt  */
#define FIFOINT_BIT	0x4	/* input fifo interrupt (hi/lo water)  */
#define GETRAP_BIT(n)	(1<<(n+2))	/* GE instruction trap bits
					/* GE's are numbered 1-12;
					/* note inversion
					 */
#define HIWATER_BIT	0x8000	/* input fifo high-water  */

/*==============================================================*/
/* GE flags written	(trailing _ indicates low-active)	*/
/*==============================================================*/

#define GERESET_BIT		1
#define SUBSTBPCCODE_BIT_	0x2	/* BPC cmd code 0-3 <= DI bus   */
#define ENABFIFOINT_BIT_	0x4	/* enable input fifo interrupts */
#define ENABTRAPINT_BIT_	0x8	/* enable GE trap interrupts	*/
#define ENABTOKENINT_BIT_	0x10	/* enable GE port token ints	*/
#define ENABVERTINT_BIT_	0x20	/* enable vertical retrace ints.*/
#define ENABFBCINT_BIT_		0x400	/* enable FBC program interpts. */
#define AUTOCLEAR_BIT		0x800	/* enable FBC int clr after rd  */
/*	MICROMSB(adr)		0x1c00 & adr	MSB's of micro wd adrs	*/
/*	MICROSLICE(slice)	slice<<13	slice bits are at 0x6000*/
#define MICROACCESS_BIT_	0x8000	/* enable ucode read/write	*/

	/* for downward compatibility:		*/
#define GERESET1		0x843f	/* standard reset sequence	*/
#define GERESET3		0x8036	/* (trap interrupts enabled)	*/
#define GEDEBUG			0x813e	/* (trap & token ints disabled -
					 *  FBC interrupts enabled)	*/
#define GERUNMODE		0x843e	/* all interrupts disabled	*/

/*==============================================================*/
/* FBC flags read  (trailing _ indicates low-active)		*/
/*==============================================================*/

#define BPCACK_BIT	0x800	/* BPC ACK FBC signal		*/
#define FBCACK_BIT	0x400	/* FBC ACK GE signal		*/
#define GET_BIT		0x100	/* GET signal- FBC needs input	*/
#define NEWVERT_BIT_	0x80	/* new vertical interrupt	*/
#define VERTINT_BIT	0x40	/* BPC vertical interval	*/
#define TOKEN_BIT_	0x20	/* GE port token flag		*/
#define INTERRUPT_BIT_	0x10	/* FBC programmed interrupt	*/
#define FOTRAP_BIT_	8	/* output GEPA fifo trap	*/
#define FITRAP_BIT_	4	/* input  GEPA fifo trap	*/
#define FBCREQ_BIT_	2	/* output request flag		*/
#define GEREQ_BIT_	1	/* GE request to FBC		*/

/*==============================================================*/
/* FBC flags written	(trailing _ indicates low-active)	*/
/*==============================================================*/

#define RUNBIT		1	/* MAINTSEL0 */
#define SUBSTDI		2	/* MAINTSEL1 */
#define HOSTFLAG	4	/* MAINTSEL2 */
#define MAINT_BIT	8
#define FORCEREQ_BIT_	0x10	/* supply request to FBC (SUBSTIN_BIT set) */
#define FORCEACK_BIT_	0x20	/* supply ack to FBC when SUBSTOUT_BIT set */
#define SUBSTIN_BIT	0x40
#define SUBSTOUT_BIT	0x80

/*==============================================================*/
/* FBC flag combinations					*/
/*==============================================================*/

#define RUNMODE		0x31	/* normal operation mode		*/
#define READOUTRUN	0x32	/* spy on output reg w/o disturbing runmode */
#define READINRUN	0x34	/* spy on input right reg		*/
#define READCODERUN	0x36	/* spy on BPC bus			*/
#define RUNDEBUG	0xF3	/* run, I/O substituted - debug mode	*/
#define RUNSUBST	0x73	/* run, only input substituted		*/
#define RUNSUBSTF	0x75	/* same w/ host flag			*/
#define CYCINDEBUG	0xE3	/* use in debug mode (RUNDEBUG)		*/
#define CYCOUTDEBUG	0xD3	/* use in debug mode (RUNDEBUG)		*/
#define STARTDEV	0xF0	/* start machine in debug mode		*/
#define READOUT		0xF2	/* read output reg  */
#define WRITEMICRO	0xFE
#define READMICRO	0xFF

/*================================================================*/
/*		various macro functions
/*================================================================*/

	/* to these macros pass the rest of the flags desired */

#define Enabvert(stat) \
	{ GEflags = stat = (stat) & ~ENABVERTINT_BIT_; }
#define Disabvert(stat) \
	{ GEflags = stat = (stat) | ENABVERTINT_BIT_; }

#define GFenabvert(geflags,fbcflags) \
		GEflags = (geflags) & ~ENABVERTINT_BIT_; \
		FBCflags = (fbcflags)
#define GFdisabvert(geflags,fbcflags) \
		GEflags = (geflags) | ENABVERTINT_BIT_; \
		FBCflags = (fbcflags)

/* writing microcode slices - every 512 states, must call this macro
 * to set up the addressing.  Slice is 0..3   State is 0,512,1024,1536..3584
 * Then use FBCucode(adr) = x to write, where adr is 0,2,4...510 for ea. state
 */
#define FBCmicroslice(Slice,State) \
	GEflags = (((State)+(State)) & 0x1c00) | ((Slice)<<13) \
		  | GEDEBUG & ~MICROACCESS_BIT_;

#define FBCmicrocode(State)	FBCucode( ((State) & 0x1ff)<<1  )
	/* as in	 FBCmicrocode(0x1ff) = 0x5555;	*/

#define PIPEISBUSY 	(FBCflags & TOKEN_BIT_)
#define PIPENOTBUSY 	(!(FBCflags & TOKEN_BIT_))

#endif GF2DEF
