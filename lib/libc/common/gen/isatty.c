/*	@(#)isatty.c	1.2	*/
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
/*
 * Returns 1 iff file is a tty
 */
#include <sys/termio.h>

extern int ioctl();

int
isatty(f)
int	f;
{
	struct termio tty;

	if(ioctl(f, TCGETA, &tty) < 0)
		return(0);
	return(1);
}
