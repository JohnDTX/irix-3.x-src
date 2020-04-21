#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include "idb.h"
#include "inst.h"
#include "hist.h"

#define Maxsel		512

#define List		1
#define Long		2
#define Remove		3
#define Config		4
#define Display		5

#define Include		1
#define Exclude		2

#define visable(p)	(p + hidepath)

char		*devnm ();

int		hidepath		= 0;
int		op;
char		*sel [Maxsel];
int		nsel;
Hist		*h;
char		vname [Strsize];
int		mselect			= 0;
int		cselect			= 0;
int		verbose			= 0;
int		configat;
int		dump			= 0;
int		hwarning		= 0;
Spec		*spec;
long		instdate;

main (argc, argv)
	int		argc;
	char		*argv [];
{
	char		hpath [Strsize], *p;
	int		c;
	extern int	optind;
	extern char	*optarg;

	spec = (Spec *) idb_getmem (sizeof (Spec), NULL);
	spec->prod = spec->prodend = NULL;
	if ((p = devnm ("/")) != NULL) {
		p = p + strlen (p) - 1;
		if (*p == '0' || *p == 'a') {
			strcpy (rbase, "/");
		}
	}
	while ((c = getopt (argc, argv, "mucsvr:f:D")) != EOF) {
		switch (c) {
		case 'm': mselect = Include; break;
		case 'u': mselect = Exclude; break;
		case 'c': cselect = Include; break;
		case 's': cselect = Exclude; break;
		case 'v': ++verbose; break;
		case 'r': strcpy (rbase, optarg); break;
		case 'f': strcpy (vname, optarg); break;
		case 'D': ++dump; ++hwarning; break;
		default: usage ();
		}
	}
	idb_setbase ();
	if (*vname == '\0') {
		strcpy (vname, idb_rpath (histfile));
	}
	squeeze (rbase);
	hidepath = strlen (rbase) + 1;
	op = Display;
	if (mselect || cselect) op = List;
	if (optind >= argc) --optind;
	else if (strcmp (argv [optind], "list") == 0) op = List;
	else if (strcmp (argv [optind], "long") == 0) op = Long;
	else if (strcmp (argv [optind], "remove") == 0) op = Remove;
	else if (strcmp (argv [optind], "config") == 0) op = Config;
	else if (strcmp (argv [optind], "display") == 0) op = Display;
	++optind;
	nsel = 0;
	if (optind < argc) {
		while (optind < argc) {
			if (nsel >= Maxsel) {
				fprintf (stderr, "Ignoring some selectors\n");
				break;
			}
			sel [nsel++] = argv [optind++];
		}
	}
	if ((h = histread (vname, op != Display, 0, NULL)) == NULL) {
		perror (vname);
		fprintf (stderr, "Can't read versions file.\n");
		exit (1);
	}
#ifdef DEBUGFUNCS
	if (dump) dumphist ("", h, 1, 1);
#endif
	if (op == Config || cselect) {
		for (configat = 0; configat < h->nattr; ++configat) {
			if (strcmp (h->attr [configat], "config") == 0) break;
		}
		if (configat >= h->nattr) {
			fprintf (stderr, "config files not marked in ");
			fprintf (stderr, "versions file.\n");
			exit (1);
		}
	}
	if (op == Display) {
		display ();
	} else if (op == Remove) {
		remove ();
		if (histwrite (h, vname) < 0) {
			perror (vname);
			fprintf (stderr, "couldn't rewrite versions file.\n");
			exit (1);
		}
	} else {
		process (op, h->root, "");
	}
	exit (0);
}

usage ()
{
	fprintf (stderr, "usage: versions [ -r ] [ operator [ selectors ] ]\n");
	fprintf (stderr, "\nwhere the operators are:\n\n");
	fprintf (stderr, "\tlist\n\tlong\n\tverify\n\tremove\n");
	fprintf (stderr, "\nand operands are product, image, and/or subsystem");
	fprintf (stderr, "specifications or patterns.\n");
	exit (1);
}

