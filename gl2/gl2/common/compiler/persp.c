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

#define PERSPECTIVE_SIZE 44
INTERP_NAME (perspective);

extern int i_perspective(), i_loadmatrix();

/*
 * sets up a perspective window and loads the matrix onto the stack
 * destroying the current transformation.
 */
void perspective (fovy, aspect, near, far)
register long fovy;
float aspect, near, far;
{
    register float Zdelta;
    float sin,cotangent;
    register Matrix matrix;
    register float *farray;
    register short i;

    if (fovy <= 1 || fovy > 1800) {
 	gl_ErrorHandler(ERR_FOV, WARNING, 0);
	return;
    }
    Zdelta = far - near;
    if (aspect == 0.0 || Zdelta == 0.0) {
	gl_ErrorHandler(ERR_BADWINDOW, WARNING, "perspective");
	return;
    }
    fovy /= 2;				/* take half the angle	*/
    gl_IdentifyMatrix(matrix);
    gl_sincos (fovy,&sin,&cotangent);	/* use table lookup	*/
    cotangent = cotangent / sin;
    matrix[0][0] = cotangent / aspect;
    matrix[1][1] = cotangent;
    matrix[2][2] = -(far + near)/Zdelta;	/* note: negate Z	*/
    matrix[2][3] = -1.0;
    matrix[3][2] = -2.0 * near * far / Zdelta;
    matrix[3][3] = 0.0;

    beginpicmandef(PERSPECTIVE_SIZE);
    BEGINCOMPILE(PERSPECTIVE_SIZE);
    ADDADDR(i_perspective);
    ADDLONG(fovy);
    ADDFLOAT(aspect);
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

INTERP_LABEL (perspective, 44); /* PERSPECTIVE_SIZE == 44 */
    PC += 4;
    thread;
}
