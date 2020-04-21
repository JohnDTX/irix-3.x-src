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
#include "sys/types.h"
#include "shmem.h"
#include "printf.h"
#include "window.h"
#include "kfont.h"
#include "dcdev.h"
#include "uc4.h"
#include "immed.h"

/* XXX add me to shmem.h */
long gl_planes;
short gl_numreserved;

#undef	DEBUG

/* configuration parameters for setmode */
#define	CONFIGBOTH	0x100df

/* configuration table for ge's */

short	geconfigtab[] = {
	0x00,	0x9,	0xa,	0xb,	0xc,
		0x15,	0x10,	0x11,	0x12,
		0x13,	0x14,	0x20,	0x21,
	0x08
};

/* default fill texture */
short	getexture[] = {
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 
};

/* default cursor */
short	cursormask[] = {
	0x0300, 0x0300, 0x0600, 0x0600, 
	0x0C00, 0x8C00, 0xD800, 0xF800, 
	0xFF00, 0xFE00, 0xFC00, 0xF800, 
	0xF000, 0xE000, 0xC000, 0x8000,
};

#define defpattern(bitmap)  gl_loadmasks(FR_DEFPATTERN,bitmap,16)
#define defcursor(bitmap)  gl_loadmasks(FR_DEFCURSOR,bitmap,16)

/* default color map */
struct	colormap {
	short	red, green, blue;
} colormap[] = {
	{ 000, 000, 000 },		/* 0: BLACK */
	{ 255, 000, 000 },		/* 1: RED */
	{ 000, 255, 000 },		/* 2: GREEN */
	{ 255, 255, 000 },		/* 3: YELLOW */
	{ 000, 000, 255 },		/* 4: BLUE */
	{ 255, 000, 255 },		/* 5: MAGENTA */
	{ 000, 255, 255 },		/* 6: CYAN */
	{ 255, 255, 255 },		/* 7: WHITE */
};
#define	NCOLORS		sizeof(colormap) / sizeof(struct colormap)

#define	xpassthru(x)	*GE = (((x) << 8) | 0x08)
#define	PASSTHRU(x)	((((x) - 1) << 8) | 0x08)

#define	LGE	((long *)GE)		/* long version of GE */
#define	GEINT	0x17000000		/* used to convert ints to floats */
#define	GEZERO	0x00000000		/* one long's worth of zeros */

#ifdef IP2
#	define BUZZ(x) {register long i; for(i=0; i<x; i++);}
#else
#	define BUZZ(x)
#endif

/* return a long word with 1 bit on for every existing plane	*/

long
gl_getplaneinfo()
{
    im_setup;
    register long cd, ab;
    register short i;
    long pri;

    /* Set up configuration to write a pixel: */
    setconfig((UC_SWIZZLE<<16) | UPDATEA | UPDATEB);

    /* Set writemask and color to all ones: */
    im_passcmd(4, FBCrgbwrten);
    im_outshort(0xffff);
    im_outlong(0xffffffff);
    im_passcmd(4, FBCrgbcolor);
    im_outshort(0xffff);
    im_outlong(0xffffffff);

    /* Write a single pixel: */
    im_passcmd(3, FBCpoint);
    im_outshort(0x100);
    im_outshort(0x100);

    im_passcmd(4, FBCcharposnabs);
    im_outshort(GEpoint);
    im_outshort(0x100);
    im_outshort(0x100);
    pri = spl7();
    GEflags = gl_gestatus | ENABFBCINT_BIT_;	/* turn off fbc interrupts */
    /* Read back the pixel and return to AB mode: */
    im_passcmd(8, FBCpixelsetup);
    im_outlong(0x303ff);
    im_outlong((FBCreadpixels<<16) | 1);
    im_outshort(FBCpixelsetup);
    im_outlong(0x103ff);
    while(FBCflags & INTERRUPT_BIT_) ;		/* wait for interrupt bit */
    BUZZ(100);
    FBCflags = READOUTRUN;
    BUZZ(100);
    if(!(FBCdata == _INTPIXEL32))
	panic("something wrong with intpixel32!!");
    BUZZ(100);
    FBCflags = gl_fbcstatus;
    BUZZ(100);
    FBCclrint;					/* cmd */
    BUZZ(100);
    FBCclrint;					/* count */
    BUZZ(100);
    cd = FBCdata;	FBCclrint;
    BUZZ(100);
    ab = FBCdata;	FBCclrint;
    BUZZ(100);
    GEflags = gl_gestatus;
    splx(pri);
    im_cleanup;
    return((cd<<16) | ab);
}

