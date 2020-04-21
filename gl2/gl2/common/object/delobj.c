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

void delobj(obj)
Object obj;
{
    register objhdr	*hdr;
    register cons	*consptr, *savecons;
    objhdr		*save;
    Object		saveobj;

    if(!(hdr = gl_findobjhdr(obj)))
	return;
    consptr = hdr->chunks;
    while(consptr) {
	free((char *)(consptr->item));
	savecons = consptr;
	consptr = consptr->link;
	free((char *)savecons);
    }
    hdr->valid = 0;
    hdr->physicalsize = 0;
    hdr->chunks = 0;
    hdr->head = 0;
    save = gl_openobjhdr;
    saveobj = gl_openobj;
    gl_openobjhdr = hdr;
    gl_openobj = obj;
    while(consptr = hdr->tags)
	deltag(consptr->item);
    while(consptr = hdr->strings) {
	free(consptr->item);
	hdr->strings = consptr->link;
	free(consptr);
    }
    if(save != hdr) {
	gl_openobjhdr = save;
	gl_openobj = saveobj;
    } else {
	gl_openobjhdr = 0;
	gl_openobj = -1;
    }
    return;
}

