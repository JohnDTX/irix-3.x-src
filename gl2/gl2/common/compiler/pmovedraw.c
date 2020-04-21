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

ROOT (pmv)
ROOT (pdr)
ROOT_0 (pclos)
ROOT_0 (spclos)

#include "interp.h"

INTERP_NAMES(pmv);
INTERP_NAMES(pdr);
INTERP_NAME(pclos);
INTERP_NAME(spclos);

static bogus ()
{
    DECLARE_INTERP_REGS;

    INTERP_ROOT(pmv);
    INTERP_ROOT(pdr);
    INTERP_ROOT_0(pclos);
    INTERP_ROOT_0(spclos);
}
