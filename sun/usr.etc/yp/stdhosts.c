#ifndef lint
/* @(#)stdhosts.c	2.1 86/04/16 NFSSRC */
static  char sccsid[] = "@(#)stdhosts.c 1.1 86/02/05 Copyr 1985 Sun Micro";
#endif

/*
 * Copyright (c) 1985 by Sun Microsystems, Inc.
 */

#include <stdio.h>

/* 
 * Filter to convert addresses in /etc/hosts file to standard form
 */

main(argc, argv)
	char **argv;
{
	char line[256];
	char adr[256];
	char *any(), *trailer;
	FILE *fp;
	
	if (argc > 1) {
		fp = fopen(argv[1], "r");
		if (fp == NULL) {
			fprintf(stderr, "stdhosts: can't open %s\n", argv[1]);
			exit(1);
		}
	}
	else
		fp = stdin;
	while (fgets(line, sizeof(line), fp)) {
		if (line[0] == '#')
			continue;
		if ((trailer = any(line, " \t")) == NULL)
			continue;
		sscanf(line, "%s", adr);
		fputs(inet_ntoa(inet_addr(adr)), stdout);
		fputs(trailer, stdout);
	}
}

/* 
 * scans cp, looking for a match with any character
 * in match.  Returns pointer to place in cp that matched
 * (or NULL if no match)
 */
static char *
any(cp, match)
	register char *cp;
	char *match;
{
	register char *mp, c;

	while (c = *cp) {
		for (mp = match; *mp; mp++)
			if (*mp == c)
				return (cp);
		cp++;
	}
	return (NULL);
}
