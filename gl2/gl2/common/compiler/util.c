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
#include "interp.h"

INTERP_NAME(retsym);
INTERP_NAME(slopnop2);
INTERP_NAME(slopnop3);
INTERP_NAME(jump);
INTERP_NAME(tag);

static bogus ()
{
    DECLARE_INTERP_REGS;

/* NOTE: retsym MUST be 4 shorts (8 bytes) long in order for the object
	editting stuff to work - see Tom for details	*/
#define im_retsym(x) movl(sp@+, a4)
	INTERP_ROOT_1I(retsym);

/* The difference between the nops and the slopnops is that nops cannot
| be removed from a display list during compactification, while slopnops
| can.  nops are used to pad out variable size commands like arc, while
| slopnops fill holes caused by display list editing.
| NOTE: GT removed all nops since arcs and circles don't need them anymore
*/

#define im_slopnop2()
	INTERP_ROOT_0(slopnop2);

#define im_slopnop3(x) x
	INTERP_ROOT_1S(slopnop3);

#define im_jump(addr) PC = (long *) *PC
	INTERP_ROOT_1I(jump);

#define im_tag(t) t
	INTERP_ROOT_1I(tag);
}
