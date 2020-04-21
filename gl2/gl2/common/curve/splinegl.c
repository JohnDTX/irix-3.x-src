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

#include "gl.h"
#include "gltypes.h"
#include "splinegl.h"

short		gl_num_bases = {0};

Basis_entry	*gl_basis_list = {0};
				/* an array of ids and matrices, used
				by both curves and patches */

float	gl_curveitermat[16] = {0};	/* the current iteration matrix 
				this is either set up explicitly by calls
				to curvbasis and gl_curvprecision, or it
				is set up on the fly by gl_findprecision */

Matrix gl_B_spline = 
		{{ -1.0/6.0, 3.0/6.0, -3.0/6.0, 1.0/6.0},
		{ 3.0/6.0, -6.0/6.0, 3.0/6.0, 0.0/6.0},
		{ -3.0/6.0, 0.0/6.0, 3.0/6.0, 0.0/6.0},
		{ 1.0/6.0, 4.0/6.0, 1.0/6.0, 0.0/6.0}};

long	gl_ccurvebasis = {0};	/* holds the index of the basis 
				used in the above matrix */

long	gl_curvebasis = {0};	/* holds the index of the current basis 
				matrix */

float	gl_curveprecmat[6];  	/* holds the current forward difference
				matrix */

short	gl_ccurveprecision = {0};  /* holds the curve precision of the above
				matrix */

short	gl_curveprecision = {0};  /* holds the current curve precision, if
				0 then the precision is figured on the fly */


/* globals for patches */

float	gl_u_itermat[16] = {0};	
float	gl_v_itermat[16] = {0};	/* the current iteration matrix 
				this is either set up explicitly by calls
				to curvbasis and gl_curvprecision */

long	gl_u_basis = {0};	
long	gl_v_basis = {0};		/* holds the current basis matrix */
long	gl_cu_basis = {0};	
long	gl_cv_basis = {0};		/* holds the current basis matrix */

float	gl_u_precmat[6]; 	/* holds the current forward difference
				matrix */
float	gl_v_precmat[6]; 	/* holds the current forward difference
				matrix */

short	gl_u_precision = {0}; 
short	gl_v_precision = {0};	/* holds the current curve precision */

short	gl_cu_precision = {0}; 
short	gl_cv_precision = {0};	/* holds the curve precision used in
				the iteration matrix above */

short	gl_u_curves = {0}; 
short	gl_v_curves = {0};		
short	gl_cu_curves = {0}; 
short	gl_cv_curves = {0};		

float	gl_u_convmat[6]; 		/* converts between the above forward
				difference matrix and the one indicated by 
				the precision below */
float	gl_v_convmat[6];	

short	gl_u_mult = {0};
short	gl_v_mult = {0};	/* holds the precision at which curves 
				are actually drawn, higher than the mesh
				resolution. */

short	gl_cu_mult = {0};
short	gl_cv_mult = {0};		/* holds the precision at which curves 
				are actually drawn, higher than the mesh
				resolution. */
