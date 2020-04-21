#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include "idb.h"
#include "inst.h"
#include <termio.h>
#include <stdio.h>
#include <pwd.h>
#include <errno.h>

#define IOstyle		0x000F
#define Direct		0x0001
#define Buffered	0x0002
#define Pipe		0x0010
#define Dir		0x0020
#define Write		0x0040
#define Remote		0x0080
#define Eof		0x0100

int		rread ();
int		rwrite ();
int		read ();
int		write ();
char		*malloc ();
short		getshort ();
long		getlong ();
char		*getstr ();
char		*xrsh ();

typedef struct Tape {
	char		name [Strsize];
	int		filnum;
} Tape;

typedef struct Iob {
	DIR		*dir;
	int		flags;
	long		bsize;
	char		*buff;
	char		*buffp;
	int		cnt;
	long		pos;
	Tape		*tape;
} Iob;

Iob		iob [NOFILE];
Tape		tape [20];
int		ntape = 0;
int		tapeblocksize = 4096;

/* parsename - break a name into user, host, and filename */

static
parsename (name, who, host, fname)
	char		*name;
	char		*who;
	char		*host;
	char		*fname;
{
	char		buff [Strsize];
	static char	me [Strsize] = "";
	struct passwd	*pw, *getpwuid ();

	split (name, buff, ':', fname);
	if (*fname == '\0') {
		strcpy (fname, buff);
		*host = *who = '\0';
	} else {
		split (buff, who, '@', host);
		if (*host == '\0') {
			strcpy (host, who);
			if (*me == '\0') {
				if ((pw = getpwuid (geteuid ())) != NULL) {
					strcpy (me, pw->pw_name);
				}
			}
			if (*me != '\0') strcpy (who, me);
			else strcpy (who, "guest");
		}
	}
}

static char *
rshcmd (who, host)
	char		*who;
	char		*host;
{
	static char	buff [Strsize];

	if (*host == '\0') return ("");
	sprintf (buff, "%s %s -l %s", xrsh (), host, who);
	return (buff);
}

static Tape *
vfindtape (name)
	char		*name;
{
	int		i;

	if (typeof (name) != Is_tape) return (NULL);
	for (i = 0; i < ntape; ++i) {
		if (strcmp (name, tape [i].name) == 0) {
			break;
		}
	}
	if (i >= ntape) {
		if (ntape >= sizeof (tape) / sizeof (*tape)) {
			--i;
		} else {
			++ntape;
		}
		strcpy (tape [i].name, name);
		tape [i].filnum = -1;
	}
	return (tape + i);
}

vfilnum (name, filnum)
	char		*name;
	int		filnum;
{
	Tape		*tp;
	int		r, t;
	char		cmd [Strsize];
	char		who [Strsize], host [Strsize], fname [Strsize];

	if ((tp = vfindtape (name)) == NULL) return (0);
	parsename (name, who, host, fname);
	r = 0;
	if (tp->filnum < 0 || tp->filnum > filnum) {
		r = vrewind (name);
	}
	if (filnum < tp->filnum || filnum - tp->filnum > 64) {
		r = -1;
	}
	if (r == 0 && filnum > tp->filnum) {
		sprintf (cmd, "%s /bin/mt -t %s fsf %d", rshcmd (who, host),
			fname, filnum - tp->filnum);
		r = runline (cmd, Noisy);
	}
	if (r == 0) tp->filnum = filnum;
	else tp->filnum = -1;
	return (r);
}

vrewind (name)
	char		*name;
{
	char		who [Strsize], host [Strsize], fname [Strsize];
	char		cmd [Strsize];
	Tape		*tp;
	int		r;

	if ((tp = vfindtape (name)) == NULL) return (0);
	parsename (name, who, host, fname);
	sprintf (cmd, "%s /bin/mt -t %s rewind", rshcmd (who, host), fname);
	if ((r = runline (cmd, Noisy)) == 0) {
		tp->filnum = 0;
	}
	return (r);
}

