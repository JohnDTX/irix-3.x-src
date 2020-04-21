static char *sccsid = "@(#)printenv.c	4.1 (Berkeley) 10/2/80";
/*
 * printenv
 *
 * Bill Joy, UCB
 * February, 1979
 */

extern	char **environ;

main(argc, argv)
	int argc;
	char *argv[];
{
	register char **ep;
	int found = 0;

	argc--, argv++;
	if (environ)
		for (ep = environ; *ep; ep++)
			if (argc == 0 || prefix(argv[0], *ep)) {
				register char *cp = *ep;

				found++;
				if (argc) {
					while (*cp && *cp != '=')
						cp++;
					if (*cp == '=')
						cp++;
				}
				printf("%s\n", cp);
			}
	exit (!found);
}

prefix(cp, dp)
	char *cp, *dp;
{

	while (*cp && *dp && *cp == *dp)
		cp++, dp++;
	if (*cp == 0)
		return (*dp == '=');
	return (0);
}
