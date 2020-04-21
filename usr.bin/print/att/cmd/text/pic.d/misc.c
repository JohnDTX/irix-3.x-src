/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)pic:misc.c	1.7	*/
#include	<stdio.h>
#include	"pic.h"
#include	"y.tab.h"

setdir(n)	/* set direction from n */
	int n;
{
	switch (n) {
	case UP:	hvmode = U_DIR; break;
	case DOWN:	hvmode = D_DIR; break;
	case LEFT:	hvmode = L_DIR; break;
	case RIGHT:	hvmode = R_DIR; break;
	}
	return(hvmode);
}

float getcomp(p, t)	/* return component of a position */
	obj *p;
	int t;
{
	switch (t) {
	case DOTX:
		return p->o_x;
	case DOTY:
		return p->o_y;
	case DOTWID:
		switch (p->o_type) {
		case BOX:
		case BLOCK:
			return p->o_val[0];
		case CIRCLE:
		case ELLIPSE:
			return 2 * p->o_val[0];
		case LINE:
		case ARROW:
			return p->o_val[0] - p->o_x;
		}
	case DOTHT:
		switch (p->o_type) {
		case BOX:
		case BLOCK:
			return p->o_val[1];
		case CIRCLE:
		case ELLIPSE:
			return 2 * p->o_val[1];
		case LINE:
		case ARROW:
			return p->o_val[1] - p->o_y;
		}
	case DOTRAD:
		switch (p->o_type) {
		case CIRCLE:
		case ELLIPSE:
			return p->o_val[0];
		}
	}
}

makefattr(type, sub, f)	/* float attr */
	int type, sub;
	float f;
{
	YYSTYPE val;
	val.f = f;
	makeattr(type, sub, val);
}

makeoattr(type, o)	/* obj* attr */
	obj *o;
{
	YYSTYPE val;
	val.o = o;
	makeattr(type, 0, val);
}

makeiattr(type, i)	/* int attr */
	int i;
{
	YYSTYPE val;
	val.i = i;
	makeattr(type, 0, val);
}

maketattr(sub, p)	/* text attribute: takes two */
	char *p;
{
	YYSTYPE val;
	val.p = p;
	makeattr(TEXTATTR, sub, val);
}

makeattr(type, sub, val)	/* add attribute type and val */
	int type, sub;
	YYSTYPE val;
{
	if (type == 0 && val.i == 0) {	/* clear table for next stat */
		nattr = 0;
		return;
	}
	if (nattr >= nattrlist)
		attr = (Attr *) grow(attr, "attr", nattrlist += 100, sizeof(Attr));
	dprintf("attr %d:  %d %d %d\n", nattr, type, sub, val.i);
	attr[nattr].a_type = type;
	attr[nattr].a_sub = sub;
	attr[nattr].a_val = val;
	nattr++;
}

printexpr(f)	/* print expression for debugging */
	float f;
{
	fprintf(stderr, "%g\n", f);
}

printpos(p)	/* print position for debugging */
	obj *p;
{
	fprintf(stderr, "%g, %g\n", p->o_x, p->o_y);
}

char *tostring(s)
	register char *s;
{
	register char *p;

	p = malloc(strlen(s)+1);
	if (p == NULL) {
		yyerror("out of space in tostring on %s", s);
		exit(1);
	}
	strcpy(p, s);
	return(p);
}

obj *makepos(x, y)	/* make a osition cell */
	float x, y;
{
	obj *p;

	p = makenode(PLACE, 0);
	p->o_x = x;
	p->o_y = y;
	return(p);
}

obj *makebetween(f, p1, p2)	/* make position between p1 and p2 */
	float f;
	obj *p1, *p2;
{
	obj *p;

	dprintf("fraction = %.2f\n", f);
	p = makenode(PLACE, 0);
	p->o_x = p1->o_x + f * (p2->o_x - p1->o_x);
	p->o_y = p1->o_y + f * (p2->o_y - p1->o_y);
	return(p);
}

obj *getpos(p, corner)	/* find position of point */
	obj *p;
	int corner;
{
	float x, y;

	whatpos(p, corner, &x, &y);
	return makepos(x, y);
}

