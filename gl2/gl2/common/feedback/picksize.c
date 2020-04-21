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

void picksize(deltax, deltay)
register short	deltax, deltay;
{
    if (deltax == 0 || deltay == 0) {
	gl_ErrorHandler(ERR_ZEROPICK, WARNING, 0);
	return;
    }
    gl_picksizex = deltax;
    gl_picksizey = deltay;
}

