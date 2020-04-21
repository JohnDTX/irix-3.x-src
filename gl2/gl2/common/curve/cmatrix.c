#include "gl.h"
#include "globals.h"
#include "immed.h"
#include "curvmac.h"


gl_multmatrix(mat)
float *mat;
{
    im_setup;
    im_multmatrix(mat);
    im_cleanup;
}

gl_restorematrix(mat)
float *mat;
{
    im_setup;
    im_restorematrix(mat);
    im_cleanup;
}

gl_multprecmat(mat)
float *mat;
{
    im_setup;
    im_multprecmatrix(mat);
    im_cleanup;
}
