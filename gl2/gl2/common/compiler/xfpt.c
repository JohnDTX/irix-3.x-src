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

ROOT (xfpt)
ROOT_4S(xfpt4s)
ROOT_4I(xfpt4i)
ROOT_4F(xfpt4)

#include "interp.h"

INTERP_NAMES(xfpt);
INTERP_NAME(xfpt4s);
INTERP_NAME(xfpt4i);
INTERP_NAME(xfpt4);

static bogus ()
{
    DECLARE_INTERP_REGS;

    INTERP_ROOT(xfpt);
    INTERP_ROOT_4S(xfpt4s);
    INTERP_ROOT_4I(xfpt4i);
    INTERP_ROOT_4F(xfpt4);
}