display ()
{
	Hist		*h;
	Prod		*pr;
	Image		*im;
	Subsys		*ss;

	if ((h = histread (vname, 0, 0, NULL)) == NULL) {
		perror (vname);
		fprintf (stderr, "Can't read versions file.\n");
		exit (1);
	}
	for (pr = h->spec->prod; pr < h->spec->prodend; ++pr) {
		showprod (pr);
	}
}

showprod (pr)
	Prod		*pr;
{
	Image		*im;

	if (pr->name == NULL || pr->name [0] == '\0') return;
	printf ("%-20s %-45s\n", pr->name, pr->id);
	for (im = pr->image; im < pr->imgend; ++im) {
		showimage (pr, im);
	}
}

showimage (pr, im)
	Prod		*pr;
	Image		*im;
{
	Subsys		*ss;

	if (im->name == NULL || im->name [0] == '\0') return;
	printf ("%-20s %-45s\n", cat (pr->name, im->name, NULL), im->id);
	printf ("%-20s version %ld\n", "", im->version);
	if (!verbose) return;
	for (ss = im->subsys; ss < im->subend; ++ss) {
		showsubsys (pr, im, ss);
	}
}

showsubsys (pr, im, ss)
	Prod		*pr;
	Image		*im;
	Subsys		*ss;
{
	if (ss->name == NULL || ss->name [0] == '\0') return;
	printf ("%-20s %-45s\n", cat (pr->name, im->name, ss->name), ss->id);
	printf ("%-20s installed %s\n", "", ctime (&ss->instdate));
}

remove ()
{
	Prod		*pr;
	Image		*im;
	Subsys		*ss;
	int		i;
	char		*p;

	fluffinit (h);
	for (pr = h->spec->prod; pr < h->spec->prodend; ++pr) {
		for (im = pr->image; im < pr->imgend; ++im) {
			for (ss = im->subsys; ss < im->subend; ++ss) {
				for (i = 0; i < nsel; ++i) {
					p = cat (pr->name, im->name, ss->name);
					if (filematch (p, sel [i])) {
						if (ss->flags & Main) {
							fprintf (stderr, "Can't remove %s.%s.%s\n", pr->name, im->name, ss->name);
						} else {
							fluffadd (pr, im, ss);
#ifdef notdef
							histdelattr (h, p);
#endif
						}
						break;
					}
				}
			}
		}
	}
	fluff (!verbose);
}

process (op, hnode, parent)
	int		op;
	H_node		*hnode;
	char		*parent;
{
	char		hpath [Strsize];
	H_node		*ch, *chend;
	int		i, t, mod;
	long		sum, size;
	struct stat	st;

	for (ch = hnode->child; ch < hnode->chend; ++ch) {
		if (ch->type == 0) continue;
		makepath (hpath, parent, ch->name);
		if (lstat (idb_rpath (hpath), &st) < 0) {
			perror (hpath);
			if (ch->type == S_IFDIR) process (op, ch, hpath);
			continue;
		}
		t = st.st_mode & S_IFMT;
		if (t == S_IFLNK && t != ch->type) {
			if (stat (idb_rpath (hpath), &st) < 0) {
				perror (hpath);
				continue;
			}
			if ((st.st_mode & S_IFMT) == ch->type) {
				t = st.st_mode & S_IFMT;
			}
		}
		if (t == S_IFDIR && ch->type != S_IFDIR) {
			fprintf (stderr, "%s: unexpected directory\n", hpath);
			if (ch->chend == ch->child) continue;
		}
		if (t != S_IFDIR && ch->type == S_IFDIR) {
			fprintf (stderr, "%s: expected directory\n", hpath);
			if (ch->chend != ch->child) continue;
		}
		if (selectnode (ch, &mod, op, hpath)) {
			if (t != ch->type) {
				printf ("%s: unexpected type %0o %0o\n",
					hpath, t, ch->type);
			} else switch (op) {
			case List:
				printf ("%s\n", hpath);
				break;
			case Long:
				printlong (ch, hpath, &st);
				break;
			case Config:
				doconfig (hpath, ch, mod);
				break;
			}
		}
		if (ch->type == S_IFDIR) {
			process (op, ch, hpath);
		}
	}
}

