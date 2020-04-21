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
#include "imsetup.h"

/*
 *  this whole routine should be rewritten to use the GE microcode
 *  change I hope marc hannah is going to incorporate. Even if he doesn't
 *  it should probobly use the GEs 
 */
gl_iterate(count, buffer)
register short	count;
register float	*buffer;
{
    float	buff[16];
    register	float	*from;

    getmatrix(buff);
    while (--count != -1) {
	/* pull out the bottom row */
	from = &buff[12];
	*buffer++ = *from++;
	*buffer++ = *from++;
	*buffer++ = *from++;
	*buffer++ = *from++;
	gl_iter(buff);
    }
}

gl_iter(buff)
register Matrix buff;
{
    register short i, j;
    register float *p;

    for (i = 3; i > 0; i--) {
	p = &buff[i][0];
	for (j = 4; --j != -1;) {
	    *p += *(p-4);
	    p++;
	}
    }
}
