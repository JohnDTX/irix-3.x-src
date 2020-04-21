char _Origin_[] = "System V";

/*	@(#)kill.c	1.1	*/
/*
**	kill - send signal to process
*/

#include	<stdio.h>
#ifdef RT
#include	<signal.h>
#include	<errno.h>
#else
#include	<signal.h>
#include	<sys/errno.h>
#endif

main(argc, argv)
char **argv;
{
	register signo, pid, res;
	int	errlev = 0, neg = 0, zero = 0;
	extern	errno;
	char	*msg;

	if (argc <= 1)
		usage();
	if (*argv[1] == '-') {
		signo = atoi(argv[1]+1);
		argc--;
		argv++;
	} else
		signo = SIGTERM;
	argv++;
	while (argc > 1) {
		if (**argv == '-') neg++;
		if (**argv == '0') zero++;
		pid = atoi(*argv);
		if (	((pid == 0) && !zero)
		     || ((pid < 0) && !neg)
#ifndef RT
		     || (pid > 32000)
		     || (pid < -32000)
#endif
		      ) usage();
		res = kill(pid, signo);
		if (res<0) {
			if(pid <= 0) {
				pid = abs(pid);
				msg = "not a killable process group";
			}
			else if (errno == EPERM)
				msg = "permission denied";
			else if (errno == EINVAL)
				msg = "invalid signal";
			else msg = "no such process";
			fprintf(stderr,"kill: %d: %s\n", pid, msg);
			errlev = 2;
		}
		argc--;
		argv++;
		neg = zero = 0;
	}
	return(errlev);
}
usage()
{
	fprintf(stderr, "usage: kill [ -signo ] pid ...\n");
	exit(2);
}
