#ifndef SPLINEDEF
#define SPLINEDEF
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

/* include the curve macros */
#include "curvmac.h"

typedef struct {
    short	id;
    long	internal_id;
    float	basis[16];
} Basis_entry;

typedef struct {
    short	precision;
    long	basis;
    float	geom[16];
    float	itermat[16];
} Curve;

typedef struct {
    short	precision;
    long	basis;
    short	npts;
    float	*geom;
    float	*itermat;
} Curves;

typedef struct {
    objhdr	*objectheader;
    float	gx[16];
    float	gy[16];
    float	gz[16];
    float	gw[16];

    /* specifies the number of "u" curves drawn in approximating the
    surface */
    short	u_curves;

    /* specifies the minimum precision with which mesh curves are drawn */
    short	u_precision;

    /* these values are the precision of the mesh curves after the increase
    in resolution of the curves */
    short	u_mult;

    /* specifies the number of "u" curves drawn in approximating the
    surface */
    short	v_curves;

    /* specifies the minimum precision with which mesh curves are drawn */
    short	v_precision;

    /* these values are the precision of the mesh curves after the increase
    in resolution of the curves */
    short 	v_mult;

    long 	u_basis;
    long 	v_basis;

    /* the first u_precision+1 entries are curves of constant u 
    the remaining curves are of constant v */
    float	*itermats;

} Patch;

extern Basis_entry	*gl_findbasis();

extern short		gl_num_bases;

extern Basis_entry	*gl_basis_list;
				/* an array of ids and matrices, used
				by both curves and patches, stored in
				Transposed GE format */

extern float	gl_curveitermat[16];	
				/* the current iteration matrix 
				this is set up by calls
				to curvbasis and curvprecision.
				in GE format*/

extern Matrix	gl_B_spline;	

extern long	gl_ccurvebasis;	
				/* holds the index of the current basis 
				matrix */
extern long	gl_curvebasis;	

extern float	gl_curveprecmat[6];	
				/* holds the current forward difference
				matrix, in GE precision mat format */

extern short	gl_ccurveprecision; /* holds the current curve precision, if
				0 then the precision is figured on the fly */
extern short	gl_curveprecision; 

/* globals for patches */

extern float	gl_u_itermat[16];	
extern float	gl_v_itermat[16];	
				/* the current iteration matrix 
				this is either set up explicitly by calls
				to curvbasis and gl_curvprecision, or it
				is set up on the fly by gl_findprecision */

extern long	gl_u_basis;	
extern long	gl_v_basis;	
extern long	gl_cu_basis;	
extern long	gl_cv_basis;	
				/* holds the current basis matrix */

extern float	gl_u_precmat[6];	
extern float	gl_v_precmat[6];	
				/* holds the current forward difference
				matrix */

extern short	gl_u_precision; 
extern short	gl_v_precision; 
extern short	gl_cu_precision; 
extern short	gl_cv_precision; 
				/* holds the minimum current mesh curve 
				precision */

extern short	gl_u_curves;
extern short	gl_v_curves;
extern short	gl_cu_curves;
extern short	gl_cv_curves;
				/* holds the number of curves used to
				represent each patch */

extern float	gl_u_convmat[6]; 	/* converts between the above forward
					difference matrix and the one 
					indicated by the precision below */
extern float	gl_v_convmat[6];	

extern short	gl_u_mult;
extern short	gl_v_mult;
extern short	gl_cu_mult;
extern short	gl_cv_mult;		/* holds the precision at which curves 
				are actually drawn, higher than the mesh
				resolution. */

#endif SPLINEDEF
