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
#include "immatrix.h"
#include "glerror.h"

extern int i_rotatex(),i_rotatey(),i_rotatez();

void rotate(angle,axis)
    register long angle;
    register char axis;
{
    float cosine, sine;
    register long cmd;

    gl_sincos (angle,&sine,&cosine);	/* use table lookup	*/

    switch(axis & 0x7f) {
	case 'x':
	case 'X':
		if (gl_openobjhdr == 0) {
		    im_setup;
		    im_rotatex (cosine,sine,-sine,cosine);
		    im_cleanup;
		    return;
		}
		cmd = (long)i_rotatex;
		break;
	case 'y':
	case 'Y':
		if (gl_openobjhdr == 0) {
		    im_setup;
		    im_rotatey (cosine,-sine,sine,cosine);
		    im_cleanup;
		    return;
		}
		sine = -sine;
		cmd = (long)i_rotatey;
		break;
	case 'z':
	case 'Z':
		if (gl_openobjhdr == 0) {
		    im_setup;
		    im_rotatez (cosine,sine,-sine,cosine);
		    im_cleanup;
		    return;
		}
		cmd = (long)i_rotatez;
		break;
	default:
		gl_ErrorHandler(ERR_BADAXIS, WARNING, "rotate");
		return;
    }

    if (gl_checkspace(10) == 0) return;
    BEGINCOMPILE(10);
    ADDADDR(cmd);
    ADDFLOAT(cosine);
    ADDFLOAT(sine);	/* note rotatey negated sine	*/
    ADDFLOAT(-sine);
    ADDFLOAT(cosine);
    ENDCOMPILE;
}

#include "interp.h"

INTERP_NAME(rotatex);
INTERP_NAME(rotatey);
INTERP_NAME(rotatez);

static bogus ()
{
    DECLARE_INTERP_REGS;

    INTERP_ROOT_4F(rotatex);
    INTERP_ROOT_4F(rotatey);
    INTERP_ROOT_4F(rotatez);
}
