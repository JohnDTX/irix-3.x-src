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

extern objhdr	*gl_findobjhdr(), *gl_getnewobjhdr();

void closeobj()
{
    register psize, dsize;

    if(gl_openobjhdr) {
	psize = gl_openobjhdr->physicalsize;
	dsize = gl_openobjhdr->datasize;
	if(psize > (gl_objchunksize<<2) && ((dsize<<1) < psize))
	    compactify(gl_openobj);
	gl_openobjhdr = 0;
	gl_openobj = -1;
	gl_replacemode = 0;
    }
}

objhdr *gl_findobjhdr(n)
Object n;
{
    return (objhdr *)gl_findhash(n, -1);
}

/* isobj: returns 0 if the object does not exist or is not valid */

long isobj(obj)
Object obj;
{
    register objhdr *hdr;

    if(!(hdr = (objhdr *)gl_findhash(obj, -1)))
	return 0;
    if(hdr->valid == 0)
	return 0;
    return 1;
}

