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
#include "glerror.h"

#define ORTHO_SIZE 48
INTERP_NAME (ortho);

extern int i_ortho(), i_loadmatrix();

void ortho(left, right, bottom, top, near, far)
float	left, right, bottom, top, near, far;
{
    register Matrix matrix;
    register float *farray;
    register float Xdelta, Ydelta, Zdelta;
    register short i;

    Xdelta = right - left;
    Ydelta = top - bottom;
    Zdelta = far - near;
    if (Xdelta == 0.0 || Ydelta == 0.0 || Zdelta == 0.0)
	gl_ErrorHandler(ERR_BADWINDOW, WARNING, ortho_n);
    gl_IdentifyMatrix(matrix);
    matrix[0][0] = 2.0/Xdelta;
    matrix[3][0] = -(right + left)/Xdelta;
    matrix[1][1] = 2.0/Ydelta;
    matrix[3][1] = -(top + bottom)/Ydelta;
    matrix[2][2] = -2.0/Zdelta;		/* note: negate Z	*/
    matrix[3][2] = -(far + near)/Zdelta;

    beginpicmandef(ORTHO_SIZE);
    BEGINCOMPILE(ORTHO_SIZE);
    ADDADDR(i_ortho);
    ADDFLOAT(left);
    ADDFLOAT(right);
    ADDFLOAT(bottom);
    ADDFLOAT(top);
    ADDFLOAT(near);
    ADDFLOAT(far);

    ADDADDR(i_loadmatrix);
    farray = &matrix[0][0];
    for (i=16; i>0; i--) {
	ADDFLOAT(*farray++);
    }
    ENDCOMPILE;
    endpicmandef;
}

#include "interp.h"

static bogus ()
{
    DECLARE_INTERP_REGS;

INTERP_LABEL (ortho, 48); /* ORTHO_SIZE == 48 */
    PC += 6;
    thread;
}
