/* mklist [ -options ] [ name ] ...

Create a sorted list of pathnames on stdout.  Options:

	-d		restrict output to directory names (equiv -od)
	-e		traverse directories as encountered, rather than
			delaying until asciibetically correct.
	-f		restrict output to file names (equiv -obcflp)
	-i		read names to be processed from stdin
	-l		produce a long listing (similar to "ls -l")
	-n <limit>	limit recursion to "n" levels
	-o <which>	include only <which> types "bcdflp"
	-q		be quiet about non-existing files
	-r <root>	pathnames are relative to <root> (default ".")
	-t <which>	include times in the long form.
	-u		unsorted.

The default root directory for relative names is "."; this may be
altered with the "-r" option (i.e. command-line names are relative to
<root>).  The root prefix is elided in the output, except when the
name being processed has been specified (either on the command line or
on stdin via "-i") as an absolute pathname.

If no names are given, everything under <root> is listed.  If names are
given, they are traversed in lexical order.  If name is "-", base names
are read from stdin and traversed.  The -s option prevents additional
traversal of directories if they have already been traversed; this is
useful for converting "short" listings into "long" listings.

The "short" output is roughly equivalent to a "find" with some cosmetic
editing piped into "sort".  The "long" form is roughly equivalent to
piping the short form into "xargs ls -ld", except that dates are not
included by default.

The "-t" option is followed by a string consisting of one or more of
the letters "a", "c", and "m", which cause the access, change, and
modify times to be printed, respectively.

The "-i" option bypasses the directory traversal algorithms.  It is
useful with option options, e.g. "-r", "-l" and/or "-f", to transform
the list or names in some way.  Names specified on the command line are
still processed in the usual way once end of file is reached on stdin.

The normal behaviour is to produce output that would be unchanged if it
where passed through sort.  In some cases, directory contents will not
immediately follow the directory name, i.e. when there are names lexically
between "foo" and "foo/".  The "-e" option causes the directories be
traversed immediately upon encounter, thus the output will be sorted
with respect to the names in the current directory only, and not necessarily
with respect to lower-level names.

The "-u" option will bypass all sorting algorithms and produce the pathnames
as encountered.

*/

#include <stdio.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/sysmacros.h>
#include <dirent.h>

#define Buffsize	1024
#define Maxuser		2048
#define Maxgrp		2048

#define Atime		0x0001
#define Ctime		0x0002
#define Mtime		0x0004

#define stackcnt(p)	(p->cnt)
#define stackvec(p)	(p->vec)

typedef struct Stack {
	int		size;
	int		cnt;
	char		**vec;
} Stack;

extern char	*malloc ();
extern char	*realloc ();
extern struct group *getgrgid ();

Stack		*newstack ();
char		*top ();
char		*pop ();
char		*cvuser ();
char		*cvgroup ();
char		*cvtime ();
int		namecmp ();
char		*realpath ();
char		*catpath ();

int		encount	= 0;
int		input	= 0;
int		longfmt	= 0;
int		limit	= 32767;
int		quiet	= 0;
char		root [Buffsize] = ".";
int		showtime	= 0;
int		unsorted	= 0;

ushort		types [10];
int		ntypes	= 0;

main (argc, argv)
	int		argc;
	char		*argv [];
{
	char		*p, name [Buffsize], buff [Buffsize];
	struct stat	st;
	int		c, i;
	extern int	optind;
	extern char	*optarg;

	while ((c = getopt (argc, argv, "defiln:o:qr:t:u")) != EOF) {
		switch (c) {
		case 'd': addtype (S_IFDIR); break;
		case 'e': ++encount; break;
		case 'f':
			addtype (S_IFBLK);
			addtype (S_IFCHR);
			addtype (S_IFREG);
			addtype (S_IFLNK);
			addtype (S_IFIFO);
			break;
		case 'i': ++input; break;
		case 'l': ++longfmt; break;
		case 'n': limit = atoi (optarg); break;
		case 'o': while (*optarg) {
				switch (*optarg++) {
				case 'b': addtype (S_IFBLK); break;
				case 'c': addtype (S_IFCHR); break;
				case 'd': addtype (S_IFDIR); break;
				case 'f': addtype (S_IFREG); break;
				case 'l': addtype (S_IFLNK); break;
				case 'p': addtype (S_IFIFO); break;
				default: usage (); break;
				}
			}
			break;
		case 'q': ++quiet; break;
		case 'r': strcpy (root, optarg); break;
		case 't': while (*optarg) {
				switch (*optarg++) {
				case 'a': showtime |= Atime; break;
				case 'c': showtime |= Ctime; break;
				case 'm': showtime |= Mtime; break;
				default: usage (); break;
				}
			}
			break;
		case 'u': ++unsorted; break;
		default: usage (); break;
		}
	}
	if (input) {
		while (fgets (name, sizeof (name), stdin) != NULL) {
			if (name [i = strlen (name) - 1] == '\n') {
				name [i] = '\0';
			}
			if (lstat (realpath (name), &st) < 0) {
				if (!quiet) perror (buff);
			} else {
				processname (name, &st);
			}
		}
	}
	if (optind < argc) {
		processlist ("", argc - optind, argv + optind, limit);
	} else if (!input) {
		strcpy (name, "");
		argv [0] = name;
		processlist ("", 1, argv, limit);
	}
	exit (0);
}

