#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

/*
 * check [-l] [rcsdir]:
 *	- print out in a nice format, all the files that a person
 *	  has checked out in the RCS directory
 *	- if the "-l" flag is on, just print the real name of the
 *	  file (used the same as egrep -l is used)
 *
 * Written by: Kipp Hickman
 */

char	filename[2000];			/* current file name */
char	line[2000];			/* rcs file line being parsed */
char	owner[50];			/* owner of file co'd */
char	revision[10];			/* revision of file co'd */
short	lflag;				/* like egrep -l, print just names */

char	*rcsdir;			/* place to look in */

#define	RCSDIR_DEFAULT	"RCS"

char	*realname();

main(argc, argv)
	int argc;
	char *argv[];
{
	register struct dirent *np;
	register short i;
	register char *truename;
	register DIR *d;
	int found;

	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			switch (argv[i][1]) {
			  case 'l':
				lflag++;
				break;
			  default:
				goto usage;
			}
		} else {
			if (rcsdir == NULL)
				rcsdir = argv[i];
			else {
usage:				fprintf(stderr, "Usage:\n%s [-l] [rcsdir]\n",
						argv[0]);
				exit(2);
			}
		}
	}

	if (rcsdir == NULL) {
		rcsdir = RCSDIR_DEFAULT;
		if (!exists(rcsdir)) {
			rcsdir = ".";
			if (!exists(rcsdir))
				goto no_rcsdir;
		}
	} else {
		if (!exists(rcsdir))
			goto no_rcsdir;
	}

    /* read in the contents of the directory */

	if ((d = opendir(rcsdir)) == NULL) {
no_rcsdir:
		fprintf(stderr, "%s: unable to open \"%s\"\n",
				argv[0], rcsdir);
		exit(2);
	}

	found = 0;
	while (np = readdir(d)) {
		sprintf(filename, "%s/%s", rcsdir, np->d_name);
		if (check(filename)) {
			truename = realname(filename);
			if (lflag)
				printf("%s\n", truename);
			else
				printf("%-20s\t%s revision %s\n",
					    truename, owner, revision);
			found++;
		}
	}
	if (found)
		exit(1);
	else
		exit(0);
}

/*
 * exists:
 *	- see if "name" exists (is stat'able)
 */
exists(name)
	char *name;
{
	struct stat sbuf;

	if (stat(name, &sbuf) == -1)
		return 0;
	if ((sbuf.st_mode & S_IFMT) == S_IFDIR)
		return 1;
	return 0;
}

/*
 * realname:
 *	- given an rcs name, rip its lips off, and leave just the
 *	  actual file name (no path in front, no ,v in back)
 */
char *
realname(s)
	char *s;
{
	register char *cp;
	extern char *strrchr();

	if (cp = strrchr(s, '/'))
		s = cp + 1;
	if (cp = strrchr(s, ','))
		*cp = 0;
	return s;
}

/*
 * islocked:
 *	- see if the line contains locking info
 *	- this code depends on the format of an rcs file
 *	  and assumes a lock line looks like this:
 *		"locks    kipp:1.1; strict;"
 */
islocked(lp)
	register char *lp;
{
	register char *cp;

	lp = lp + sizeof("locks");		/* skip locks */

    /* skip white space between locks and ";" */

	while ((*lp == ' ') || (*lp == '\t'))
		lp++;
	if (*lp == 0)
		return 0;			/* strange file */
	if (*lp == ';')
		return 0;			/* no locks on */

    /* copy owner of lock into "owner" */

	cp = owner;
	while (*lp && (*lp != ':'))
		*cp++ = *lp++;
	*cp = 0;
	if (*lp != ':')
		return 0;			/* strange file */

    /* copy revision number into "revision" */

	cp = revision;
	lp++;					/* skip ":" */
	while (*lp && (*lp != ';'))
		*cp++ = *lp++;
	*cp = 0;
	if (*lp != ';')
		return 0;			/* strange file */
	return 1;
}

/*
 * check:
 *	- see if the file name has a lock
 *	- check gives up after MAXLINES of the file have been
 *	  read in (or EOF)
 *	- zap \n at end of line, left there by fgets
 */

#define	MAXLINES	100		/* should be right at the top */

check(name)
	char *name;
{
	register short lines;
	register char *cp;
	FILE *fd;

	if (fd = fopen(name, "r")) {

    /* search file for "locks" line */

		for (lines = 0; lines < MAXLINES; lines++) {
			if (fgets(line, sizeof(line)-1, fd) == NULL)
				break;
			line[sizeof(line)-1] = 0;
			line[strlen(line)-1] = 0;	/* zap \n */
			if (strncmp(line, "locks", 5))
				continue;
			if (!islocked(line))
				break;
			fclose(fd);
			return 1;
		}
		fclose(fd);
	}
	return 0;
}
