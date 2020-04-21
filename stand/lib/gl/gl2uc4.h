/*
 * $Source: /d2/3.7/src/stand/lib/gl/RCS/gl2uc4.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:15:13 $
 */
#ifndef UC4DEF
#define UC4DEF
/*
 *	Kurt Akeley			2/17/83
 *
 *	Definitions for the UC3 and UC4 boards (UC2 is extinct).
 */

/* multibus port constants */

#ifdef UC4
#   ifdef PM1
#       define UCIOLOWADR	0x1f0000
#   endif PM1
#   ifdef PM2
#	ifdef V		/* v kernel on pm2 */
#           define UCIOLOWADR	0x30000
#	else		/* unix on pm2 */
#           define UCIOLOWADR	0xf70000
#	endif
#   endif PM2
#   ifdef IP2
#           define UCIOLOWADR	SEG_MBIO
#   endif IP2
#   define UCDEV		(0x3000)
#   define UCMBM(x)	(UCIOLOWADR | UCDEV | (x))
#endif UC4

/* command numbers for fbc interface (there is no multibus interface) */
#ifdef UC3
#define LOADVIEWPORT	0x00
#define WRITEFONT	0x01
#define LOADCODES	0x02
#define LOADXYADDR	0x03
#define READWORD	0x04
#define WRITEWORD	0x05
#define ROTATEWORD	0x06
#define NOOP		0x07
#define WRITEPIXEL	0x08
#define DRAWCHAR	0x09
#define FILLRECT	0x0a
#define CLEAR		0x0b
#define DRAWLINE1	0x0c	/* The n in DRAWLINEn specifies the hour */
#define DRAWLINE2	0x0d	/*    on a clock face which corresponds  */
#define DRAWLINE4	0x0e	/*    to the octant into which the line  */
#define DRAWLINE5	0x0f	/*    is drawn.	 Thus DRAWLINE1 draws    */
#define DRAWLINE7	0x1f	/*    steeply up and to the right, while */
#define DRAWLINE8	0x1e	/*    DRAWLINE8 draws sharply to the	 */
#define DRAWLINE10	0x1d	/*    left and down.			 */
#define DRAWLINE11	0x1c
#endif UC3

#ifdef UC4
/* command numbers for both fbc and multibus interfaces */
#define UC_READFONT	0x00
#define UC_WRITEFONT	0x01
#define UC_READREPEAT	0x02
#define UC_SETADDRS	0x03
#define UC_SAVEWORD	0x04
#define UC_DRAWWORD	0x05
#define UC_READLSTIP	0x06
#define UC_NOOP		0x07
#define UC_DRAWCHAR	0x09
#define UC_FILLRECT	0x0a
#define UC_FILLTRAP	0x0b
#define UC_DRAWLINE1	0x0c
#define UC_DRAWLINE2	0x0d
#define UC_DRAWLINE4	0x0e
#define UC_DRAWLINE5	0x0f
#define UC_SETSCRMASKX	0x10
#define UC_SETSCRMASKY	0x11
#define UC_SETCOLORCD	0x14
#define UC_SETCOLORAB	0x15
#define UC_SETWECD	0x16
#define UC_SETWEAB	0x17
#define UC_READPIXELCD	0x18
#define UC_READPIXELAB	0x19
#define UC_DRAWPIXELCD	0x1a
#define UC_DRAWPIXELAB	0x1b
#define UC_DRAWLINE11	0x1c
#define UC_DRAWLINE10	0x1d
#define UC_DRAWLINE8	0x1e
#define UC_DRAWLINE7	0x1f
/* force address transfer, multibus only */
#define UC_MBSETADR		UCBIT(5)  /* will be shifted 1 to left */
#define UC_READFONT_SETADR	(UC_MBSETADR | UC_READFONT)
#define UC_WRITEFONT_SETADR	(UC_MBSETADR | UC_WRITEFONT)
#define UC_READPIXELAB_SETADR	(UC_MBSETADR | UC_READPIXELAB)
#define UC_READPIXELCD_SETADR	(UC_MBSETADR | UC_READPIXELCD)
#define UC_DRAWPIXELAB_SETADR	(UC_MBSETADR | UC_DRAWPIXELAB)
#define UC_DRAWPIXELCD_SETADR	(UC_MBSETADR | UC_DRAWPIXELCD)
#endif UC4

