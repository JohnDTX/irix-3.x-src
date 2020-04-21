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

void rot(angle,axis)
    register float angle;
    register char axis;
{
    float cosine1, sine1;
    float cosine, sine;
    float nangle;
    register long cmd;
    register int iangle;

    while(angle<0.0)
	angle += 360.0;
    angle *= 10;
    iangle = angle;
    gl_sincos(iangle,&sine,&cosine);		/* use table lookup	*/
    gl_sincos(iangle+1,&sine1,&cosine1);	/* use table lookup	*/
    angle = angle - iangle;
    nangle = 1.0-angle;
    sine = nangle*sine +  angle*sine1;
    cosine = nangle*cosine +  angle*cosine1;
    
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
		gl_ErrorHandler(ERR_BADAXIS, WARNING, "rot");
		return;
    }

    if (gl_checkspace(10) == 0) return;
    BEGINCOMPILE(10);
    ADDADDR(cmd);
    ADDFLOAT(cosine);
    ADDFLOAT(sine);	/* note roty negated sine	*/
    ADDFLOAT(-sine);
    ADDFLOAT(cosine);
    ENDCOMPILE;
}