reservebits(num)
short num;
{
    gl_numreserved = num;
    setbitplanemasks(0);
}

setbitplanemasks(mask)
long mask;
{
static long oldmask;
register long numbitplanes;
register short i, usablebitplanes, maxusableplanes;

	if(mask)
	    oldmask = mask;
	else
	    mask = oldmask;
	if(gl_dcr & DCRGBMODE)
	    maxusableplanes = 24;
	else if(gl_dcr & DCMULTIMAP)
	    maxusableplanes = 8;
	else
	    maxusableplanes = 12;
	numbitplanes = 0;
	for(i=0; i<32; i++, mask>>=1)
	    if(mask & 1)
		numbitplanes++;
	    else
		break;
	if(numbitplanes >= maxusableplanes)
	    usablebitplanes = maxusableplanes;
	else
	    usablebitplanes = numbitplanes;
	gl_kwritemask = (1 << usablebitplanes) - 1;
	gl_userwritemask = (1 << (usablebitplanes - gl_numreserved)) - 1;
	numbitplanes >>= 1;
	if(numbitplanes >= maxusableplanes)
	    usablebitplanes = maxusableplanes;
	else
	    usablebitplanes = numbitplanes;
	gl_kdbwritemask = (1 << usablebitplanes) - 1;
	gl_userdbwritemask = (1 << (usablebitplanes - gl_numreserved)) - 1;
	updatebitmasks();
}

void
greset()
{
	ginit();
}

void
ginit()
{
	fbcge_reset();
	loadcolormap();
	setfontbaseaddr(0);
	cursoff();
	setbitplanemasks(gl_planes = gl_getplaneinfo());

	/* init font ram stuff (order is important) */
	defpattern(getexture);
	setpattern();
	defcursor((char *)cursormask);

	/*
	 * Initialize cursor.  Make second cursor invisible so that it doesn't
	 * display until the user initializes it.
	 */
	{
		im_setup;

		/*
		 * Handle two color cursor
		 */
		im_passcmd(8, FBCloadram);
		im_outshort(84 /*_SECONDCURSOR*/); /* ucode/include/consts.h */
		im_outlong((5 << 16) | 0);
		im_outlong(0);			/* make it black */
		im_last_outlong(0);		/* disable writing */
	}

	gl_cursorxorgin = 0;
	gl_cursoryorgin = 16;
	curson(1);
	defrasterfont(0, gl_charheight, defont_nc, defont_font, defont_nr,
		defont_bits);

	gl_displayab = DISPLAYA | DISPLAYB;
	gl_blankmode = 0;
	gl_isblanked = 0;
	setmode(CONFIGBOTH);
	gl_init_dcr();
   	mapblack();

	/* setup defaults */
	linewidth(1);
	linestyle(0xFFFF);

	/* clear entire screen */
	gl_rgbtxport();
	    viewport(0, XMAXSCREEN, 0, YMAXSCREEN);
	    ortho2i();
	    color(0);
	    rectfi(0, 0, XMAXSCREEN, YMAXSCREEN);
	gl_sbtxport();

	/* set writemask to appropriate state */
	writemask(gl_kwritemask);
}

/*
 * geconfigure:
 *	- reset the pipe then,
 *	- configure the ge's according to the ones we found in gefind()
 */
geconfigure()
{
	im_setup;
	register short masksave = gemask;

	(void) gereset();
	(void) gereset();

    /* fifo chip with high water mark at 60, and low water at 0 */
#ifdef PM2
	im_outshort(0x3C00);
#endif
#ifdef IP2
	im_outshort(0x3800);
#endif

	gemask = gemask & ~1;	/* don't config tail cfp yet */
	justconfigure(geconfigtab);
	gemask = masksave;

    /* set water marks for fifo chip at end of pipe */
	im_outshort(0x3C02);
	if (gemask&1) im_outshort(geconfigtab[13]);

	im_last_outshort(GEclearhitmode);
	im_cleanup;
}

/*
 * justconfigure:
 *	- configure the ge's according to the ones we found in gefind()
 *	- also configures CFP's if found
 */
