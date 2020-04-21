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

#include "globals.h"
#include "TheMacro.h"
#include "imdraw.h"
#include "immatrix.h"
#include "glerror.h"

#define	PRECISION	80
#define	INCREMENT	(3600/PRECISION)

extern void i_circ(), i_circi(), i_circs(),i_circf(), i_circfi(), i_circfs(),
	    i_arc(), i_arci(), i_arcs(), i_arcf(), i_arcfi(), i_arcfs();

short gl_circle_flag = FALSE;
float gl_circle_array[PRECISION][2];

void circs(x, y, rad)
    Scoord x, y, rad;
{
    gl_circs(x,y,rad,i_circs);
}

void circi(x, y, rad)
    Icoord x, y, rad;
{
    gl_circi(x,y,rad,i_circi);
}

void circ(x, y, rad)
    Coord x, y, rad;
{
    gl_circ(x,y,rad,i_circ);
}

void circfs(x, y, rad)
    Scoord x, y, rad;
{
    gl_circs(x,y,rad,i_circfs);
}

void circfi(x, y, rad)
    Icoord x, y, rad;
{
    gl_circi(x,y,rad,i_circfi);
}

void circf(x, y, rad)
    Coord x, y, rad;
{
    gl_circ(x,y,rad,i_circf);
}

#define CIRCLE_SIZE 8

gl_circs(x, y, rad, routine)
    Scoord x, y, rad;
    int (*routine) ();
{
    if (!gl_circle_flag) gl_circle_init ();
    beginpicmandef(CIRCLE_SIZE-3);
    BEGINCOMPILE(CIRCLE_SIZE-3);
    ADDADDR(routine);
    ADDSCOORD(x);
    ADDSCOORD(y);
    ADDSCOORD(rad);
    ENDCOMPILE;
    endpicmandef;
}

gl_circi(x, y, rad, routine)
    Icoord x, y, rad;
    int (*routine) ();
{
    if (!gl_circle_flag) gl_circle_init ();
    beginpicmandef(CIRCLE_SIZE);
    BEGINCOMPILE(CIRCLE_SIZE);
    ADDADDR(routine);
    ADDICOORD(x);
    ADDICOORD(y);
    ADDICOORD(rad);
    ENDCOMPILE;
    endpicmandef;
}

gl_circ(x, y, rad, routine)
    Coord x, y, rad;
    int (*routine) ();
{
    if (!gl_circle_flag) gl_circle_init ();
    beginpicmandef(CIRCLE_SIZE);
    BEGINCOMPILE(CIRCLE_SIZE);
    ADDADDR(routine);
    ADDFLOAT(x);
    ADDFLOAT(y);
    ADDFLOAT(rad);
    ENDCOMPILE;
    endpicmandef;
}

void arcs(xc, yc, rad, start, end)
    Scoord xc, yc, rad;
    Angle start, end;
{
    gl_arcs(xc,yc,rad, start,end, i_arcs);
}

void arci(xc, yc, rad, start, end)
    Icoord xc, yc, rad;
    Angle start, end;
{
    gl_arci(xc,yc,rad, start,end, i_arci);
}

void arc(xc, yc, rad, start, end)
    Coord xc, yc, rad;
    Angle start, end; 
{
    gl_arc(xc,yc,rad, start,end, i_arc);
}

void arcfs(xc, yc, rad, start, end)
    Scoord xc, yc, rad;
    Angle start, end;
{
    gl_arcs(xc,yc,rad, start,end, i_arcfs);
}

void arcfi(xc, yc, rad, start, end)
    Icoord xc, yc, rad;
    Angle start, end;
{
    gl_arci(xc,yc,rad, start,end, i_arcfi);
}

void arcf(xc, yc, rad, start, end)
    Coord xc, yc, rad;
    Angle start, end; 
{
    gl_arc(xc,yc,rad, start,end, i_arcf);
}

#define ARC_SIZE 22
extern long *gl_arc_guts();

gl_arcs(xc,yc,rad, start,end, routine)
    Scoord xc, yc, rad;
    Angle start, end;
    int (*routine) ();
{
    if (!gl_circle_flag) gl_circle_init ();
    beginpicmandef(ARC_SIZE-3);
    BEGINCOMPILE(ARC_SIZE-3);
    ADDADDR(routine);
    ADDSCOORD(xc);
    ADDSCOORD(yc);
    ADDSCOORD(rad);
    ADDSHORT(start);
    ADDSHORT(end);
    _curpos = gl_arc_guts (_curpos,start,end);
    ENDCOMPILE;
    endpicmandef;
}

