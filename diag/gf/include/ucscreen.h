/*
 * ucscreen.h
 *
 *		Mark Grossman	11 Oct 84
 *		after betascreen.h
 */

/* Font parameters: */

#define	FONTBASEADDR		0xf400	/* 128*16 + 1K  */
#define CHARHEIGHT		0x10
#define CHARWIDTH		0x08
#define	CHARSPACING		0x08
#define	LOGBYTES		0x04
#define	NUMCHARS		0x92	/* includes circle  */

#define CharToAddr(ch)		( (ch<<LOGBYTES) + FONTBASEADDR )

#define POLYSTIPCHARNUM		0x01
#define ERASECHARNUM		0x01
#define	CURSORCHARNUM		0x8e
#define POLYSTIPCHARADDR	CharToAddr(POLYSTIPCHARNUM)
#define ERASECHARADDR		CharToAddr(ERASECHARNUM)
#define CURSORCHARADDR		CharToAddr(CURSORCHARNUM)

#define LOGOWIDTH		40
#define LOGOHEIGHT		40
#define LOGOWIDTH_HEIGHT	0x2828
#define LOGOCHARADDR		FONTBASEADDR + 2048
					/* sizeof char font (logo at end)
					 */

extern short	MapChar[];		/* The character masks. */

/* IRIS screen parameters: */

extern int xmax,ymax;

/* Configuration parameters: */

#define UCMODE			1
#define	CONFIG			0xdf
#define	OFFCODE			0x0000	/* bit plane data	*/
#define ONCODE			0x0001	/* bit plane data	*/
#define IRISCODE		0x0002	/* bit plane data	*/
#define BLACK			0x0000	/* color map entry	*/
#define	WHITE			0xffff	/* color map entry	*/
#define IRISR			0x8888
#define IRISG			0x6666
#define IRISB			0x8888
#define	CHARWE			0x0001
#define BLACKWE			0xffff
#define IRISWE			0xffff