justconfigure(table)
register short *table;
{
	im_setup;
	register short mask, found, temp;
	extern short gefound, gemask;

	mask = gemask;
	found = gefound;

	for (temp = 0; temp < 14; temp++) {
		if ((mask <<= 1) & 0x4000) {
			im_outshort((--found<<8) | *(table+temp));
		}
	}
	im_cleanup;		/* no last out here ok? */
}

/*
 * setmode:
 *	- change the mode of display being used
 */
setmode(newmode)
	long newmode;
{
	gl_cfr = newmode;
	setconfig(gl_cfr);
}

/*
 * loadcolormap:
 *	- the color map is loaded into map 0, with the standard iris
 *	  default colors
 */
loadcolormap()
{
	register struct colormap *cp;
	register short i;

	DCflags = DCBUSOP;
	for (i = 0, cp = &colormap[0]; i < NCOLORS; i++, cp++)
		DCMapColor(i, cp->red, cp->green, cp->blue);
	DCflags = DCMULTIMAP;
	*UCRAddr = 0;	/* turn on UC4 */
}

mapblack()
{
	register short i;
	register short maxcolor;

	maxcolor = (gl_planes & 0xff)+1;
	DCflags = gl_dcr | DCBUSOP;
	for (i = NCOLORS; i<maxcolor; i++) 
		DCMapColor(i, 0, 0, 0);
	DCflags = gl_dcr;
}

/*
 * mapcolor:
 *	- assign a particular color table entry the given r,g,b values
 */
void
mapcolor(index, r, g, b)
	register short index;
	register u_char r, g, b;
{
    if (gl_dcr & DCMULTIMAP) {
	DCflags = gl_dcr | DCBUSOP;
    } else {
	DCflags = gl_dcr | DCBUSOP | DCIndexToReg (index);
	index &= DCMULTIMASK;
    }
    DCMapColor (index, r, g, b);
    DCflags = gl_dcr;
}

void
getmcolor (index, red, green, blue)
register Colorindex index;		/* color index to be remapped */
register short *red, *green, *blue;	/* component intensities, 0..ff */
{
    /*
     *	Index is remapped to the specified color component intensities.
     *    DC3 has the property that indexes are invarient across mode
     *    changes.  DC2 has this property forced on it by the requirement
     *	  that at least 2 boards are present during singlebuffer mode
     *    operation.
     *  This routine returns the intensity components stored in the hardware
     *    colormap at the specified index.  If in multimap mode,
     *	  contents of the current map are returned.
     */

    if (gl_dcr & DCMULTIMAP) {
	DCflags = gl_dcr | DCBUSOP;
    }
    else {
	DCflags = gl_dcr | DCBUSOP | DCIndexToReg (index);
	index &= DCMULTIMASK;
    }
    DCReadMap (index, *red, *green, *blue);
    *red &= 0xff;
    *green &= 0xff;
    *blue &= 0xff;
    DCflags = gl_dcr;
}

gl_getcmmode()
{
    if(gl_dcr & DCMULTIMAP)
	return 0;		/* CMAPMULTI */
    else
	return 1;		/* CMAPONE */
}


/*
 * setfontbaseaddr:
 *	- set the offset the fbc will add to all fontram addresses
 */
setfontbaseaddr(x)
register ushort x;
{
	im_setup;

	im_passcmd(2, FBCbaseaddress);
	im_last_outshort(x);
}

/*
 * pixelinit:
 *	- initial harmless setup for pixel readback
 */
pixelinit(bits)
long bits;
{
	im_setup;

	im_passcmd(3, FBCpixelsetup);
	im_last_outlong(bits);
}

/*
 * gl_WaitForEOF:
 *	- wait for the pipe to clear
 */
gl_WaitForEOF(sendcmd)
short sendcmd;
{
    im_setup;
    register short i;
    register long passes;
    register struct shmem *sh = gl_shmemptr;

    if(sendcmd) {
	passes = 0x80008;

    /* pipe only holds about 140 goobies.  when all these passthrus */
    /* have been stuffed down the pipe,  eof must have been reached */
	for(i=0; i<19; i++) {		/* rocky and paul were here */
	    im_outlong(passes);
	    im_outlong(passes);
	    im_outlong(passes);
	    im_outlong(passes);
	}
	im_freepipe;
    } else {
	while(sh->EOFpending & EOFPENDINGBITS)
	    ;
    }
}

