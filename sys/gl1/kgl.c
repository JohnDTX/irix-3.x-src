#include "../h/param.h"
#include "../h/systm.h"
#include "../h/setjmp.h"
#include "../h/printf.h"
#include "machine/cpureg.h"
#include "../gl1/kgl.h"
#include "../gl1/shmem.h"
#include "../gl1/textport.h"
#include "../gl1/gfdev.h"
#include "../gl1/betacodes.h"
#include "../gl1/font.h"
#include "../gl1/dcdev.h"
#include "../gl1/device.h"
#include "../gl1/bpccodes.h"

#undef	DEBUG

/* configuration table for ge's */
short	geconfigtab[] = {
	0x9,	0xa,	0xb,	0xc,
	0x15,	0x10,	0x11,	0x12,
	0x13,	0x14,	0x20,	0x21
};

/* default fill texture */
char	getexture[] = {
	0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 
};

/* default cursor */
short	cursormask[] = {
	0xfe00, 0xfc00, 0xf800, 0xf800,
	0xfc00, 0xde00, 0x8f00, 0x0780,
	0x03c0, 0x01e0, 0x00f0, 0x0078,
	0x003c, 0x001e, 0x000e, 0x0004
};

/* XXX */
/* table for texture address's in the font ram */
#define	MAXTEXTURES	7			/* HACK */
#define	MAXCURSORS	5			/* HACK */
ushort	getextures[MAXTEXTURES];
ushort	gecursors[MAXCURSORS];

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

/* send a passthru command to GE for "x" shorts */
#define	passthru(x)	*GE = ((((x) - 1) << 8) | 0x08)
#define	xpassthru(x)	*GE = (((x) << 8) | 0x08)
#define	PASSTHRU(x)	((((x) - 1) << 8) | 0x08)

#define	LGE	((long *)GE)		/* long version of GE */
#define	GEONE	0x01400000		/* a floating point 1 (ge style) */
#define	GEINT	0x17000000		/* used to convert ints to floats */
#define	GEZERO	0x00000000		/* one long's worth of zeros */

extern long sglbpcxlate(), dblbpcxlate(), rgbbpcxlate();
long (*bpcxfunc)() = sglbpcxlate;
u_short swiztab[64] =
{
	/*
	B2 A2 B1 A1 B0 A0
	 */
	00|00|00|00|00|00,
	00|00|00|00|00|A0,
	00|00|00|00|B0|00,
	00|00|00|00|B0|A0,
	00|00|00|A1|00|00,
	00|00|00|A1|00|A0,
	00|00|00|A1|B0|00,
	00|00|00|A1|B0|A0,
	00|00|B1|00|00|00,
	00|00|B1|00|00|A0,
	00|00|B1|00|B0|00,
	00|00|B1|00|B0|A0,
	00|00|B1|A1|00|00,
	00|00|B1|A1|00|A0,
	00|00|B1|A1|B0|00,
	00|00|B1|A1|B0|A0,
	00|A2|00|00|00|00,
	00|A2|00|00|00|A0,
	00|A2|00|00|B0|00,
	00|A2|00|00|B0|A0,
	00|A2|00|A1|00|00,
	00|A2|00|A1|00|A0,
	00|A2|00|A1|B0|00,
	00|A2|00|A1|B0|A0,
	00|A2|B1|00|00|00,
	00|A2|B1|00|00|A0,
	00|A2|B1|00|B0|00,
	00|A2|B1|00|B0|A0,
	00|A2|B1|A1|00|00,
	00|A2|B1|A1|00|A0,
	00|A2|B1|A1|B0|00,
	00|A2|B1|A1|B0|A0,
	B2|00|00|00|00|00,
	B2|00|00|00|00|A0,
	B2|00|00|00|B0|00,
	B2|00|00|00|B0|A0,
	B2|00|00|A1|00|00,
	B2|00|00|A1|00|A0,
	B2|00|00|A1|B0|00,
	B2|00|00|A1|B0|A0,
	B2|00|B1|00|00|00,
	B2|00|B1|00|00|A0,
	B2|00|B1|00|B0|00,
	B2|00|B1|00|B0|A0,
	B2|00|B1|A1|00|00,
	B2|00|B1|A1|00|A0,
	B2|00|B1|A1|B0|00,
	B2|00|B1|A1|B0|A0,
	B2|A2|00|00|00|00,
	B2|A2|00|00|00|A0,
	B2|A2|00|00|B0|00,
	B2|A2|00|00|B0|A0,
	B2|A2|00|A1|00|00,
	B2|A2|00|A1|00|A0,
	B2|A2|00|A1|B0|00,
	B2|A2|00|A1|B0|A0,
	B2|A2|B1|00|00|00,
	B2|A2|B1|00|00|A0,
	B2|A2|B1|00|B0|00,
	B2|A2|B1|00|B0|A0,
	B2|A2|B1|A1|00|00,
	B2|A2|B1|A1|00|A0,
	B2|A2|B1|A1|B0|00,
	B2|A2|B1|A1|B0|A0
};

