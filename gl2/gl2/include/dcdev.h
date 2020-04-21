#ifndef DCDEVDEF
#define DCDEVDEF
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

/*
 *	Definition for the DC4 display controller.
 */

#include "addrs.h"

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

#define DCRegToIndex(x)	(((x)<<8)&0xf00)
#define DCIndexToReg(x)	(((x)>>8)&0xf)
#define DCNumToIndex(x)	(((x)<<8)&0xf00)
#define DCIndexToNum(x)	(((x)>>8)&0xf)
#define DCNumToReg(x)	((x)&0xf)
#define DCRegToNum(x)	((x)&0xf)

/*
 * Read and write macros - must be preceded by flag setup and, in some
 *   cases, index massage.
 */
#define DCMapColor(index,red,grn,blu)			\
	      { DCramRed(index) = (red);		\
		DCramGrn(index) = (grn);		\
		DCramBlu(index) = (blu);		\
		}

#define DCReadMap(index,red,grn,blu)		\
	      {	red = DCramRed(index);		\
		grn = DCramGrn(index);		\
		blu = DCramBlu(index);		\
		}

/*
 *	Examples of writes to a single colormap rgb location
 *
 *	DC4 multimode:
 *		DCflags = DCBUSOP | DCNumToReg (mapnumber);
 *		DCMapColor (index, red, green, blue)
 *		DCflags = DCMULTIMAP | DCNumToReg (mapnumber);
 *
 *	DC4 singlemode:
 *		DCflags = DCBUSOP | DCIndexToReg (index);
 *		mapindex = index & DCMULTIMASK;
 *		DCMapColor (mapindex, red, green, blue);
 *		DCflags = DCSINGLEMAP;
 *
 */
#endif DCDEVDEF
