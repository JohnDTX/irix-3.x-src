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

long getzbuffer()
{
    return (gl_wstatep -> myzbuffer);
}

long getdcm()
{
    return ((gl_wstatep->curatrdata.myconfig & (UC_DEPTHCUE<<16)) != 0);
}

long	getbackface()
{
    return(gl_wstatep->mybackface);
}

long	getlstyle()
{
    return(gl_wstatep->curatrdata.mylstyle);
}

long	getlwidth()
{
    return(gl_wstatep->curatrdata.mylwidth+1);
}

long	getlsrepeat()
{
    return(gl_wstatep->curatrdata.mylsrepeat+1);
}

long	getlsbackup()
{
    return((gl_wstatep->curatrdata.myconfig & BACKLINE) != 0);
}

long	getresetls()
{
    return((gl_wstatep->curatrdata.myconfig & LDLINESTIP) != 0);
}

long getpattern()
{
    return(gl_wstatep->curatrdata.mytexture);
}

long getshade()
{
    return gl_currentshade;
}

long getcolor()
{
    if (gl_wstatep -> curatrdata.myconfig & ((UC_DOUBLE | UC_SWIZZLE)<<16))
	return(gl_wstatep->curatrdata.mycolor);
    else
	gl_ErrorHandler(ERR_INRGB, WARNING, "getcolor");
}

long getwritemask()
{
    if (gl_wstatep -> curatrdata.myconfig & ((UC_DOUBLE | UC_SWIZZLE)<<16))
	return(gl_wstatep->curatrdata.mywenable);
    else
	gl_ErrorHandler(ERR_INRGB, WARNING, "getwritemask");
}

void	gRGBcolor(red, green, blue)
short *red, *green, *blue;
{
    if (!(gl_wstatep->curatrdata.myconfig & ((UC_DOUBLE | UC_SWIZZLE)<<16))) {
	*red = gl_wstatep -> curatrdata.myr;
	*green = gl_wstatep -> curatrdata.myg;
	*blue = gl_wstatep -> curatrdata.myb;
    } else
	gl_ErrorHandler(ERR_NOTINRGB, WARNING, "gRGBcolor");
}

void	gRGBmask(red, green, blue)
short *red, *green, *blue;
{
    if (!(gl_wstatep->curatrdata.myconfig & ((UC_DOUBLE | UC_SWIZZLE)<<16))) {
	*red = gl_wstatep -> curatrdata.myrm;
	*green = gl_wstatep -> curatrdata.mygm;
	*blue = gl_wstatep -> curatrdata.mybm;
    } else
	gl_ErrorHandler(ERR_NOTINRGB, WARNING, "gRGBmask");
}

void getscrmask(x1, x2, y1, y2)
short	*x1, *x2, *y1, *y2;
{
    register windowstate *ws = gl_wstatep;

    *x1 = ws->curvpdata.llx;
    *x2 = ws->curvpdata.urx;
    *y1 = ws->curvpdata.lly;
    *y2 = ws->curvpdata.ury;
}

void getviewport(x1, x2, y1, y2)
short	*x1, *x2, *y1, *y2;
{
    register windowstate *ws = gl_wstatep;

    *x1 = (ws->curvpdata.vcx - ws->curvpdata.vsx)>>8;
    /* make up for extra .5 */
    *x2 = (ws->curvpdata.vcx + ws->curvpdata.vsx - 0x00000100)>>8;
    *y1 = (ws->curvpdata.vcy - ws->curvpdata.vsy)>>8;
    *y2 = (ws->curvpdata.vcy + ws->curvpdata.vsy - 0x00000100)>>8;
}

void getdepth(z1, z2)
short	*z1, *z2;
{
    register windowstate *ws = gl_wstatep;

    *z1 = ws->zmin;
    *z2 = ws->zmax;
}

void getwindow(x1, x2, y1, y2)
short	*x1, *x2, *y1, *y2;
{
    register windowstate *ws = gl_wstatep;

    *x1 = ws->xmin;
    *x2 = ws->xmax;
    *y1 = ws->ymin;
    *y2 = ws->ymax;
}