vretension (name)
	char		*name;
{
	char		host [Strsize], fname [Strsize], prefix [Strsize];
	char		cmd [Strsize], who [Strsize];
	Tape		*tp;
	int		r;

	if ((tp = vfindtape (name)) == NULL) return (0);
	parsename (name, who, host, fname);
	sprintf (cmd, "%s /bin/mt -t %s retension", rshcmd (who, host), fname);
	if ((r = runline (cmd, Noisy)) == 0) {
		tp->filnum = 0;
	}
	return (r);
}

vopen (name, mode)
	char		*name;
	int		mode;
{
	Iob		*p;
	Tape		*tp;
	struct stat	st;
	int		f, i, buffsize;
	static char	buff [Strsize];
	char		who [Strsize], host [Strsize], fname [Strsize];
	char		*argv [4];

	parsename (name, who, host, fname);
	if (*host == '\0' && mode == 1 && stat (fname, &st) < 0) {
		close (creat (fname, 0644));
	}
	if ((tp = vfindtape (name)) != NULL) {
		buffsize = tapeblocksize;
	} else {
		buffsize = 4096;
	}
	if (*host) {
		if ((f = ropen (host, who, fname, mode, buffsize)) < 0) {
			return (-1);
		}
		p = iob + f;
		p->flags = Remote;
	} else {
		if ((f = open (name, mode)) < 0) {
			return (-1);
		}
		p = iob + f;
		p->flags = 0;
	}
	p->flags |= Buffered;
	if (mode != 0) p->flags |= Write;
	p->bsize = buffsize;
	p->tape = tp;
	if ((p->buff = malloc (p->bsize)) == NULL) {
		fprintf (stderr, "Can't get buffer memory for %s\n", name);
		return (-1);
	}
	p->buffp = p->buff;
	p->cnt = 0;
	p->pos = 0L;
	return (f);
}

vcreat (name, mode)
	char		*name;
	int		mode;
{
	char		part1 [Strsize], part2 [Strsize];
	int		f;

	split (name, part1, ':', part2);
	if (*part2 == '\0') {
		if ((f = creat (name, mode)) < 0) return (-1);
		close (f);
	}
	return (vopen (name, 1));
}

vclose (f)
	int		f;
{
	Iob		*p;
	int		e, n, i, j, k, (*reader) ();

	p = iob + f;
	e = 0;
	if (p->flags & Buffered) {
		if ((p->flags & Write) && p->cnt > 0) {
			if (write (f, p->buff, p->cnt) != p->cnt) e = -1;
		} else if (p->tape != NULL) {
#ifndef IP2
			reader = p->flags & Remote ? rread : read;
			while ((n = reader (f, p->buff, p->bsize)) > 0) 
				;
#endif
			++p->tape->filnum;
		}
	}
	if (p->buff != NULL) free (p->buff);
	if (p->flags & Remote) {
		if (rclose (f) < 0) e = -1;
	} else {
		if (close (f) < 0) e = -1;
	}
	if (p->flags & Pipe) {
		if (waitpipe (f) < 0) e = -1;
	}
	p->flags = 0;
	return (e);
}

vdirect (f)
	int		f;
{
	Iob		*p;

	p = iob + f;
	p->flags &= ~IOstyle;
	p->flags |= Direct;
}

long
vseek (f, offset, whence)
	int		f;
	long		offset;
	int		whence;
{
	Iob		*p;
	int		b;
	long		pos, lseek ();

	p = iob + f;
	if (p->flags & (Remote | Pipe | Dir | Write)) return (-1);
	if (p->flags & Direct) return (lseek (f, offset, whence));
	if (whence == 0) {
		pos = offset;
	} else if (whence == 1) {
		pos = offset + p->pos;
	} else if (whence == 2) {
		if ((pos = lseek (f, 0L, 2)) < 0) return (-1);
		pos += offset;
	} else {
		errno = EINVAL;
		return (-1);
	}
	if ((p->pos = lseek (f, (pos / p->bsize) * p->bsize, 0)) < 0)
		return (-1);
	if ((p->cnt = read (f, p->buffp = p->buff, p->bsize)) < 0)
		return (-1);
	if ((b = pos - p->pos) > p->cnt) b = p->cnt;
	p->buffp += b;
	p->cnt -= b;
	p->pos += b;
	return (p->pos);
}

