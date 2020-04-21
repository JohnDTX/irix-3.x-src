/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */
/*  opendir.c 1.1 */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
	opendir -- C library extension routine

*/

#include	<sys/types.h>
#include	<dirent.h>
#include	<sys/stat.h>
#include	<sys/errno.h>

extern char	*malloc();
extern int	open(), close(), fstat();
extern int	errno;

#define NULL	0

DIR *
opendir( filename )
char		*filename;	/* name of directory */
{
	register DIR	*dirp;		/* -> malloc'ed storage */
	register int	fd;		/* file descriptor for read */
	struct stat	sbuf;		/* result of fstat() */

	if ( (fd = open( filename, 0 )) < 0 )
		return NULL;
	if ( (fstat( fd, &sbuf ) < 0)
	  || ((sbuf.st_mode & S_IFMT) != S_IFDIR)
	  || ((dirp = (DIR *)malloc( sizeof(DIR) )) == NULL)
	  || ((dirp->dd_buf = (char *)malloc(DIRBUF)) == NULL)
	   )	{
		if ((sbuf.st_mode & S_IFMT) != S_IFDIR)
			errno = ENOTDIR;
		(void)close( fd );
		return NULL;		/* bad luck today */
		}

	dirp->dd_fd = fd;
	dirp->dd_loc = dirp->dd_size = 0;	/* refill needed */

	return dirp;
}