gl_arci(xc,yc,rad, start,end, routine)
    Icoord xc, yc, rad;
    Angle start, end;
    int (*routine) ();
{
    if (!gl_circle_flag) gl_circle_init ();
    beginpicmandef(ARC_SIZE);
    BEGINCOMPILE(ARC_SIZE);
    ADDADDR(routine);
    ADDICOORD(xc);
    ADDICOORD(yc);
    ADDICOORD(rad);
    ADDSHORT(start);
    ADDSHORT(end);
    _curpos = gl_arc_guts (_curpos,start,end);
    ENDCOMPILE;
    endpicmandef;
}

gl_arc(xc,yc,rad, start,end, routine)
    Coord xc, yc, rad;
    Angle start, end;
    int (*routine) ();
{
    if (!gl_circle_flag) gl_circle_init ();
    beginpicmandef(ARC_SIZE);
    BEGINCOMPILE(ARC_SIZE);
    ADDADDR(routine);
    ADDFLOAT(xc);
    ADDFLOAT(yc);
    ADDFLOAT(rad);
    ADDSHORT(start);
    ADDSHORT(end);
    _curpos = gl_arc_guts (_curpos,start,end);
    ENDCOMPILE;
    endpicmandef;
}

/* add the following to display list:
    start (x,y) point
    pointer into circle array
    number of points to use from circle array
    end (x,y) point
*/
long *gl_arc_guts (_curpos,start,end)
    register long *_curpos;
    register int start,end;
{
    register int n;
    float sin,cos;

    while (start >= 3600) start -= 3600;
    while (start < 0) start += 3600;
    while (end >= 3600) end -= 3600;
    while (end < 0) end += 3600;
    gl_sincos (start,&sin,&cos);
    ADDFLOAT (cos);			/* first arc point	*/
    ADDFLOAT (sin);
    n = 1 + start/INCREMENT;		/* index of first point	*/
    ADDADDR (&gl_circle_array[n][0]);	/* address of first pnt	*/

    /* number of points to be taken from circle array	*/
    n = end/INCREMENT - n + 1;
    if (n > 0) ;			/* this is OK and most likely	*/
    else if (n < 0) n += PRECISION;	/* start > end in diff. sectors	*/
    else if (start>end) n = PRECISION;	/* n==0, start > end in same sector*/
    ADDLONG (n);			/* number of points	*/
    gl_sincos (end,&sin,&cos);
    ADDFLOAT (cos);			/* last arc point	*/
    ADDFLOAT (sin);
    return (_curpos);
}

gl_circle_init ()
{
    register short i;
    register int angle;
    register float *fp;
    float sin,cos;

    angle = 0;
    fp = &gl_circle_array[0][0];

    for (i=PRECISION; --i != -1;) {
	gl_sincos (angle,&sin,&cos);
	*fp++ = cos;
	*fp++ = sin;
	angle += INCREMENT;
    }
    gl_circle_flag = TRUE;
}

#include "interp.h"

INTERP_NAME(circ);
INTERP_NAME(circi);
INTERP_NAME(circs);
INTERP_NAME(circf);
INTERP_NAME(circfi);
INTERP_NAME(circfs);

INTERP_NAME(arc);
INTERP_NAME(arci);
INTERP_NAME(arcs);
INTERP_NAME(arcf);
INTERP_NAME(arcfi);
INTERP_NAME(arcfs);

