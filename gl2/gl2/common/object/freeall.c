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

#include	"globals.h"
#include	"splinegl.h"

gl_freeallobjectstuff()
{
    register char	*memptr = (char *)gl_memory_list;
    register char	*gl_temp;

    gl_freeallobjects();
    while(memptr) {
	gl_temp = memptr;
	memptr = (char *)(*(long *)memptr);
	free(gl_temp);
    }
    gl_memory_list = 0;
    /*
    ** must clear basis globals since we just freed all the data (TN)
    */
    gl_basis_list = 0;
    gl_num_bases = 0;
}
