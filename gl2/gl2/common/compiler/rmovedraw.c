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
#include "imdraw.h"

ROOT (rmv)
ROOT (rdr)
ROOT (rpmv)
ROOT (rpdr)

#include "interp.h"

INTERP_NAMES(rmv);
INTERP_NAMES(rdr);
INTERP_NAMES(rpmv);
INTERP_NAMES(rpdr);

static bogus ()
{
    DECLARE_INTERP_REGS;

    INTERP_ROOT(rmv);
    INTERP_ROOT(rdr);
    INTERP_ROOT(rpmv);
    INTERP_ROOT(rpdr);
}
