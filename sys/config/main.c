/*
 * $Source: /d2/3.7/src/sys/config/RCS/main.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:27:30 $
 */
#ifndef lint
static char sccsid[] = "@(#)main.c	1.10 (Berkeley) 8/11/83";
#endif

#include <stdio.h>
#include <ctype.h>
#include "y.tab.h"
#include "config.h"

/*
 * Config builds a set of files for building a UNIX
 * system given a description of the desired system.
 */
main(argc, argv)
	int argc;
	char **argv;
{

	if (argc > 1 && eq("-p", argv[1])) {
		profiling++;
		argc--, argv++;
	}
	if (argc > 1 && eq("-b", argv[1])) {
		binaryconfig++;
		argc--, argv++;
	}
	if (argc != 2) {
		fprintf(stderr, "usage: config [ -p ] sysname\n");
		exit(1);
	}
	PREFIX = argv[1];
	if (freopen(argv[1], "r", stdin) == NULL) {
		perror(argv[1]);
		exit(2);
	}
	dtab = NULL;
	confp = &conf_list;
	if (yyparse())
		exit(3);
	switch (machine) {

	case MACHINE_VAX:
		vax_ioconf();		/* Print ioconf.c */
		ubglue();		/* Create ubglue.s */
		break;

	case MACHINE_SUN:
		sun_ioconf();
		break;

	case MACHINE_PM2:
	case MACHINE_IP2:
		sgi_ioconf(machine);
		break;

	default:
		printf("Specify machine type, e.g. ``machine vax''\n");
		exit(1);
	}
	makefile();			/* build Makefile */
	headers();			/* make a lot of .h files */
	swapconf();			/* swap config files */
	printf("Don't forget to run \"make depend\"\n");
	exit(0);
}

/*
 * get_word
 *	returns EOF on end of file
 *	NULL on end of line
 *	pointer to the word otherwise
 */
char *
get_word(fp)
	register FILE *fp;
{
	static char line[80];
	register int ch;
	register char *cp;

	while ((ch = getc(fp)) != EOF)
		if (ch != ' ' && ch != '\t')
			break;
	if (ch == EOF)
		return ((char *)EOF);
	if (ch == '\n')
		return (NULL);
	cp = line;
	*cp++ = ch;
	while ((ch = getc(fp)) != EOF) {
		if (isspace(ch))
			break;
		*cp++ = ch;
	}
	*cp = 0;
	if (ch == EOF)
		return ((char *)EOF);
	(void) ungetc(ch, fp);
	return (line);
}

/*
 * prepend the path to a filename
 */
char *
path(file)
	char *file;
{
	register char *cp;

	cp = malloc((unsigned)(strlen(PREFIX)+strlen(file)+5));
	(void) strcpy(cp, "../");
	(void) strcat(cp, PREFIX);
	(void) strcat(cp, "/");
	(void) strcat(cp, file);
	return (cp);
}