/*
 * gl_loadmasks:
 *	- load masks into the fontram
 */
gl_loadmasks(addr,masks,n)
	register short addr;
	register short *masks;
	register short n;
{
	im_setup;
	register ushort amount;
	register short i;

	while (n) {
		amount = MIN(n, 120);
		im_passthru(amount + 2);
		im_outshort(FBCloadmasks);	/* add in fbc command */
		im_outshort(addr);
		for (i = 0; i < amount; i++) {
			im_outshort((short) *masks++);
		}
		addr += amount;
		n -= amount;
	}	
	im_freepipe;
}


/*
 * setpattern:
 *	- switch to the default pattern
 */
void
setpattern()
{
	im_setup;

	im_passcmd(2, FBCpolystipple);
	im_last_outshort(FR_DEFPATTERN);
}

/*
 * defrasterfont:
 *	- load in font 0 into fbc font ram
 */
/* ARGSUSED */
void
defrasterfont(n, ht, nc, cdesc, nr, bitmap)
	short n, ht, nc;
	struct fontchar *cdesc;
	short  nr;
	short *bitmap;
{
	static short firsttime = 1;

	if (firsttime) {
		shiftfontbase(cdesc,nc);
		firsttime = 0;
		gl_loadmasks(FR_DEFFONT,bitmap,nr);
		gl_lowfont(FR_DEFFONT+nr);
	}
}

/*
 * shiftfontbase:
 *	- add the font base to the char descriptions for speed
 */
shiftfontbase(cdesc,nc)
	register struct fontchar *cdesc;
 	register short nc;	
{
	register short i;

	for (i = 0; i < nc; i++) {
		cdesc->offset += FR_DEFFONT;
		cdesc++;
	}
}

/*
 * viewport:
 *	- set up the graphics viewport
 *	- the compuations below are an optimization of (example):
 *		((right + left + 1) / 2) << 8
 */
void
viewport(left, right, bottom, top)
	ushort left, right, bottom, top;
{
	im_setup;
	register long gezero = GEZERO;

	im_outshort(GEloadviewport);
	im_outlong(((right + left) + 1) << 7);	/* vcx (center: x, y) */
	im_outlong(((bottom + top) + 1) << 7);	/* vcy */
	im_outlong(((right - left) + 1) << 7);	/* vsx (half-size: x, y) */
	im_outlong(((top - bottom) + 1) << 7);	/* vsy */
	im_outlong(gezero);			/* vcs (normal: near, far) */
	im_outlong(((XMAXSCREEN + 0) + 1) << 7);	/* vcz */
	im_outlong(gezero);			/* vss (stereo: near, far) */
	im_last_outlong(((XMAXSCREEN - 0) + 1) << 7);	/* vsz */
	im_passcmd(5, FBCloadviewport);
	im_outshort(left);
	im_outshort(bottom);
	im_outshort(right);
	im_last_outshort(top);

	scrmask(left, right, bottom, top);
}


/*
 * viewportall:
 *	- set up the graphics viewport for the whole screen
 *	- does not set scrmask!!
 */
viewportall()
{
	im_setup;
	register long gezero = GEZERO;
	register long xmax, ymax;

	xmax = (XMAXSCREEN + 1) << 7;
	ymax = (YMAXSCREEN + 1) << 7;
	im_outshort(GEloadviewport);
	im_outlong(xmax);		/* vcx (center: x, y) */
	im_outlong(ymax);		/* vcy */
	im_outlong(xmax);		/* vsx (half-size: x, y) */
	im_outlong(ymax);		/* vsy */
	im_outlong(gezero);		/* vcs (normal: near, far) */
	im_outlong(xmax);		/* vcz */
	im_outlong(gezero);		/* vss (stereo: near, far) */
	im_last_outlong(xmax);		/* vsz */
}

/*
 * ortho2i:
 *	- setup the world coordinate system (load the first matrix)
 *	- the magic numbers below represent the equivalent of doing
 *	  an ortho2(-0.5, XMAXSCREEN + 0.5, -0.5, YMAXSCREEN + 0.5);
 *	  except without having to use floating point
 */
