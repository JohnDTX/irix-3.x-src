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
#include "glerror.h"

extern objhdr	*gl_findobjhdr(), *gl_getnewobjhdr();
extern int i_jump();

void deltag(tag)
Tag tag;
{
    register short	*tagloc;
    register cons	*consptr, *saveptr;

    if(gl_openobjhdr == 0)
	return;
    if(tag < 0)
	return;
    if(!(tagloc = (short *)gl_findhash(gl_openobj, tag)))
	return;
    *(long *)tagloc = (long)i_jump;
    tagloc += 2;
    *(long *)tagloc = (long)(tagloc + 2);
    gl_removehash(gl_openobj, tag);
    gl_openobjhdr->datasize -= 4;
    if((consptr = gl_openobjhdr->tags)->item == (short *)tag) {
	gl_openobjhdr->tags = consptr->link;
	free((char *)consptr);
	return;
    }
    saveptr = consptr;
    consptr = consptr->link;
    while(consptr) {
	if(consptr->item == (short *)tag) {
	    saveptr->link = consptr->link;
	    free((char *)consptr);
	    return;
	}
	saveptr = consptr;
	consptr = consptr->link;
    }
    gl_ErrorHandler(ERR_DELTAG, FATAL, "deltag");
    return;	/* should never get here */
}

