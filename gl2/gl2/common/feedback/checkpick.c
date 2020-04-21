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

/* 
 *  Check if in picking mode.  Feedback commands are illegal in
 *  picking mode. 
 */
gl_checkpick(routine)
char	*routine;
{
    if(!gl_pickselect)
	    return(0);
    else {
	gl_ErrorHandler(ERR_FEEDPICK, WARNING, routine);
	return(1);
    }
}

static short picksuspended;
static short feedsuspended;

gl_checkpickfeed()
{
    picksuspended = 0;
    feedsuspended = 0;
    if (gl_pickselect) {
	picksuspended = 1;
	gl_suspendselect();
    } else if (gl_fbmode) {		    /* in feedback mode */ 
	feedsuspended = 1;
	gl_suspendfeedback();
    }
}

gl_restorepickfeed()
{
    if (picksuspended)
	gl_resumeselect();
    else if (feedsuspended)
	gl_resumefeedback();
}
