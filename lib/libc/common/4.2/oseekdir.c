/*
 * $Source: /d2/3.7/src/lib/libc/common/4.2/RCS/oseekdir.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 16:13:40 $
 */

/* Copyright (c) 1982 Regents of the University of California */

#include <sys/types.h>
#include <sys/dir.h>

/*
 * seek to an entry in a directory.
 * Only values returned by ``telldir'' should be passed to seekdir.
 */
void
seekdir(dirp, loc)
	register DIR *dirp;
	long loc;
{
	register long curloc;

	if (telldir(dirp) == loc)
		return;
	
	/* 
	 * The requested location is not in the current buffer.
	 * Discard the buffer and reposition the file pointer to the
	 * requested location.
	 */
	dirp->dd_loc = 0;
	lseek(dirp->dd_fd, loc, 0);
	dirp->dd_size = 0;
}
