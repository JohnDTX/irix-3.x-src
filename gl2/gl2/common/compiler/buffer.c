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
#include "get.h"
#include "shmem.h"

#define im_swapbuffers() \
	if (gl_personal_buffer_mode == DMDOUBLE && \
		WS->curatrdata.myconfig & (UC_DOUBLE<<16)) gl_waitforswap();

ROOT_0 (swapbuffers)
ROOT_1S (frontbuffer)
ROOT_1S (backbuffer)

#include "interp.h"

INTERP_NAME (swapbuffers);
INTERP_NAME (frontbuffer);
INTERP_NAME (backbuffer);

static bogus ()
{
    DECLARE_INTERP_REGS;

    INTERP_ROOT_0(swapbuffers);
    INTERP_ROOT_1S(frontbuffer);
    INTERP_ROOT_1S(backbuffer);
}
