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

static Matrix ones =	{
    { 1.0, 1.0, 1.0, 1.0},
    { 1.0, 1.0, 1.0, 1.0},
    { 1.0, 1.0, 1.0, 1.0},
    { 1.0, 1.0, 1.0, 1.0}
};

void patch(geomx, geomy, geomz)
float	*geomx;
float	*geomy;
float	*geomz;
{
    rpatch(geomx, geomy, geomz, ones);
}

void rpatch(geomx, geomy, geomz, geomw)
float	*geomx;
float	*geomy;
float	*geomz;
float	*geomw;
{
    extern	gl_drawpatch();
    extern	i_callfunc();
    float	*xbuff, *ybuff, *zbuff, *wbuff;
    float	sx[16];
    float	sy[16];
    float	sz[16];
    float	sw[16];
    char	err_flag;
    int		buffsize;

    if(gl_openobjhdr == 0) {
        /* declare and initialize the immediate mode registers */
        im_setup;

	buffsize = max(gl_u_curves, gl_v_curves) *
			4 * sizeof(float);

	xbuff = (float *)malloc(buffsize);
	ybuff = (float *)malloc(buffsize);
	zbuff = (float *)malloc(buffsize);
	wbuff = (float *)malloc(buffsize);
	if(wbuff == 0) {
		gl_outmem("patch");
		return;
	}
	im_pushmatrix ();
        /* first bring the iteration matrices up to date */

	err_flag = 0;
	if((gl_cv_basis != gl_v_basis) || (gl_cv_precision != gl_v_precision) ||
	     				(gl_cu_curves != gl_u_curves))
	    err_flag = !gl_update_v_itermat();

	if((gl_cu_basis!=gl_u_basis) || (gl_cu_precision!=gl_u_precision) ||
	     				(gl_cv_curves != gl_v_curves))
	    err_flag |= !gl_update_u_itermat();

	if(err_flag) {
	    /* one of the basis id was not found, print an error
	    message and return after cleaning up */
	    /* fix this message someday !!!!!!!!!!!!!!!!*/
	    gl_ErrorHandler(ERR_BASISID, WARNING, "patch");
	    im_popmatrix ();
	    free(xbuff);
	    free(ybuff);
	    free(zbuff);
	    free(wbuff);
            im_cleanup;
	    return;
	}
	gl_restorematrix(gl_v_itermat);
	im_pushmatrix();
	    gl_multmatrix(geomx);
	    gl_multmatrix(gl_u_itermat);
	    getmatrix(sx);
	    gl_iterate(gl_v_curves, xbuff);
	im_popmatrix();
	im_pushmatrix();
	    gl_multmatrix(geomy);
	    gl_multmatrix(gl_u_itermat);
	    getmatrix(sy);
	    gl_iterate(gl_v_curves, ybuff);
	im_popmatrix();
	im_pushmatrix();
	    gl_multmatrix(geomz);
	    gl_multmatrix(gl_u_itermat);
	    getmatrix(sz);
	    gl_iterate(gl_v_curves, zbuff);
	im_popmatrix();
	im_pushmatrix();
	    gl_multmatrix(geomw);
	    gl_multmatrix(gl_u_itermat);
	    getmatrix(sw);
	    gl_iterate(gl_v_curves, wbuff);
	im_popmatrix();

	/* also pop the v_itermat */
	im_popmatrix();

	gl_dump_curves(xbuff, ybuff, zbuff, wbuff,  gl_v_curves,
		gl_v_mult, gl_v_convmat);

	/* save the viewing transform */
	im_pushmatrix();
	    gl_restorematrix(sx);
	    gl_iterate(gl_u_curves, xbuff);
	    gl_restorematrix(sy);
	    gl_iterate(gl_u_curves, ybuff);
	    gl_restorematrix(sz);
	    gl_iterate(gl_u_curves, zbuff);
	    gl_restorematrix(sw);
	    gl_iterate(gl_u_curves, wbuff);
	im_popmatrix();
	gl_dump_curves(xbuff, ybuff, zbuff, wbuff, gl_u_curves,
		gl_u_mult, gl_u_convmat);
	free(xbuff);
	free(ybuff);
	free(zbuff);
	free(wbuff);
        im_cleanup;
	return;
    }
    if(gl_checkspace(7) == 0) 
	return;
    BEGINCOMPILE(7);
    Patch	*patch_st;

    ADDADDR(i_callfunc);
    ADDSHORT(1);
    ADDADDR(gl_drawpatch);
    patch_st = (Patch *)gl_objalloc(gl_openobjhdr, sizeof(Patch));
    if(!patch_st) {
  	gl_outmem("patch");
	return;
    }
    ADDADDR(patch_st);
    patch_st->objectheader = gl_openobjhdr;

/* move the geometry into the display list */
    bcopy(geomx,patch_st->gx,16*sizeof(float));
    bcopy(geomy,patch_st->gy,16*sizeof(float));
    bcopy(geomz,patch_st->gz,16*sizeof(float));
    bcopy(geomw,patch_st->gw,16*sizeof(float));

    /* set these up to be "illegal" there's no point in calculating
    iteration matrices now, cause who knows what the bases etc. will be
    at display time */
    patch_st->u_basis = 0;
    patch_st->u_precision = 0;
    patch_st->u_curves = 0;
    patch_st->u_mult = 0;
    patch_st->v_basis = 0;
    patch_st->v_precision = 0;
    patch_st->v_curves = 0;
    patch_st->v_mult = 0;
    patch_st->itermats = NULL;
    ENDCOMPILE;
}

