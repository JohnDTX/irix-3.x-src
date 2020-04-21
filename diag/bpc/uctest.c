/*
 *	Kurt Akeley			7/17/82
 *
 *	These routines are intended to be used in several BPC and
 *	  BPM test programs.  They are general purpose in nature.
 *
 *	Functions:
 *		boolean	writefont (fontaddr, value)
 *		short	readfont (address)
 *		boolean isreadcommand (command)
 *
 *	Procedures:
 *		setcodes (colorcode, wecode)
 *		clear (fontaddr)
 *		setstipple (fontaddr, stipple)
 *		writeword (x, y, value)
 *		readpixels (pixels, x, y, sigplanes)
 *		cmplword (x, y)
 *
 *	Updated 9/10/82  KBA	Modified for the version 2 bpc
 *	Updated 10/16/82 KBA	Sequence of color command changed
 *	Updated 11/2/82  KBA	Speed improvements
 *	Updated 2/21/83  KBA	UC3 mods
 */

#include "console.h"
#include "uctest.h"
#include "ucdev.h"

#define boolean		short
#define TEMPFONTADDR	0	/* Base address in the font memory for	*/
				/*   16 byte temporary storage.  Used	*/
				/*   by setcode, etc.			*/
#define WORDMASKFONTADDR 0x1000 /* Base address in the font memory fot	*/
				/*   256 bytes used by wordwrite	*/

boolean	writefont (fontaddr, value)
long	fontaddr;
short	value;
{
    /*
     *	The value is written into the font memory at the specified
     *	  address.
     */
    short	readback;

    LDFMADDR (fontaddr)
#ifdef UC3
    REQUEST (WRITEFONT, value)
#endif UC3
#ifdef UC4
    REQUEST (UC_WRITEFONT_SETADR, value);
#endif UC4
    return TRUE;
    }

short		readfont (address)
long		address;
{
    /*
     *	Returns the contents of the specified font memory address.
     */
#ifdef UC3
    LDFMADDR (address)
    REQUEST (LOADXYADDR, 0)
    REQUEST (LOADXYADDR, 0)	/* insure synchronization */
    return (RDFONT & 0xff);
#endif UC3
#ifdef UC4
    LDFMADDR (address);
    return *UCCommandAddr(UC_READFONT_SETADR);
#endif UC4
    }

setcodes (colorcode, wecode)
long	colorcode;	/* 0..ffffffff, known on BPM as color */
long	wecode;		/* 0..ffffffff, enable or disable plane */
			/* A planes bits 0...7,  B planes bits 8...15 */
			/* C planes bits 16..23, D planes bits 24..31 */
{
    /*
     *	  This routine sets the codes and write enables
     *	  for all planes as per its arguments.
     */
#ifdef	UC3
    writefont (TEMPFONTADDR  , colorcode);
    writefont (TEMPFONTADDR+1, colorcode>>8);
    writefont (TEMPFONTADDR+2, colorcode>>16);
    writefont (TEMPFONTADDR+3, colorcode>>24);
    writefont (TEMPFONTADDR+4, wecode);
    writefont (TEMPFONTADDR+5, wecode>>8);
    writefont (TEMPFONTADDR+6, wecode>>16);
    writefont (TEMPFONTADDR+7, wecode>>24);
    LDFMADDR (TEMPFONTADDR)
    REQUEST (LOADCODES, 0)
#endif UC3
#ifdef UC4
    short s_mdb;
    s_mdb = uc_mdb;
    LDMODE (s_mdb & ~(UC_SWIZZLE|UC_DOUBLE));
    *((long*)(UCCommandAddr(UC_SETCOLORCD))) = colorcode;
    *((long*)(UCCommandAddr(UC_SETWECD))) = wecode;
    LDMODE (s_mdb);
#endif UC4
    }



clear (fontaddr)
short	fontaddr;	/* font address of the stipple pattern */
{
    /*
     *	The entire 1024 x 1024 memory is cleared to the present code
     *    with the stipple pattern (which must already exist in the font
     *	  memory at the specified address).
     *	Note that neither color/we nor configuration are changed.
     */
    Save s;
    save (&s);
    LDXS (0)
    LDYS (0)
    LDXE (1023)
    LDYE (1023)
    LDFMADDR (fontaddr)
    REQUEST (CLEAR, 0)
    restore (&s);
    }



setstipple (fontaddr, stipple)
long	fontaddr;		/* font address of first stipple byte */
short	stipple[];		/* array of 16 stipple bytes/shorts */
{
    /*
     *	The contents of the specified stipple array are loaded into the
     *    font memory starting at the specified address.
     */
    long	endaddr;

    for (endaddr=fontaddr+16; fontaddr<endaddr;)
	writefont (fontaddr++, *stipple++);
    }



writeword (x, y, value)
short	x;		/* x word address, 0..63 */
short	y;		/* y line address, 0..1023 */
short	value;		/* number to be written */
{
    /*
     *	The value is written into the specified word with numeric
     *    and memory bits aligned.  That is, bit 0 to the left
     *    (as viewed on the screen), and bit 15 to the right.
     *    It is assumed that the word was previously cleared to
     *    the "zero color".
     */

    short s_cfb;
    short s_mdb;
    x <<= 4;
    s_cfb = uc_cfb;
#ifdef UC3
    LDCONFIG ((s_cfb & ~(BACKLINE)) | LDLINESTIP);
#endif UC3
#ifdef UC4
    s_mdb = uc_mdb;
    LDCONFIG ((uc_cfb & ~(UC_FINISHLINE|UC_INVERT)) | UC_LDLINESTIP);
    LDMODE (0);
    LDREPEAT (0);
#endif UC4
    LDXS (x);
    LDXE (x+15);
    LDYS (y);
    LDED (0);
    LDEC (0);
    REQUEST (DRAWLINE2, value);
    LDCONFIG (s_cfb);
#ifdef UC4
    LDMODE (s_mdb);
#endif UC4
    }

