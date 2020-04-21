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

ROOT (move)
ROOT (draw)

#include "interp.h"

INTERP_NAMES(move);
INTERP_NAMES(draw);
INTERP_NAME(movezero);

static bogus ()
{
    DECLARE_INTERP_REGS;

    INTERP_ROOT(move);
    INTERP_ROOT(draw);
    INTERP_ROOT_0(movezero);
}
