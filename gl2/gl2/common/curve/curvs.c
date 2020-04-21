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

extern int i_callfunc();

void crvn(n, geom)
float (*geom)[3];
{
    float (*rgeom)[4];
    int i;

    /* hpm Thu Nov 21 21:47:44 PST 1985 */
    rgeom = (float **)malloc(n * 4 * sizeof(float));
    for (i = 0; i < n; i++) {
	bcopy(geom[i],rgeom[i],3*sizeof(float));
	rgeom[i][3] = 1.0;
    }

    rcrvn(n,rgeom);
    free(rgeom);
}


void rcrvn (n,geom)
short 	n;
float	*geom;
{
    objhdr		*objecthdr;
    extern		gl_drawcurves();
    Curves		*temp;

    if(n < 4) {
        gl_ErrorHandler(ERR_TOOFEWPTS, WARNING, "rcurvs");
	return;
    }
    if(gl_openobjhdr == 0) {
        /* declare and initialize the immediate mode registers */
        im_setup;
	
	{
	    register float		*points;
    
	    im_pushmatrix ();
	    if((gl_curvebasis != gl_ccurvebasis) ||
		    (gl_curveprecision != gl_ccurveprecision)) {
		if(!gl_update_itermat()) {
		    gl_ErrorHandler(ERR_BASISID, WARNING, "curvs");
		    im_popmatrix();
		    im_cleanup;
		    return;
		}
		/* fix the stack after builditermat messes it up */
		im_popmatrix();
		im_pushmatrix();
	    }
	    points = geom;
	    {
		register short	j;
		j = n-3;
		while (--j != -1) {
		    im_pushmatrix();
		    gl_multmatrix(points);
		    gl_multmatrix(gl_curveitermat);
		    im_movezero();
		    im_curveit(gl_curveprecision);
		    im_popmatrix();

		/* move the pointer to the next set of four points */
		    points += 4;
		}
	    }
	    im_popmatrix ();
	    im_cleanup;
	    return;
	}
    }
    if(gl_checkspace(7) == 0) 
	return;
    BEGINCOMPILE(7);
    Curves	*curve_st;
    ADDADDR(i_callfunc);
    ADDSHORT(1);
    ADDADDR(gl_drawcurves);
    curve_st = (Curves *)gl_objalloc(gl_openobjhdr,sizeof(Curves));
    if(!curve_st)
	goto outmemerror;
    ADDADDR(curve_st);
    curve_st->npts = n;
    curve_st->geom = (float *)gl_objalloc(gl_openobjhdr, 
					sizeof(float)*n*4);
    curve_st->itermat = (float *)gl_objalloc(gl_openobjhdr, 
					sizeof(float)*16*(n-3));
    if(!curve_st->geom || !curve_st->itermat) 
	goto outmemerror;
    /* move the geometry into the display list */
    bcopy(geom,curve_st->geom,n*4*sizeof(float));
    curve_st->precision = 0;
    curve_st->basis = 0;
    temp = curve_st;
    ENDCOMPILE;
    return;
outmemerror:
    gl_outmem("crvn");
    return;
}

gl_drawcurves(n, curve_st)
int		n;
Curves	*curve_st;
{
    /* declare and initialize the immediate mode registers */
    float		*points;
    float		*itermat;

    im_setup;
    im_pushmatrix ();

    if( ((curve_st->precision != gl_curveprecision) ||
			(curve_st->basis != gl_curvebasis))) {    
        register short	j;

	/* first update the partial iteration matrices */
	if((gl_curvebasis != gl_ccurvebasis) ||
		    (gl_curveprecision != gl_ccurveprecision)) {
	    if(!gl_update_itermat()) {
		gl_ErrorHandler(ERR_BASISID, WARNING, "curvs");
		im_popmatrix();
		im_cleanup;
		return(0);
	    }
	    /* fix the stack after builditermat messes it up */
	    im_popmatrix();
	    im_pushmatrix();
	}
	/* recalculate the iteration matrices */
	points = curve_st->geom;
        itermat = curve_st->itermat;
        j = curve_st->npts - 3;
        while (--j != -1) {
	    im_pushmatrix();
	    im_do_loadmatrix(points);
	    gl_multmatrix(gl_curveitermat);
	    getmatrix(itermat);
	    im_popmatrix();

	    /* move the pointer to the next set of four points */
	    points += 4;
	    /* move the other pointer to the next matrix */
	    itermat += 16;
	}
	curve_st->precision = gl_curveprecision;
	curve_st->basis = gl_curvebasis;
    }

    /* draw the curves */
    points = curve_st->geom;
    itermat = curve_st->itermat;
    {
	register short	j;
	j = curve_st->npts - 3;
	while (--j != -1) {
	    im_pushmatrix();
	    gl_multmatrix(itermat);
	    im_movezero();
	    im_curveit(gl_curveprecision);
	    im_popmatrix();

	    /* move the pointer to the next matrix */
	    itermat += 16;
	}
	im_popmatrix ();
	im_cleanup;
	return(0);
    }
}
