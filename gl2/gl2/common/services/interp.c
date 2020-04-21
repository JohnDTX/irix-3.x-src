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
#include "imsetup.h"
#include "interp.h"

asm("gl_dummy:	.long	gl_interpreturn");

gl_interpret (addr)
long *addr;
{
    DECLARE_INTERP_REGS;
    register int *a2;
    register int d2,d3,d4,d5,d6,d7;
    char save_some_stack_space[256];

    GE = &GEPORT;
    PC = addr;
    WS = gl_wstatep;
    movl(#gl_dummy,sp@-);
    thread;

asm("gl_foo:");
    /* must use regs to force cc68 to save them	*/
    a2 = 0;
    d2 = d3 = d4 = d5 = d6 = d7 = 0;

asm("gl_interpreturn:");
    return;
}