ortho2i()
{
	static float orthomat[4][4] = {
		2.0/1024.0,	0.0,		0.0,		0.0,
		0.0,		2.0/768.0,	0.0,		0.0,
		0.0,		0.0,		-1.0,		0.0,
		-1023.0/1024.0,	-767.0/768.0,	0.0,		1.0
	};
	im_setup;

	im_do_loadmatrix(orthomat);
	im_cleanup;
}

/*
 * translatei:
 *	- translate dx, dy, dz. All MUST be positive!!
 */
translatei(x, y, z)
	short x, y, z;
{
	im_setup;

	im_do_translatei(x,y,z);
	im_cleanup;
}

/*
 * scrmask:
 *	- set up the fbc screen mask
 */
void
scrmask(left, right, bottom, top)
	ushort left, right, bottom, top;
{
	im_setup;

	im_passcmd(7, FBCmasklist);
	im_outshort(0);
	im_outshort(left);
	im_outshort(bottom);
	im_outshort(right);
	im_outshort(top);
	im_last_outshort(0);	/* single vp */
}

/*
 * setfbcpieces:
 *	- load the fbc piece list given a pointer to the head of a
 *	  software piece list
 */
void
setfbcpieces(pp)
	register struct piece *pp;
{
	im_setup;
	register short firsttime = 1;
	register short numrects;

	numrects = 0;
	while (pp) {
		numrects++;
		if(firsttime) { 
		    im_passthru(7);
		    im_outlong(FBCmasklist<<16);
		    firsttime = 0;
		} else {
		    im_outshort(1);		/* for previous rectangle */
		    im_passthru(7);
		    im_outlong((FBCmasklist<<16) | 1);
		}
		im_outshort(pp->p_xmin);
		im_outshort(pp->p_ymin);
		im_outshort(pp->p_xmax);
		im_outshort(pp->p_ymax);
		pp = pp->p_next;
	}
	if (numrects) {
		if (numrects > 1)
			im_outshort(1);	/* for previous rectangle */
		else
			im_outshort(0);		/* for previous rectangle */
	}
	im_freepipe;
}

/*
 * rectfi:
 *	- draw a filled rectangle from (x1,y1) to (x2,y2)
 */
void
rectfi(x1, y1, x2, y2)
	register ulong x1, y1, x2, y2;
{
	im_setup;

    im_rectfi(x1,y1,x2,y2);
    im_cleanup;
}

/*
 * xcharstr:
 *	- display a character string
 *	- MAXC is the maximum # of chars in the string we can output per
 *	  passthru
 *	- WARNING: this code knows that "struct ufontchar" contains two
 *	  longs, and that ufontchar overlaps fontchar
 */
#define	MAXC	30
xcharstr(s, len)
	register unsigned char *s;
	register short len;
{
	im_setup;
	register long *lp;
/* a3 */
	register struct ufontchar *blp = (struct ufontchar *)&defont_font[0];
/* a2 */
	register long fbcdrawchars = (PASSTHRU(4*MAXC+1) << 16) | FBCdrawchars;
	register u_char c;		/* d5 */
	register short count;		/* d4 */

#ifndef	USE_C_CODE
asm("	moveq	#0, d5");		/* clear all of d5 */
#endif
	while (len > 0) {
		if (len >= MAXC) {
			count = MAXC / 5;
			im_outlong(fbcdrawchars);
		    /* do 5 per loop here */
			do {
				c = *s++;
				asm("	movl	d5, d0");
				asm("	asll	#3, d0");
				asm("	movl	d0, a3");
				asm("	addl	a2, a3");
				im_outlong(*lp++);
				im_outlong(*lp);

				c = *s++;
				asm("	movl	d5, d0");
				asm("	asll	#3, d0");
				asm("	movl	d0, a3");
				asm("	addl	a2, a3");
				im_outlong(*lp++);
				im_outlong(*lp);

				c = *s++;
				asm("	movl	d5, d0");
				asm("	asll	#3, d0");
				asm("	movl	d0, a3");
				asm("	addl	a2, a3");
				im_outlong(*lp++);
				im_outlong(*lp);

				c = *s++;
				asm("	movl	d5, d0");
				asm("	asll	#3, d0");
				asm("	movl	d0, a3");
				asm("	addl	a2, a3");
				im_outlong(*lp++);
				im_outlong(*lp);

				c = *s++;
				asm("	movl	d5, d0");
				asm("	asll	#3, d0");
				asm("	movl	d0, a3");
				asm("	addl	a2, a3");
				im_outlong(*lp++);
				im_outlong(*lp);
			} while (--count);
			len -= MAXC;

		} else {
			count = len;
#ifdef	USE_C_CODE
			xpassthru(count<<2);
#else
			asm("	movw	d4, d0");
			asm("	moveq	#10, d1");
			asm("	aslw	d1, d0");
			asm("	orw	#0x8, d0");
			asm("	movw	d0, a4@");
#endif
			im_outshort(FBCdrawchars);
			len -= count;
			/* do 1 per loop here */
			do {
				c = *s++;
#ifdef	USE_C_CODE
				lp = (long *)&blp[c];
#else
/* gross */
				asm("	movl	d5, d0");
				asm("	asll	#3, d0");
				asm("	movl	d0, a3");
				asm("	addl	a2, a3");
#endif
				im_outlong(*lp++);
				im_outlong(*lp);
			} while (--count);
		}
	}
	im_freepipe;
}

