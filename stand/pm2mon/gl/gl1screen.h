/*
 * screen.h
 *
 *		Mark Grossman	4 Apr 83
 */

/* IRIS commands utilized in this module: */

#define	BLOCKFILLCMD		0x39
#define	POINTCMD		0x12
#define COLORCMD		0x14
#define WRTENCMD		0x15
#define	CONFIGCMD		0x16
#define	LOADMASKSCMD		0x17
#define	VIEWPORTCMD		0x18
#define	CHARPOSITIONCMD		0x1a
#define	DRAWCHARCMD		0x1c
#define	SELECTCURSORCMD		0x1d
#define	DRAWCURSORCMD		0x1e
#define	UNDRAWCURSORCMD		0x1f
#define	POLYSTIPPLECMD		0x21
#define MOVEPOLYCMD		0x30
#define DRAWPOLYCMD		0x31
#define CLOSEPOLYCMD		0x33
#define FIXCHARLOADCMD		0x2c
#define FIXCHARDRAWCMD		0x2d

/* Font parameters: */

#define	FONTBASEADDR		0xf400	/* 128*16 + 1K  */
#define	LOGBYTES		0x04

#define CharToAddr(ch)		( (ch<<LOGBYTES) + FONTBASEADDR )

#define POLYSTIPCHARNUM		0x01
#define ERASECHARNUM		0x01
#define	CURSORCHARNUM		0x8e

#define LOGOWIDTH		40
#define LOGOHEIGHT		40
#define LOGOWIDTH_HEIGHT	0x2828
#define LOGOCHARADDR		FONTBASEADDR + 2048
					/* sizeof char font (logo at end)
					 */

/* Configuration parameters: */

#define	CONFIGPARAM		0xdf
#define BLACK			0x0000	/* color map entry	*/
#define WHITE			0xFFFF
