char _Origin_[] = "UC Berkeley";

/* Copyright (c) 1979 Regents of the University of California */
#include <pwd.h>
/*
 * whoami
 */
struct	passwd *getpwuid();

main()
{
	register struct passwd *pp;

	pp = getpwuid(getuid());
	if (pp == 0) {
		printf("Intruder alert.\n");
		exit(1);
	}
	printf("%s\n", pp->pw_name);
	exit(0);
}
