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
#include "splinegl.h"
#include "glerror.h"

static int	basis_ids = {1};

void defbasis(id, basis)
short 	id;
float	*basis;
{
    register float	*from;
    register float	*to;
    register Basis_entry	*ptr;

    if(!(ptr = gl_findbasis(id,FALSE))) {
        if(gl_basis_list) {
            ptr = (Basis_entry *) gl_grealloc(gl_basis_list, 
			    ((short)(gl_num_bases+1))*sizeof(Basis_entry));
	    if(!ptr)
	        return;
	    gl_basis_list = ptr;	
        } else {
	    ptr = (Basis_entry *) gl_galloc(sizeof(Basis_entry));
	    if(!ptr)
	        return;
	    gl_basis_list = ptr;	
	    gl_num_bases = 0;
        }
        ptr = &gl_basis_list[gl_num_bases++];
    }
    ptr->id = id;
    ptr->internal_id = basis_ids++;
    to = ptr->basis;
    from = basis;

    /* this should be put in the other major order */
    *to++ = (from[0]);
    *to++ = (from[4]);
    *to++ = (from[8]);
    *to++ = (from[12]);

    *to++ = (from[1]);
    *to++ = (from[5]);
    *to++ = (from[9]);
    *to++ = (from[13]);

    *to++ = (from[2]);
    *to++ = (from[6]);
    *to++ = (from[10]);
    *to++ = (from[14]);

    *to++ = (from[3]);
    *to++ = (from[7]);
    *to++ = (from[11]);
    *to++ = (from[15]);
}

Basis_entry *gl_findbasis(id, internal)
short 	id;
short	internal;
{
    register short 		i;
    register Basis_entry	*ptr = gl_basis_list;
    register int		found;

    found = FALSE;
    i = gl_num_bases;
    if(ptr)
        while (--i != -1) {
	    if((internal ? ptr->internal_id : ptr->id) != id)
	        ptr++;
	    else {
	        found = TRUE;
	        break;
	    }
        }
    if(found)
        return(ptr);
    else
	return(0);
}