/* XXX */
/* static stash area for kernel printf's while hardware is busy */
static	char kgr_stash[400];
static	short kgr_stashindex = 0;

char	gl_version[] = VERSIONSTRING;

/*
 * kgreset:
 *	- reset the graphics hardware and the screen for usage
 *	- only used when things get strange
 *	- repaint of text port is done by caller!
 */
kgreset(howto)
	short howto;
{
	register ushort *GE = &GEdata;
	extern jmp_buf txenv;

	if (howto & 1) {
		fbcreset();
		gefind();
		geconfigure();		/* configure the ge's */
		setmode(MD_SINGLE);
	}

	loadcolormap();
	if (howto & 1) {
		foinit();
		deftexture(0, getexture);
		deftexture(1, getexture);
		deftexture(2, getexture);
		deftexture(3, getexture);
		deftexture(4, getexture);
		deftexture(5, getexture);
		deftexture(6, getexture);		/* JUNK */
		defrasterfont(0, defont_ht, defont_nc, defont_font, defont_nr,
				 defont_bits);
		defcursor(0, (char *)cursormask);
	}
	settexture(0);

    /* setup defaults */
	writemask((u_long)0xFFFFFFFF);
	linewidth(1);
	linestyle(0xFFFF);

    /* attempt repaint, if requested */
	if (howto & 2) {
		tx_resetconsole();
		if (kgr & KGR_WANTED)
			wakeup((caddr_t)&kgr);		/* ??? */
		kgr &= ~(KGR_BUSY | KGR_WANTED);	/* its okay, now */
		tx_redisplay(&txport[0]);
	}
	if (shmem_pa)
		qreset();
}

/*
 * gr_init:
 *	- initialize the graphics hardware and the screen for usage
 */
gr_init()
{
	extern int (*softintr[])();
	extern int kb_softintr();
	int s;

	s = spl7();
	kgreset(1);			/* everything but textport */
	gr_tpblank = 0;
	tx_console();
	if (shmem_pa)
		qreset();

	viewport(0, XMAXSCREEN, 0, YMAXSCREEN);
	ortho2i();
	color(0);
	rectfi(0, 0, XMAXSCREEN, YMAXSCREEN);

	softintr[0] = kb_softintr;
	fbcalive = 1;
	splx(s);
}

/*
 * greset:
 *	- reset the hardware to a known state
 */
greset()
{
	register struct shmem *sh = (struct shmem *)SHMEM_VBASE;
	register int s;

	s = spl5();			/* block softclock */
	while (kgr & KGR_BUSY) {
		kgr |= KGR_WANTED;
		sleep((caddr_t)&kgr, PRIBIO);
	}
	kgr |= KGR_BUSY;

	sh->autocursor = 0;
	sh->IdleMode = 1;		/* ??? */
	sh->gr_cfr = CONFIGBOTH;
	sh->gr_dcr &= ~DCRGBMODE;
	kgreset(3);
	kgr &= ~KGR_BUSY;
	splx(s);
}

gr_reset()
{
	int s;

	s = spl7();
	kgreset(3);
	grunstash();
	splx(s);
}

/*
 * geconfigure:
 *	- configure the ge's according to the ones we found in gefind()
 */
