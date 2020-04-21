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
#include "splinegl.h"
#include "glerror.h"

extern long	i_callfunc();

void curvebasis (basis)
long	basis;
{
    extern	gl_update_basis();
    Basis_entry	*basis_ptr;

    if(!(basis_ptr = gl_findbasis(basis,FALSE))) {
	defbasis(basis,gl_B_spline);
        basis_ptr = gl_findbasis(basis,FALSE);
    }
    if(gl_openobjhdr == 0) {
	gl_curvebasis = basis_ptr->internal_id;
	return;
    }
    if(gl_checkspace(7) == 0) 
	return;
    BEGINCOMPILE(7);
    ADDADDR(i_callfunc);
    ADDSHORT(1);		/* one parameter - the matrix */
    ADDADDR(gl_update_basis);
    ADDLONG(basis_ptr->internal_id);
    ENDCOMPILE;
}

gl_update_basis(n, basis)
int	n;
long	basis;
{
    gl_curvebasis = basis;
}

void curveprecision (precision)
short	precision;
{
    extern gl_update_precision();

    if(precision < 1) {
	gl_ErrorHandler(ERR_CURVPREC, WARNING, "curvprecision");
	return;
    }
    if(gl_openobjhdr == 0) {
	gl_curveprecision = precision;
	return;
    }
    if(gl_checkspace(7) == 0) return;
    BEGINCOMPILE(7);
    ADDADDR(i_callfunc);
    ADDSHORT(1);
    ADDADDR(gl_update_precision);
    ADDLONG(precision);
    ENDCOMPILE;
}

gl_update_precision(n, precision)
int	n;
short	precision;
{
    gl_curveprecision = precision;
}
