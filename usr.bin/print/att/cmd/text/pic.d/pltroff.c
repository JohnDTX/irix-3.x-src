/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)pic:pltroff.c	1.4	*/
#include <stdio.h>
#include <math.h>
/*
#include <math.h.5.3>
*/
#include "pic.h"
extern int dbg;

#ifndef PI			/* for compatibility... */
#define PI M_PI
#endif

#define	abs(n)	(n >= 0 ? n : -(n))
#define	max(x,y)	((x)>(y) ? (x) : (y))

extern	int	res;

extern	int	devtype;
float	minline	= 0.25;	/* draw lines shorter than this with dots on 202 */
			/* ought to be point-size dependent, but what's that? */
			/* this is big enough to handle 202 up to 36 points */

int	useDline	= 0;	/* if set, produce \D for all lines */

float	hshift	= 0;	/* move this far left for text (in em's) */
float	vshift	= 0.2;	/* this far down */

/* scaling stuff, specific to typesetter */
/* defined by s command as X0,Y0 to X1,Y1 */
/* output dimensions set by -l,-w options to 0,0 to hmax, vmax */
/* default output is 6x6 inches */


float	xscale;
float	yscale;

float	hpos	= 0;	/* current horizontal position in output coordinate system */
float	vpos	= 0;	/* current vertical position; 0 is top of page */

float	htrue	= 0;	/* where we really are */
float	vtrue	= 0;

float	X0, Y0;		/* left bottom of input */
float	X1, Y1;		/* right top of input */

float	hmax;		/* right end of output */
float	vmax;		/* top of output (down is positive) */

extern	float	deltx;
extern	float	delty;
extern	float	xmin, ymin, xmax, ymax;

float	xconv(), yconv(), xsc(), ysc();

openpl(s)	/* initialize device */
	char *s;	/* residue of .PS invocation line */
{
	float maxdelt;

	hpos = vpos = 0;
	maxdelt = max(deltx, delty);
	if (maxdelt > 8) {	/* 8 inches */
		fprintf(stderr, "pic: %g X %g picture shrunk to", deltx, delty);
		deltx *= 8/maxdelt;
		delty *= 8/maxdelt;
		fprintf(stderr, " %g X %g\n", deltx, delty);
	}
	space(xmin, ymin, xmax, ymax);
	printf("... %g %g %g %g\n", xmin, ymin, xmax, ymax);
	printf("... %.3fi %.3fi %.3fi %.3fi\n",
		xconv(xmin), yconv(ymin), xconv(xmax), yconv(ymax));
	printf(".nr 00 \\n(.u\n");
	printf(".nf\n");
	printf(".PS %.3fi %.3fi %s", yconv(ymin), xconv(xmax), s);
		/* assumes \n comes as part of s */
	printf(".br\n");
}

space(x0, y0, x1, y1)	/* set limits of page */
	float x0, y0, x1, y1;
{
	X0 = x0;
	Y0 = y0;
	X1 = x1;
	Y1 = y1;
	xscale = deltx == 0.0 ? 1.0 : deltx / (X1-X0);
	yscale = delty == 0.0 ? 1.0 : delty / (Y1-Y0);
}

float xconv(x)	/* convert x from external to internal form */
	float x;
{
	return (x-X0) * xscale;
}

float xsc(x)	/* convert x from external to internal form, scaling only */
	float x;
{

	return (x) * xscale;
}

float yconv(y)	/* convert y from external to internal form */
	float y;
{
	return (Y1-y) * yscale;
}

float ysc(y)	/* convert y from external to internal form, scaling only */
	float y;
{
	return (y) * yscale;
}

closepl(type)	/* clean up after finished */
	int type;
{
	movehv(0.0, 0.0);	/* get back to where we started */
	if (type == 'F')
		printf(".PF\n");
	else {
		printf(".sp 1+%.3fi\n", yconv(ymin));
		printf(".PE\n");
	}
	printf(".if \\n(00 .fi\n");
}

move(x, y)	/* go to position x, y in external coords */
	float x, y;
{
	hgoto(xconv(x));
	vgoto(yconv(y));
}

movehv(h, v)	/* go to internal position h, v */
	float h, v;
{
	hgoto(h);
	vgoto(v);
}

hmot(n)	/* generate n units of horizontal motion */
	float n;
{
	hpos += n;
}

vmot(n)	/* generate n units of vertical motion */
	float n;
{
	vpos += n;
}

hgoto(n)
	float n;
{
	hpos = n;
}

vgoto(n)
	float n;
{
	vpos = n;
}

hvflush()	/* get to proper point for output */
{
	if (hpos != htrue) {
		printf("\\h'%.3fi'", hpos - htrue);
		htrue = hpos;
	}
	if (vpos != vtrue) {
		printf("\\v'%.3fi'", vpos - vtrue);
		vtrue = vpos;
	}
}

flyback()	/* return to upper left corner (entry point) */
{
	printf(".sp -1\n");
	htrue = vtrue = 0;
}

troff(s)	/* output troff right here */
	char *s;
{
	printf("%s\n", s);
}