geconfigure()
{
	register short *table = geconfigtab;
	register short mask, found, temp;
	extern short gefound, gemask;

	(void) gereset();

	mask = gemask;
	found = gefound;

    /* fifo chip with high water mark at 44, and low water at 43 */
	GEOUT(0x2C00);

	for (temp = 0; temp < 12; temp++) {
		if ((mask <<= 1) & 0x2000)
			GEOUT((--found<<8) | *(table+temp));
	}

    /* set water marks for fifo chip at end of pipe (does this do anything?) */
	GEOUT(0x2A02);

	GEOUT(GEclearhitmode);
}

/*
 * setmode:
 *	- change the mode of display being used
 */
setmode(newmode)
	short newmode;
{
	gr_mode = newmode;
	changemode(0);
}

/*
 * changemode:
 *	- set the cfr to either the users cfr (in shmem) or the kernels
 *	  cfr (known as CONFIGBOTH)
 */
changemode(usermode)
	short usermode;
{
	register ushort *GE = &GEdata;
	register ushort cfr;

	gewait();
	if (shmem_pa) {
		if (((struct shmem *)SHMEM_VBASE)->gr_dcr & DCRGBMODE)
			gr_mode = MD_RGB;
		else {
			cfr = ((struct shmem *)SHMEM_VBASE)->gr_cfr;
			passthru(2);
			*GE = FBCconfig;
			if (usermode)
				*GE = cfr;
			else {
				*GE = cfr | 0x0C;
				if ((cfr & CONFIGMASK) != CONFIGMASK)
					gr_mode = MD_DOUBLE;
				else
					gr_mode = MD_SINGLE;
			}
		}
	} else {
		passthru(2);
		*GE = FBCconfig;
		*GE = CONFIGBOTH;
	}

	switch (gr_mode) {
	case MD_SINGLE:
		bpcxfunc = sglbpcxlate;
		break;

	case MD_DOUBLE:
		bpcxfunc = dblbpcxlate;
		break;

	case MD_RGB:
		bpcxfunc = rgbbpcxlate;
		break;
	}
}

# define bitmask(x)	((1<<(x))-1)
long
sglbpcxlate(x)
	register long x;
{
	/*
	 ((long)swiztab[(x>>6) & bitmask(6)] << 3)
		| ((long)swiztab[(x>>0) & bitmask(6)] << 0);
	 */
	return ((long)swiztab[(x>>6) & bitmask(6)] << 3)
		| swiztab[(x) & bitmask(6)];
}

long
dblbpcxlate(x)
	register long x;
{
	x &= 0xFF;
	return (x) | (x<<8);
}

long
rgbbpcxlate(x)
	long x;
{
	return x ? 0xFFFFFFFF : 0;
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
}

/*
 * mapcolor:
 *	- assign a particular color table entry the given r,g,b values
 */
mapcolor(index, r, g, b)
	register short index;
	register u_char r, g, b;
{
	DCflags = DCBUSOP;
	DCMapColor(index, r, g, b);
	DCflags = DCMULTIMAP;
}

/*
 * foinit:
 *	- init font ram stuff
 */
foinit()
{
	fo_baseaddr = 0x20;		/* first 32 bytes are reserved */
	fo_freebytes = 0xffe0;		/* total # of bytes left */
}

/*
 * foalloc:
 *	- allocate font ram space, returning the base address
 */
foalloc(nb)
	register short nb;
{
	short rv;

	nb = (nb + 15) & 0xFFF0;		/* round up to mod 16 */
	if (nb > fo_freebytes)
		return -1;

	rv = fo_baseaddr;
	fo_baseaddr += nb;
	fo_freebytes -= nb;

	return rv;
}

/*
 * defcursor:
 *	- define a cursor, allocating font ram space
 */
defcursor(n, bitmap)
	short n;
	register char *bitmap;
{
	register ushort *GE = &GEdata;
	register ushort base;
	register short i;

	if (n >= MAXCURSORS)
		return;			/* ignore garbage */
	if ((base = foalloc(32)) == -1)
		return;			/* ignore overflow */

	gewait();
	passthru(32 + 2);
	*GE = FBCloadmasks;
	*GE = base;
	for (i = 0; i < 32; i++)  {
		gewait();
		*GE = (short)*bitmap++;
	}
	gecursors[n] = base;
}

