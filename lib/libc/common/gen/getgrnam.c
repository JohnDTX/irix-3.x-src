/*	@(#)getgrnam.c	1.2	*/
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
#include <grp.h>

extern struct group *getgrent();
extern int setgrent(), endgrent();
extern int strcmp();

struct group *
getgrnam(name)
register char *name;
{
	register struct group *p;

	(void) setgrent();
	while((p = getgrent()) && strcmp(p->gr_name, name));
	(void) endgrent();
	return(p);
}
