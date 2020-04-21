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
#include "device.h"
#include "mouse.h"
#include "shmem.h"

#define TIMES1	(1<<4)

static int oldleftmouse, oldmiddlemouse, oldrightmouse;
static int lastX = -10000, lastY = -10000;
static int warpx, warpy;
static int lastwarpx, lastwarpy;
static int warpmin, warpmult = TIMES1;

#define ABS(x)	((x)>0 ? (x) : (-(x)))

mousetick()
{
    struct mouse m;
    register int dx, dy;

    mouse_check(&m);

    if(m.x != lastX || m.y != lastY) {
	dx = m.x-lastX;
	dy = m.y-lastY;
	if( warpmult <= TIMES1 || (ABS(dx)+ABS(dy)) < warpmin) {
	    warpx += dx<<4;
	    warpy += dy<<4;
	} else {
	    warpx += warpmult*dx;
	    warpx &= ((MOUSEMAXRAW<<4)-1);
	    warpy += warpmult*dy;
	    warpy &= ((MOUSEMAXRAW<<4)-1);
	}
	lastX = m.x;
	lastY = m.y;
	if(warpx != lastwarpx) {
	    ChangeValuator(MOUSEX,warpx>>4);
	    lastwarpx = warpx;
	}
	if(warpy != lastwarpy) {
	    ChangeValuator(MOUSEY,warpy>>4);
	    lastwarpy = warpy;
	}
    }
    if(m.left != oldleftmouse) {
	ChangeButton(MOUSE3,m.left);
	oldleftmouse = m.left;
    }
    if(m.middle != oldmiddlemouse) {
	ChangeButton(MOUSE2,m.middle);
	oldmiddlemouse = m.middle;
    }
    if(m.right != oldrightmouse) {
	ChangeButton(MOUSE1,m.right);
	oldrightmouse = m.right;
    }
}

#if 0
static warpmouse(oldval,val,warp)
register int oldval, val, warp;
{
    register int dval;

    if(warpmult == 1) 
	return val;
    dval = val-oldval;
    if(dval>warpmin)
	warp = warp+(warpmult*dval);
    else if(dval<(-warpmin))
	warp = warp+(warpmult*dval);
    else 
	warp = warp+dval;
    return (warp & (MOUSEMAXRAW-1));
}
#endif

gl_mousewarp(min,mult)
int min, mult;
{
    warpmin = min;
    warpmult = mult;
}
