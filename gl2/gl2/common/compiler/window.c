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

#define WINDOW_SIZE 48
INTERP_NAME (window);

extern int i_window(), i_loadmatrix();

/*
 * sets up a 3d PERSPECTIVE window and loads the matrix onto
 * the stack destroying the current transformation.
 */
void window(left, right, bottom, top, near, far)
float left, right, bottom, top, near, far;
{
    register float Xdelta, Ydelta, Zdelta;
    register Matrix mat;
    register float *farray;
    register short i;

    Xdelta = right - left;
    Ydelta = top - bottom;
    Zdelta = far - near;
    if (Xdelta == 0.0 || Ydelta == 0.0 || Zdelta == 0.0)
	gl_ErrorHandler(ERR_BADWINDOW, WARNING, "window");
    mat[0][0] = near * 2.0/Xdelta;
    mat[1][1] = near * 2.0/Ydelta;
    mat[2][0] = (right + left)/Xdelta;		/* note: negate Z	*/
    mat[2][1] = (top + bottom)/Ydelta;
    mat[2][2] = -(far + near)/Zdelta;
    mat[2][3] = -1.0;
    mat[3][2] = -2.0 * near * far / Zdelta;
    mat[0][1] = mat[0][2] = mat[0][3] =
    mat[1][0] = mat[1][2] = mat[1][3] =
    mat[3][0] = mat[3][1] = mat[3][3] = 0.0;

    beginpicmandef(WINDOW_SIZE);
    BEGINCOMPILE(WINDOW_SIZE);
    ADDADDR(i_window);
    ADDFLOAT(left);
    ADDFLOAT(right);
    ADDFLOAT(bottom);
    ADDFLOAT(top);
    ADDFLOAT(near);
    ADDFLOAT(far);

    ADDADDR(i_loadmatrix);
    farray = &mat[0][0];
    for (i=16; i>0; i--) {
	ADDFLOAT(*farray++);
    }
    ENDCOMPILE;
    endpicmandef;
}

#include "interp.h"

static bogus()
{
    DECLARE_INTERP_REGS;

INTERP_LABEL (window, 48); /* WINDOW_SIZE == 48 */
    PC += 6;
    thread;
}
