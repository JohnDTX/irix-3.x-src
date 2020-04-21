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
#include "immatrix.h"
#include "splinegl.h"

save_curves(buffer, xbuff, ybuff, zbuff, wbuff, count, mult, convmat)
register float	*buffer;
float		*xbuff;
float		*ybuff;
float		*zbuff;
float		*wbuff;
short		count;
short		mult;
float		*convmat;	
{
    register short	i;
    register short	q;
    float		*ptr;
    float		*sav;

    im_setup;
    ptr = buffer;
    i = count;
    {
	register	float *x = xbuff;
	register	float *y = ybuff;
	register	float *z = zbuff;
	register	float *w = wbuff;

	while (--i != -1) {
	    *buffer++ = *x++;
	    *buffer++ = *y++;
	    *buffer++ = *z++;
	    *buffer++ = *w++;

	    *buffer++ = *x++;
	    *buffer++ = *y++;
	    *buffer++ = *z++;
	    *buffer++ = *w++;

	    *buffer++ = *x++;
	    *buffer++ = *y++;
	    *buffer++ = *z++;
	    *buffer++ = *w++;

	    *buffer++ = *x++;
	    *buffer++ = *y++;
	    *buffer++ = *z++;
	    *buffer++ = *w++;
	}
    }

    /* now convert each iteration matrix to a higher precision */
    buffer = ptr;
    i = count;
    while (--i != -1) {
	im_do_loadmatrix(buffer);
	gl_multprecmat(convmat);
	getmatrix(buffer);
	buffer += 16;
    }
    im_cleanup;
}
