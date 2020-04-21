/*
 * $Source: /d2/3.7/src/stand/lib/dev/RCS/perror.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:14:43 $
 */

/*
 * Print the error indicated
 * in the cerror cell.
 */

#include "errno.h"

extern int errno, sys_nerr;
extern char *sys_errlist[];

void
perror(s)
char	*s;
{
	register char *c = (char *)0;
	register int n;

	if (errno < sys_nerr)
		c = sys_errlist[errno];
	else
		printf("Unknown error %d\n", errno);
	if ( *s != 0 )
		printf("%s: ", s );

	if ( c != 0 )
		printf("%s\n", c);
}
