char _Origin_[] = "System V";

static char ID[] = "@(#)fortune.c	1.1";

#include <stdio.h>

char line[500];
char bline[500];

main()
{
	double p, pow();
	register char * l;
	long t;
	FILE *f;

	f = fopen("/usr/games/lib/fortunes", "r");
	if (f == NULL) {
		printf("Memory fault -- core dumped\n");
		exit(1);
	}
	time(&t);
	srand(getpid() + (int)((t>>16) + t));
	p = 1.;
	for(;;) {
		l = fgets(line, 500, f);
		if(l == NULL)
			break;
		if(rand() < 32767/p)
			strcpy(bline, line);
		p += 1.;
	}
	fputs(bline, stdout);
	return(0);
}