whatpos(p, corner, px, py)	/* what is the position (no side effect) */
	obj *p;
	int corner;
	float *px, *py;
{
	float x, y, x1, y1;
	extern double sqrt();

	dprintf("whatpos %o %d\n", p, corner);
	x = p->o_x;
	y = p->o_y;
	x1 = p->o_val[0];
	y1 = p->o_val[1];
	switch (p->o_type) {
	case PLACE:
		break;
	case BOX:
	case BLOCK:
		switch (corner) {
		case NORTH:	y += y1 / 2; break;
		case SOUTH:	y -= y1 / 2; break;
		case EAST:	x += x1 / 2; break;
		case WEST:	x -= x1 / 2; break;
		case NE:	x += x1 / 2; y += y1 / 2; break;
		case SW:	x -= x1 / 2; y -= y1 / 2; break;
		case SE:	x += x1 / 2; y -= y1 / 2; break;
		case NW:	x -= x1 / 2; y += y1 / 2; break;
		case START:
			if (p->o_type == BLOCK)
				return whatpos(objlist[(int)p->o_val[2]], START, px, py);
		case END:
			if (p->o_type == BLOCK)
				return whatpos(objlist[(int)p->o_val[3]], END, px, py);
		}
		break;
	case ARC:
		switch (corner) {
		case START:
			if (p->o_attr & CW_ARC) {
				x = p->o_val[2]; y = p->o_val[3];
			} else {
				x = x1; y = y1;
			}
			break;
		case END:
			if (p->o_attr & CW_ARC) {
				x = x1; y = y1;
			} else {
				x = p->o_val[2]; y = p->o_val[3];
			}
			break;
		}
		if (corner == START || corner == END)
			break;
		x1 = y1 = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y));
		/* Fall Through! */
	case CIRCLE:
	case ELLIPSE:
		switch (corner) {
		case NORTH:	y += y1; break;
		case SOUTH:	y -= y1; break;
		case EAST:	x += x1; break;
		case WEST:	x -= x1; break;
		case NE:	x += 0.707 * x1; y += 0.707 * y1; break;
		case SE:	x += 0.707 * x1; y -= 0.707 * y1; break;
		case NW:	x -= 0.707 * x1; y += 0.707 * y1; break;
		case SW:	x -= 0.707 * x1; y -= 0.707 * y1; break;
		}
		break;
	case LINE:
	case SPLINE:
	case ARROW:
	case MOVE:
		switch (corner) {
		case START:	break;	/* already in place */
		case END:	x = x1; y = y1; break;
		case CENTER:	x = (x+x1)/2; y = (y+y1)/2; break;
		case NORTH:	if (y1 > y) { x = x1; y = y1; } break;
		case SOUTH:	if (y1 < y) { x = x1; y = y1; } break;
		case EAST:	if (x1 > x) { x = x1; y = y1; } break;
		case WEST:	if (x1 < x) { x = x1; y = y1; } break;
		}
		break;
	}
	dprintf("whatpos returns %g %g\n", x, y);
	*px = x;
	*py = y;
}

obj *gethere(n)	/* make a place for curx,cury */
{
	dprintf("gethere %g %g\n", curx, cury);
	return(makepos(curx, cury));
}

obj *getlast(n, t)	/* find n-th previous occurrence of type t */
	int n, t;
{
	int i, k;
	obj *p;

	k = n;
	for (i = nobj-1; i >= 0; i--) {
		p = objlist[i];
		if (p->o_type == BLOCKEND) {
			i = p->o_val[4];
			continue;
		}
		if (p->o_type != t)
			continue;
		if (--k > 0)
			continue;	/* not there yet */
		dprintf("got a last of x,y= %g,%g\n", p->o_x, p->o_y);
		return(p);
	}
	yyerror("there is no %dth last", n);
	return(NULL);
}

obj *getfirst(n, t)	/* find n-th occurrence of type t */
	int n, t;
{
	int i, k;
	obj *p;

	k = n;
	for (i = 0; i < nobj; i++) {
		p = objlist[i];
		if (p->o_type == BLOCK && t != BLOCK) {	/* skip whole block */
			i = p->o_val[5] + 1;
			continue;
		}
		if (p->o_type != t)
			continue;
		if (--k > 0)
			continue;	/* not there yet */
		dprintf("got a first of x,y= %g,%g\n", p->o_x, p->o_y);
		return(p);
	}
	yyerror("there is no %dth ", n);
	return(NULL);
}