gl_drawpatch(n, patch_st)
short	n;
Patch	*patch_st;
{
    float			mat[16];
    register float		*itermatrix;
    register short		i,j;
    im_setup;

    if( !(
		(patch_st->u_curves == gl_u_curves) &&
		(patch_st->v_curves == gl_v_curves) &&
       	 	(patch_st->u_precision == gl_u_precision) &&
         	(patch_st->v_precision == gl_v_precision) &&
		(patch_st->u_basis == gl_u_basis) && 
	 	(patch_st->v_basis == gl_v_basis)
							)) {
	/* recalculate the interation matrices */
	if(patch_st->itermats)
	    gl_objfree(patch_st->objectheader,patch_st->itermats);
	if(!gl_make_itermats(patch_st))
	    return;
    }

    /* draw the curves.........*/
    im_pushmatrix ();
    itermatrix = patch_st->itermats;

    j = gl_v_curves;
    while (--j != -1) {
	im_pushmatrix();
	gl_multmatrix(itermatrix);
	im_movezero();
	im_curveit(gl_v_mult);
	im_popmatrix();
	itermatrix += 16;
    }

    j = gl_u_curves;
    while (--j != -1) {
	im_pushmatrix();
	gl_multmatrix(itermatrix);
	im_movezero();
	im_curveit(gl_u_mult);
	im_popmatrix();
	itermatrix += 16;
    }
    im_popmatrix();
    im_cleanup;
}

