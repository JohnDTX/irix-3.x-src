/*
 * $Source: /d2/3.7/src/lib/libc/common/4.2/RCS/oopendir.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 16:13:37 $
 */

/* Copyright (c) 1982 Regents of the University of California */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/dir.h>

extern 	int errno;
/*
 * open a directory.
 */
DIR *
opendir(name)
	char *name;
{
	register DIR *dirp;
	register int fd;	/* file descriptor fo read */
	struct stat sbuf;

	if ( (fd = open(name, 0) ) < 0 )
		return NULL;

	if ( (fstat( fd, &sbuf ) < 0)
	  || ((sbuf.st_mode & S_IFMT) != S_IFDIR)
	  || ((dirp = (DIR *)malloc( sizeof(DIR) )) == NULL)
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
