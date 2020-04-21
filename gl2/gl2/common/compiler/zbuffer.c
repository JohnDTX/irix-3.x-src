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
#include "shmem.h"

#define im_zbuffer(m) {						\
    im_lockpipe;						\
    im_zshade(0,m);						\
    if(!(WS->curatrdata.myconfig & (UC_DEPTHCUE<<16))) {	\
	if (m) {						\
	    if (!WS->myzbuffer) { 				\
		gl_3dconfig();					\
	    } 							\
	} else {						\
	    if (WS->myzbuffer) { 				\
		gl_normalconfig();				\
	    } 							\
	} 							\
    } 								\
    if (WS->curatrdata.myconfig & (UC_DOUBLE<<16)) {		\
	if (m) {						\
	    if (!WS-> myzbuffer) {				\
	        im_do_writemask(WS->curatrdata.mywenable);	\
	        im_frontbuffer(1);				\
	        im_backbuffer(0);				\
	    }							\
        } else	{						\
	    if (WS-> myzbuffer) {				\
		im_buffcopy(1);					\
		im_do_writemask(WS->curatrdata.mywenable);	\
		im_frontbuffer(1);				\
		im_backbuffer(1);				\
	    }							\
	}							\
    } 								\
    WS -> myzbuffer = m;					\
    im_freepipe;						\
}

ROOT_1S(zbuffer)

#include "interp.h"

INTERP_NAME(zbuffer);

static bogus ()
{
    DECLARE_INTERP_REGS;

    INTERP_ROOT_1S(zbuffer);
}
