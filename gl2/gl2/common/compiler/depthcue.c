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
#include "imdraw.h"
#include "uc4.h"
#include "shmem.h"

#define im_depthcue(b) {	\
    im_lockpipe;		\
    if (b) {	\
	if (!(WS->curatrdata.myconfig & (UC_DEPTHCUE<<16))) {\
	    WS->curatrdata.myconfig |= (UC_DEPTHCUE<<16);	\
	    if(!WS->myzbuffer)	\
	        gl_3dconfig();	\
	}\
    }	\
    else {	\
	if ((WS->curatrdata.myconfig & (UC_DEPTHCUE<<16))) {\
	    WS->curatrdata.myconfig &= ~(UC_DEPTHCUE<<16);	\
	    if(!WS->myzbuffer)	\
	        gl_normalconfig();	\
	}\
    }	\
    im_outconfig();	\
}

ROOT_1S(depthcue)

#define SHADERANGE_SIZE 8
extern int i_shaderange();

void shaderange(imin,imax,zmin,zmax)
    register short imin,imax,zmin,zmax;
{
    register long a;

    a = ((imin-imax)<<14)/((zmax==zmin) ? 1 : (zmax-zmin));
    beginpicmandef(SHADERANGE_SIZE);
    BEGINCOMPILE(SHADERANGE_SIZE);
    ADDADDR(i_shaderange);
    ADDSHORT(a);
    ADDSHORT(imax - ((a*zmin)>>14));
    ADDSHORT(imin);
    ADDSHORT(imax);
    ADDSHORT(zmin);
    ADDSHORT(zmax);
    ENDCOMPILE;
    endpicmandef;
}

#include "interp.h"

INTERP_NAME(depthcue);
INTERP_NAME(shaderange);

static bogus ()
{
    DECLARE_INTERP_REGS;

    INTERP_ROOT_1S(depthcue);

INTERP_LABEL(shaderange,8);	/* SHADERANGE_SIZE == 8 */
    WS->a = *(short *)PC++;
    WS->b = *(short *)PC++;
    WS->imin = *(short *)PC++;
    WS->imax = *(short *)PC++;
    WS->z1 = *(short *)PC++;
    WS->z2 = *(short *)PC++;
    im_passcmd(7,FBCdepthsetup);
    im_outshort (WS->a);
    im_outshort (WS->b);
    im_outshort (WS->imin);
    im_outshort (WS->imax);
    im_outshort (WS->z1);
    im_outshort (WS->z2);
    im_rmv2s(0, 0);
    thread;
}
