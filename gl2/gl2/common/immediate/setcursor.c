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
#include "glerror.h"
#include "uc4.h"

static short gl_cursor_index;

void setcursor(index, colorindex, colormask)
short index;
register Colorindex colorindex, colormask;
{
    short xoffset, yoffset;
    short offset;

    colorindex &= gl_wstatep -> bitplanemask;
    colormask &= gl_wstatep -> bitplanemask;

/* HACK: use absolute address for cursor offset: */
    offset = gl_findcursor(index,&xoffset,&yoffset);
    gl_wstatep->cursorbase = offset;
    gl_setcursor(offset+gl_wstatep->fontrambase,colorindex,colormask);
    gl_setcursoroffset(xoffset,yoffset);
    gl_cursor_index = offset < 0 ? 0 : index;
}

void getcursor(index, cindex, wtm, b)
short		*index;
Colorindex	*cindex, *wtm;
Boolean		*b;
{
    long addr;

    if (gl_wstatep -> curatrdata.myconfig & ((UC_DOUBLE | UC_SWIZZLE)<<16)) {
	gl_getcursor(&addr, cindex, wtm, b);
	*index = gl_cursor_index;
    } else 
       gl_ErrorHandler(ERR_INRGB, WARNING, "getcursor");
}

void RGBcursor(index, r, g, b, rm, gm, bm)
short	index;
RGBvalue r,g,b,rm,gm,bm;
{
    short xoffset, yoffset;
    short offset;

/* HACK: use absolute address for cursor offset: */
    offset = gl_findcursor(index,&xoffset,&yoffset);
    gl_wstatep->cursorbase = offset;
    gl_RGBsetcursor(offset+gl_wstatep->fontrambase,r,g,b,rm,gm,bm);
    gl_setcursoroffset(xoffset,yoffset);
    gl_cursor_index = offset < 0 ? 0 : index;
}

void gRGBcursor(index,r,g,b,rm,gm,bm,autocurs)
short		*index;
short 		*r,*g,*b,*rm,*gm,*bm;
Boolean		*autocurs;
{
    long addr;

    if (!(gl_wstatep->curatrdata.myconfig & ((UC_DOUBLE | UC_SWIZZLE)<<16))) {
	gl_RGBgetcursor(&addr,r,g,b,rm,gm,bm,autocurs);
	*index = gl_cursor_index;
    } else 
	gl_ErrorHandler(ERR_NOTINRGB, WARNING, "gRGBcursor");
}