/*
 * deftexture:
 *	- define a texture, allocating font ram space
 */
deftexture(n, bitmap)
	short n;
	register char *bitmap;
{
	register ushort *GE = &GEdata;
	register ushort base;
	register short i;

	if (n >= MAXTEXTURES)
		return;			/* ignore garbage */
	if ((base = foalloc(16)) == -1)
		return;			/* ignore overflow */

	gewait();
	passthru(16 + 2);
	*GE = FBCloadmasks;
	*GE = base;
	for (i = 0; i < 16; i++) {
		gewait();
		*GE = (short)*bitmap++;
	}
	getextures[n] = base;
}

/*
 * settexture:
 *	- switch to the given texture number
 */
settexture(n)
	short n;
{
	register ushort *GE = &GEdata;

	if (n >= MAXTEXTURES)
		return;			/* ignore garbage */
	gewait();
	passthru(2);
	*GE = FBCpolystipple;
	*GE = getextures[n];
}

/*
 * defrasterfont:
 *	- load in font 0 into fbc font ram
 */
/* ARGSUSED */
defrasterfont(n, ht, nc, cdesc, nr, bitmap)
	short n, ht, nc;
	register struct fontchar *cdesc;
	register short  nr;
	register char *bitmap;
{
	register ushort *GE = &GEdata;
	register ushort ramaddr, base;
	register short i, amount;
	static short firsttime = 1;

    /* allocate some font ram space */
	base = foalloc(nr);
	ramaddr = base;

    /* send down font masks */
	while (nr) {
		amount = MIN(nr, 120);
		passthru(amount + 2);		/* add in fbc command */
		*GE = FBCloadmasks;
		*GE = ramaddr;
		for (i = 0; i < amount; i++) {
			gewait();
			*GE = (short) *bitmap++;
		}
		ramaddr += amount;
		nr -= amount;
	}	

    /* adjust font base offset to include "base" (first time only) */
	if (firsttime) {
		shiftfontbase(cdesc, nc, base);

		firsttime = 0;
	}
}

shiftfontbase(cdesc, nc, base)
	register struct fontchar *cdesc;
	register int nc;
	register ushort base;
{
	while (--nc >= 0)
		cdesc++ ->offset += base;
}

/*
 * viewport:
 *	- set up the graphics viewport
 *	- the compuations below are an optimization of (example):
 *		((right + left + 1) / 2) << 8
 */
viewport(left, right, bottom, top)
	ushort left, right, bottom, top;
{
	register ushort *GE = &GEdata;

	gewait();
	*GE = GEloadviewport;
	*LGE = ((right + left) + 1) << 7;	/* vcx (center: x, y) */
	*LGE = ((bottom + top) + 1) << 7;	/* vcy */
	*LGE = ((right - left) + 1) << 7;	/* vsx (half-size: x, y) */
	*LGE = ((top - bottom) + 1) << 7;	/* vsy */
	gewait();
	*LGE = gezero;				/* vcs (normal: near, far) */
	*LGE = ((XMAXSCREEN + 0) + 1) << 7;		/* vcz */
	*LGE = gezero;				/* vss (stereo: near, far) */
	*LGE = ((XMAXSCREEN - 0) + 1) << 7;		/* vsz */

	scrmask(left, right, bottom, top);
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
	register ushort *GE = &GEdata;

	gewait();
	*GE = GEloadmm;
	*LGE = 0xF8400000;		/* column 0 */
	*LGE = gezero;
	*LGE = gezero;
	*LGE = 0x802001;

	gewait();
	*LGE = gezero;			/* column 1 */
	*LGE = 0xF8555555;
	*LGE = gezero;
	*LGE = 0x802aab;

	gewait();
	*LGE = gezero;			/* column 2 */
	*LGE = gezero;
	*LGE = 0x800001;
	*LGE = gezero;

	gewait();
	*LGE = gezero;			/* column 3 */
	*LGE = gezero;
	*LGE = gezero;
	*LGE = GEONE;
}

/*
 * pushviewport:
 *	- push the current viewport on the viewport stack
 */
