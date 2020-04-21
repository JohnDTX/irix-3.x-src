#ifdef	SHRINK

#include "gsh.h"
#include "window.h"
#include "gl.h"

extern	int	img_xsize, img_ysize;

static	rect_t	windowRect, iconRect;

#define	TUMBLESTEPS	16

/*
 * Logarithmic table for varying the shrinkage of one window to another.
 * Using this table the shrinkage goes rapidly at the beginning, and
 * very slow at the end.
 */
float	shrinkTable[] = {
	0.000000, /* 0 */
	0.250000, /* 1 */
	0.396241, /* 2 */
	0.500000, /* 3 */
	0.580482, /* 4 */
	0.646241, /* 5 */
	0.701839, /* 6 */
	0.750000, /* 7 */
	0.792481, /* 8 */
	0.830482, /* 9 */
	0.864858, /* 10 */
	0.896241, /* 11 */
	0.925110, /* 12 */
	0.951839, /* 13 */
	0.976723, /* 14 */
	1.000000, /* 15 */
};
float	unshrinkTable[] = {
	0.000000, /* 0 */
	0.018622, /* 1 */
	0.038529, /* 2 */
	0.059912, /* 3 */
	0.083007, /* 4 */
	0.108114, /* 5 */
	0.135614, /* 6 */
	0.166015, /* 7 */
	0.200000, /* 8 */
	0.238529, /* 9 */
	0.283008, /* 10 */
	0.335614, /* 11 */
	0.400000, /* 12 */
	0.483008, /* 13 */
	0.600000, /* 14 */
	0.800000, /* 15 */
};

void
iconInit()
{
	img_readit(icon_name);
	iconRect.xorg = XMAXSCREEN+1 - 5 - img_xsize;
	iconRect.xlen = img_xsize;
	iconRect.yorg = YMAXSCREEN+1 - 80 - pty_num*(img_ysize + 7);
	iconRect.ylen = img_ysize;
}

/*
 * Setup default sizes for the windowRect and the iconRect
 */
void
setRegularSize(xsize, ysize)
	int xsize, ysize;
{
	windowRect.xlen = xsize;
	windowRect.ylen = ysize;
	windowRect.xorg = (XMAXSCREEN - xsize) / 2;
	windowRect.yorg = (YMAXSCREEN - ysize) / 2;
	tx_textport(&txport[0], 0, windowRect.xlen, 0, windowRect.ylen);
}

void
setIconConstraints()
{
	int xleft, xright;
	int ybottom, ytop;

	fudge(0, 0);
	stepunit(0, 0);
	prefsize(iconRect.xlen, iconRect.ylen);
	prefposition(iconRect.xorg, iconRect.xorg + iconRect.xlen - 1,
				    iconRect.yorg,
				    iconRect.yorg + iconRect.ylen - 1);
}

void
shrink()
{
	int xleft, xright;
	int ybottom, ytop;

	if (opened) {
		getsize(&windowRect.xlen, &windowRect.ylen);
		getorigin(&windowRect.xorg, &windowRect.yorg);
		opened = 0;
	}

	/* start movie (hide the window) */
	if (!flag_startsmall && flag_movie) {
		startMovie(&windowRect, 1);

		/* move window off screen */
		winconstraints();
		stepunit(0, 0);
		fudge(0, 0);
		minsize(0, 0);
		maxsize(0, 0);
		winposition(0, 0, 0, 0);
		winconstraints();
		sginap(5);

		/* show the movie */
		tumbleMovie(&windowRect, &iconRect, 0, 1);
	}

	/* put the icon up in the new position */
	wintitle("");
	winconstraints();
	fudge(0, 0);
	stepunit(0, 0);
	prefsize(iconRect.xlen, iconRect.ylen);
	winposition(iconRect.xorg, iconRect.xorg + iconRect.xlen - 1,
				   iconRect.yorg,
				   iconRect.yorg + iconRect.ylen - 1);
	winconstraints();
	img_drawit();

	/* unfade the icon so we can see it */
	if (!flag_startsmall && flag_movie)
		unfade();
}

unshrink()
{
	if (opened == 0) {
		getsize(&iconRect.xlen, &iconRect.ylen);
		getorigin(&iconRect.xorg, &iconRect.yorg);
		opened = 1;
	}

	if (flag_movie) {
			startMovie(&iconRect, 0);

		/* move window off screen */
		winconstraints();
		stepunit(0, 0);
		fudge(0, 0);
		minsize(0, 0);
		maxsize(0, 0);
		winposition(0, 0, 0, 0);
		winconstraints();
		sginap(5);

		/* show the movie */
		tumbleMovie(&iconRect, &windowRect, 1, 0);
	}

	/* put window up in the new position */
	winpop();
	winconstraints();
	winposition(windowRect.xorg, windowRect.xorg + windowRect.xlen - 1,
				     windowRect.yorg,
				     windowRect.yorg + windowRect.ylen - 1);
	winconstraints();
	reshapeit(txport[0].tx_rows, txport[0].tx_cols);
	wintitle(title);
	sginap(5);

	/* unfade the window so we can see it */
	if (flag_movie)
		unfade();
}

extern	float	sqrt();

drawWindow(from)
	register rect_t *from;
{
	pupcolor(PUP_WHITE);
	rectfi(from->xorg, from->yorg,
			   from->xorg + from->xlen - 1,
			   from->yorg + from->ylen - 1);
}

