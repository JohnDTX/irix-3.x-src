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
#include "immed.h"
#include "splinegl.h"
#include "glerror.h"

void crv(geom)
float	geom[4][3];
{
    float rgeom[4][4];

    /* 
    bcopy(geom,rgeom,16*sizeof(float));
    */

    /* hpm-Thu Nov 21 21:42:52 PST 1985 */
    bcopy(geom,rgeom,3*sizeof(float));
    bcopy(geom[1],rgeom[1],3*sizeof(float));
    bcopy(geom[2],rgeom[2],3*sizeof(float));
    bcopy(geom[3],rgeom[3],3*sizeof(float));
    rgeom[0][3] = 1.0;
    rgeom[1][3] = 1.0;
    rgeom[2][3] = 1.0;
    rgeom[3][3] = 1.0;

    rcrv(rgeom);
}

void rcrv(geom)
float	geom[4][4];
{
    extern		gl_drawcurve();
    extern		i_callfunc();

    if(gl_openobjhdr == 0) {
        /* declare and initialize the immediate mode registers */
        im_setup;

	im_pushmatrix();
	if((gl_curvebasis != gl_ccurvebasis) ||
			(gl_curveprecision != gl_ccurveprecision)) {
	    if(!gl_update_itermat()) {
		gl_ErrorHandler(ERR_BASISID, WARNING, "curve");
		im_popmatrix();
		im_cleanup;
		return;
	    }
	    im_popmatrix();
	    im_pushmatrix();
	}
	gl_multmatrix(geom);
	gl_multmatrix(gl_curveitermat);
	im_movezero();
	im_curveit(gl_curveprecision);
	im_popmatrix();
	im_cleanup;
	return;
    }
    if(gl_checkspace(7) == 0) 
	return;
    BEGINCOMPILE(7);
    register Curve	*curve_st;
    ADDADDR(i_callfunc);
    ADDSHORT(1);
    ADDADDR(gl_drawcurve);
    curve_st = (Curve *) gl_objalloc(gl_openobjhdr,sizeof(Curve));
    if(!curve_st) {
	gl_outmem("crv");
	return;
    }
    ADDADDR(curve_st);
    {
	/* move the geometry into the display list */
	register float	*from;
	register float	*to;
	register int 	i, j;

	from = (float *)geom;
	to = curve_st->geom;

	/* this should be put in the other major order */
	*to++ = (from[0]);
	*to++ = (from[4]);
	*to++ = (from[8]);
	*to++ = (from[12]);

	*to++ = (from[1]);
	*to++ = (from[5]);
	*to++ = (from[9]);
	*to++ = (from[13]);

	*to++ = (from[2]);
	*to++ = (from[6]);
	*to++ = (from[10]);
	*to++ = (from[14]);

	*to++ = (from[3]);
	*to++ = (from[7]);
	*to++ = (from[11]);
	*to++ = (from[15]);
    }
    curve_st->precision = 0;
    curve_st->basis = 0;
    ENDCOMPILE;
}

gl_drawcurve(n , curve_st)
int	n;
Curve	*curve_st;
{
    /* declare and initialize the immediate mode registers */
    im_setup;
    im_pushmatrix();


    if((curve_st->precision == gl_curveprecision) && 
				    (curve_st->basis == gl_curvebasis))
	gl_multmatrix(curve_st->itermat);
    else {
	if((gl_curvebasis != gl_ccurvebasis) ||
	    (gl_curveprecision != gl_ccurveprecision)) {
	    if(!gl_update_itermat()) {
		gl_ErrorHandler(ERR_BASISID, WARNING, "curve");
		im_popmatrix();
		im_cleanup;
		return(0);
	    }
	}
	gl_restorematrix(curve_st->geom);
        gl_multmatrix(gl_curveitermat);
	getmatrix(curve_st->itermat);
	im_popmatrix();
	im_pushmatrix();
	gl_multmatrix(curve_st->itermat);
	curve_st->precision = gl_curveprecision;
	curve_st->basis = gl_curvebasis;
    }
    im_movezero();
    im_curveit(gl_curveprecision);
    im_popmatrix();
    im_cleanup;
    return(1);
}
		
gl_update_itermat ()
{
    if(gl_ccurveprecision != gl_curveprecision) {
	gl_makediffmatrix(gl_curveprecision,gl_curveprecmat);
        gl_ccurveprecision = gl_curveprecision;
    }
    gl_ccurvebasis = gl_curvebasis;
    return(gl_builditermat(gl_curvebasis, gl_curveprecmat, gl_curveitermat));
}