usage ()
{
	fprintf (stderr, "usage: mklist [-defilqu] [-n lim] [-o whichtypes] ");
	fprintf (stderr, "[-r root] [-t whichtimes]\n");
	exit (1);
}

addtype (m)
	ushort		m;
{
	int		i;

	for (i = 0; i < ntypes; ++i) {
		if (types [i] == m) return;
	}
	if (ntypes < sizeof (types) / sizeof (*types)) {
		types [ntypes++] = m;
	}
}

wanted (m)
	ushort		m;
{
	int		i;

	if (ntypes == 0) return (1);
	for (i = 0; i < ntypes; ++i) {
		if (types [i] == m) return (1);
	}
	return (0);
}

processname (name, st)
	char		*name;
	struct stat	*st;
{
	int		type, c;
	char		buff [Buffsize];

	if (!wanted (type = st->st_mode & S_IFMT)) return;
	if (longfmt) {
		switch (type) {
		case S_IFBLK:	c = 'b'; break;
		case S_IFCHR:	c = 'c'; break;
		case S_IFDIR:	c = 'd'; break;
		case S_IFREG:	c = 'f'; break;
		case S_IFLNK:	c = 'l'; break;
		case S_IFIFO:	c = 'p'; break;
		default:	c = '?'; break;
		}
		printf ("%c %04o %2d %-8s %-8s ", c, st->st_mode & 07777,
			st->st_nlink, cvuser (st->st_uid),
			cvgroup (st->st_gid));
		if (type == S_IFCHR || type == S_IFBLK) {
			printf ("  %3d/%3d ", major (st->st_rdev),
				minor (st->st_rdev));
		} else {
			printf ("%9d ", st->st_size);
		}
	}
	if (showtime & Atime) printf ("%s ", cvtime (st->st_atime));
	if (showtime & Ctime) printf ("%s ", cvtime (st->st_ctime));
	if (showtime & Mtime) printf ("%s ", cvtime (st->st_mtime));
	printf ("%s\n", *name ? name : ".");
}

/* processlist takes a directory name and list of names within that directory,
 * sorts them if necessary, and processes each name and directory in the list.
 * Consideration is given to the lexical ordering of the output; see dcmp().
 */

processlist (dname, argc, argv, lim)
	char		*dname;
	int		argc;
	char		**argv;
	int		lim;
{
	char		rname [Buffsize], *p;
	Stack		*pending;
	int		i;
	struct stat	st;

	if (lim < 0) return;
	if (!unsorted && argc > 1) qsort (argv, argc, sizeof (*argv), namecmp);
	pending = newstack ();
	for (i = 0; i < argc; ++i) {
		strcpy (rname, catpath (dname, argv [i]));
		while ((p = top (pending)) != NULL && dcmp (p, rname) < 0) {
			processdir (pop (pending), lim - 1);
		}
		if (lstat (realpath (rname), &st) < 0) {
			if (!quiet) perror (rname);
			continue;
		}
		processname (rname, &st);
		if ((st.st_mode & S_IFMT) == S_IFDIR) {
			if (unsorted || encount) processdir (rname);
			else push (pending, rname);
		}
	}
	while ((p = pop (pending)) != NULL) processdir (p, lim - 1);
	freestack (pending);
}

/* processdir takes a relative directory name, loads up all the names in
 * it into a list, and processes the list.
 */

processdir (dname, lim)
	char		*dname;
	int		lim;
{
	DIR		*dir;
	Stack		*dstk;
	struct dirent	*d;

	if (lim < 0) return;
	if ((dir = opendir (realpath (dname))) == NULL) {
		if (!quiet) perror (dname);
		return;
	}
	dstk = newstack ();
	while (d = readdir (dir)) {
		if (strcmp (d->d_name, ".") != 0 &&
		    strcmp (d->d_name, "..") != 0) {
			push (dstk, d->d_name);
		}
	}
	closedir (dir);
	processlist (dname, stackcnt (dstk), stackvec (dstk), lim);
	freestack (dstk);
}

/* newstack creates a new, empty stack */

Stack *
newstack ()
{
	Stack		*p;

	if ((p = (Stack *) malloc (sizeof (Stack))) == NULL) {
		fprintf (stderr, "out of memory.\n"); exit (1);
	}
	p->cnt = 0;
	p->vec = NULL;
	p->size = 0;
	return (p);
}

/* push pushes a copy of a string onto a stack */