vfill (f)
	int		f;
{
	Iob		*p;
	int		(*reader) ();

	p = iob + f;
	if ((p->flags & Buffered) == 0) return (0);
	reader = p->flags & Remote ? rread : read;
	if (p->cnt <= 0) p->cnt = reader (f, p->buffp = p->buff, p->bsize);
	return (p->cnt);
}

vread (f, buff, n)
	int		f;
	char		*buff;
	int		n;
{
	int		nr, b, (*reader) ();
	Iob		*p;

	p = iob + f;
	reader = p->flags & Remote ? rread : read;
	switch (p->flags & IOstyle) {
	case Direct:
		if (p->flags & Eof) { nr = 0; break; }
		nr = reader (f, buff, n);
		if (nr < n) p->flags |= Eof;
		break;
	case Buffered:
		nr = 0;
		while (n) {
			if (p->cnt == 0) {
				if (p->flags & Eof) break;
				p->buffp = p->buff;
				p->cnt = reader (f, p->buff, p->bsize);
				if (p->cnt < p->bsize) p->flags |= Eof;
				if (p->cnt <= 0) break;
			}
			if ((b = n) > p->cnt) b = p->cnt;
			bcopy (p->buffp, buff, b);
			buff += b;
			p->buffp += b;
			p->cnt -= b;
			n -= b;
			nr += b;
		}
		if (p->cnt < 0 && nr == 0) nr = -1;
		break;
	default:
		nr = -1;
		break;
	}
	if (nr >= 0) p->pos += nr;
	return (nr);
}

vwrite (f, buff, n)
	int		f;
	char		*buff;
	int		n;
{
	Iob		*p;
	int		nw, b, (*writer) ();

	p = iob + f;
	writer = p->flags & Remote ? rwrite : write;
	switch (p->flags & IOstyle) {
	case Direct:
		nw = writer (f, buff, n);
		break;
	case Buffered:
		nw = 0;
		while (n) {
			if (p->cnt >= p->bsize) {
				if (writer (f, p->buff, p->bsize) != p->bsize) {
					nw = -1;
					break;
				}
				p->buffp = p->buff; p->cnt = 0;
			}
			if ((b = n) + p->cnt > p->bsize) {
				b = p->bsize - p->cnt;
			}
			bcopy (buff, p->buffp, b);
			p->buffp += b;
			p->cnt += b;
			buff += b;
			nw += b;
			n -= b;
		}
		break;
	default:
		nw = -1;
	}
	if (nw >= 0) p->pos += nw;
	return (nw);
}

vskip (f, n)
	int		f;
	int		n;
{
	char		space [4096];
	int		i, b;
	Iob		*p;

	while (n > 0) {
		if ((b = n) > sizeof (space)) b = sizeof (space);
		if (vread (f, space, b) != b) return (-1);
		n -= b;
	}
	return (0);
}

vpad (f, n)
	int		f;
	long		n;
{
	long		nb, there;
	Iob		*p;
	static char	buff [1024];		/* zeros */

	p = iob + f;
	there = ((p->pos + n) / n) * n;
	while ((nb = there - p->pos) > 0) {
		if (nb > sizeof (buff)) nb = sizeof (buff);
		if (vwrite (f, buff, nb) != nb) return (-1);
	}
	return (p->pos);
}

vgets (line, linelen)
	char		line [];
	int		linelen;
{
	char		*s, c;

	(void) fflush (stdout);
	(void) ioctl (0, TCFLSH, 0);
	s = line;
	while (read (0, &c, 1) == 1) {
		if (linelen > 1) { *s++ = c; --linelen; }
		if (c == '\n') break;
	}
	*s = '\0';
	return (s - line);
}