readpixels (pixels, x, y, sigplanes)
long	*pixels;	/* array of 16 32-bit words */
short	x;		/* x word address, 0..63 */
short	y;		/* y line address, 0..1023 */
long	sigplanes;
{
    /*
     *	Pixel readback is used to return the value of the specified
     *    word.  All 32 bits of all 16 pixels in that word are returned
     *	  in the array "pixels".  pixels[0] contains bit 0 of each of
     *	  the 32 planes.
     */
#ifdef UC3
    short	*low, *high;	/* pointers to low and high 16-bit	*/
				/*   portions of each pixel int		*/
    short	i;
    register long	abconfig, cdconfig;
    short s_cfb;

    s_cfb = uc_cfb;
    high = (short *)pixels;
    low = ((short *)pixels) + 1;
    abconfig = (s_cfb & ~ENABLECD) | UPDATEA | UPDATEB;
    cdconfig = s_cfb | ENABLECD | UPDATEA | UPDATEB;
    LDXS (x<<4)
    LDYS (y)
    REQUEST (READWORD, 0)
    
    for (i=0; i<16; i++) {
/*	REQUEST (LOADXYADDR, 0)		   insure synchronization */
	*low = RDPIXEL;
	LDCONFIG (cdconfig)
	REQUEST (LOADXYADDR, 0)
/*	REQUEST (LOADXYADDR, 0) 	   insure synchronization */
	*high = RDPIXEL;
	*(long*)high &= sigplanes; 	/* added August 84, KBA */

	LDCONFIG (abconfig);
	REQUEST (LOADXYADDR, 0);
	low += 2;
	high += 2;
	REQUEST (ROTATEWORD, 0)
	}
    LDCONFIG (s_cfb);
#endif UC3
#ifdef UC4
    register i;
    short s_cfb, s_mdb;
    s_cfb = uc_cfb;
    s_mdb = uc_mdb;
    LDCONFIG (s_cfb | UPDATEA | UPDATEB | UC_PFIREAD);
    LDMODE (0);
    x <<= 4;
    LDXS (x);
    LDXE (x+15);
    LDYS (y);
    REQUEST (UC_SETADDRS, 0);
    for (i=0; i<16; i++) {
	*pixels++ = *((long*)UCCommandAddr(UC_READPIXELCD)) & sigplanes;
	}
    LDCONFIG (s_cfb);
    LDMODE (s_mdb);
#endif UC4
    }



cmplword (x, y)
short	x;		/* word x address, 0..63 */
short	y;		/* word y address, 0..1023 */
{
    /*
     *	The specified word in the BPM memory is bit complemented.
     *    This is accomplished by first reading it into the on board
     *    register, then rotating it 16 steps (it recirculates inverted),
     *	  and then rewriting it from this register.
     */
    short i;
    short s_cfb;

    x <<= 4;
    LDXS (x)
    LDYS (y)
    REQUEST (READWORD, 0)
#ifdef UC3
    REQUEST (ROTATEWORD, 0)	/* sixteen rotatewords */
    REQUEST (ROTATEWORD, 0)
    REQUEST (ROTATEWORD, 0)
    REQUEST (ROTATEWORD, 0)
    REQUEST (ROTATEWORD, 0)
    REQUEST (ROTATEWORD, 0)
    REQUEST (ROTATEWORD, 0)
    REQUEST (ROTATEWORD, 0)
    REQUEST (ROTATEWORD, 0)
    REQUEST (ROTATEWORD, 0)
    REQUEST (ROTATEWORD, 0)
    REQUEST (ROTATEWORD, 0)
    REQUEST (ROTATEWORD, 0)
    REQUEST (ROTATEWORD, 0)
    REQUEST (ROTATEWORD, 0)
    REQUEST (ROTATEWORD, 0) 
    REQUEST (WRITEWORD, 0)
#endif UC3
#ifdef UC4
    s_cfb = uc_cfb;
    LDCONFIG (s_cfb | UPDATEA | UPDATEB | UC_INVERT);
    REQUEST (WRITEWORD, 0xffff);
    LDCONFIG (s_cfb);
#endif UC4
    }



long	planeword (plane, pixels)
short	plane;		/* 0..31 */
long	*pixels;	/* array of 16 pixels */
{
    /*
     *	Returns the plane word made up of the sixteen pixels stored
     *	  one bit each in the 16 ints in "pixels".  The leftmost
     *    pixel bit (as viewed on the screen) becomes the low order
     *    bit of the returned word.  Thus the subroutine sequence
     *    "writeword", "readpixels", "planeword" is consistent; the
     *    value written is equal to the value returned.
     */

    register long word;
    register long pixelmask;
    register long wordmask;

    for (word=0, pixelmask=(1<<plane), wordmask=1;
		wordmask&0xffff;
		wordmask<<=1)
	if (*pixels++ & pixelmask)
	    word |= wordmask;
    return (word);
    }

isreadcommand (command)
short command;
{
#ifdef UC3
    return FALSE;
#endif UC3
#ifdef UC4
    switch (command) {
	case UC_READFONT:
	case UC_READREPEAT:
	case UC_READLSTIP:
	case UC_READPIXELAB:
	case UC_READPIXELCD:
	    return TRUE;
	default:
	    return FALSE;
	}
#endif UC4
    }
