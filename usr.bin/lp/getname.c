/*
 *	getname(name)  --  get logname
 *
 *		getname tries to find the user's logname from:
 *			${LOGNAME}, if set and if it is telling the truth
 *			/etc/passwd, otherwise
 *
 *		The logname is returned as the value of the function.
 *
 *		Getname returns the user's user id converted to ASCII
 *		for unknown lognames.
 *
 */

#include	"lp.h"

SCCSID("@(#)getname.c	3.1")

char *
getname(name)
char *name;
{
	int uid;
	struct passwd *p, *getpwnam(), *getpwuid();
	static char logname[LOGMAX + 1];
	char *l, *getenv(), *strncpy();

	uid = getuid();
	setpwent();

	if((l = getenv("LOGNAME")) == NULL ||
	   (p = getpwnam(l)) == NULL || p -> pw_uid != uid)
		if((p = getpwuid(uid)) != NULL)
			l = p->pw_name;
		else
			l = NULL;

	if(l != NULL) {
		strncpy(logname, l, LOGMAX);
		logname[LOGMAX] = '\0';
	}
	else
		sprintf(logname, "%d", uid);
	endpwent();
	return(logname);
}