vopendir (name)
	char		*name;
{
	DIR		*dir;
	int		f;
	char		who [Strsize], host [Strsize], fname [Strsize];
	char		buff [Strsize], *argv [32];
	Iob		*p;

	parsename (name, who, host, fname);
	if (*host != '\0') {
		argv [0] = xrsh ();
		argv [1] = host;
		argv [2] = "-l";
		argv [3] = who;
		sprintf (buff, "/bin/ls %s", fname);
		argv [4] = buff;
		if ((f = runpipe (5, argv, 0)) < 0) return (-1);
		p = iob + f;
		p->flags = Buffered | Pipe;
		p->bsize = 4096;
		p->buff = p->buffp = malloc (p->bsize);
		if (p->buff == NULL) {
			fprintf (stderr, "out of memory.\n"); exit (1);
		}
		p->pos = p->cnt = 0;
	} else {
		if ((dir = opendir (name)) == NULL) return (-1);
		iob [f = dir->dd_fd].dir = dir;
		iob [f].flags = Dir;
	}
	return (f);
}

char *
vreaddir (f, mset)
	int		f;
	Memset		*mset;
{
	struct dirent	*dp;
	register Iob	*p;
	char		buff [Strsize];
	register char	*s;

	p = iob + f;
	do {
		if (p->flags & Buffered) {
			s = buff;
			while (1) {
				if (p->cnt <= 0 && vfill (f) <= 0)
					break;
				--p->cnt, ++p->pos;
				if ((*s = *p->buffp++) == '\n') break;
				++s;
			}
			*s = '\0';
			s = buff;
		} else {
			if ((dp = readdir (iob [f].dir)) == NULL) return (NULL);
			s = dp->d_name;
		}
	} while (strcmp (s, ".") == 0 || strcmp (s, "..") == 0);
	if (*s == '\0') return (NULL);
	else return (idb_stash (s, mset));
}

void
vclosedir (f)
	int		f;
{
	if (iob [f].flags & Pipe) vclose (f);
	else {
		if (iob [f].buff != NULL) {
			free (iob [f].buff);
			iob [f].buff = NULL;
		}
		closedir (iob [f].dir);
	}
}

struct {
	char		*name;
	int		type;
} typecache [128];
int ncache = 0;

typeof (fname)
	char		*fname;
{
	struct stat	st;
	char		host [Strsize], file [Strsize], who [Strsize];
	char		buff [Strsize], *argv [32];
	int		i, t, argc;
	Memset		*mset;

	for (i = 0; i < ncache; ++i) {
		if (strcmp (typecache [i].name, fname) == 0) {
			return (typecache [i].type);
		}
	}
	parsename (fname, who, host, file);
	if (filematch (file, "/dev/tape") ||
	    filematch (file, "/dev/nrtape") ||
	    filematch (file, "/dev/mt*") ||
	    filematch (file, "/dev/rmt*") ||
	    filematch (file, "/dev/SA/*") ||
	    filematch (file, "/dev/rSA/*")) {
		t = Is_tape;
	} else if (*host != '\0') {
		argv [0] = xrsh ();
		argv [1] = host;
		argv [2] = "-l";
		argv [3] = who;
		argv [4] = "/bin/ls";
		argv [5] = "-ld";
		argv [6] = file;
		if (vline (7, argv, buff) < 0) {
			fprintf (stderr, "%s: can't access remotely\n",
				fname);
			t = -1;
		} else {
			argc = words (buff, argv, mset = idb_newset());
			if (argc < 5 ||
			    argv [0][1] != 'r' && argv [0][1] != '-') {
				fprintf (stderr, "%s", buff);
				t = -1;
			} else {
				switch (argv [0][0]) {
				case 'd': t = Is_dir; break;
				case '-': t = Is_file; break;
				default: t = -1;
				}
			}
			idb_dispose (mset);
		}
	} else {
		if (stat (file, &st) < 0) {
			t = -1;
		} else {
			switch (st.st_mode & S_IFMT) {
			case S_IFDIR:
				t = Is_dir; break;
			case S_IFREG:
				t = Is_file; break;
			default:
				t = -1; break;
			}
		}
	}
	if (t != -1 && ncache + 1 < sizeof (typecache) / sizeof (*typecache)) {
		typecache [ncache].name = idb_stash (fname, NULL);
		typecache [ncache].type = t;
		++ncache;
	}
	return (t);
}
