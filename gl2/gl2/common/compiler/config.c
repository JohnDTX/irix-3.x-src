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
#include "imattrib.h"
#include "uc4.h"
#include "glerror.h"
#include "shmem.h"

ROOT_1S (font)
ROOT_1S (lsrepeat)
ROOT_1S (setlinestyle)
ROOT_1S (linewidth)
ROOT_1S (lsbackup)
ROOT_1S (resetls)
ROOT_1S (setpattern)
ROOT_1S (color)
ROOT_1S (buffcopy)
ROOT_1S (backface)
ROOT_1S (writemask)

#include "interp.h"

INTERP_NAME (font);
INTERP_NAME (lsrepeat);
INTERP_NAME (setlinestyle);
INTERP_NAME (linewidth);
INTERP_NAME (lsbackup);
INTERP_NAME (resetls);
INTERP_NAME (setpattern);
INTERP_NAME (color);
INTERP_NAME (buffcopy);
INTERP_NAME (backface);
INTERP_NAME (writemask);

static bogus ()
{
    DECLARE_INTERP_REGS;

    INTERP_ROOT_1S(font);
    INTERP_ROOT_1S(lsrepeat);
    INTERP_ROOT_1S(setlinestyle);
    INTERP_ROOT_1S(linewidth);
    INTERP_ROOT_1S(lsbackup);
    INTERP_ROOT_1S(resetls);
    INTERP_ROOT_1S(setpattern);
    INTERP_ROOT_1S(color);
    INTERP_ROOT_1S(buffcopy);
    INTERP_ROOT_1S(backface);
    INTERP_ROOT_1S(writemask);
}
