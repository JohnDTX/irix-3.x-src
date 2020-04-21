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

#define then

extern float gl_xtable[901];

gl_sincos (angle,sin,cos)
    register long angle;
    register float *sin, *cos;
{
    register long quad;
    register long temp;
    register float *f;

    while (angle < 0) angle += 3600;
    quad = 0;
    while (angle > 900) {
	quad++;
	angle -= 900;
    }
    if (quad & 1)
    then f = &gl_xtable[900-angle];
    else f = &gl_xtable[angle];

    if (sin) {
	if (quad & 2)
	then *sin = -*f;
	else *sin = *f;
    }
    if (cos) {
	quad++;
	f = (float *) ((int)&gl_xtable[0] + (int)&gl_xtable[900] - (int)f);
	if (quad & 2)
	then *cos = -*f;
	else *cos = *f;
    }
}
