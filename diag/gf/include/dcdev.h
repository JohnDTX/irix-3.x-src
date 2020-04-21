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

/*
 *	Definition for the DC3 and DC4 display controllers.
 */

#ifndef DCDEVDEF
#define DCDEVDEF

#ifdef PM1
#   define DCMBM(x)	(0x1F0000 + (x))
#endif PM1

#ifdef PM2
#   ifdef V  /* v kernel on pm2 */
#       define DCMBM(x)	    (0x30000 + (x))
#   endif V
#   ifdef UNIX
#       ifdef KERNEL
#           define DCMBM(x)	(0xEEE000 + (x))
#       else
#           define DCMBM(x)	(0xEF4000 + (x))
#       endif KERNEL
#   endif UNIX
#   ifdef PROMALONE
#	define DCMBM(x)		(0xF70000 + (x))
#   endif PROMALONE
#endif PM2

#ifdef PM3 /* IP2 */
#   define DCMBM(x)		(0x50000000 + (x))
#endif PM3

/* addressing constants  */

#define DCB(x)		(1<<(x))
#define DCDEV		DCB(14)		/* address of the board 	*/
#define DCOFFSET	9

/*	DC4 Address Segment is only 512 Bytes				*/

#define DCFLAGS		(0<<DCOFFSET)
#define DCRAMRED	(1<<DCOFFSET)
#define DCRAMGRN	(2<<DCOFFSET)
#define DCRAMBLU	(3<<DCOFFSET)
#define DCADDR(x)	((x)<<1)

/* flag register  */

#ifdef DC3
#define DCBUSADRMAP	DCB(0)	/* rw Map address lines 0..7 taken from	*/
				/*    multibus address lines 1..8.	*/
#define DCREGADRMAP	DCB(1)	/* rw Map address lines 8..11 are taken	*/
				/*    from DCflags 4..7			*/
#define DCRGBMODE	DCB(2)	/* rw DACs are driven directly from the	*/
				/*    bitplane outputs, rather than from*/
				/*    the colormap outputs		*/
				/*    Red:   a7 a6 a5 a4 a3 a2 a1 a0	*/
				/*    Green: b7 b6 b5 b4 b3 b2 b1 b0	*/
				/*    Blue:  d3 c3 d2 c2 d1 c1 d0 c0	*/
#define DCMAINT		DCB(3)	/* rw Sets the maintenance LED		*/
#define DCREGADR0	DCB(4)	/* rw Map read and written, and displayed */
#define DCREGADR1	DCB(5)	/*    if DCREGADRMAP is asserted.	*/
#define DCREGADR2	DCB(6)	/*    Range 0..15			*/
#define DCREGADR3	DCB(7)	/*    Map address lines 8..11		*/
#define DCPALCTRL0	DCB(8)	/* w  VFSM PAL control signals, range	*/
#define DCPALCTRL1	DCB(9)	/*    0..7.				*/
#define DCPALCTRL2	DCB(10) /*					*/
#endif	DC3

#ifdef	DC4
#define	DCREGADR0	DCB(0)	/* rw					*/
#define DCREGADR1	DCB(1)	/* rw					*/
#define DCREGADR2	DCB(2)	/* rw					*/
#define DCREGADR3	DCB(3)	/* rw					*/
#define DCBUSADRMAP	DCB(4)	/* rw					*/
#define DCREGADRMAP	DCB(5)	/* rw					*/
#define DCRGBMODE	DCB(6)	/* rw					*/
#define DCHIGHMAP	DCB(7)	/* w  Select upper half of color map	*/
				/*    for readback.			*/
#define DCODD		DCB(7)	/* r  Oddline indicator from DPP state	*/
				/*    machine.	Bit dpp.00.		*/
#define DCPALCTRL0	DCB(8)	/* w  VFSM PAL control signals, range	*/
#define DCPALCTRL1	DCB(9)	/*    0..7.				*/
#define DCPALCTRL2	DCB(10) /*					*/
#define	DCOPTCLK	DCB(11)	/* w  Use alternate/external pixel clock*/
#define DCPIPE4		DCB(12)	/* w  Set DC4 pipeline depth to 4, not 2*/
#define DCPROM		DCB(13)	/* w  Select alternate video format	*/
#define DCD1K		DCB(14)	/* w  Magic bit.  High for RS343 and	*/
				/*    non-interlaced operation, low	*/
				/*    for RS170a.			*/
