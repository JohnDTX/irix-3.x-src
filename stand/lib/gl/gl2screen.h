/*
 * $Source: /d2/3.7/src/stand/lib/gl/RCS/gl2screen.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:15:12 $
 */
/*
 * screen.h
 *
 *		Mark Grossman	11 Oct 84
 *		after cmds.h
 */

/* Font parameters: */

#define	FONTBASEADDR		0xf400	/* 128*16 + 1K  */
#define	CHARSPACING		0x08
#define	LOGBYTES		0x04

#define CharToAddr(ch)		( ((ch&0xff)<<LOGBYTES) + FONTBASEADDR )

#define POLYSTIPCHARNUM		0x01
#define ERASECHARNUM		0x01
#define	CURSORCHARNUM		0x8C

/* Configuration parameters: */

#define UCMODE			1
#define	CONFIGPARAM		0xdf