/*
 * color:
 *	- set current color to "c" (non-RGB mode)
 */
void
color(c)
	register long c;
{
	im_setup;

	im_do_color(c);
	im_cleanup;
}

/*
 * backface:
 *	- set mode controlling elimination of backfacing polygons
 */
void
backface(bool)
	long bool;
{
	im_setup;

	im_passcmd(2, FBCsetbackfacing);
	im_last_outlong(bool);
	im_cleanup;
}

/*
 * linewidth:
 *	- set line width to the given value
 */
void
linewidth(n)
	ushort n;
{
	im_setup;

	im_passcmd(2, FBClinewidth);
	im_last_outshort(n - 1);
}

/*
 * linestyle:
 *	- set up the line style
 */
linestyle(pat)
	ushort pat;
{
	im_setup;

	im_passcmd(3, FBClinestipple);
	im_outshort(0);
	im_outshort(pat);
}

/*
 * writemask:
 *	- tell the fbc into which planes it can write into
 */
void
writemask(bits)
	u_long bits;
{
	im_setup;
	register long mask;

	if(gl_cfr & (UC_DOUBLE << 16))
		mask = gl_userdbwritemask;
	else
		mask = gl_userwritemask;
	im_do_writemask(bits & mask);
	im_cleanup;
}

/*
 * blankscreen:
 *	- blank or unblank the screen
 */
void
blankscreen(turnitoff)
    short turnitoff;
{
    long cfr;

    if(gl_gfport)	/* user has the hardware */
	cfr = gl_shmemptr->ws.curatrdata.myconfig;
    else		/* kernel has the hardware */
	cfr = gl_cfr;
    if (turnitoff) {
	if (!gl_blankmode) {
		gl_blankmode = 1;
		setconfig(cfr);
		gl_setcursor(gl_cursoraddr, gl_cursorcolor, gl_cursorwenable);
		gl_shmemptr->isblanked = 1;
	}
    } else {
	if (gl_blankmode) {
		gl_blankmode = 0;
		setconfig(cfr);
		if(gl_dcr & DCRGBMODE)
		    gl_RGBsetcursor(gl_cursoraddr, gl_rcursorcolor,
			gl_gcursorcolor, gl_bcursorcolor, gl_rcursorwenable,
			gl_gcursorwenable, gl_bcursorwenable);
		else
		  gl_setcursor(gl_cursoraddr,gl_cursorcolor,gl_cursorwenable);
		gl_shmemptr->isblanked = gl_isblanked;
	}
    }
}

kunblanklater()
{
    gl_lastupdate = gl_framecount;
    if (gl_isblanked == 0)		/* already unblanked */
	return;
    gl_isblanked = -1;			/* turn it on later */
}

/*
 * kblankscreen:
 *	- blank the screen (from the kernel)
 */
