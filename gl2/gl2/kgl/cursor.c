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
#include "shmem.h"
#include "immed.h"
#include "gf2.h"
#include "window.h"

/*
 *	curson -
 *		turn the cursor on.
 *
 */
void curson(waitforeof)
	int waitforeof;
{
    	im_setup;

    	if(gl_cursordrawn == 0) {
		im_passcmd(3, FBCdrawcursor);
		im_outshort(gl_cursorx-gl_cursorxorgin);
		im_last_outshort(gl_cursory-gl_cursoryorgin);
		if (waitforeof)
			gl_WaitForEOF(1);
		gl_cursordrawn = 1;
    	}
    	gl_autocursor = 1;
}

/*
 *	cursoff -
 *		turn the cursor off.
 *
 */
void cursoff()
{
    	im_setup;
	
    	gl_autocursor = 0;
    	if(gl_cursordrawn) {
		gl_cursordrawn = 0;
		im_last_passcmd(1, FBCundrawcursor);
    	}
}

/*
 * 	gl_getcurstate -
 *		used by the GR_GETCURSOR grioctl to get the cursor state.
 *
 */
gl_getcurstate(addr, color, wtm, b)
long	*addr;
short	*color, *wtm, *b;
{
    	*addr = gl_cursoraddr;
    	*color = gl_cursorcolor;
    	*wtm = gl_cursorwenable;
    	*b = gl_autocursor;
}

/*
 * 	gl_RGBgetcurstate -
 *		used by the GR_RGBGETCURSOR grioctl to get the cursor state.
 *
 */
gl_RGBgetcurstate(addr, cr, cg, cb, wtmr, wtmg, wtmb, b)
long	*addr;
short	*cr, *cg, *cb, *wtmr, *wtmg, *wtmb, *b;
{
    	*addr = gl_cursoraddr;
    	*cr = gl_rcursorcolor;
    	*cg = gl_gcursorcolor;
    	*cb = gl_bcursorcolor;
    	*wtmr = gl_rcursorwenable;
    	*wtmg = gl_gcursorwenable;
    	*wtmb = gl_bcursorwenable;
    	*b = gl_autocursor;
}