#define UCBIT(x)	(1<<(x))

#ifdef UC3
/* configuration register (CFR) bits */
#define DISPLAYA	UCBIT(0)
#define DISPLAYB	UCBIT(1)
#define UPDATEA		UCBIT(2)
#define UPDATEB		UCBIT(3)
#define VIEWPORTMASK	UCBIT(4)
#define ENABLECD	UCBIT(5)
#define BACKLINE	UCBIT(6)
#define LDLINESTIP	UCBIT(7)
#endif UC3

#ifdef UC4
/* configuration register (CFR) bits */
#define UC_DISPLAYA	UCBIT(0)
#define UC_DISPLAYB	UCBIT(1)
#define UC_UPDATEA	UCBIT(2)
#define UC_UPDATEB	UCBIT(3)
#define UC_SCREENMASK	UCBIT(4)
#define UC_INVERT	UCBIT(5)
#define UC_FINISHLINE	UCBIT(6)
#define UC_LDLINESTIP	UCBIT(7)
#define UC_PFICD	UCBIT(8)
#define UC_PFIREAD	UCBIT(9)
#define UC_PFICOLUMN	UCBIT(10)
#define UC_PFIXDOWN	UCBIT(11)
#define UC_PFIYDOWN	UCBIT(12)
#define UC_ALLPATTERN	UCBIT(13)
#define UC_PATTERN32	UCBIT(14)
#define UC_PATTERN64	UCBIT(15)
/* mode register (MDR) bits */
#define UC_SWIZZLE	UCBIT(0)
#define UC_DOUBLE	UCBIT(1)
#define UC_DEPTHCUE	UCBIT(2)
/* for backward compatibility */
#define DISPLAYA	UC_DISPLAYA
#define DISPLAYB	UC_DISPLAYB
#define UPDATEA		UC_UPDATEA
#define UPDATEB		UC_UPDATEB
#define VIEWPORTMASK	UC_SCREENMASK
#define BACKLINE	UC_FINISHLINE
#define LDLINESTIP	UC_LDLINESTIP

/* buffer numbers */
#define UC_EDB		0x01
#define UC_ECB		0x02
#define UC_XSB		0x03
#define UC_XEB		0x04
#define UC_YSB		0x05
#define UC_YEB		0x06
#define UC_FMAB		0x07
#define UC_DDASAF	0x08
#define UC_DDASAI	0x09
#define UC_DDAEAF	0x0a
#define UC_DDAEAI	0x0b
#define UC_DDASDF	0x0c
#define UC_DDASDI	0x0d
#define UC_DDAEDF	0x0e
#define UC_DDAEDI	0x0f
#define UC_MDB		0x10
#define UC_RPB		0x11
#define UC_CFB		0x12

/* Multibus update register (UCR) bits */
/*#define UCR_BOARDENAB	UCBIT(8)	/* read/write */
#define UCR_BOARDDISAB	UCBIT(8)	/* read/write */
#define UCR_MBENAB	UCBIT(9)	/* read/write */
#define UCR_INTRENAB	UCBIT(10)	/* read/write */
#define UCR_DMAENAB	UCBIT(11)	/* read/write */
#define UCR_ZERO	UCBIT(12)	/* read only */
#define UCR_VERTICAL	UCBIT(13)	/* read only */
#define UCR_VERTINTR	UCBIT(14)	/* read only */
#define UCR_BUSY	UCBIT(15)	/* read only */

/* write all 17 buffers, read only UC_DDA*A* */
#define UCBufferAddr(buffer)	(short*)(UCMBM(0x80|((buffer)<<1)))

/* UC multibus register (UCR), read and write */
#define UCRAddr			(short*)(UCMBM(0x180))

/* execute a command, some read, some write */
#define UCCommandAddr(command)	(short*)(UCMBM(0x200|((command)<<1)))

/* specify the dma command, write only */
#define UCDMAInitAddr(command)	(short*)(UCMBM|((command)<<1))
#endif UC4

#endif UC4DEF
