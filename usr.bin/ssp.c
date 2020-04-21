char _Origin_[] = "UC Berkeley";

static	char	*Sccsid = "@(#)ssp.c	2.3";

/* Copyright (c) 1979 Regents of the University of California */
#include <stdio.h>
/*
 * ssp - single space output
 *
 * Bill Joy UCB August 25, 1977
 *
 * Compress multiple empty lines to a single empty line.
 * Option - compresses to nothing.
 */

char	poof, hadsome;

int	ibuf[256];


main(argc, argv)
	int argc;
	char *argv[];
{
	register int c;
	FILE *f;
	static char _obuf[BUFSIZ];

	setbuf(stdout, _obuf);
	argc--, argv++;
	do {
		while (argc > 0 && argv[0][0] == '-') {
			poof = 1;
			argc--, argv++;
		}
	f = stdin;
		if (argc > 0) {
			if ((f=fopen(argv[0], "r")) == NULL) {
				fflush(f);
				fflush(stdout);
				perror(argv[0]);
				exit(1);
			}
			argc--, argv++;
		}
		for (;;) {
			c = getc(f);
			if (c == -1)
				break;
			if (c != '\n') {
				hadsome = 1;
				putchar(c);
				continue;
			}
			/*
			 * Eat em up
			 */
			if (hadsome)
				putchar('\n');
			c = getc(f);
			if (c == -1)
				break;
			fflush(stdout);
			if (c != '\n') {
				putchar(c);
				hadsome = 1;
				continue;
			}
			do
				c = getc(f);
			while (c == '\n');
			if (!poof && hadsome)
				putchar('\n');
			if (c == -1)
				break;
			putchar(c);
			hadsome = 1;
		}
		fflush(stdout);
	} while (argc > 0);
}
