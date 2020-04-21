/*	@(#)sleep.c	1.2	*/
/*
**	sleep -- suspend execution for an interval
**
**		sleep time
*/
static	char	*Sccsid = "@(#)$Header: /d2/3.7/src/bin/RCS/sleep.c,v 1.1 89/03/27 14:51:07 root Exp $";
/*
 * $Log:	sleep.c,v $
 * Revision 1.1  89/03/27  14:51:07  root
 * Initial check-in for 3.7
 * 
 * Revision 1.3  85/03/14  20:57:27  bob
 * Fixed to deal with long argv[0] strings.
 * 
 * Revision 1.2  85/03/14  20:39:23  bob
 * Changed so that a "ps" will show time left to sleep rather than the initial
 * time.
 * 
 */

#include	<stdio.h>
#include	<sys/types.h>

time_t	target;
time_t	time();

main(argc, argv)
char **argv;
{
	int	c;
	time_t	n;
	char	*s;

	target = time((time_t *)0);
	n = 0;
	if(argc != 2) {
		fprintf(stderr, "usage: sleep time\n");
		exit(2);
	}
	s = argv[1];
	while(c = *s++) {
		if(c<'0' || c>'9') {
			fprintf(stderr, "sleep: bad character in argument\n");
			exit(2);
		}
		n = n*10 + c - '0';
	}
	target += n;
	while ((n = target - time((time_t *)0)) > 0) {
		sprintf(argv[1],"%d",n);
		if (n > 300) {
			if (n % 300)
				sleep(n % 300);
			else
				sleep(300);
		} else if (n > 60) {
			if (n % 60)
				sleep(n % 60);
			else
				sleep(60);
		} else if (n > 10)
			sleep(5);
		else
			sleep(1);
	}
	exit(0);
}