kblankscreen(turnitoff)
    short turnitoff;
{
    long cfr;

    if (turnitoff == gl_isblanked)		/* been done already */
	return;
    if (fbc_pipeisbusy())			/* can't do it now */
	return;

    /*
     * see if we are going to mess up, because fbc is hung waiting for
     * cursor and the pipe is full...
     */
    if (gl_fbcstatus & HOSTFLAG) {
	if (!turnitoff)
		gl_isblanked = -1;		/* turn it on later */
	return;
    }

    gl_isblanked = turnitoff;
    gl_shmemptr->isblanked = gl_isblanked | gl_blankmode;
    if (gl_gfport)				/* user has the hardware */
	cfr = gl_shmemptr->ws.curatrdata.myconfig;
    else					/* kernel has the hardware */
	cfr = gl_cfr;
    setconfig(cfr);
    if(gl_dcr & DCRGBMODE)
	gl_RGBsetcursor(gl_cursoraddr, gl_rcursorcolor, gl_gcursorcolor,
		gl_bcursorcolor, gl_rcursorwenable, gl_gcursorwenable,
		gl_bcursorwenable);
    else
	gl_setcursor(gl_cursoraddr, gl_cursorcolor, gl_cursorwenable);
    if(turnitoff == 0)
	gl_lastupdate = gl_framecount;
}

/*
 * setconfig:
 *	- set configuration register
 */
setconfig(x)
    long x;
{
    im_setup;

    im_passcmd(3, FBCconfig);
    if (gl_isblanked || gl_blankmode)
	x &= ~(DISPLAYA | DISPLAYB);
    im_last_outlong(x);
}

/*
 * gl_mode:
 *	- switch to a new mode (MD_SINGLE, MD_DOUBLE, MD_RGB)
 */
gl_mode(how)
    int how;
{
    register struct inputchan *ic;
    register struct gfport *gf;
    register windowstate *ws;
    register short i;
    register long ctx;

    ctx = gr_setshmem(0);		/* get original context */
    for(i=0, ic = &inchan[0]; i<NINCHANS; i++,ic++) {
	if (ic->ic_shmemptr) {
	    (void) gr_setshmem(ic->ic_oshandle);
	    ws = &(ic->ic_shmemptr->ws);
	} else
	    continue;
	gf = ic->ic_gf;

/* hack: need to do this for all saved ws's as well. */

	switch (how) {
	  case MD_SINGLE:
	    ws->curatrdata.myconfig &= ~(UC_DOUBLE << 16);
	    ws->curatrdata.myconfig |= (UC_SWIZZLE << 16);
	    if(ic == gl_wmport)
		ws->bitplanemask = gl_kwritemask;
	    else
		ws->bitplanemask = gl_userwritemask;
	    break;
	  case MD_DOUBLE:
	    ws->curatrdata.myconfig &= ~(UC_SWIZZLE << 16);
	    ws->curatrdata.myconfig |= (UC_DOUBLE << 16);
	    if(ic == gl_wmport)
		ws->bitplanemask = gl_kdbwritemask;
	    else
		ws->bitplanemask = gl_userdbwritemask;
	    break;
	  case MD_RGB:
	    ws->curatrdata.myconfig &=
			~((UC_DOUBLE | UC_SWIZZLE) << 16);
	    ws->bitplanemask = gl_kwritemask;
	    break;
	}
	ws->curatrdata.myconfig &= ~(DISPLAYA | DISPLAYB);
	ws->curatrdata.myconfig |= gl_cfr&(DISPLAYA | DISPLAYB);

	if(ic->ic_doqueue & DQ_MODECHANGE)
	    gr_qenter(ic,MODECHANGE,how);
    }
    (void) gr_restoreshmem(ctx);
}

/*
 * Screw up the textport colors when a mode change occurs
 */
breaktxcolors()
{
	register struct txport *tx;

	for (tx = &txport[0]; tx < &txport[NTXPORTS]; tx++)
		tx_initcolors(tx);
}

gl_sbtxport()
{
    gl_cfr &= ~(UC_DOUBLE<<16);
    gl_cfr |= (UC_SWIZZLE<<16) | DISPLAYA | DISPLAYB | UPDATEA | UPDATEB;
    gl_cursorconfig = (UC_SWIZZLE<<16) |
				DISPLAYA | DISPLAYB | UPDATEA | UPDATEB;
    gl_setcursor(gl_cursoraddr, gl_cursorcolor, gl_cursorwenable);
    setconfig(gl_cfr);
    gl_dcr &= ~DCRGBMODE;
    DCflags = gl_dcr;
    setbitplanemasks(0);
    breaktxcolors();
    pixelinit(0x103ff);
}

