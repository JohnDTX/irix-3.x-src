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

#define ORTHO2_SIZE 44
INTERP_NAME (ortho2);

extern int i_ortho2(), i_loadmatrix();

void ortho2(left, right, bottom, top)
float	left, right, bottom, top;
{
    register Matrix matrix;
    register float *farray;
    register float Xdelta, Ydelta;
    register short i;

    Xdelta = right - left;
    Ydelta = top - bottom;
    if (Xdelta == 0.0 || Ydelta == 0.0)
	gl_ErrorHandler(ERR_BADWINDOW, WARNING, ortho2_n);

    gl_IdentifyMatrix(matrix);
    matrix[0][0] = 2.0/Xdelta;
    matrix[3][0] = -(right + left)/Xdelta;
    matrix[1][1] = 2.0/Ydelta;
    matrix[3][1] = -(top + bottom)/Ydelta;
    matrix[2][2] = -1.0;

    beginpicmandef(ORTHO2_SIZE);
    BEGINCOMPILE(ORTHO2_SIZE);
    ADDADDR(i_ortho2);
    ADDFLOAT(left);
    ADDFLOAT(right);
    ADDFLOAT(bottom);
    ADDFLOAT(top);

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

INTERP_LABEL (ortho2,44); /* ORTHO2_SIZE  == 44 */
    PC += 4;
    thread;
}
