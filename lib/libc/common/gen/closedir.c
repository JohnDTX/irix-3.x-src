/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */
/*  closedir.c 1.1 */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
	closedir -- C library extension routine

*/

#include	<sys/types.h>
#include	<dirent.h>


extern void	free();
extern int	close();

int
closedir( dirp )
register DIR	*dirp;		/* stream from opendir() */
{
	free( dirp->dd_buf );
	free( (char *)dirp );
	return(close( dirp->dd_fd ));
}
