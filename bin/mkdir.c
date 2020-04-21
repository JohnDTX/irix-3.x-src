static char *sccsid = "@(#)mkdir.c	4.4 (Berkeley) 12/19/82";
/*
 * make directory
 */
#include <stdio.h>

main(argc, argv)
	char *argv[];
{
	int errors = 0;

	if (argc < 2) {
		fprintf(stderr, "mkdir: arg count\n");
		exit(1);
	}
	while (--argc)
		if (mkdir(*++argv, 0777) < 0) {
			fprintf(stderr, "mkdir: ");
			perror(*argv);
			errors++;
		}
	exit(errors != 0);
}
