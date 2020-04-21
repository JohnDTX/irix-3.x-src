#ifdef	NOTDEF
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

#include "sys/types.h"
#include "timeb.h"

static struct timeb tb_start, tb_end;

gl_start_watch()
{
    ftime(&tb_start);
}

gl_stop_watch()
{
    ftime(&tb_end);
    return(gl_millisecs());
}

gl_millisecs ()
{
    register int msecs;

    msecs = 1000 * (tb_end.time - tb_start.time) +
		    tb_end.millitm - tb_start.millitm;
    if(msecs == 0) 
        msecs = 1;
    return (msecs);
}
#endif	/* NOTDEF */