selectnode (hnode, mod, op, hpath)
	H_node		*hnode;
	int		*mod;
	int		op;
	char		*hpath;
{
	int		hit, i;
	char		*at;
	long		size, sum;

	hit = 0;
	for (at = hnode->attr; at < hnode->attr + hnode->nattr; ++at) {
		for (i = 0; i < nsel; ++i) {
			if (filematch (h->attr [*at], sel [i])) ++hit;
		}
		if (hit) break;
	}
	if (nsel > 0 && !hit) return (0);
	if (cselect) {
		if (hnode->type != S_IFREG) {
			if (cselect == Include) return (0);
		} else {
			for (i = 0; i < hnode->nattr; ++i) {
				if (hnode->attr [i] == configat) break;
			}
			if (cselect == Include && i >= hnode->nattr ||
			    cselect == Exclude && i < hnode->nattr) {
				return (0);
			}
		}
	}
	if ((mselect || op == Config) && hnode->type == S_IFREG) {
		if (chksum (idb_rpath (hpath), &size, &sum) < 0) {
			perror (hpath);
			return (0);
		}
		*mod = hnode->chksum != sum;
	}
	if (mselect) {
		if (hnode->type != S_IFREG) {
			if (mselect == Include) return (0);
		} else {
			if (*mod) {
				if (mselect == Exclude) return (0);
			} else {
				if (mselect == Include) return (0);
			}
		}
	}
	return (1);
}

doconfig (hpath, ch, mod)
	char		*hpath;
	H_node		*ch;
	int		mod;
{
	char		name [Strsize], *p;
	long		sum, size;
	int		i;

	if (ch->type != S_IFREG) return;
	for (i = 0; i < ch->nattr; ++i) {
		if (ch->attr [i] == configat) break;
	}
	if (i >= ch->nattr) return;
	printf ("%-16s %s\n", mod ? "modified:" : "unmodified:", hpath);
	strcpy (name, hpath); p = name + strlen (name);
	strcpy (p, ".N");
	if (access (idb_rpath (name), 4) >= 0) {
		if (chksum (idb_rpath (name), &size, &sum) < 0) {
			perror (name);
		} else {
			printf ("%-16s %s\n", ch->chksum == sum ?
				"unmodified:" : "modified:", name);
		}
	} 
	strcpy (p, ".O");
	if (access (idb_rpath (name), 4) >= 0) {
		printf ("%-16s %s\n", "old version:", name);
	}
	printf ("\n");
}

printlong (hnode, path, st)
	H_node		*hnode;
	char		*path;
	struct stat	*st;
{
	int		t;

	switch (st->st_mode & S_IFMT) {
	case S_IFREG: t = 'f'; break;
	case S_IFBLK: t = 'b'; break;
	case S_IFCHR: t = 'c'; break;
	case S_IFDIR: t = 'd'; break;
	case S_IFIFO: t = 'p'; break;
	case S_IFLNK: t = 'l'; break;
	default: t = '?';
	}
	printf ("%c %04o %-8s %-8s %s\n", t, st->st_mode & 07777,
		idb_uname (st->st_uid), idb_gname (st->st_gid), path);
}

makepath (buff, base, name)
	char		*buff;
	char		*base;
	char		*name;
{
	int		len;
	char		ibuff [Strsize];

	strcpy (buff, base);
	strcat (buff, "/");
	strcat (buff, name);
	squeeze (buff);
}
