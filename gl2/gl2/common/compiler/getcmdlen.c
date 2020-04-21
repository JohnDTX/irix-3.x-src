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

#include "gl.h"
#include "TheMacro.h"

gl_getcmdlength(x, size)
long	*x;
register short	size;
{
    register long r;
    register short retval;

    if ((r = *(x - 1)) < MAXSTANDARDLENGTH)
	return r;

    /* note that for polygons, size is the number of points - 1	*/
    retval = ++size;

    switch (r) {
	case SPOLYLENGTH:	/* NOTE: size * 7	*/
	    retval += size;
	case POLYLENGTH:	/* NOTE: size * 6	*/
	    retval += size;
	case SPOLY2LENGTH:	/* NOTE: size * 5	*/
	    retval += size;
	case POLY2LENGTH:	/* NOTE: size * 4	*/
	case SPOLYLENGTH_S:
	    retval += size;
	case POLYLENGTH_S:	/* NOTE: size * 3	*/
	case SPOLY2LENGTH_S:
	    retval += size;
	case POLY2LENGTH_S:	/* NOTE: size * 2	*/
	    retval += size;
	    return (retval + 3);
	case CALLFUNCLENGTH:
	    return((size-1) * 2 + 5);
	default:
	    printf("gl_getcmdlength: BUG!! illegal length\n");
    }
}
