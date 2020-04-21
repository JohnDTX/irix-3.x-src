char _Origin_[] = "System V";

/*
**	nice
*/

/*	@(#)nice.c	1.4	*/

#include	<stdio.h>
#include	<ctype.h>

main(argc, argv)
int argc;
char *argv[];
{
	int	nicarg = 10;
	extern	errno;
	extern	char *sys_errlist[];

	if(argc > 1 && argv[1][0] == '-') {
		register char	*p = argv[1];

		if(*++p != '-') {
			--p;
		}
		while(*++p)
			if(!isdigit(*p)) {
				fprintf(stderr, "nice: argument must be numeric.\n");
				exit(2);
			}
		nicarg = atoi(&argv[1][1]);
		argc--;
		argv++;
	}
	if(argc < 2) {
		fprintf(stderr, "nice: usage: nice [-num] command\n");
		exit(2);
	}
	nice(nicarg);
	execvp(argv[1], &argv[1]);
	fprintf(stderr, "%s: %s\n", sys_errlist[errno], argv[1]);
	exit(2);
}
