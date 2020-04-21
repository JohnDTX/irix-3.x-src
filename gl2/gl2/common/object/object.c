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
short *gl_addchunk();
extern int i_retsym();
extern int gl_checkspacefunc();
extern int gl_freeallobjectstuff();

Object getopenobj()
{
    return gl_openobj;
}

gl_docloseobj()
{
    closeobj();
}

void makeobj(obj)
register Object obj;
{
    if(obj < 0) {
	gl_ErrorHandler(ERR_ILLEGALID, WARNING, "makeobj");
	return;
    }
    if(!gl_checkspace) {
	gl_checkspace = gl_checkspacefunc;
	gl_closeobj = gl_docloseobj;
	gl_initobj = gl_freeallobjectstuff;
	/* gl_freeallobjectstuff(); */
    }
    gl_objsizefrozen = 1;
    gl_makeobject(obj);
}


chunksize(n)
long n;
{
    if(n <= 0)
	n = 1020;
    if(gl_objsizefrozen) {
	gl_ErrorHandler(ERR_SIZEFIXED, WARNING, "chunksize");
	return;
    }
    gl_objchunksize = n;
    gl_objsizefrozen = 1;
}

gl_makesysobj(obj)
register Object obj;
{
    if(obj > 0) {
	gl_ErrorHandler(ERR_ILLEGALID, WARNING, "gl_makesysobj");
	return(0);
    }
    return(gl_makeobject(obj));
}

/*
 * gl_makeobject: deletes the contents of the old object n (if it exists),
 * and creates (or re-initializes) an objhdr.  Opens the new object for
 * editing.  0 is returned if makeobj fails.
 */

gl_makeobject(obj)
register Object obj;
{
    if(gl_openobjhdr) 
	closeobj();
    if(isobj(obj)) 
	delobj(obj);
    if(!(gl_openobjhdr = gl_findobjhdr(obj))) {
	if(0 == (gl_openobjhdr = gl_getnewobjhdr(obj)))
	    goto outmemerror;
    }
    if(!(gl_openobjhdr->head = gl_addchunk())) {
	gl_openobjhdr = 0;
	goto outmemerror;
    }
    gl_currentpos = gl_openobjhdr->head;
    gl_openobjhdr->tailptr = gl_currentpos;
    gl_openobjhdr->tailend = gl_currentpos + (gl_objchunksize>>1);
    gl_openobjhdr->valid = 1;
    gl_openobjhdr->datasize = 0;
    *(long *)gl_currentpos = (long)i_retsym;
    gl_currentend = gl_currentpos + (gl_objchunksize>>1);
    gl_openobj = obj;
    gl_replacemode = 0;
    return 1;
outmemerror:
    gl_outmem("makeobj");
    return 0;
}

objhdr *gl_getnewobjhdr(obj)
Object obj;
{
    register objhdr *newhdr;

    if(!(newhdr = (objhdr *)malloc(sizeof(objhdr))))
	return 0;
    newhdr->head = 0;
    newhdr->valid = 0;
    newhdr->chunks = 0;
    newhdr->physicalsize = 0;
    newhdr->tags = 0;
    newhdr->strings = 0;
    newhdr->tailptr = 0;
    if(gl_addhash(newhdr, obj, -1) == 0) {
	free((char *)newhdr);
	return 0;
    }
    return newhdr;
}

short *gl_addchunk()
{
    register cons	*consptr;
    register short	*newchunk;

    if(!(consptr = (cons *)malloc(sizeof(cons))))
	return 0;
    if(!(newchunk = (short *)malloc(gl_objchunksize))) {
	free((char *)consptr);
	return 0;
    }
    consptr->item = newchunk;
    consptr->link = gl_openobjhdr->chunks;
    gl_openobjhdr->chunks = consptr;
    gl_openobjhdr->physicalsize += (gl_objchunksize>>1);
    return newchunk;
}
