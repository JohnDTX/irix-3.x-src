/*
 * $Source: /d2/3.7/src/stand/lib/gl/RCS/gl2dcdev.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:15:09 $
 */
#ifndef DCDEVDEF
#define DCDEVDEF
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
/* 	dcdev.h 14 May 84 include support for of DC4 			    */

/*
 *	Definition for the DC3 and DC4 display controllers.
 */
#ifdef PM1
#   define DCMBM(x)	(0x1F0000 + (x))
#endif PM1

/* GB - replaced above section with following one */
#ifdef PM2
#   ifdef V /* v kernel on pm2 */
#       define DCMBM(x)		(0x30000 + (x))
#   endif V
#   ifdef UNIX
#   	/* unix stuff on pm2 */
#       ifdef KERNEL
#           define DCMBM(x)	(0xEEE000 + (x))
#       else
#           define DCMBM(x)	(0xEF4000 + (x))
#       endif KERNEL
#   endif UNIX
#   ifdef PROMALONE
#       define DCMBM(x)	(0xF70000 + (x))
#   endif PROMALONE
#   ifdef GLUSER
#       define DCMBM(x)	(0xEF4000 + (x))
#   endif GLUSER
#endif PM2

#ifdef IP2
#define	DCMBM(x)	(SEG_MBIO + (x))
#endif

/* addressing constants  */

#define DCDEV		(1<<14)		/* address of the board 	*/
#define DCOFFSET	9

/*	DC4 Address Segment is only 512 Bytes				*/

#define DCFLAGS		(0<<DCOFFSET)
#define DCRAMRED	(1<<DCOFFSET)
#define DCRAMGRN	(2<<DCOFFSET)
#define DCRAMBLU	(3<<DCOFFSET)
#define DCADDR(x)	((x)<<1)



/* flag register  */

#ifdef DC3
#define DCBUSADRMAP	0x01	/* Map address lines 0..7 taken from	*/
				/*   multibus address lines 1..8	*/
#define DCREGADRMAP	0x02	/* Map address lines 8..11 are taken	*/
				/*   from DCflags 4..7			*/
#define DCRGBMODE	0x04	/* DACs are driven directly from the	*/
				/*   bitplane outputs, rather than from	*/
				/*   the colormap outputs		*/
				/*   Red:   a7 a6 a5 a4 a3 a2 a1 a0	*/
				/*   Green: b7 b6 b5 b4 b3 b2 b1 b0	*/
				/*   Blue:  d3 c3 d2 c2 d1 c1 d0 c0	*/
#define DCMAINT		0x08	/* Sets the maintenance LED		*/

#define DCREGADR0	0x10	/* Map read and written, and displayed	*/
#define DCREGADR1	0x20	/*   if DCREGADRMAP is asserted.	*/
#define DCREGADR2	0x40	/*   Range 0..15			*/
#define DCREGADR3	0x80	/* Map address lines 8..11		*/
#endif	DC3
				/* PALCTRL definitions same for DC3/4	*/
#define DCPALCTRL0	0x100	/* VFSM PAL control signals, range 0..7	*/
#define DCPALCTRL1	0x200	/*   Write only!			*/
#define DCPALCTRL2	0x400

#ifdef	DC4
#define DCMAINT		0

#define	DCREGADR0	0x01	/* DC4 signals got moved		*/
#define DCREGADR1	0x02
#define DCREGADR2	0x04
#define DCREGADR3	0x08
#define DCBUSADRMAP	0x10
#define DCREGADRMAP	0x20
#define DCRGBMODE	0x40

#define DCUPH		0x80	/* Added DC4 signal UPH (upper half)
				used to select which half of doubled double
				width color map is read back.		*/

#define DCDP00		0x80	/* UPH is WRITE Only, DP00 is oddline
				indicator from DPP state machine READ only */

#define	OPTCLK		0x800	/* Added DC4 signal Optional Clock, selects
				external or secound oscillator		*/

#define PIPE4		0x1000	/* Added DC4 signal used to specify the
				pipeline depth of the DC video shift
				registers				*/

#define PROM		0x2000	/* Added DC4 signal, used to specify
				which of two sets of video formats stored
				in the DPP state machine PROM is used	*/

#define D1K		0x4000	/* Added DC4 signal, allows the DC4 to free
				run, otherwise is externally synced using
				the START line				*/

#endif	DC4

/*	VFSM is same for DC4 and DC3					*/

#define DCVFSM0		0x0100	/* VFSM outputs - Read only!		*/
#define DCVFSM1		0x0200
#define DCVFSM2		0x0400
#define DCVFSM3		0x0800
#define DCVFSM4		0x1000
#define DCVFSM5		0x2000
#define DCVFSM6		0x4000
#define DCVFSM7		0x8000

/* mem space "constants"  */

#define DCflagsAddr	(short *)(DCMBM(DCDEV|DCFLAGS))
#define DCflags		*DCflagsAddr
#define DCramRedAddr(x)	(short *)(DCMBM(DCDEV|DCRAMRED|DCADDR(x)))
#define DCramGrnAddr(x)	(short *)(DCMBM(DCDEV|DCRAMGRN|DCADDR(x)))
#define DCramBluAddr(x)	(short *)(DCMBM(DCDEV|DCRAMBLU|DCADDR(x)))
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

/* dc4 configuration prom magic numbers */
#define DC4_PROMVAL0 0xf0
#define DC4_PROMVAL1 0xc5

/*
 * Read and write macros - must be preceded by flag setup and, in some
 *   cases, index massage.
 */
#ifdef 	DC2
#define DCMapColor(index,red,grn,blu)			\
	      { DCramRed (index) = (red) | ((red)<<8);	\
		DCramGrn (index) = (grn) | ((grn)<<8);	\
		DCramBlu (index) = (blu) | ((blu)<<8);	\
		}
#endif	DC2
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
 *	DC2:
 *		DCflags = DCBUSOP;
 *		mapindex = index | DCNumToIndex (mapnumber);
 *		DCMapColor (mapindex, red, green, blue)
 *		DCflags = DCMULTIMAP | DCNumToReg (mapnumber);
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
 */

#endif DCDEVDEF
