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
extern short	*gl_addchunk();
extern int i_jump();

void editobj(obj)
Object obj;
{
    if(gl_openobjhdr) closeobj();
    if((!(gl_openobjhdr = gl_findobjhdr(obj))) || (gl_openobjhdr->valid == 0)) {
	    gl_ErrorHandler(ERR_NOOPENOBJ, WARNING, "editobj");
	    return;
    }
    if (gl_currentpos == 0) return;
    gl_currentpos = gl_openobjhdr->tailptr;
    gl_currentend = gl_openobjhdr->tailend;
    gl_openobj = obj;
    gl_replacemode = 0;
}

static char objreplace_n[] = "objreplace";

void objreplace(tag)
register Tag tag;
{
    short *tempptr;

    if(gl_openobjhdr == 0) {
	gl_ErrorHandler(ERR_NOOPENOBJ, WARNING, objreplace_n);
	return;
    }
    if(tag == ENDTAG)
	return;
    if(tag == STARTTAG) {
	gl_currentpos = gl_openobjhdr->head;
	gl_replacemode = 1;
	return;
    }
    if(!(tempptr = (short *)gl_findhash(gl_openobj, tag))) {
	gl_ErrorHandler(ERR_NOSUCHTAG, WARNING, objreplace_n);
	return;
    }
    gl_currentpos = tempptr;
    gl_replacemode = 1;
}

static char objinsert_n[] = "objinsert";

void objinsert(tag)
register Tag tag;
{
    register short	*tagloc, *newblock;

    if(gl_openobjhdr == 0) {
	gl_ErrorHandler(ERR_NOOPENOBJ, WARNING, objinsert_n);
	return;
    }
    if(tag == ENDTAG) {
	editobj(gl_openobj);
	gl_replacemode = 0;
	return;
    }
    if(tag == STARTTAG) {
	if(!(newblock = gl_addchunk()))
	    goto outmemerror;
	*(long *)newblock = (long)i_jump;
	*(long *)(newblock +2) = (long)gl_openobjhdr->head;
	gl_openobjhdr->head = newblock;
	gl_currentpos = newblock;
	gl_currentend = newblock+(gl_objchunksize>>1);
	gl_replacemode = 0;
	return;
    }
    if(!(tagloc = (short *)gl_findhash(gl_openobj, tag))) {
	gl_ErrorHandler(ERR_NOSUCHTAG, WARNING, objinsert_n);
	return;
    }
    if(!(newblock = gl_addchunk()))
	goto outmemerror;
    *(long *)tagloc = (long)i_jump;
    tagloc += 2;
    *(long *)tagloc = (long) newblock;
    tagloc += 2;
    gl_currentpos = newblock;
    gl_currentend = newblock + (gl_objchunksize>>1);
    *(long *)newblock = (long)i_jump;
    *(long *)(newblock+2) = (long)tagloc;
    gl_replacehash(newblock, gl_openobj, tag);
    gl_addtag(tag);
    gl_replacemode = 0;
    return;    
outmemerror:
    gl_outmem(objinsert_n);
    return;
}

