/* @(#)assert.c	1.4 */
#include "uucp.h"
#include <time.h>
#include <sys/types.h>

/*
 * print out assetion error
 * return: 
 *  	none
 */
assert(s1, s2, i1)
char *s1, *s2;
{
	register FILE *errlog;
	register struct tm *tp;
	extern struct tm *localtime();
	time_t clock, time();
	int pid;
	int	mask;

	if (Debug)
		errlog = stderr;
	else{
		mask = umask(0);
		errlog = fopen(ERRLOG, "a");
		umask(mask);
	}
	if (errlog == NULL)
		return;

	pid = getpid();
	fprintf(errlog, "ASSERT ERROR (%.9s)  ", Progname);
	fprintf(errlog, "pid: %d  ", pid);
	time(&clock);
	tp = localtime(&clock);
	fprintf(errlog, "(%d/%d-%d:%2.2d:%2.2d) ", tp->tm_mon + 1,
		tp->tm_mday, tp->tm_hour, tp->tm_min, tp->tm_sec);
	fprintf(errlog, "%s %s (%d)\n", s1, s2, i1);
	fclose(errlog);
	return;
}
