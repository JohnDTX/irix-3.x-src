/*
 *	fullpath(name, curdir) -- returns full pathname of file "name" when
 *	in current directory "curdir"
 */

#include	"lp.h"

SCCSID("@(#)fullpath.c	3.1")

char *
fullpath(name, curdir)
char *name;
char *curdir;
{
	static char fullname[FILEMAX];
	char *strcpy();

	if(*name == '/')
		return(name);
	else if(*curdir == '\0')
		return(NULL);
	else {
		sprintf(fullname, "%s/%s", curdir, name);
		return(fullname);
	}
}