push (p, v)
	Stack		*p;
	char		*v;
{
	int		inc, i;

	if (p->cnt >= p->size) {
		if ((inc = p->size / 4) < 32) inc = 32;
		else if (inc > 1024) inc = 1024;
		p->size += inc;
		if (p->vec == NULL) {
			p->vec = (char **) malloc (p->size * sizeof (char *));
		} else {
			p->vec = (char **) realloc (p->vec,
				p->size * sizeof (char *));
		}
		if (p->vec == NULL) {
			fprintf (stderr, "out of memory\n"); exit (1);
		}
		for (i = p->cnt; i < p->size; ++i) {	
			p->vec [i] = NULL;
		}
	} else {
		if (p->vec [p->cnt] != NULL) free (p->vec [p->cnt]);
	}
	if ((p->vec [p->cnt] = malloc (strlen (v) + 1)) == NULL) {
		fprintf (stderr, "out of memory\n"); exit (1);
	}
	strcpy (p->vec [p->cnt++], v);
}

/* top returns the top string on the stack, or NULL if empty */

char *
top (p)
	Stack		*p;
{
	if (p->cnt <= 0) return (NULL);
	else return (p->vec [p->cnt - 1]);
}

/* pop returns the top string on the stack, or NULL if empty, and pops it. */

char *
pop (p)
	Stack		*p;
{
	if (p->cnt <= 0) return (NULL);
	else return (p->vec [--p->cnt]);
}

/* freestack frees all memory in a stack, including values pushed and the
 * stack itself.
 */

freestack (p)
	Stack		*p;
{
	int		i;

	if (p == NULL) return;
	if (p->size > 0) {
		for (i = 0; i < p->size; ++i) {
			if (p->vec [i] != NULL) free (p->vec [i]);
		}
	}
	if (p->vec != NULL) free (p->vec);
	free (p);
}

/* cvuser converts a user id into a name, stashing names to avoid duplicate
 * calls to getpwuid() (which can be very slow).
 */

char *
cvuser (uid)
	int		uid;
{
	int		i;
	char		buff [Buffsize];
	struct passwd	*p;
	static int	nuser = 0;
	static struct {
		short		uid;
		char		*name;
	} user [Maxuser];

	for (i = 0; i < nuser; ++i) {
		if (user [i].uid == uid) return (user [i].name);
	}
	if ((p = getpwuid (uid)) == NULL) {
		sprintf (buff, "%d", uid);
		return (buff);
	}
	if (nuser >= Maxuser - 1) return (p->pw_name);
	user [nuser].uid = uid;
	if ((user [nuser].name = malloc (strlen (p->pw_name) + 1)) == NULL) {
		fprintf (stderr, "out of memory.\n");
		exit (1);
	}
	strcpy (user [nuser].name, p->pw_name);
	return (user [nuser++].name);
}

/* cvgroup converts a group id into a name, stashing names to avoid duplicate
 * calls to getgrid ().
 */

char *
cvgroup (gid)
	int		gid;
{
	int		i;
	char		buff [Buffsize];
	struct group	*p;
	static int	ngrp = 0;
	static struct {
		short		gid;
		char		*name;
	} grp [Maxgrp];

	for (i = 0; i < ngrp; ++i) {
		if (grp [i].gid == gid) return (grp [i].name);
	}
	if ((p = getgrgid (gid)) == NULL) {
		sprintf (buff, "%d", gid);
		return (buff);
	}
	if (ngrp >= Maxgrp - 1) return (p->gr_name);
	grp [ngrp].gid = gid;
	if ((grp [ngrp].name = malloc (strlen (p->gr_name) + 1)) == NULL) {
		fprintf (stderr, "out of memory.\n");
		exit (1);
	}
	strcpy (grp [ngrp].name, p->gr_name);
	return (grp [ngrp++].name);
}

char *
cvtime (t)
	long		t;
{
	static char	buff [Buffsize];
	struct tm	*p;

	p = localtime (&t);
	sprintf (buff, "%02d/%02d/%02d %02d:%02d:%02d", p->tm_year,
		p->tm_mon + 1, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
	return (buff);
}

/* namecmp compares two pointers to characters; i.e. two elements of a vector.
 * It is used for qsorts of vectors.
 */

namecmp (s1, s2)
	char		**s1;
	char		**s2;
{
	return (strcmp (*s1, *s2));
}

/* dcmp compares pname and cname with a virtual slash appended to pname; i.e.
 * the comparison is based on pname being part of a longer pathname when it
 * is traversed as a directory.  It is used to decide when a directory should
 * be traversed so that output will be asciibetically sorted.
 */

dcmp (pname, cname)
	char		*pname;		/* pending directory name */
	char		*cname;		/* current name */
{
	int		c;

	while (*pname) {
		if (*cname == 0) return (1);
		if ((c = *pname - *cname) != 0) return (c);
		++pname, ++cname;
	}
	return ('/' - *cname);
}

char *
realpath (name)
	char		*name;
{
	static char	buff [Buffsize];

	if (*name == '/') {
		return (name);
	} else {
		return (catpath (root, name));
	}
}

char *
catpath (dir, name)
	char		*dir;
	char		*name;
{
	static char	buff [Buffsize];

	strcpy (buff, dir);
	if (*buff && *name) strcat (buff, "/");
	strcat (buff, name);
	return (buff);
}
