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

extern int i_callfunc();

void callfunc (foo, nargs, args)
    int		(*foo)();
    register long nargs;
    long 	args;
{
    register int num_shorts;
    register long *j;

    num_shorts = 5 + 2*nargs;
    beginpicmandef(num_shorts);
    BEGINCOMPILE(num_shorts);
    ADDADDR(i_callfunc);
    ADDSHORT(nargs);
    ADDADDR(foo);
    if (nargs > 0) {
	j = &args + nargs;
	while (nargs-- > 0) {
	    ADDLONG(*--j);
	}
    }
    ENDCOMPILE;
    endpicmandef;
}

#include "interp.h"

INTERP_NAME(callfunc);

static bogus ()
{
    DECLARE_INTERP_REGS;
    register int (*addr)();
    register long ln;	/* must end up in d7	*/
    register short n;

INTERP_LABEL(callfunc, 1000003); /* CALLFUNCLENGTH == 1000003 */
    ln = n = *(short *)PC++;	/* parameter count	*/
    addr = (int (*)() )*PC++;	/* routine address	*/
    while (--n != -1) {
	movl(a4@+,sp@-);	/* push the parameters	*/
    }
    movl(d7,sp@-);		/* push the count on the stack	*/
    addr();			/* call the function		*/
    ln <<= 2;			/* convert parameter count to	*/
    ln += 4;			/* a byte count and pop stack	*/
    addl(d7,sp);
    thread;
}

