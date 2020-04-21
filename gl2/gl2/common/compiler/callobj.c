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
#include "TheMacro.h"

void callobj(obj)
register long	obj;
{
    objhdr	*objhead, *gl_findobjhdr(), *gl_getnewobjhdr();
    extern int i_callobj();

    if (!(objhead = gl_findobjhdr(obj)))
	if (!(objhead = gl_getnewobjhdr(obj)))
	    return;
    if (gl_openobjhdr == 0) {
	if (objhead -> head) gl_interpret (objhead -> head);
	return;
    }
    if (gl_checkspace(4) == 0) return;
    BEGINCOMPILE(4);
    ADDADDR(i_callobj);
    ADDADDR(objhead);
    ENDCOMPILE;
}

#include "interp.h"

INTERP_NAME(callobj);

static bogus ()
{
    DECLARE_INTERP_REGS;
    register objhdr *objhead;

/* NOTE: retsym MUST be 4 shorts (8 bytes) long in order for the object
	editting stuff to work - see Tom for details	*/
#define im_retsym(x) movl(sp@+, a4)

INTERP_LABEL (callobj,4);
	objhead = (objhdr *)*PC++;	/* get pointer to objheader	*/
	movl(a4, sp@-);		/* push current PC onto stack	*/
	if (! (PC = (long *)(objhead -> head))) {
	    im_retsym(x);
	}
	thread;
}
