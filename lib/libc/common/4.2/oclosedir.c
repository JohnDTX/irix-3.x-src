/*
 * $Source: /d2/3.7/src/lib/libc/common/4.2/RCS/oclosedir.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 16:13:36 $
 */

/* Copyright (c) 1982 Regents of the University of California */

#include <sys/types.h>
#include <sys/dir.h>

/*
 * close a directory.
 */
void
closedir(dirp)
	register DIR *dirp;
{
	(void)close(dirp->dd_fd);
	free((char *)dirp);
}