#define DCMBIT		DCB(15)	/* w  Output to 10-pin edge connector	*/
				/*    No internal connection		*/
#endif	DC4

/* VFSM is same for DC4 and DC3 */

#define DCVFSM0		DCB(8)	/* VFSM outputs - Read only!		*/
#define DCVFSM1		DCB(9)
#define DCVFSM2		DCB(10)
#define DCVFSM3		DCB(11)
#define DCVFSM4		DCB(12)
#define DCVFSM5		DCB(13)
#define DCVFSM6		DCB(14)
#define DCVFSM7		DCB(15)

/* mem space "constants"  */

#define DCflagsAddr	(short *)(DCMBM(DCDEV+DCFLAGS))
#define DCflags		*DCflagsAddr
#define DCramRedAddr(x)	(short *)(DCMBM(DCDEV+DCRAMRED+DCADDR(x)))
#define DCramGrnAddr(x)	(short *)(DCMBM(DCDEV+DCRAMGRN+DCADDR(x)))
#define DCramBluAddr(x)	(short *)(DCMBM(DCDEV+DCRAMBLU+DCADDR(x)))
#define DCramRed(x)	*DCramRedAddr(x)
#define DCramGrn(x)	*DCramGrnAddr(x)
#define DCramBlu(x)	*DCramBluAddr(x)

/* miscellaneous definitions */

#define DCMULTIMASK	0xff		/* multimap index bits		*/
#define DCSINGLEMASK	0xfff		/* singlemap index bits		*/
#define DCMAPNUM	16		/* number of colormaps		*/

/* flag settings - constants and macros */

#define DCBUSOP		(DCBUSADRMAP | DCREGADRMAP)
#define DCSINGLEMAP	0
#define DCMULTIMAP	DCREGADRMAP
#define DCMaskIndex(x)	((x)&0xff)

#ifdef	DC3
#define DCRegToIndex(x)	(((x)<<4)&0xf00)
#define DCIndexToReg(x)	(((x)>>4)&0xf0)
#define DCNumToIndex(x)	(((x)<<8)&0xf00)
#define DCIndexToNum(x)	(((x)>>8)&0xf)
#define DCNumToReg(x)	(((x)<<4)&0xf0)
#define DCRegToNum(x)	(((x)>>4)&0xf)
#endif	DC3

#ifdef	DC4
#define DCRegToIndex(x)	(((x)<<8)&0xf00)
#define DCIndexToReg(x)	(((x)>>8)&0xf)
#define DCNumToIndex(x)	(((x)<<8)&0xf00)
#define DCIndexToNum(x)	(((x)>>8)&0xf)
#define DCNumToReg(x)	((x)&0xf)
#define DCRegToNum(x)	((x)&0xf)
#endif	DC4

/*
 * Read and write macros - must be preceded by flag setup and, in some
 *   cases, index massage.
 */
#ifdef 	DC3
#define DCMapColor(index,red,grn,blu)			\
	      { DCramRed (index) = (red) | ((red)<<8);	\
		DCramGrn (index) = (grn) | ((grn)<<8);	\
		DCramBlu (index) = (blu) | ((blu)<<8);	\
		}
#endif	DC3
#ifdef 	DC4
#define DCMapColor(index,red,grn,blu)			\
	      { DCramRed (index) = (red);		\
		DCramGrn (index) = (grn);		\
		DCramBlu (index) = (blu);		\
		}
#endif	DC4

#define DCReadMap(index,red,grn,blu)		\
	      {	red = DCramRed (index);		\
		grn = DCramGrn (index);		\
		blu = DCramBlu (index);		\
		}



/*
 *	Examples of writes to a single colormap rgb location
 *
 *	DC3 multimode:
 *		DCflags = DCBUSOP | DCNumToReg (mapnumber);
 *		DCMapColor (index, red, green, blue)
 *		DCflags = DCMULTIMAP | DCNumToReg (mapnumber);
 *
 *	DC3 singlemode:
 *		DCflags = DCBUSOP | DCIndexToReg (index);
 *		mapindex = index & DCMULTIMASK;
 *		DCMapColor (mapindex, red, green, blue);
 *		DCflags = DCSINGLEMAP;
 *
 *	DC4 - same as DC3
 */

#endif DCDEVDEF
