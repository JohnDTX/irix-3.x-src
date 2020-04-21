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

ROOT_4F (rect)
ROOT_4I (recti)
ROOT_4S (rects)
ROOT_4F (rectf)
ROOT_4I (rectfi)
ROOT_4S (rectfs)

#include "interp.h"

INTERP_NAME(rect);
INTERP_NAME(recti);
INTERP_NAME(rects);
INTERP_NAME(rectf);
INTERP_NAME(rectfi);
INTERP_NAME(rectfs);

static bogus ()
{
    DECLARE_INTERP_REGS;

/* NOTE: this is a hack - it assumes both Icoords and Coords are 4 bytes */
#define INTERP_RECT_ROOT(name)	\
INTERP_LABEL(name,10);		\
{register long llx,lly,urx,ury;	\
    llx = *PC++;		\
    lly = *PC++;		\
    urx = *PC++;		\
    ury = *PC++;		\
    im_/**/name(llx,lly,urx,ury);	\
    thread;				\
}

#define INTERP_RECT_ROOT_S(name)	\
INTERP_LABEL(name,6);		\
{register short llx,lly,urx,ury;	\
    llx = *(short *)PC++;		\
    lly = *(short *)PC++;		\
    urx = *(short *)PC++;		\
    ury = *(short *)PC++;		\
    im_/**/name(llx,lly,urx,ury);	\
    thread;				\
}

#ifndef CLOVER
#undef im_outfloat
#undef im_last_outfloat
#define im_outfloat(x)	*(long *)GE = x
#define im_last_outfloat(x)	*(long *)LASTGE = x
#endif

    INTERP_RECT_ROOT (rect);
    INTERP_RECT_ROOT (rectf);
    INTERP_RECT_ROOT (recti);
    INTERP_RECT_ROOT (rectfi);
    INTERP_RECT_ROOT_S (rects);
    INTERP_RECT_ROOT_S (rectfs);
}
