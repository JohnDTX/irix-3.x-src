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
#include "shmem.h"
#include "imattrib.h"
#include "imdraw.h"
#include "uc4.h"

ROOT_0 (clear)
ROOT_0 (zclear)
ROOT_1S(curveit)

#include "interp.h"

INTERP_NAME(clear);
INTERP_NAME(zclear);
INTERP_NAME(curveit);

static bogus ()
{
    DECLARE_INTERP_REGS;

    INTERP_ROOT_0(clear);
    INTERP_ROOT_0(zclear);
    INTERP_ROOT_1S (curveit);
}