label(s, t, nh)	/* text s of type t nh half-lines up */
	char *s;
	int t, nh;
{
	int q;
	char *p;

	hvflush();
	printf("\\h'-%.1fm'\\v'%.1fm'", hshift, vshift);	/* shift down and left */
			/*  .3 .3 is best for PO in circuit diagrams */
	if (t == ABOVE)
		nh++;
	else if (t == BELOW)
		nh--;
	if (nh)
		printf("\\v'%du*\\n(.vu/2u'", -nh);
	/* just in case the text contains a quote: */
	q = 0;
	for (p = s; *p; p++)
		if (*p == '\'') {
			q = 1;
			break;
		}
	switch (t) {
	case LJUST:
	default:
		printf("%s", s);
		break;
	case CENTER:
	case ABOVE:
	case BELOW:
		if (q)
			printf("\\h\\(ts-\\w\\(ts%s\\(tsu/2u\\(ts%s\\h\\(ts-\\w\\(ts%s\\(tsu/2u\\(ts", s, s, s);
		else
			printf("\\h'-\\w'%s'u/2u'%s\\h'-\\w'%s'u/2u'", s, s, s);
		break;
	case RJUST:
		if (q)
			printf("\\h\\(ts-\\w\\(ts%s\\(tsu\\(ts%s", s, s);
		else
			printf("\\h'-\\w'%s'u'%s", s, s);
		break;
	}
	/* don't need these if flyback called immediately */
	printf("\n");
	flyback();
}

line(x0, y0, x1, y1)	/* draw line from x0,y0 to x1,y1 */
	float x0, y0, x1, y1;
{
	move(x0, y0);
	cont(x1, y1);
}

arrow(x0, y0, x1, y1, w, h)	/* draw arrow (without line), head wid w & len h */
	float x0, y0, x1, y1, w, h;
{
	double alpha, rot, hyp;
	float dx, dy;

	rot = atan2( w / 2, h );
	hyp = sqrt(w/2 * w/2 + h * h);
	alpha = atan2(y1-y0, x1-x0);
	dprintf("rot=%g, hyp=%g, alpha=%g\n", rot, hyp, alpha);
	dx = hyp * cos(alpha + PI + rot);
	dy = hyp * sin(alpha + PI + rot);
	dprintf("dx,dy = %g,%g\n", dx, dy);
	line(x1+dx, y1+dy, x1, y1);
	dx = hyp * cos(alpha + PI - rot);
	dy = hyp * sin(alpha + PI - rot);
	dprintf("dx,dy = %g,%g\n", dx, dy);
	line(x1+dx, y1+dy, x1, y1);
}

box(x0, y0, x1, y1)
	float x0, y0, x1, y1;
{
	move(x0, y0);
	cont(x0, y1);
	cont(x1, y1);
	cont(x1, y0);
	cont(x0, y0);
}

cont(x, y)	/* continue line from here to x,y */
	float x, y;
{
	float h1, v1;
	float dh, dv;

	h1 = xconv(x);
	v1 = yconv(y);
	dh = h1 - hpos;
	dv = v1 - vpos;
	hvflush();
	if (!useDline && dv == 0 && abs(dh) > minline)	/* horizontal */
		printf("\\l'%.3fi'\n", dh);
	else if (!useDline && dh == 0 && abs(dv) > minline) {	/* vertical */
		if (devtype == DEV202)
			printf("\\L'%.3fi\\(vr'\n", dv);
		else
			printf("\\v'-.25m'\\L'%.3fi\\(br'\\v'.25m'\n", dv);	/* add -.25m correction if use \(br */
	} else {
		printf("\\D'l%.3fi %.3fi'\n", dh, dv);
	}
	flyback();	/* expensive */
	hpos = h1;
	vpos = v1;
}

circle(x, y, r)
	float x, y, r;
{
	move(x-r, y);
	hvflush();
	printf("\\D'c%.3fi'\n", xsc(2 * r));
	flyback();
}

spline(x, y, n, p)
	float x, y, *p;
	float n;	/* sic */
{
	int i;
	float dx, dy;
	float xerr, yerr;

	move(x, y);
	hvflush();
	printf("\\D'~");
	xerr = yerr = 0.0;
	for (i = 0; i < 2 * n; i += 2) {
		dx = xsc(xerr += p[i]);
		xerr -= dx/xscale;
		dy = ysc(yerr += p[i+1]);
		yerr -= dy/yscale;
		printf(" %.3fi %.3fi", dx, -dy);	/* WATCH SIGN */
	}
	printf("'\n");
	flyback();
}

ellipse(x, y, r1, r2)
	float x, y, r1, r2;
{
	float ir1, ir2;

	move(x-r1, y);
	hvflush();
	ir1 = xsc(r1);
	ir2 = ysc(r2);
	printf("\\D'e%.3fi %.3fi'\n", 2 * ir1, 2 * abs(ir2));
	flyback();
}

arc(x, y, x0, y0, x1, y1, r)	/* draw arc with center x,y */
	float x, y, x0, y0, x1, y1, r;
{

	move(x0, y0);
	hvflush();
	printf("\\D'a%.3fi %.3fi %.3fi %.3fi'\n",
		xsc(x-x0), -ysc(y-y0), xsc(x1-x), -ysc(y1-y));	/* WATCH SIGNS */
	flyback();
}

dot() {
	hvflush();
	/* what character to draw here depends on what's available. */
	/* on the 202, l. is good but small. */
	/* on other typesetters, use a period and hope */
	if (devtype == DEV202)
		printf("\\z\\(l.\\(l.\\z\\(l.\\(l.\n");
	else
		printf("\\&.\n");
	flyback();
}
