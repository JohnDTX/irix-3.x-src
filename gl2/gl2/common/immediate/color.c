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

/*
 *	Gary Tarolli			10/8/84
 *
 *	Routines to support UC/DC manipulation in the IRIS GL2 terminal.
 *
 *	Visible functions:
 *		long		getdisplaymode ()
 *		long		getplanes ()
 *		long		getbuffer ()
 *
 *	Visible procedures:		
 *		void	gl_resolvecolor ()
 *		void	singlebuffer ()
 *		void	doublebuffer ()
 *		void	RGBmode ()
 *		void	onemap ()
 *		void	multimap ()
 *
 *	Updates:
 *		10/8/84 GMT	Created
 */

#include "globals.h"
#include "uc4.h"
#include "get.h"

/*
 *	Internal static variables
 */

static short gl_nextdisplaymode;/* Will become the display mode after	*/
				/*   a call to gl_resolvecolor.  Changed*/
				/*   by singlebuffer(), doublebuffer(),	*/
				/*   and RGBmode().			*/

static short gl_nextcolormapmode;/* Will become the colormap mode after	*/
				/*   a call to gl_resolvecolor.  Changed*/
				/*   by onemap() and multimap().	*/

long getdisplaymode () 
{
    if (gl_wstatep->curatrdata.myconfig & (UC_DOUBLE<<16)) 
	return(DMDOUBLE);
    else if (gl_wstatep->curatrdata.myconfig & (UC_SWIZZLE<<16))
	return(DMSINGLE);
    else 
	return (DMRGB);
}


long getplanes () 
{
    register long i,mask,planes;

    planes = 0;
    mask = gl_wstatep->bitplanemask;
    for (i = 0; i < 32; i++) {
	if (mask & 0x1) planes++;
	else break;
	mask >>= 1;
    }
    return (planes);
}


long getbuffer () 
{
    register long cfr = gl_wstatep -> curatrdata.myconfig;

    if (cfr & (UC_DOUBLE<<16)) {
	switch (cfr &(UPDATEA | UPDATEB)) {
	    case UPDATEA:
		return (cfr&DISPLAYA?FRNTBUFFER:BCKBUFFER);
	    case UPDATEB:
		return (cfr&DISPLAYB?FRNTBUFFER:BCKBUFFER);
	    case UPDATEA | UPDATEB:
		return (BOTHBUFFERS);
	}
    } else 
	return (NOBUFFER);
}


void gl_resolvecolor () 
{
    /*
     *	Changes the display mode, and colormap mode
     *	  to correspond to the requested values. 
     */

    gl_flushtext();
    if (gl_nextdisplaymode == DMRGB) gl_rgbmode();
    else {
	gl_nextdisplaymode == DMSINGLE ? gl_singlebuffer():gl_doublebuffer();

	/* this must occur after single/double buffer mode change !	*/
	/* to keep the kernel's bitplanemasks happy			*/
	gl_nextcolormapmode == CMAPMULTI ? gl_multimap() : gl_onemap();
    }
}

void singlebuffer ()
{
    gl_nextdisplaymode = DMSINGLE;
}


void doublebuffer () 
{
    gl_nextdisplaymode = DMDOUBLE;
}


void RGBmode () 
{
    gl_nextdisplaymode = DMRGB;
}


void onemap () 
{
    gl_nextcolormapmode = CMAPONE;
}


void multimap () 
{
    gl_nextcolormapmode = CMAPMULTI;
}
