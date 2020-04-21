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

extern objhdr *gl_findobjhdr(), *gl_getnewobjhdr();
extern short  *gl_addchunk();
extern int i_jump(), i_retsym(), i_tag(), i_slopnop2(), i_slopnop3();

/*
 * newtag inserts a new tag offset distance from the old tag.  The current
 * editing position points just beyond the new tag.
 */
void newtag(newtag, oldtag, offset)
Tag	newtag, oldtag;
long	offset;
{
    register short	*posn;
    register long	i;
    register short	*newptr;
    short		*saveposn, *savenew;

    if(gl_openobjhdr == 0) {
	gl_ErrorHandler(ERR_NOOPENOBJ, WARNING, "newtag");
	return;
    }
    if(newtag < 0) {
	gl_ErrorHandler(ERR_NEGTAG, WARNING, "newtag");
	return;
    }
    if(istag(newtag)) {
	gl_ErrorHandler(ERR_TAGEXISTS, WARNING, "newtag");
	return;
    }
    if(oldtag == STARTTAG)
	posn = gl_openobjhdr->head - 4;
    else if(!(posn = (short *)gl_findhash(gl_openobj, oldtag))) {
	gl_ErrorHandler(ERR_NOSUCHTAG, WARNING, "newtag");
	goto errexit;
    }
    posn += 4;
    while(offset) {
	if((i = *(long *)posn) == (long)i_tag)
	    posn += 4;
	else if(i == (long)i_slopnop2)
	    posn += 2;
	else if(i == (long)i_slopnop3)
	    posn += 3;
	else if(i == (long)i_jump)
	    posn = (short *)(*(long *)(posn + 2));
	else if(i == (long)i_retsym) {
	    gl_ErrorHandler(ERR_OFFTOOBIG, WARNING, "newtag");
	    return;
	} else /* default */ {
	    --offset;
	    posn += gl_getcmdlength(*(long *)posn, *(short *)(posn + 2));
	}
    }
    if(!(newptr = gl_addchunk()))
	goto outmemerror;
    saveposn = posn;
    savenew = newptr;
    gl_currentpos = newptr;
    gl_currentend = newptr + (gl_objchunksize>>1);
    maketag(newtag);
    newptr += 4;
    i = gl_getcmdlength(*(long *)posn, *(short *)(posn + 2));
    if(i < 4) {	/* have to move at least 4 shorts to make room	*/
	while(i--)	/* note it cannot be a tag cause length < 4	*/
	    *newptr++ = *posn++;
	i = gl_getcmdlength(*(long *)posn, *(short *)(posn + 2));
    }
    /* if next command is a tag then need to relocate it	*/
    if(*(long *)posn == (long)i_tag)
	gl_replacehash(newptr,gl_openobj,*(long *)(posn+2));
    else if(*(long *)posn == (long)i_retsym) {
	gl_openobjhdr -> tailptr = newptr;
	gl_openobjhdr -> tailend = gl_currentend;
    }

    while(i--)				/* move command to new location	*/
	*newptr++ = *posn++;
    *(long *)saveposn = (long)i_jump;	/* replace command with a jump	*/
    *(long *)(saveposn + 2) = (long)savenew;	/* to the new location	*/
    *(long *)newptr = (long)i_jump;	/* and then jump back to the	*/
    *(long *)(newptr + 2) = (long)posn;	/* location after the moved cmd	*/
    objinsert(newtag);
    return;
outmemerror:
    gl_outmem("newtag");
errexit:
    editobj (gl_openobj);
}
