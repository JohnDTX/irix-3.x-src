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
 *	Definition for the DC2 and DC3 display controllers.
 */

#ifdef DC2
#define DCMBMEMLOWADR	0x100000		/* was in pcmap.h */
#define DCMBM(x)	(DCMBMEMLOWADR + (x))
#endif

#ifdef DC3
#define DCIOLOWADR	MBioVA		/* was in pcmap.h */
#define DCMBM(x)	(DCIOLOWADR + (x))
#endif

/* addressing constants  */

# ifndef DCDEV

#define DCDEV		(1<<14)		/* address of the board 	*/

#ifdef DC2
#define DCOFFSET	12
#endif

#ifdef DC3
#define DCOFFSET	9
#endif

#define DCFLAGS		(0<<DCOFFSET)
#define DCRAMRED	(1<<DCOFFSET)
#define DCRAMGRN	(2<<DCOFFSET)
#define DCRAMBLU	(3<<DCOFFSET)
#define DCADDR(x)	((x)<<1)



/* flag register  */

#ifdef DC2
#define DCREGADR0	0x01	/* Map address 8, except during busop	*/
#define DCREGADR1	0x02	/* Map address 9, except during busop	*/
#define DCBUSADRMAP	0x04	/* Map address lines 0..9 are taken	*/
				/*   from multibus address lines 1..10	*/
#define DCRGBMODE	0x08	/* DACs are driven directly from the	*/
				/*   bitplane outputs, rather than from	*/
				/*   the colormap outputs		*/
				/*   Red:   a7 a6 a5 a4 a3 a2 a1 a0	*/
				/*   Green: b7 b6 b5 b4 b3 b2 b1 b0	*/
				/*   Blue:  d3 d2 d1 d0 c3 c2 c1 c0	*/
#define DCBINSEL	0x10	/* Monochrome display control		*/
#define DCPALCTRL0	0x20	/* VFSM PAL control signals, range 0..7 */
#define DCPALCTRL1	0x40
#define DCPALCTRL2	0x80
#endif

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

#define DCPALCTRL0	0x100	/* VFSM PAL control signals, range 0..7	*/
#define DCPALCTRL1	0x200	/*   Write only!			*/
#define DCPALCTRL2	0x400
#endif

#define DCVFSM0		0x0100	/* VFSM outputs - Read only!		*/
#define DCVFSM1		0x0200
#define DCVFSM2		0x0400
#define DCVFSM3		0x0800
#define DCVFSM4		0x1000
#define DCVFSM5		0x2000
#define DCVFSM6		0x4000
#define DCVFSM7		0x8000

/* mem space "constants"  */

#define DCflagsAddr	((short *)(DCMBM(DCDEV|DCFLAGS)))
#define DCflags		(*DCflagsAddr)
#define DCramRedAddr(x)	((short *)(DCMBM(DCDEV|DCRAMRED|DCADDR(x))))
#define DCramGrnAddr(x)	((short *)(DCMBM(DCDEV|DCRAMGRN|DCADDR(x))))
#define DCramBluAddr(x)	((short *)(DCMBM(DCDEV|DCRAMBLU|DCADDR(x))))
#define DCramRed(x)	(*DCramRedAddr(x))
#define DCramGrn(x)	(*DCramGrnAddr(x))
#define DCramBlu(x)	(*DCramBluAddr(x))



/* miscellaneous definitions */

#ifdef DC2
#define DCMULTIMASK	0xff		/* index bits, not address bits	*/
#define DCMAPNUM	4		/* number of colormaps		*/
#endif

#ifdef DC3
#define DCMULTIMASK	0xff		/* multimap index bits		*/
#define DCSINGLEMASK	0xfff		/* singlemap index bits		*/
#define DCMAPNUM	16		/* number of colormaps		*/
#endif



/* flag settings - constants and macros */

#ifdef DC2
#define DCBUSOP		DCBUSADRMAP
#define DCMULTIMAP	0
#define DCRegToIndex(x)	(((x)<<8)&0x300)
#define DCIndexToReg(x)	(((x)>>8)&0x3)
#define DCNumToIndex(x)	(((x)<<8)&0x300)
#define DCIndexToNum(x)	(((x)>>8)&0x3)
#define DCNumToReg(x)	((x)&0x3)
#define DCRegToNum(x)	((x)&0x3)
#endif

#ifdef DC3
#define DCBUSOP		(DCBUSADRMAP | DCREGADRMAP)
#define DCSINGLEMAP	0
#define DCMULTIMAP	DCREGADRMAP
#define DCMaskIndex(x)	((x)&0xff)
#define DCRegToIndex(x)	(((x)<<4)&0xf00)
#define DCIndexToReg(x)	(((x)>>4)&0xf0)
#define DCNumToIndex(x)	(((x)<<8)&0xf00)
#define DCIndexToNum(x)	(((x)>>8)&0xf)
#define DCNumToReg(x)	(((x)<<4)&0xf0)
#define DCRegToNum(x)	(((x)>>4)&0xf)
#endif

/*
 * Read and write macros - must be preceded by flag setup and, in some
 *   cases, index massage.
 */

#define DCMapColor(index,red,grn,blu)			\
	      { DCramRed (index) = (red) | ((red)<<8);	\
		DCramGrn (index) = (grn) | ((grn)<<8);	\
		DCramBlu (index) = (blu) | ((blu)<<8);	\
		}
#define DCReadMap(index,red,grn,blu)		\
	      {	red = DCramRed (index);		\
		grn = DCramGrn (index);		\
		blu = DCramBlu (index);		\
		}

# endif  DCDEV



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