pushviewport()
{
	register ushort *GE = &GEdata;

	gewait();
	*GE = GEpushviewport;
}

/*
 * popviewport:
 *	- pop the top viewport off the viewport stack
 */
popviewport()
{
	register ushort *GE = &GEdata;

	gewait();
	*GE = GEpopviewport;
}

/*
 * pushmatrix:
 *	- push the current matrix on the matrix stack
 */
pushmatrix()
{
	register ushort *GE = &GEdata;

	gewait();
	*GE = GEpushmm;
}

/*
 * popmatrix:
 *	- pop the top matrix off the matrix stack
 */
popmatrix()
{
	register ushort *GE = &GEdata;

	gewait();
#if	GEpopmm == 0
	*GE = gezero;
#else
	*GE = GEpopmm;
#endif
}

/*
 * translatei:
 *	- translate the eye to a new position
 */
translatei(x, y, z)
	ushort x, y, z;
{
	register ushort *GE = &GEdata;

	gewait();
	*GE = GEcompletemm3;
	*LGE = GEINT | x;
	*LGE = GEINT | y;
	*LGE = GEINT | z;
	*LGE = GEONE;
}

/*
 * scrmask:
 *	- set up the fbc screen mask
 */
scrmask(left, right, bottom, top)
	ushort left, right, bottom, top;
{
	register ushort *GE = &GEdata;

	gewait();
	passthru(9);
	*GE = FBCfbviewport;
	*GE = left >> 8;
	*GE = left << 8;
	*GE = bottom >> 8;
	*GE = bottom << 8;
	*GE = right >> 8;
	*GE = right << 8;
	*GE = top >> 8;
	*GE = top << 8;
}

/*
 * move2i:
 *	- move the graphics position to (x,y)
 */
move2i(x, y)
	register ushort x, y;
{
	register ushort *GE = &GEdata;
	register long gezero = GEZERO;
	register long geint = GEINT;

	gewait();
	*GE = GEmove;
	*LGE = geint | x;
	*LGE = geint | y;
	*LGE = gezero;
	*LGE = GEONE;
}

/*
 * draw2i:
 *	- draw from the current position to (x,y)
 */
draw2i(x, y)
	register ushort x, y;
{
	register ushort *GE = &GEdata;
	register long gezero = GEZERO;
	register long geint = GEINT;

	gewait();
	*GE = GEdraw;
	*LGE = geint | x;
	*LGE = geint | y;
	*LGE = gezero;
	*LGE = GEONE;
}

/*
 * cmov2i:
 *	- move the character position to (x,y)
 */
cmov2i(x, y)
	register ushort x, y;
{
	register ushort *GE = &GEdata;
	register long gezero = GEZERO;
	register long geint = GEINT;

	gewait();
	passthru(1);
	*GE = FBCcharposnabs;
	*GE = GEpoint;
	*LGE = geint | x;
	*LGE = geint | y;
	*LGE = gezero;
	*LGE = GEONE;
	*GE = GEpassthru;
	*GE = GEpassthru;
}

/*
 * rectfi:
 *	- draw a filled rectangle from (x1,y1) to (x2,y2)
 */