startMovie(from, addtitle)
	register rect_t *from;
	int addtitle;
{
	rect_t new;

	cursoff();
	pupmode();
	fullscrn();
	perspective(900, ((float) XMAXSCREEN / (float) YMAXSCREEN),
			 0.5, 100000.0);
	translate((float) -((XMAXSCREEN + 1) / 2),
		  (float) -((YMAXSCREEN + 1) / 2),
		  (float) -((YMAXSCREEN + 1) / 2));
	/* XXX add in borders and title stripe */
	new.xorg = from->xorg - 2;
	new.yorg = from->yorg - 3;
	if (addtitle) {
		new.xlen = from->xlen + 5;
		new.ylen = from->ylen + 5 + 15;
	} else {
		new.xlen = from->xlen + 5;
		new.ylen = from->ylen + 5;
	}
	drawWindow(&new);
	curson();
	endpupmode();
	endfullscrn();
}

tumbleMovie(from, to, addtitle, useShrinkTable)
	register rect_t *from, *to;
	int addtitle, useShrinkTable;
{
	register int i;
	float xinc, yinc, dx, dy, xlen, ylen;
	float x, y, xs, ys;
	float xrot, zrot, xangle, zangle;
	int tcx, tcy, fcx, fcy;
	short t, v;
	rect_t new;

	cursoff();
	pupmode();
	fullscrn();
	perspective(900, ((float) XMAXSCREEN / (float) YMAXSCREEN),
			 0.5, 100000.0);
	translate((float) -((XMAXSCREEN + 1) / 2),
		  (float) -((YMAXSCREEN + 1) / 2),
		  (float) -((YMAXSCREEN + 1) / 2));

	/* compute centers */
	tcx = to->xorg + to->xlen / 2;
	tcy = to->yorg + to->ylen / 2;
	fcx = from->xorg + from->xlen / 2;
	fcy = from->yorg + from->ylen / 2;

	/* compute x and y increment for movement of rectangle */
	xinc = sqrt((float) (tcx - fcx) * (tcx - fcx)) /
		TUMBLESTEPS;
	yinc = sqrt((float) (tcy - fcy) * (tcy - fcy)) /
		TUMBLESTEPS;
	if (fcx > tcx)
		xinc = -xinc;
	if (fcy > tcy)
		yinc = -yinc;
	xrot = 3600.0 / TUMBLESTEPS;
	zrot = 1800.0 / TUMBLESTEPS;
	xangle = 0.0;
	zangle = 0.0;
	xlen = (float) from->xlen / 2.0;
	ylen = (float) from->ylen / 2.0;
	x = fcx;
	y = fcy;

	/* delta x & y for scaling down/up of rectangle sizes */
	dx = (float) (to->xlen - from->xlen) / 2.0;
	dy = (float) (to->ylen - from->ylen) / 2.0;
	for (i = 0; i < TUMBLESTEPS; i++) {
		if (useShrinkTable) {
			xs = xlen + dx * shrinkTable[i];
			ys = ylen + dy * shrinkTable[i];
		} else {
			xs = xlen + dx * unshrinkTable[i];
			ys = ylen + dy * unshrinkTable[i];
		}
		pupcolor(PUP_CLEAR);
		fclear();
		pushmatrix();
		    translate(x, y, 0.0);
		    rotate((int) zangle, 'z');
		    rotate((int) xangle, 'x');
		    pupcolor(PUP_WHITE);
		    rectf(-xs, -ys, xs, ys);
		popmatrix();
		/* sginap(1); */
		gsync();
		x += xinc;
		y += yinc;
		xangle += xrot;
		zangle += zrot;
	}

	/* splat up destination with all bits on */
	new.xorg = to->xorg - 2;
	new.yorg = to->yorg - 3;
	if (addtitle) {
		new.xlen = to->xlen + 5;
		new.ylen = to->ylen + 5 + 15;
	} else {
		new.xlen = to->xlen + 5;
		new.ylen = to->ylen + 5;
	}
	drawWindow(&new);

	endpupmode();
	endfullscrn();
}

unfade()
{
	register int i;

	pupmode();
	fullscrn();
	pupcolor(PUP_CLEAR);
	for (i = 16; --i >= 0; ) {
		fclear();
	}
	endpupmode();
	endfullscrn();
	curson();
}

/*
 * 	shadow -
 *		Make it easy to select a halftone gray pattern for shadows.
 *
 *				Paul Haeberli - 1985
 */
static short shadow[16] = {
	0x5555, 0xaaaa, 0x5555, 0xaaaa, 
	0x5555, 0xaaaa, 0x5555, 0xaaaa, 
	0x5555, 0xaaaa, 0x5555, 0xaaaa, 
	0x5555, 0xaaaa, 0x5555, 0xaaaa
}; 

#define SHADOWPAT	1255
#define DITHERPAT	1256

static int firsted;

shadowpattern()
{
    if(!firsted) {
	defpattern(SHADOWPAT,16,shadow);
	firsted++;
    }
    setpattern(SHADOWPAT);
}

fclear()
{
    int i, k;
    static int texno;

    setdither(texno++);
    clear();
    setpattern(0);
}

static int shifts[16] = {
    0, 2, 2, 0,
    1, 3, 3, 1,
    0, 2, 2, 0,
    1, 3, 3, 1,
};

static int wheres[16] = {
    0, 2, 0, 2,
    1, 3, 1, 3,
    1, 3, 1, 3,
    0, 2, 0, 2,
};

setdither(n)
int n;
{
    short tex[16];

    ditherpat(n,tex);
    defpattern(DITHERPAT,16,tex);	
    setpattern(DITHERPAT);
}

ditherpat(n,tex)
int n;
short tex[16];
{
    register int i;
    register int shift, where, pattern;

    n = n&0xf;
    for(i=0; i<16; i++)
	tex[i] = 0;
    shift = shifts[n]; 	
    where = wheres[n]; 	
    pattern = 0x1111<<shift;
    tex[where+0] = pattern;
    tex[where+4] = pattern;
    tex[where+8] = pattern;
    tex[where+12] = pattern;
}

#endif
