#include "globals.h"
#include "glerror.h"
#include "immatrix.h"
#include "splinegl.h"

gl_builditermat(basis, precmat, itermat)
long	basis;
Coord	precmat[6];
Matrix	itermat;
{
    register Basis_entry	*basis_ptr;

    if(!(basis_ptr = gl_findbasis(basis,TRUE))) {
	gl_ErrorHandler(ERR_BASISID, WARNING, "builditermat");
	return(0);
    }
    {
	im_setup;

	gl_restorematrix(basis_ptr->basis);
	gl_multprecmat(precmat);
	getmatrix(itermat);
	im_cleanup;
    }
    return(1);
}
