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

/******************* T R A C E ************************************
*
*   Provides the trace logging in pxlog
*
******************************************************************/

#include <stdio.h>
#include <varargs.h>

extern	_doprnt();

static FILE *tfile = 0;

tr_close()
{
	if (tfile) {
		(void)fflush (tfile);
		(void)fclose (tfile);
		(void)printf("closed pxlog ");
		tfile = 0;
	}
}

tr_init()
{
	if (tfile == 0) {
		if ((tfile = fopen("pxlog", "w"))==0)
			(void)printf("Cannot open trace file\n");
		setbuf(tfile, (char *)0);
	}
}

/* VARARGS1 */
trace_msg(fmt, args)
char *fmt;
int args;
{
	if (tfile)
		_doprnt (fmt, &args, tfile);
}

tr_flush()
{
	if (tfile)
		(void)fflush (tfile);
}