obj *getblock(p, s)	/* find variable s in block p */
	obj *p;
	char *s;
{
	struct symtab *stp;

	if (p->o_type != BLOCK) {
		yyerror(".%s is not in that block", s);
		return(NULL);
	}
	for (stp = (struct symtab *) p->o_dotdash; stp != NULL; stp = stp->s_next)
		if (strcmp(s, stp->s_name) == 0) {
			dprintf("getblock found x,y= %g,%g\n",
				(stp->s_val.o)->o_x, (stp->s_val.o)->o_y);
			return(stp->s_val.o);
		}
	yyerror("there is no .%s in that []", s);
	return(NULL);
}

obj *fixpos(p, x, y)
	obj *p;
	float x, y;
{
	dprintf("fixpos returns %g %g\n", p->o_x + x, p->o_y + y);
	return makepos(p->o_x + x, p->o_y + y);
}

obj *makenode(type, n)
	int type, n;
{
	obj *p;
	int i;

	p = (obj *) malloc(sizeof(obj) + (n-1)*sizeof(float));
	if (p == NULL) {
		yyerror("out of space in makenode\n");
		exit(1);
	}
	p->o_type = type;
	p->o_count = n;
	p->o_nobj = nobj;
	p->o_mode = hvmode;
	p->o_x = curx;
	p->o_y = cury;
	p->o_nt1 = ntext1;
	p->o_nt2 = ntext;
	ntext1 = ntext;	/* ready for next caller */
	p->o_attr = p->o_dotdash = p->o_ddval = 0;
	for (i = 0; i < n; i++)
		p->o_val[i] = 0;
	if (nobj >= nobjlist)
		objlist = (obj **) grow(objlist, "objlist",
			nobjlist += 100, sizeof(obj *));
	objlist[nobj++] = p;
	return(p);
}

extreme(x, y)	/* record max and min x and y values */
	float x, y;
{
	if (x > xmax)
		xmax = x;
	if (y > ymax)
		ymax = y;
	if (x < xmin)
		xmin = x;
	if (y < ymin)
		ymin = y;
}

#ifndef sgi
/* atan2 is included to avoid bugs in libm version */
/*LINTLIBRARY*/
/*
 *	atan returns the value of the arctangent of its
 *	argument in the range [-pi/2, pi/2].
 *
 *	atan2 returns the arctangent of x/y
 *	in the range [-pi, pi].
 *
 *	There are no error returns.
 *
 *	Coefficients are #5077 from Hart & Cheney (19.56D).
 */

#include <math.h>
#define SQ_TWO	1.41421356237309504880
#define PI	3.14159265358979323846

/*
 *	atan makes its argument positive and
 *	calls the inner routine s_atan.
 */

double
atan(x)
double x;
{
	extern double s_atan();

	return (x < 0 ? -s_atan(-x) : s_atan(x));
}

/*
 *	x_atan evaluates a series valid in the
 *	range [-0.414..., +0.414...].
 */

static double
x_atan(x)
double x;
{
	static double p[] = {
		0.161536412982230228262e2,
		0.26842548195503973794141e3,
		0.11530293515404850115428136e4,
		0.178040631643319697105464587e4,
		0.89678597403663861959987488e3,
	}, q[] = {
		1.0,
		0.5895697050844462222791e2,
		0.536265374031215315104235e3,
		0.16667838148816337184521798e4,
		0.207933497444540981287275926e4,
		0.89678597403663861962481162e3,
	};
	double xsq = x * x;

	return (x * _POLY4(xsq, p)/_POLY5(xsq, q));
}

/*
 *	s_atan reduces its argument (known to be positive)
 *	to the range [0, 0.414...] and calls x_atan.
 */

static double
s_atan(x)
double x;
{
	return (x < SQ_TWO - 1 ? x_atan(x) :
	    x > SQ_TWO + 1 ? PI/2 - x_atan(1/x) :
	    PI/4 + x_atan((x - 1)/(x + 1)));
}

/*
 *	atan2 discovers what quadrant the angle
 *	is in and calls s_atan.
 */

double
atan2(x, y)
double x, y;
{
	if (x + y == x)
		return (x < 0 ? -PI/2 : PI/2);
	if (y < 0)
		return (x < 0 ? -PI + s_atan(x/y) : PI - s_atan(-x/y));
	return (x < 0 ? -s_atan(-x/y) : s_atan(x/y));
}
#endif !sgi