rectfi(x1, y1, x2, y2)
	register ushort x1, y1, x2, y2;
{
	register ushort *GE = &GEdata;
	register long geint = GEINT;
	register long gezero = GEZERO;

	gewait();
	*GE = GEmovepoly;
	*LGE = geint | x2;
	*LGE = geint | y1;
	*LGE = gezero;
	*LGE = GEONE;

	gewait();
	*GE = GEdrawpoly;
	*LGE = geint | x2;
	*LGE = geint | y2;
	*LGE = gezero;
	*LGE = GEONE;

	gewait();
	*GE = GEdrawpoly;
	*LGE = geint | x1;
	*LGE = geint | y2;
	*LGE = gezero;
	*LGE = GEONE;

	gewait();
	*GE = GEdrawpoly;
	*LGE = geint | x1;
	*LGE = geint | y1;
	*LGE = gezero;
	*LGE = GEONE;

	gewait();
	*GE = GEclosepoly;
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
	register u_char *s;
	register short len;
{
	register ushort *GE = &GEdata;
	register long *lp;
	register struct ufontchar *blp = (struct ufontchar *)&defont_font[0];
	register ushort count;

	while (len > 0) {
		if (len >= MAXC) {
			count = MAXC;
			*LGE = (PASSTHRU(4*MAXC+1) << 16) | FBCdrawchars;
		} else {
			count = len;
			xpassthru(count<<2);
			*GE = FBCdrawchars;
		}
		len -= count;
		do {
			lp = (long *)&blp[*s++];
			gewait();
			*LGE = *lp++;
			*LGE = *lp;
		} while (--count);
	}
}

/*
 * color:
 *	- set current color to "c"
 */
color(c)
	int c;
{
	register ushort *GE = &GEdata;
	register long b;

	b = bpcxlate((long)c);
	gewait();
	passthru(3);
	*GE = FBCcolor;
	*GE = b;
	*GE = b >> 16;
}

/*
 * recti:
 *	- draw an unfilled rectangle from (x1,y1) to (x2,y2)
 */
recti(x1, y1, x2, y2)
	register ushort x1, y1, x2, y2;
{
	register ushort *GE = &GEdata;

	gewait();
	*GE = GEmove;
	*LGE = GEINT | x1;
	*LGE = GEINT | y1;
	*LGE = gezero;
	*LGE = GEONE;

	gewait();
	*GE = GEdraw;
	*LGE = GEINT | x2;
	*LGE = GEINT | y1;
	*LGE = gezero;
	*LGE = GEONE;

	gewait();
	*GE = GEdraw;
	*LGE = GEINT | x2;
	*LGE = GEINT | y2;
	*LGE = gezero;
	*LGE = GEONE;

	gewait();
	*GE = GEdraw;
	*LGE = GEINT | x1;
	*LGE = GEINT | y2;
	*LGE = gezero;
	*LGE = GEONE;

	gewait();
	*GE = GEdraw;
	*LGE = GEINT | x1;
	*LGE = GEINT | y1;
	*LGE = gezero;
	*LGE = GEONE;
}

/*
 * linewidth:
 *	- set line width to the given value
 */
linewidth(n)
	ushort n;
{
	register ushort *GE = &GEdata;

	gewait();
	passthru(2);
	*GE = FBClinestyle;
	*GE = n - 1;
}

/*
 * linestyle:
 *	- set up the line style
 */
linestyle(pat)
	short pat;
{
	register ushort *GE = &GEdata;

	gewait();
	passthru(2);
	*GE = FBClinestipple;
	*GE = pat;
}

/*
 * writemask:
 *	- tell the fbc into which planes it can write into
 */
writemask(bits)
	register u_long bits;
{
	register ushort *GE = &GEdata;

	gewait();
	passthru(3);
	*GE = FBCwrten;
	bits = bpcxlate(bits);
	*GE = bits;
	*GE = bits >> 16;
}

/*
 * blankscreen:
 *	- shut off screen
 */
blankscreen()
{
	register ushort *GE = &GEdata;

	if (!gr_blanked) {
		gr_blanked = 1;
		gewait();
		passthru(2);
		*GE = FBCconfig;
		*GE = CONFIGOFF;
	}
}

/*
 * unblankscreen:
 *	- if the screen is blanked, un blank it
 */
unblankscreen()
{
	register ushort *GE = &GEdata;

	if (gr_blanked) {
		gewait();
		passthru(2);
		*GE = FBCconfig;
		*GE = CONFIGBOTH;
		gr_blanked = 0;
	}
}

/*
 * qenter:
 *	- enter data into the queue, if possible
 */
qenter(type, value)
	short type, value;
{
	register int s;

	s = spl6();

    /* see if queue is full */
	if (nqueued == NQUEUES) {
		splx(s);
		return 0;
	}

    /* add element to queue */
	nqueued++;
	queuein->type = type;
	queuein->value = value;
	if (++queuein == &queue[NQUEUES])		/* wrap around */
		queuein = queue;

    /* check for anybody waiting for data */
	if (queuewaiting) {
		queuewaiting = 0;
		wakeup((caddr_t)&queuewaiting);
	}
	splx(s);
	return 1;
}

int
gr_isqempty(x)
{
	return (nqueued == 0);
}

/*
 * qread:
 *	- read an element from the queue
 */
qread(type, value)
	short *type, *value;
{
	register int s;

	s = spl6();
	while (nqueued == 0) {
		queuewaiting++;
		sleep((caddr_t)&queuewaiting, PWAIT);
	}

	*type = queueout->type;
	*value = queueout->value;
	if (++queueout == &queue[NQUEUES])		/* wrap around */
		queueout = queue;
	nqueued--;

	splx(s);
}

/*
 * qtest:
 *	- see if the queue has data, and if so, peek the next read'able
 *	  element
 */
qtest(type)
	short *type;
{
	register int s;

	s = spl6();
	if (nqueued == 0) {
		splx(s);
		return 0;
	} else {
		*type = queueout->type;
		splx(s);
		return 1;
	}
}

/*
 * qreset:
 *	- reset the queue's to an initial state
 */
qreset()
{
	register struct shmem *sh = (struct shmem *)SHMEM_VBASE;
	register short i;

	nqueued = 0;
	queuein = queueout = queue;
	queuewaiting = 0;
	sh->queuedkeyboard = 0;
	sh->cursorxvaluator = MOUSEX - VALOFFSET;
	sh->cursoryvaluator = MOUSEY - VALOFFSET;

	for (i = 0; i < NBUTTONS; i++) {
		sh->Buttons[i].state = 0;
		sh->Buttons[i].queue = 0;
	}

	for (i = 0; i < NVALUATORS; i++) {
		sh->valuators[i].queue = 0;
		sh->valuators[i].minvalue = 0;
		sh->valuators[i].maxvalue = 1023;
		if ((i == MOUSEX - VALOFFSET) || (i == MOUSEY - VALOFFSET))
			sh->valuators[i].value = 1023 / 2;
		else
			sh->valuators[i].value = 0;
	}

	sh->valuators[MOUSEX - VALOFFSET].minvalue = 0;
	sh->valuators[MOUSEX - VALOFFSET].maxvalue = 1023;
	sh->valuators[MOUSEY - VALOFFSET].minvalue = 0;
	sh->valuators[MOUSEY - VALOFFSET].maxvalue = 767;
}

/*
 * grputchar:
 *	- output a character to the screen at the current location
 *	- if the hardware is in use, save the data in the kgr_stash buffer,
 *	  which will be looked at later for potential delayed output
 */
grputchar(c)
	register short c;
{
	register struct textport *tx = &txport[0];
	register short i;
	char b[1];
	int s;

	grlastupdate = time;
	unblankscreen();
	if (tx->tx_state & TX_BUSY) {
		kgr_stash[kgr_stashindex++] = c;
		return;
	}
	grunstash();
	b[0] = c;
	tx->tx_state |= TX_BUSY;
	tx_addchars(0, b, 1);
	tx->tx_state &= ~TX_BUSY;
}

/*
 * grsputchar:
 *	- put a character into the stash buffer
 *	- used by dprintf()
 */
grsputchar(c)
	register short c;
{
	kgr_stash[kgr_stashindex++] = c;	/* save for later */
}

/*
 * grunstash:
 *	- called to unplug blocked characters which were stashed while the
 *	  hardware was busy
 */
grunstash()
{
	register struct textport *tx = &txport[0];

	if (((tx->tx_state & TX_BUSY) == 0) && kgr_stashindex) {
		tx->tx_state |= TX_BUSY;
		tx_addchars(0, kgr_stash, kgr_stashindex);
		tx->tx_state &= ~TX_BUSY;
		kgr_stashindex = 0;
	}
}

/*
 * grgetchar:
 *	- get character from graphics screen (keyboard, that is)
 */
grgetchar()
{
	return kbgetchar();
}

/*
 * dprintf:
 *	- like printf, except the putchar routine is a variant of grputchar
 *	  which just stashes data instead of actually printing
 */
/*VARARGS1*/
dprintf(fmt, x1)
	char *fmt;
	u_int x1;
{
	doprnt(grsputchar, fmt, &x1);
	doprnt(duputchar, fmt, &x1);
}

/*
 * Stub for source compatability
 */
lpentick()
{
}
