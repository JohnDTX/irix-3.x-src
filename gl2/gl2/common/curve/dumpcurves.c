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
#include "immed.h"
#include "splinegl.h"
#include "glerror.h"

gl_dump_curves(xbuff, ybuff, zbuff, wbuff, count, mult, convmat)
register float	*xbuff;
register float	*ybuff;
register float	*zbuff;
register float	*wbuff;
short	mult;
short	count;
{
    register short	i;
    float		mat[16];

    i = count;
    {
	im_setup;

	while (--i != -1) {
	    im_pushmatrix();
	    im_itermultmatrix(xbuff, ybuff, zbuff, wbuff);
	    gl_multprecmat(convmat);
	    im_movezero();
	    im_curveit(mult);
	    im_popmatrix();
	}
	im_cleanup;
    }
}
