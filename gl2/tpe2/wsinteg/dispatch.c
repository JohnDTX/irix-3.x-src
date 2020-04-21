/**************************************************************************
 *									  *
 * 		 Copyright (C) 1985, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/

/*
**			Iris terminal dispatch table
**
**			 Paul Haeberli - Sept 1983
*/

#include "term.h"
#include "hostio.h"
#include "dispatch.h"
int     dispatchLen = sizeof dispatch / sizeof *dispatch;

#ifdef GL1
extern char *gl_cmdnames[];
#endif

/*
**	getcmdname - return a pointer to a string giving the name of
**		     a GL routine given its command number
**
*/
char *
getcmdname(cmd)
unsigned cmd;
{
    char *cp;
    static char buf[10];

    if (cmd >= maxcom) {
	sprintf(buf,"%d",cmd);
	return buf;
    }
    cp = cmdnametab[cmd];
#ifdef GL1
    if (cp)
	return cp;
    else {
	cp = gl_cmdnames[ dispatch[cmd].token ];
	if (cp)
	    return cp;
	else {
	    sprintf(buf,"%d",cmd);
	    return buf;
	}
    }
#else
    return cp;
#endif
}    
