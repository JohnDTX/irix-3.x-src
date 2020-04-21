/* @(#)gnamef.c	1.2 */
#include "uucp.h"
#include <sys/types.h>
#include <dirent.h>



/*
 * get next file name from directory
 *	p	 -> file description of directory file to read
 *	filename -> address of buffer to return filename in
 *		    must be of size NAMESIZE
 * returns:
 *	FALS	-> end of directory read
 *	TRUE	-> returned name
 */
gnamef(p, filename)
register char *filename;
DIR *p;
{
	register int i;
	register char *s;
	register struct dirent *dp;

	while (1) {
		if ((dp = readdir(p)) == NULL)
			return(FALSE);
		if (dp->d_ino != 0)
			break;
	}

	for (i = 0, s = dp->d_name; i < MAXBASENAME; i++)
		if ((filename[i] = *s++) == '\0')
			break;
	filename[MAXBASENAME] = '\0';
	return(TRUE);
}

