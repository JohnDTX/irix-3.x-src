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
extern int i_tag();

void maketag(tag)
register Tag tag;
{
    register cons	*consptr;

    if(tag < 0) {
	gl_ErrorHandler(ERR_NEGTAG, WARNING, "maketag");
	return;
    }
    if(gl_openobjhdr == 0)
	return;
    if(istag(tag)) {
	gl_ErrorHandler(ERR_TAGEXISTS, WARNING, "maketag");
	return;
    }
    if(gl_replacemode == 1) {
	gl_ErrorHandler(ERR_TAGINREPLACE, WARNING, "maketag");
	return;
    }
    if(!gl_makeroom(4))
	goto outofmem;
    if(!(gl_addhash(gl_currentpos, gl_openobj, tag)))
	goto outofmem;
    if(!(consptr = (cons *)malloc(sizeof(cons)))) {
	gl_removehash(gl_openobj, tag);
	goto outofmem;
    }
    consptr->link = gl_openobjhdr->tags;
    consptr->item = (short *)tag;
    gl_openobjhdr->tags = consptr;
    gl_openobjhdr->datasize += 4;
    gl_addtag(tag);
    return;
outofmem:
    gl_outmem("maketag");
    return;
}

long istag(tag)
Tag tag;
{
    if(tag == STARTTAG || tag == ENDTAG)
	return 1;
    if(gl_findhash(gl_openobj, tag) && (tag != -1))
	return 1;
    return 0;
}

gl_addtag(n)
Tag n;
{
    if(!(gl_makeroom(4))) {
	gl_outmem("gl_addtag");
	return 0;
    }
    if(gl_currentpos == gl_openobjhdr->tailptr)
	gl_openobjhdr->tailptr += 4;
    *(long *)(gl_currentpos + 6) = *(long *)(gl_currentpos + 2);
    *(long *)(gl_currentpos + 4) = *(long *)(gl_currentpos);
    *(long *)gl_currentpos = (long)i_tag;
    gl_currentpos += 2;
    *(long *)gl_currentpos = n;
    gl_currentpos += 2;
    return 1;
}
