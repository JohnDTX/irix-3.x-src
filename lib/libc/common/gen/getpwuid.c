/*	@(#)getpwuid.c	1.2	*/
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
#include <pwd.h>

struct passwd *
getpwuid(uid)
register uid;
{
	register struct passwd *p;

	setpwent();
	while((p = getpwent()) && p->pw_uid != uid)
		;
	endpwent();
	return(p);
}
