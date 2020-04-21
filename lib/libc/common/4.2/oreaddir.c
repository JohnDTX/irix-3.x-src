/*
 * $Source: /d2/3.7/src/lib/libc/common/4.2/RCS/oreaddir.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 16:13:38 $
 */

/* Copyright (c) 1982 Regents of the University of California */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/dir.h>
#include <sys/dirent.h>

/*
 * Use 'getdents' system call to read a directory and present it
 * to the caller as if it were a 4.2 style directory.
 */

/*
 * Return next entry in a directory.
 */
struct direct *
readdir(dirp)
	register DIR *dirp;
{
	register struct dirent *dp;	/* struct returned by getdents */
	register struct direct *retp;   /* BSD4.2 directory struct */
	int saveloc = 0;

	if (dirp->dd_size != 0) {
		dp = (struct dirent *)&dirp->dd_buf[dirp->dd_loc];
		saveloc = dirp->dd_loc;   /* save for possible EOF */
		dirp->dd_loc += dp->d_reclen;
	}
	if (dirp->dd_loc >= dirp->dd_size)
		dirp->dd_loc = dirp->dd_size = 0;

	if (dirp->dd_size == 0 	/* refill buffer */
	    && (dirp->dd_size = getdents(dirp->dd_fd, dirp->dd_buf,
					sizeof dirp->dd_buf)) <= 0) {
		if (dirp->dd_size == 0)	/* This means EOF */
			dirp->dd_loc = saveloc;  /* EOF so save for telldir */
		return NULL;	/* error or EOF */
	}

	dp = (struct dirent *)&dirp->dd_buf[dirp->dd_loc];
	/*
	|| copy the data from getdents buffer to the 
	|| BSD struct data
	*/
	retp = &dirp->dd_direct;
	retp->d_ino = dp->d_ino;

	/*
	|| Names returned by getdents are guaranteed null terminated
	*/
	retp->d_namlen = strlen(dp->d_name);
	strcpy(retp->d_name, dp->d_name);
	retp->d_reclen = DIRSIZ(retp);
	return (retp);
}
