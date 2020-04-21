/*	@(#)abort.c	1.2	*/
/*	3.0 SID #	1.4	*/
/*LINTLIBRARY*/
/*
 *	abort() - terminate current process with dump via SIGIOT
 */

#include <signal.h>

extern int kill(), getpid();

int
abort()
{
	return(kill(getpid(), SIGIOT));
}
