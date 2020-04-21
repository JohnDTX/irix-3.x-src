/*
 * BSD compatibility routine
 *
 * Use 'fcntl' to implement 'dup2'
 */

#include <sys/fcntl.h>
#include <errno.h>

/*
 * dup2 returns a new descriptor with value 'newd' that is
 *	a dup of 'oldd'.
 */
int					/* return 0 or <0 */
dup2(oldd, newd)
int oldd, newd;
{
	register int dupd;

	if (oldd == newd)		/* quit if nothing to do */
		return(0);

	/*
	 * Close the desired descriptor first to ensure that it
	 * is available (it may already be available, so errors
	 * are ignored).
	 */
	close(newd);

	/*
	 * fcntl F_DUPFD clones 'oldd' and uses the first available file
	 * descriptor greater than or equal to the third argument.
	 */
	if ((dupd = fcntl(oldd, F_DUPFD, newd)) < 0)
		return(dupd);

	/*
	 * Make sure that we got the right one
	 */
	if (dupd != newd) {
		close(dupd);
		errno = EINVAL;
		return(-1);
	}

	return(0);
}