gl_make_itermats(patch_st)
Patch	*patch_st;
{
    /* save the current u and v precision and basis */
    patch_st->u_basis = gl_u_basis;
    patch_st->u_precision = gl_u_precision;
    patch_st->u_curves = gl_u_curves;
    patch_st->u_mult = gl_u_mult;
    patch_st->v_basis = gl_v_basis;
    patch_st->v_precision = gl_v_precision;
    patch_st->v_curves = gl_v_curves;
    patch_st->v_mult = gl_v_mult;

    /* allocate the storage for the iteration matrices, this is 
    u_curves + v_curves */

    patch_st->itermats = (float *)gl_objalloc(patch_st->objectheader, 
				sizeof(float) * 16 * 
					(gl_u_curves + gl_v_curves));
    if(!patch_st->itermats) {
	gl_outmem("gl_make_itermats");
	return 0;
    }
    {
    /* declare and initialize the immediate mode registers */
    float		*itermat;
    float		sx[16];
    float		sy[16];
    float		sz[16];
    float		sw[16];
    float		*xbuff,*ybuff,*zbuff,*wbuff;
    int			buffsize;
    int			i;
    char		err_flag;
    im_setup;

    buffsize = max(gl_u_curves, gl_v_curves) * 4*sizeof(float);
    xbuff = (float *)malloc(buffsize);
    ybuff = (float *)malloc(buffsize);
    zbuff = (float *)malloc(buffsize);
    wbuff = (float *)malloc(buffsize);
    if(wbuff == 0) {
	gl_outmem("gl_make_itermats");
	return 0;
    }
    im_pushmatrix ();
    /* first bring the iteration matrices up to date */

    err_flag = 0;
    if((gl_cv_basis != gl_v_basis) || (gl_cv_precision != gl_v_precision) ||
         				(gl_cu_curves != gl_u_curves))
        err_flag = !gl_update_v_itermat();

    if((gl_cu_basis != gl_u_basis) || (gl_cu_precision != gl_u_precision) ||
         				(gl_cv_curves != gl_v_curves))
        err_flag |= !gl_update_u_itermat();

    if(err_flag) {
        /* one of the basis id was not found, print an error
				    message and return after cleaning up */
        gl_ErrorHandler(ERR_BASISID, WARNING, "patch");
	im_popmatrix();
	free(xbuff);
	free(ybuff);
	free(zbuff);
	free(wbuff);
        im_cleanup;
	return(0);
    }
    gl_restorematrix(gl_v_itermat);
    im_pushmatrix();
	gl_multmatrix(patch_st->gx);
	gl_multmatrix(gl_u_itermat);
	getmatrix(sx);
	gl_iterate(gl_v_curves, xbuff);
    im_popmatrix();
    im_pushmatrix();
	gl_multmatrix(patch_st->gy);
	gl_multmatrix(gl_u_itermat);
	getmatrix(sy);
	gl_iterate(gl_v_curves, ybuff);
    im_popmatrix();
    im_pushmatrix();
	gl_multmatrix(patch_st->gz);
	gl_multmatrix(gl_u_itermat);
	getmatrix(sz);
	gl_iterate(gl_v_curves, zbuff);
    im_popmatrix();
    im_pushmatrix();
	gl_multmatrix(patch_st->gw);
	gl_multmatrix(gl_u_itermat);
	getmatrix(sw);
	gl_iterate(gl_v_curves, wbuff);

    /* also pop the v_itermat */
    im_popmatrix();

    save_curves(patch_st->itermats, xbuff, ybuff, zbuff, wbuff,
		gl_v_curves, gl_v_mult, gl_v_convmat);

    gl_restorematrix(sx);
    gl_iterate(gl_u_curves, xbuff);
    gl_restorematrix(sy);
    gl_iterate(gl_u_curves, ybuff);
    gl_restorematrix(sz);
    gl_iterate(gl_u_curves, zbuff);
    gl_restorematrix(sw);
    gl_iterate(gl_u_curves, wbuff);

    save_curves(&patch_st->itermats[16*gl_v_curves], 
		xbuff, ybuff, zbuff, wbuff, 
		gl_u_curves, gl_u_mult, gl_u_convmat);

    im_popmatrix();
    free(xbuff);
    free(ybuff);
    free(zbuff);
    free(wbuff);
    im_cleanup;
    }
    return 1;
}

gl_update_v_itermat()
{
    int mult;
    int flag = 0;
    int num_spans; /* min of 1 */

    if(gl_u_curves < 1)
	return(0);
    num_spans = ((gl_u_curves-1)>0?gl_u_curves-1:1);
    if(gl_cu_curves != gl_u_curves) {
	flag = 1;
	gl_cu_curves = gl_u_curves;
	gl_makediffmatrix(num_spans,gl_v_precmat);
    }
    if((gl_cv_precision != gl_v_precision) || flag) {
	gl_cv_precision = gl_v_precision;
	mult = gl_v_mult = num_spans;
	while (gl_v_mult < gl_v_precision)
	    gl_v_mult += mult;
	gl_makeconvmatrix(num_spans, gl_v_mult, gl_v_convmat);
    }
    gl_cv_basis = gl_v_basis;
    return(gl_builditermat(gl_v_basis, gl_v_precmat, gl_v_itermat));
}

gl_update_u_itermat()
{
    int mult;
    int flag = 0;
    int num_spans; /* min of 1 */

    if(gl_v_curves < 1)
	return(0);
    num_spans = ((gl_v_curves-1)>0?gl_v_curves-1:1);
    if(gl_cv_curves != gl_v_curves) {
	flag = 1;
	gl_cv_curves = gl_v_curves;
	gl_makediffmatrix(num_spans, gl_u_precmat);
    }
    if((gl_cu_precision != gl_u_precision) || flag) {
	gl_cu_precision = gl_u_precision;
	mult = gl_u_mult = num_spans;
	while (gl_u_mult < gl_u_precision)
	    gl_u_mult += mult;
	gl_makeconvmatrix(num_spans, gl_u_mult, gl_u_convmat);
    }
    gl_cu_basis = gl_u_basis;
    return(gl_builditermat(gl_u_basis, gl_u_precmat, gl_u_itermat));
}
