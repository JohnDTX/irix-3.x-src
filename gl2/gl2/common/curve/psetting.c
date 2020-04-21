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

extern int i_callfunc();

void patchbasis(ubasis, vbasis)
long	ubasis, vbasis;
{
    extern	gl_update_pbasis();
    Basis_entry *ubasis_ptr, *vbasis_ptr;

    if(!(ubasis_ptr = gl_findbasis(ubasis,FALSE))) {
	defbasis(ubasis,gl_B_spline);
        ubasis_ptr = gl_findbasis(ubasis,FALSE);
    }
    if(!(vbasis_ptr = gl_findbasis(vbasis,FALSE))) {
	defbasis(vbasis,gl_B_spline);
        vbasis_ptr = gl_findbasis(vbasis,FALSE);
    }
    if(gl_openobjhdr == 0) {
	gl_u_basis = ubasis_ptr->internal_id;
	gl_v_basis = vbasis_ptr->internal_id;
	return;
    }
    if(gl_checkspace(9) == 0)  
	return;
    BEGINCOMPILE(9);
    ADDADDR(i_callfunc);
    ADDSHORT(2);		/* two parameters - the two ids */
    ADDADDR(gl_update_pbasis);
    ADDLONG(ubasis_ptr->internal_id);
    ADDLONG(vbasis_ptr->internal_id);
    ENDCOMPILE;
}

gl_update_pbasis(n, ubasis, vbasis)
int	n;
long	ubasis;
long	vbasis;
{
    gl_u_basis = ubasis;
    gl_v_basis = vbasis;
}

void patchprecision (uprecision, vprecision)
short	uprecision;
short	vprecision;
{
    extern gl_update_pprecision();

    if((uprecision < 1) || (vprecision < 1)) {
	gl_ErrorHandler(ERR_PATPREC, WARNING, "patchprecision");
	return;
    }
    if(gl_openobjhdr == 0) {
	gl_u_precision = uprecision;
	gl_v_precision = vprecision;
        return;
    }
    if(gl_checkspace(9) == 0) 
	return;
    BEGINCOMPILE(9);
    ADDADDR(i_callfunc);
    ADDSHORT(2);
    ADDADDR(gl_update_pprecision);
    ADDLONG(uprecision);
    ADDLONG(vprecision);
    ENDCOMPILE;
}

gl_update_pprecision(n, uprecision, vprecision)
int		n;
short	uprecision;
short	vprecision;
{
    gl_u_precision = uprecision;
    gl_v_precision = vprecision;
}

void patchcurves (ucurves, vcurves)
short	ucurves;
short	vcurves;
{
    extern gl_update_pcurves();

    if((ucurves < 1) || (vcurves < 1)) {
	gl_ErrorHandler(ERR_PATCURVES, WARNING, "patchcurves");
	return ;
    }
    if(gl_openobjhdr == 0) {
        gl_v_curves = vcurves;
	gl_u_curves = ucurves;
        return ;
    }
    if(gl_checkspace(9) == 0) 
	return ;
    BEGINCOMPILE(9);

    ADDADDR(i_callfunc);
    ADDSHORT(2);
    ADDADDR(gl_update_pcurves);
    ADDLONG(ucurves);
    ADDLONG(vcurves);

    ENDCOMPILE;

    return ;
}

gl_update_pcurves(n, ucurves, vcurves)
int	n;
short	ucurves;
short	vcurves;
{
    gl_v_curves = vcurves;
    gl_u_curves = ucurves;
}