gl_dbtxport()
{
    gl_cfr &= ~((UC_SWIZZLE<<16) | DISPLAYA | DISPLAYB | UPDATEA | UPDATEB);
    gl_cfr |= (UC_DOUBLE<<16) | DISPLAYA | UPDATEB;
    gl_cursorconfig = (UC_DOUBLE<<16) | DISPLAYA | UPDATEA;
    gl_setcursor(gl_cursoraddr, gl_cursorcolor, gl_cursorwenable);
    setconfig(gl_cfr);
    gl_dcr &= ~DCRGBMODE;
    DCflags = gl_dcr;
    setbitplanemasks(0);
    breaktxcolors();
    pixelinit(0x203ff);
}

gl_rgbtxport()
{
    gl_cfr &= ~((UC_DOUBLE | UC_SWIZZLE)<<16);
    gl_cfr |= DISPLAYA | DISPLAYB | UPDATEA | UPDATEB;
    gl_cursorconfig = DISPLAYA | DISPLAYB | UPDATEA | UPDATEB;
    gl_setcursor(gl_cursoraddr, gl_cursorcolor, gl_cursorwenable);
    setconfig(gl_cfr);
    gl_dcr |= DCRGBMODE;
    DCflags = gl_dcr;
    setbitplanemasks(0);
    breaktxcolors();
    pixelinit(0x703ff);
}

gl_setcursor(addr, c, wtm)
long	addr;
short	c, wtm;
{
 	im_setup;
	short	curswason;

	gl_cursoraddr = addr;
	gl_cursorcolor = c;
	gl_cursorwenable = wtm;
	curswason = gl_autocursor;
	if(curswason) {
/*	    gl_WaitForEOF(1);	/* ??? */
	    cursoff();
	}
	im_passcmd(8, FBCselectcursor);
	im_outshort(addr);
	if(gl_blankmode || gl_isblanked)
	    im_outlong(gl_cursorconfig & ~(DISPLAYA | DISPLAYB));
	else
	    im_outlong(gl_cursorconfig);
	im_outlong(c);
	im_last_outlong(wtm);
	if(curswason)
	    curson(1);
}

gl_RGBsetcursor(addr, cr, cg, cb, wtmr, wtmg, wtmb)
long	addr;
short	cr, cg, cb, wtmr, wtmg, wtmb;
{
	im_setup;
	short	curswason;

	gl_cursoraddr = addr;
	gl_rcursorcolor = cr;
	gl_gcursorcolor = cg;
	gl_bcursorcolor = cb;
	gl_rcursorwenable = wtmr;
	gl_gcursorwenable = wtmg;
	gl_bcursorwenable = wtmb;
	curswason = gl_autocursor;
	if(curswason) {
/*	    gl_WaitForEOF(1);	/* ??? */
	    cursoff();
	}
	im_passcmd(10, FBCselectrgbcursor);
	im_outshort(addr);
	im_outlong(gl_cursorconfig);
	im_outshort(cr);
	im_outshort(cg);
	im_outshort(cb);
	im_outshort(wtmr);
	im_outshort(wtmg);
	im_last_outshort(wtmb);
	if(curswason)
	    curson(1);
}

updatebitmasks()
{
    register struct gfport *gf;
    register struct inputchan *ic;
    register short i;
    register long ctx;
    register long kmask, umask;

    if(gl_cfr & (UC_DOUBLE<<16)) {	/* double buffer mode */
	kmask = gl_kdbwritemask;
	umask = gl_userdbwritemask;
    } else if(gl_cfr & (UC_SWIZZLE<<16)) {	/* single buffer mode */
	kmask = gl_kwritemask;
	umask = gl_userwritemask;
    } else				/* rgb mode */
	umask = kmask = gl_kwritemask;
    ctx = gr_setshmem(0);		/* get original context */
    for(i=0, ic = &inchan[0]; i<NINCHANS; i++, ic++) {
	if (ic->ic_shmemptr) {
	    (void) gr_setshmem(ic->ic_oshandle);
	} else
	    continue;

/* hack: need to do this for all saved ws's as well */

	gf = ic->ic_gf;
	if(ic == gl_wmport)
	    ic->ic_shmemptr->ws.bitplanemask = kmask;
	else
	    ic->ic_shmemptr->ws.bitplanemask = umask;
    }
    (void) gr_restoreshmem(ctx);
}