static bogus ()
{
    DECLARE_INTERP_REGS;
    register short n;
    register float *p;
    extern i_setup_circ(), i_setup_circi(), i_setup_circs();

/* NOTE: the following code uses asm's to do returns and jumps to fool
	the optimizer which otherwise removes labels	*/

/* setup a circle or arc by translating to center and scaling by radius	*/
/* note this can only be called from within the interpreter with the PC	*/
asm ("_i_setup_circ:");
	im_pushmatrix ();
	im_do_translate (*(float *)PC++, *(float *)PC++, 0.0);
	im_do_scale2(*(float *)PC,*(float *)PC++);
	rts();

/* setup a circle or arc by translating to center and scaling by radius	*/
/* note this can only be called from within the interpreter with the PC	*/
asm ("_i_setup_circi:");
	im_pushmatrix ();
	im_do_translatei (*PC++, *PC++, 0);
	im_do_scale2i(*PC,*PC++);
	rts();

/* setup a circle or arc by translating to center and scaling by radius	*/
/* note this can only be called from within the interpreter with the PC	*/
asm ("_i_setup_circs:");
	im_pushmatrix ();
	im_do_translates (*(short *)PC++, *(short *)PC++, 0);
	im_do_scale2s(*(short *)PC,*(short *)PC++);
	rts();

INTERP_LABEL(circ,8); /* CIRCLE_SIZE == 8 */
	i_setup_circ ();
	bra(_i_circdata);
INTERP_LABEL(circs,5); /* CIRCLE_SIZE-3 == 5 */
	i_setup_circs ();
	bra(_i_circdata);
INTERP_LABEL(circi,8); /* CIRCLE_SIZE == 8 */
	i_setup_circi ();
asm("_i_circdata:");
	p = &gl_circle_array[0][0];
	im_move2 (*(float *)p++,*(float *)p++);
	for (n = 79; --n != -1;)
	    im_draw2 (*(float *)p++,*(float *)p++);

	im_draw2 (gl_circle_array[0][0],gl_circle_array[0][1]);
	im_popmatrix ();
	thread;

INTERP_LABEL(circf,8); /* CIRCLE_SIZE == 8 */
	i_setup_circ ();
	bra(_i_circfdata);
INTERP_LABEL(circfs,5); /* CIRCLE_SIZE-3 == 5 */
	i_setup_circs ();
	bra(_i_circfdata);
INTERP_LABEL(circfi,8); /* CIRCLE_SIZE == 8 */
	i_setup_circi ();
asm("_i_circfdata:");
	p = &gl_circle_array[0][0];
	im_pmv2 (*(float *)p++,*(float *)p++);
	for (n = 79; --n != -1;)
	    im_pdr2 (*(float *)p++,*(float *)p++);

	im_pclos ();
	im_popmatrix ();
	thread;

INTERP_LABEL(arc, 22); /* ARC_SIZE == 22 */
	i_setup_circ ();
	bra(_i_arcdata);
INTERP_LABEL(arcs, 19); /* ARC_SIZE-3 == 19 */
	i_setup_circs ();
	bra(_i_arcdata);
INTERP_LABEL(arci, 22); /* ARC_SIZE == 22 */
	i_setup_circi ();
asm("_i_arcdata:");
	PC += 1;		/* skip over angles		*/
	/* output first point from display list	*/
	im_move2 (*(float*)PC++,*(float *)PC++);
	p = (float *) (*PC++);	/* address of next point	*/
	for (n = *PC++; --n != -1;) {
	    /* test wrap around	*/
	    if (p == &gl_circle_array[PRECISION][0])	
		p = &gl_circle_array[0][0];
	    im_draw2 (*(float *)p++,*(float *)p++);
	}
	/* output last point from display list	*/
	im_draw2 (*(float *)PC++,*(float *)PC++);
	im_popmatrix ();
	thread;

INTERP_LABEL(arcf, 22); /* ARC_SIZE == 22 */
	i_setup_circ ();
	bra(_i_arcfdata);
INTERP_LABEL(arcfs, 19); /* ARC_SIZE-3 == 19 */
	i_setup_circs ();
	bra(_i_arcfdata);
INTERP_LABEL(arcfi, 22); /* ARC_SIZE == 22 */
	i_setup_circi ();
asm("_i_arcfdata:");
	PC += 1;		/* skip over angles		*/
	im_pmovezero();		/* output first point (0,0)	*/
	/* output first point from display list	*/
	im_pdr2 (*(float *)PC++,*(float *)PC++);
	p = (float *) (*PC++);	/* address of next point	*/
	for (n = *PC++; --n != -1;) {
	    /* test wrap around gl_circle_array	*/
	    if (p == &gl_circle_array[PRECISION][0])
		p = &gl_circle_array[0][0];
	    /* if more than 1/2 way then start a new poly	*/
	    if (n == PRECISION-1 || n == PRECISION/2 - 1) {
		im_pdr2 (*(float *)p,*(float *)(p+1));
		im_pclos ();
		im_pmovezero();	/* output first point (0,0)	*/
	    }
	    im_pdr2 (*(float *)p++,*(float *)p++);
	}
	/* output last point from display list	*/
	im_pdr2 (*(float *)PC++,*(float *)PC++);
	im_pclos ();
	im_popmatrix ();
	thread;
}
