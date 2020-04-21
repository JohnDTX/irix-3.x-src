#ifndef lint
static char sccsid[] = "@(#)telldir.c	4.5 (Berkeley) 7/1/83";
#endif

#include <sys/param.h>
#include <sys/dir.h>
#include <sys/dirent.h>

/*
 * return a pointer into a directory
 */
long
telldir(dirp)
	register DIR *dirp;
{
	struct dirent *dp;
	extern long lseek();


	if (lseek(dirp->dd_fd, 0, 1) == 0)	/* if at beginning of dir */
		return(0);			/* return 0 */
	dp = (struct dirent *)&dirp->dd_buf[dirp->dd_loc];
	return(dp->d_off);
}
