char _Origin_[] = "System V";

/*	@(#)echo.c	1.1	*/
/* $Source: /d2/3.7/src/bin/RCS/echo.c,v $ */
static	char	*Sccsid = "@(#)$Revision: 1.1 $";
/* $Date: 89/03/27 14:50:23 $ */

#include <stdio.h>

main(argc, argv)
char **argv;
{
	register char	*cp;
	register int	i, wd;
	int	j, nflg = 0;

	if(--argc == 0) {
		putchar('\n');
		fflush(stdout);
		if (ferror(stdout))
			perror("echo: write error"), exit(2);
		exit(0);
	}
	if (!strcmp(argv[1], "-n"))
		nflg++, argc--, argv++;
	for(i = 1; i <= argc; i++) {
		for(cp = argv[i]; *cp; cp++) {
			if(*cp == '\\')
			switch(*++cp) {
				case 'b':
					putchar('\b');
					continue;

				case 'c':
					fflush(stdout);
					if (ferror(stdout))
						perror("echo: write error"),
						  exit(2);
					exit(0);

				case 'f':
					putchar('\f');
					continue;

				case 'n':
					putchar('\n');
					continue;

				case 'r':
					putchar('\r');
					continue;

				case 't':
					putchar('\t');
					continue;

				case '\\':
					putchar('\\');
					continue;
				case '0':
					j = wd = 0;
					while ((*++cp >= '0' && *cp <= '7') && j++ < 3) {
						wd <<= 3;
						wd |= (*cp - '0');
					}
					putchar(wd);
					--cp;
					continue;

				default:
					cp--;
			}
			putchar(*cp);
		}
		if (i == argc) {
			if (!nflg)
				putchar('\n');
		}
		else
			putchar(' ');
	}
	fflush(stdout);
	if (ferror(stdout))
		perror("echo: write error"), exit(2);
	exit(0);
}
