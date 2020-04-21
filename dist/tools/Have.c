/* Have -- do we have an entry installed
 *
 * Have [ -r root ] [ -v ] [ -s ] [ names ... ]
 *
 * We check to see whether the named entries are installed by searching
 * for the files in /dist/list.name for each name.  The report indicates
 * whether the entry is installed or not, or is partially installed but
 * missing some files.  The -r option specifies an alternate root
 * directory.  The -v option specifies "verbose"; i.e. the names of
 * missing files will be reported.  The -s option specifies "silent"; the
 * usual messages are suppressed in favor of exit status, where 0 indicates
 * that the entry is installed, 1 indicates that it is not installed,
 * and 2 indicates that files are missing.  The -s option exits after the
 * first "name"; the remainder are ignored.  The default names are taken
 * from column one of the "dist/toc" file.
 *
 */

#include 	<stdio.h>

char		*zap ();

char		*root		= "/";
int		silent		= 0;
int		verbose		= 0;
char		*msg [] 	= {"installed", "not installed", "incomplete"};

main (argc, argv)
	int		argc;
	char		*argv [];
{
	FILE		*f;
	char		line [1024], *p;
	int		a, c;

	for (a = 1; a < argc && *argv [a] == '-'; ++a) {
		c = argv [a][1];
		switch (c) {
		case 'r':	if (*(p = argv [a] + 2) == '\0') {
					if (a < argc - 1) p = argv [++a];
					else usage ();
				}
				root = p; break;
		case 's':	silent = 1 ; break;
		case 'v':	verbose = 1; break;
		default:	usage ();
		}
	}
	if (chdir (root) == -1) {
		perror (root); exit (1);
	}
	if (a >= argc) {
		if ((f = fopen ("dist/toc", "r")) == NULL) {
			fprintf (stderr, "no dist/toc\n"); exit (1);
		}
		while (fgets (line, sizeof (line), f) != NULL) {
			zap (line);
			have (line);
		}
	}
	else while (a < argc) have (argv [a++]);
	exit (0);
}

usage ()
{
	fprintf (stderr, "usage: Have [ -rs ] [ -r root ] [ names... ]\n");
	exit (1);
}

have (name)
	char		*name;
{
	char		buff [1024], *lastc;
	int		theoretical, actual, t;
	FILE		*f;

	strcpy (buff, "dist/list."); strcat (buff, name);
	if ((f = fopen (buff, "r")) == NULL) {
		perror (buff); return;
	}
	theoretical = actual = 0;
	while (fgets (buff, sizeof (buff), f) != NULL) {
		lastc = zap (buff); ++theoretical;
		if (access (buff, 0) == 0 || lastc >= buff &&
		    (*lastc == '+' || *lastc == '-' || *lastc == '@') &&
		    (*lastc = '\0', access (buff, 0)) == 0) ++actual;
		else if (verbose) printf ("%s: missing %s\n", name, buff);
	}
	fclose (f);
	t = (theoretical == actual) ? 0 : (actual == 0) ? 1 : 2;
	if (silent) exit (t);
	printf ("%-16.16s%s\n", name, msg [t]);
}

char *
zap(s)
	register char	*s;
{
	register int	c;

	while ((c = *s) && c != ' ' && c != '\t' && c != '\n') ++s;
	*s = '\0';
	return (s - 1);
}
