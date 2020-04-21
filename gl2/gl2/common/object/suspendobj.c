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

/* File: gl_suspendobj.c
 *
 * The routines in this file are used to suspend an object that may
 * be open for editing, and to re-open it at the same place.
 */

#include	"globals.h"

void gl_suspendobj()
{
    if(gl_openobjhdr == 0) {
	gl_issaved = 0;
	return;
    } else {
	gl_issaved = 1;
	gl_saveopenobjhdr = gl_openobjhdr;
	gl_savecurrentpos = gl_currentpos;
	gl_savecurrentend = gl_currentend;
	gl_saveopenobj = gl_openobj;
	gl_savereplacemode = gl_replacemode;
	gl_openobjhdr = 0;
    }
}

void gl_unsuspendobj()
{
    if(gl_issaved) {
	gl_issaved = 0;
	gl_openobjhdr = gl_saveopenobjhdr;
	gl_currentpos = gl_savecurrentpos;
	gl_currentend = gl_savecurrentend;
	gl_openobj = gl_saveopenobj;
	gl_replacemode = gl_savereplacemode;
    }
}
