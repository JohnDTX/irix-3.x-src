/* @(#)anyread.c	1.3 */
#include "uucp.h"
#include <sys/types.h>
#include <sys/stat.h>


/*
 * check if file has any read permissions 
 *	file	-> file to check
 * return 
 *	0	-> ok
 * 	FAIL	-> not ok
 */
anyread(file)
char *file;
{
	struct stat s;

	/*
	 * for security check a non existant file is readable
	 */
	if (stat(file, &s) != 0)
		return(0);
	if (!(s.st_mode & ANYREAD))
		return(FAIL);
	return(0);
}
someread(file)
char *file;
{
	struct stat s;

	/*
	 * for security check a non existant file is readable
	 */
	if (stat(file, &s) != 0)
		return(0);
	if (!(s.st_mode & 0444))
		return(FAIL);
	return(0);
}
