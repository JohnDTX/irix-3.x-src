#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <dirent.h>
#include <ctype.h>
#include <errno.h>
#include <sys/sysmacros.h>
#include <signal.h>
#include <setjmp.h>
#include "inst.h"
#include "idb.h"
#include "hist.h"

#if R2300 || IP4 || BSD
typedef void		(*sigfunc) ();
#else
typedef int		(*sigfunc) ();
#endif

/* interactive subsystem selection answers */

#define No	0
#define Yes	1
#define Pick	2

/* fudge values added into disk space requirements as insurance, in blocks */

#define FudgeRoot	100
#define FudgeUsr	200

char		*vreaddir ();
char		*getenv ();
char		*getstr ();
char		*devnm ();
int		caught ();
extern char	*sbrk ();
extern long	time ();

char		source [Strsize];	/* current source name */
char		sdir [Strsize];		/* the directory thereof */
int		stype;			/* file type */
int		fd = -1;		/* open image file descriptor */
int		updating;		/* update on existing system */
int		interface = Prompt;	/* interface type */
extern int	optind;			/* getopt index */
extern char	*optarg;		/* getopt argument */
int		verbose = 1;		/* show file names during install */
int		echo;			/* echo commands as read */
sigfunc		sigint;			/* inherited interrupt state */
int		quit;			/* interrupted */
Hist		*hist;			/* history */
int		force;			/* ignore history version check */
Spec		*spec;			/* known spec of products */
Spec		rspec;			/* real spec */
Memset		*bset;			/* spec memory space */
Memset		*hset;			/* history memory space */
Memset		*rset;			/* memset for idb records */
Memset		*tset;			/* temporary set for miscellaneous */
char		rootdev [Strsize];	/* root device pathname */
char		usrdev [Strsize];	/* usr device pathname */
int		imagef;			/* globally open image file */
Prod		*curprod;		/* current product */
Image		*curimage;		/* current image */
Subsys		*curss;			/* current subsystem */
int		toc;			/* table of contents */
int		ext;			/* extract individual files */
long		instdate;		/* official installation date */
char		**mach;			/* machine name(s) of interest */
int		nmach = 0;		/* number of names */
int		nofs = 0;		/* don't mess with file systems */
int		hwarning = 0;		/* warn of history corruption */
int		dumphistory = 0;	/* dump history after histread */
int		confignoise = 0;	/* show config swaps during install */
int		ds = 0;			/* dump spec prod debug */

main (argc, argv)
	int		argc;
	char		*argv [];
{
	int		c, pid;
	char		mbuff [128];
	FILE		*f;

	spec = &rspec;
	instdate = time (NULL);
	if ((f = fopen ("/tmp/.inst", "r")) != NULL &&
	    fscanf (f, "%d", &pid) == 1 && kill (pid, 0) == 0) {
		fprintf (stderr, "The installation tool is already ");
		fprintf (stderr, "running.  exit the current shell ");
		fprintf (stderr, "return to it.\n");
		exit (1);
	}
	if (f != NULL) fclose (f);
	if ((f = fopen ("/tmp/.inst", "w")) != NULL) {
		fprintf (f, "%d\n", getpid ());
		fclose (f);
	}
	while ((c = getopt (argc, argv, "r:s:i:cvR:U:f:txm:eD:")) != EOF) {
		switch (c) {
		case 'r': strcpy (rbase, optarg); break;
		case 's': strcpy (sbase, optarg); break;
		case 'i': strcpy (idb, optarg); break;
		case 'c': interface = Command; break;
		case 'v': ++verbose; break;
		case 'e': ++echo; break;
		case 'R': strcpy (rootdev, optarg); break;
		case 'U': strcpy (usrdev, optarg); break;
		case 'f': strcpy (source, optarg); break;
		case 't': interface = Special; ++toc; break;
		case 'x': interface = Special; ++ext; break;
		case 'D': /* debug options */
			while (*optarg) {
				switch (*optarg++) {
				case 'f': ++nofs; break;
				case 'h': ++hwarning; break;
				case 'H': ++dumphistory; break;
				case 's': ++ds; break;
				}
			}
			break;
		case 'm':
			mach = (char **) idb_getmore (mach,
				++nmach * sizeof (char *), NULL);
			mach [nmach - 1] = idb_stash (optarg, NULL);
			break;
		}
	}
	if (interface != Special) {
		sigint = signal (SIGINT, caught);
	}
	if (nmach == 0) {
		mach = (char **) idb_getmore (mach,
			++nmach * sizeof (char *), NULL);
		if ((mach [nmach - 1] = machname ()) == NULL) {
			fprintf (stderr, "Can't determine machine type.\n");
			exit (1);
		}
	}
	if (*rbase == '\0') {
		if (ext) {
			strcpy (rbase, ".");
		} else if ((c == *devnm ("/")) == '0' || c == 'a') {
			strcpy (rbase, "/");
		}
	}
	idb_setbase ();
	idb_passwd ("/etc/passwd", "/etc/group");
	bset = idb_newset ();
	hset = idb_newset ();
	rset = idb_newset ();
	tset = idb_newset ();
	switch (interface) {
	case Command: command (); break;
	case Prompt: getdevices (); prompt (); break;
	case Special: special (argc - optind, argv + optind); break;
	}
	exit (0);
}

/* special command-line functions */

special (argc, argv)
	int		argc;
	char		*argv [];
{
	if (getspec (source, source, sdir, &stype, spec, bset) < 0) {
		return;
	}
	sel ("*.*.*", Select);
	if (ext) {
		if (argc == 0) {
			argv [0] = "*";
			argc = 1;
		}
		extract (argc, argv);
	} else if (toc) {
		show (argc, argv);
		if (verbose) listimages (argc, argv);
	}
}

/* prompt mode reader/interpreter */

prompt ()
{
	char		whence [1024], answer [1024], def [32];

	help ("_startup");
#ifdef SVR3
	setvbuf (stdin, NULL, _IOLBF, BUFSIZ);
#endif
	strcpy (whence, DefSource);	/* for now */
	if (*source) strcpy (whence, source);
	while (1) {
		ask ("Is this an update or a build ? [%s] ", "u", answer,
			"_type");
		if (*answer == 'u' || *answer == 'U') { updating = 1; break; }
		if (*answer == 'b' || *answer == 'B') { updating = 0; break; }
		printf ("answer with 'u' or 'b'\n");
	}
	printf (updating ? "Update.\n" : "Build.\n");
	if (umountboth () < 0) {
		fprintf (stderr, "can't unmount file systems.\n");
		exit (1);
	}
	makeway (idb_rpath ("."));
	if (!updating &&
	    yesno ("Make new file systems? [%s] ", "y", "_mkfs")) {
		if (mkfsboth () < 0) {
			fprintf (stderr, "Can't make new file systems.\n");
			exit (1);
		}
	}
	if (mountboth () < 0) {
		fprintf (stderr, "Can't mount file systems.\n");
		exit (1);
	}
	if ((hist = histread (idb_rpath (histfile), 1, 0, hset)) == NULL) {
		hist = histnew (hset);
	}
	do {
		ask ("Distribution source [%s] ", whence, answer, "_source");
		if (getspec (answer, source, sdir, &stype, spec, bset) < 0) {
			strcpy (def, "y");
			continue;
		}
		sel ("*.*.*", Default);
		strcpy (whence, answer);
		help ("_pselect");
		do {
			printf ("\nAnswer \"yes\", \"no\", or \"pick\":\n");
			pselect ();
		} while (!yesno ("Proceed with installation? [%s] ", "y",
			"_proceed"));
		checkprereq ();
		install ();
		if (typeof (whence) == Is_tape) {
			printf ("\nChange tapes now, if appropriate.\n");
			strcpy (def, "y");
		} else {
			strcpy (def, "n");
		}
	} while (yesno ("More to install? [%s] ", def, "_more"));
}

yesno (format, def, helptag)
	char		*format;
	char		*def;
	char		*helptag;
{
	char		answer [Strsize], *p;

	while (1) {
		ask (format, def, answer, helptag);
		for (p = answer; *p; ++p) {
			if (isupper (*p)) *p = tolower (*p);
		}
		if (strcmp (answer, "y") == 0 || strcmp (answer, "yes") == 0)
			return (1);
		if (strcmp (answer, "n") == 0 || strcmp (answer, "no") == 0)
			return (0);
		fprintf (stderr, "Answer yes or no.\n");
	}
}

ask (format, def, answer, helptag)
	char		*format;
	char		*def;
	char		*answer;
	char		*helptag;
{
	char		line [Strsize], *v;
	int		eof;

	eof = 0;
	while (1) {
		printf (format, def);
		if (vgets (line, sizeof (line)) == 0) {
			putchar ('\n');
			if (++eof > 3) exit (1);
			continue;
		}
		if (echo) printf ("%s", line);
		v = line + strlen (line) - 1;
		while (v >= line && isspace (*v)) *v-- = '\0';
		for (v = line; isspace (*v); ++v) ;
		if (strlen (v) == 0) {
			strcpy (answer, def);
			return;
		} else {
			if (strcmp (v, "quit") == 0) exit (1);
			if (strcmp (v, "terse") == 0) verbose = 0;
			else if (strcmp (v, "verbose") == 0) verbose = 1;
			else if (strcmp (v, "help") == 0) help (helptag);
			else if (strcmp (v, "sh") == 0) {
				shell (v);
			} else {
				strcpy (answer, v);
				return;
			}
		}
	}
}

/* command mode reader/interpretter */

command ()
{
	int		argc, i;
	char		*argv [Maxargs], *com, line [Strsize];
	Memset		*cset;

	cset = idb_newset ();
	hist = histnew (hset);
	while (printf ("> "), fgets (line, sizeof (line), stdin) != NULL) {
		envsub (line);
		if (echo) printf ("%s", line);
		idb_freeset (cset);
		argc = words (line, argv, cset);
		if (argc == 0) continue;
		com = argv [0];
		if (strcmp (com, "devices") == 0) {
			if (argc < 3) getdevices ();
			else {
				strcpy (rootdev, argv [1]);
				strcpy (usrdev, argv [2]);
			}
			printf ("root %s\nusr %s\n", rootdev, usrdev);
		} else if (strcmp (com, "update") == 0) {
			++updating;
		} else if (strcmp (com, "build") == 0) {
			updating = 0;
		} else if (strcmp (com, "mount") == 0) {
			if (mountboth () < 0) {
				fprintf (stderr, "couldn't mount.\n");
			}
		} else if (strcmp (com, "umount") == 0) {
			if (umountboth () < 0) {
				fprintf (stderr, "couldn't umount.\n");
			}
		} else if (strcmp (com, "mkfs") == 0) {
			if (mkfsboth () < 0) {
				fprintf (stderr, "couldn't mkfs\n");
			}
		} else if (strcmp (com, "from") == 0) {
			if (argc >= 1) getspec (argv [1], source, sdir, &stype, spec, bset);
			else printf ("from %s\n", source);
			sel ("*.*.*", Default);
		} else if (strcmp (com, "print") == 0) {
			show (argc - 1, argv + 1);
		} else if (strcmp (com, "select") == 0) {
			for (i = 1; i < argc; ++i) sel (argv [i], Select);
		} else if (strcmp (com, "reject") == 0) {
			for (i = 1; i < argc; ++i) sel (argv [i], Reject);
		} else if (strcmp (com, "default") == 0) {
			for (i = 1; i < argc; ++i) sel (argv [i], Default);
		} else if (strcmp (com, "pselect") == 0) {
			pselect ();
		} else if (strcmp (com, "histread") == 0) {
			hist = histread (idb_rpath (histfile), 1, 0, hset);
			if (hist == NULL) {
				perror (idb_rpath (histfile));
				hist = histnew (hset);
			}
		} else if (strcmp (com, "histnew") == 0) {
			hist = histnew (hset);
		} else if (strcmp (com, "install") == 0) {
			install ();
		} else if (strcmp (com, "prereq") == 0) {
			checkprereq ();
		} else if (strcmp (com, "verbose") == 0) {
			verbose = 1;
		} else if (strcmp (com, "terse") == 0) {
			verbose = 0;
		} else if (strcmp (com, "quit") == 0) {
			break;
		} else if (strcmp (com, "sh") == 0) {
			shell (line);
		} else {
			fprintf (stderr, "%s?\n", com);
			continue;
		}
	}
	free (cset);
}

sel (pattern, op)
	char		*pattern;
	int		op;
{
	Prod		*pr, *prx;
	Image		*im, *imx;
	Subsys		*ss, *ssx;
	Mark		*rep;
	char		buff [Strsize], *p, *prpat, *impat, *sspat;
	int		ch;

	if (spec == NULL) return;
	strcpy (buff, pattern);
	prpat = buff;
	for (p = buff; *p && *p != '.'; ++p) ;
	if (*p == '\0') {
		impat = sspat = "*";
	} else {
		*p = '\0';
		for (impat = ++p; *p && *p != '.'; ++p) ;
		if (*p == '\0') {
			sspat = "*";
		} else {
			*p = '\0';
			sspat = p + 1;
		}
	}
	setscan (prpat, impat, sspat, spec);
	while (ch = nextscan (&pr, &im, &ss)) {
		ss->flags &= ~Select;
		switch (op) {
		case Select:
			ss->flags |= Select;
			break;
		case Default:
			for (rep = ss->rep; rep < ss->repend; ++rep) {
				if (locate (rep->pname, rep->iname, rep->sname,
				    hist->spec, &prx, &imx, &ssx, 0) &&
				    imx->version >= rep->lowvers &&
				    imx->version <= rep->highvers)
					break;
			}
			if (rep < ss->repend ||
			    imx == NULL && prx == NULL && ss->flags & Default)
				ss->flags |= Select;
			break;
		}
	}
	setscan ("*", "*", "*", spec);
	while (ch = nextscan (&pr, &im, &ss)) {
		if (ch & Chpr) pr->flags &= ~Select;
		if (ch & Chim) im->flags &= ~Select;
		im->flags |= ss->flags & Select;
		pr->flags |= im->flags & Select;
	}
}

/* prompt selection */

pselect ()
{
	Prod		*pr;
	Image		*im;
	Subsys		*ss;
	int		t, all, none, required;

	for (pr = spec->prod; pr < spec->prodend; ++pr) {
		required = 0;
		for (all = none = 1, im = pr->image; im < pr->imgend; ++im) {
			for (ss = im->subsys; ss < im->subend; ++ss) {
				if (ss->flags & Select) none = 0;
				else all = 0;
				if (ss->flags & Main) ++required;
			}
		}
		t = pask (all ? "y" : (none && !required) ? "n" : "p",
			"product", pr->name, pr->id, required);
		if (t == Yes) {
			sel (pr->name, Select);
		} else if (t == No) {
			sel (pr->name, Reject);
		} else {
			pselectimage (pr);
		}
		if (checkmain (pr)) --pr;
	}
}

pselectimage (pr)
	Prod		*pr;
{
	Image		*im;
	Subsys		*ss;
	int		t, all, none, required;
	char		*name;

	for (im = pr->image; im < pr->imgend; ++im) {
		name = cat (pr->name, im->name, NULL);
		required = 0;
		for (all = none = 1, ss = im->subsys; ss < im->subend; ++ss) {
			if (ss->flags & Select) none = 0;
			else all = 0;
			if (ss->flags & Main) ++required;
		}
		t = pask (all ? "y" : (none && !required) ? "n" : "p",
			"image", name, im->id, required);
		if (t == Yes) {
			sel (name, Select);
		} else if (t == No) {
			sel (name, Reject);
		} else if (t == Pick) {
			pselectsubsys (pr, im);
		}
	}
}

pselectsubsys (pr, im)
	Prod		*pr;
	Image		*im;
{
	Subsys		*ss;
	int		t;
	char		*name;
	long		v;

	for (ss = im->subsys; ss < im->subend; ++ss) {
		name = cat (pr->name, im->name, ss->name);
		t = pask (((ss->flags & Select) || (ss->flags & Main)) ?
			"y" : "n", "subsystem", name, ss->id, ss->flags & Main);
		if (t == Yes || t == Pick) {
			sel (name, Select);
		} else {
			sel (name, Reject);
		}
	}
}

pask (def, type, name, id, required)
	char		*def;
	char		*type;
	char		*name;
	char		*id;
	int		required;
{
	char		answer [Strsize], *cdr;
	int		goofs, car;

	car = *type;
	cdr = type + 1;
	if (islower (car)) car = toupper (car);
	goofs = 0;
	while (1) {
		printf ("\n%c%s %s: %s\n", car, cdr, name, id);
		if (required) {
			if (strcmp (type, "subsystem") == 0) {
				printf ("(Required for system functionality.)\n");
			} else {
				printf ("(This %s contains required subsystems.)\n", type);
			}
		}
		ask ("Install? [%s] ", def, answer, "_pick");
		if (answer [0] == 'y' || answer [0] == 'Y') return (Yes);
		if (answer [0] == 'n' || answer [0] == 'N') return (No);
		if (answer [0] == 'p' || answer [0] == 'P') return (Pick);
		switch (++goofs) {
		case 1:
			help ("_pselect");
			break;
		default:
			help ("_pick");
			goofs = 0;
			break;
		}
	}
}

checkmain (pr)
	Prod		*pr;
{
	int		any;
	Image		*im;
	Subsys		*ss;

	any = 0;
	for (im = pr->image; im < pr->imgend; ++im) {
		for (ss = im->subsys; ss < im->subend; ++ss) {
			if ((ss->flags & Main) && !(ss->flags & Select)) {
				if (!any) {
					printf ("\nThe following subsystems ");
					printf ("are required for system fun");
					printf ("ctionality:\n\n");
				}
				printf ("%s.%s.%s\n", pr->name, im->name,
					ss->name);
				++any;
			}
		}
	}
	if (!any) return (0);
	else {
		printf ("\n");
		return (yesno ("Do you want alter your selections for this product? [%s] ", "y", "_required"));
	}
}

/* show the current selections */

show (argc, argv)
	int		argc;
	char		*argv [];
{
	int		i, ch;
	Prod		*pr;
	Image		*im;
	Subsys		*ss;
	char		prpat [Strsize], impat [Strsize], sspat [Strsize];

	if (argc == 0) {
		argv [0] = "*.*.*";
		argc = 1;
	}
	for (i = 0; i < argc; ++i) {
		uncat (argv [i], prpat, impat, sspat);
		if (*prpat == '\0') strcpy (prpat, "*");
		if (*impat == '\0') strcpy (impat, "*");
		if (*sspat == '\0') strcpy (sspat, "*");
		setscan (prpat, impat, sspat, spec);
		while (ch = nextscan (&pr, &im, &ss)) {
			if (ch & Chpr) {
				printf ("%s %s\n",
				    pr->flags & Select ? "*" : " ",
				    cat (pr->name, NULL, NULL));
			}
			if (ch & Chim) {
				printf ("%s %s\n",
				    im->flags & Select ? "*" : " ",
				    cat (pr->name, im->name, NULL));
			}
			if (ch & Chss) {
				printf ("%s %s\n",
				    ss->flags & Select ? "*" : " ",
				    cat (pr->name, im->name, ss->name));
			}
		}
	}
}

/* open an image (globally) */

openimage (pr, im)
	Prod		*pr;
	Image		*im;
{
	char		name [Strsize];

	if (curimage != NULL) closeimage ();
	switch (stype) {
	case Is_file:
	case Is_dir:
		sprintf (name, "%s/%s.%s", sdir, pr->name, im->name);
		break;
	case Is_tape:
		strcpy (name, source);
		break;
	default:
		fprintf (stderr, "internal error: invalid source type\n");
		exit (1);
	}
	if (im < pr->image || im >= pr->imgend) {
		fprintf (stderr, "assertion failed: openimage reference\n");
		exit (1);
	}
	if (vfilnum (name, ImageFileno + im - pr->image) < 0) {
		fprintf (stderr, "%s: can't position tape\n");
		return (-1);
	}
	if ((imagef = vopen (name, 0)) < 0) {
		fprintf (stderr, "%s: can't open image\n", name);
		return (-1);
	}
	curprod = pr;
	curimage = im;
	return (0);
}

closeimage ()
{
	if (curimage == NULL) return;
	vclose (imagef);
	curimage = NULL;
}

/* extract idb and open as a tempory file */

FILE *
getidb ()
{
	char		name [Namesize], *buff;
	int		f, err, len, nr;
	FILE		*fin;

	sprintf (name, "/tmp/idb%d", getpid ());
	if ((f = creat (name, 0644)) < 0) {
		perror (name); return (NULL);
	}
	buff = idb_getmem (len = curimage->padsize, tset);
	err = 0;
	while ((nr = vread (imagef, buff, len)) == len) {
		if (write (f, buff, nr) != nr) {
			perror (name); ++err; break;
		}
		if (buff [nr - 1] == '\0') break; /* logical eof */
	}
	if (nr < 0) {
		perror (curimage->name); ++err;
	}
	close (f);
	if (err == 0 && (fin = fopen (name, "r")) == NULL) {
		perror (name); ++err;
	}
	unlink (name);
	idb_freeset (tset);
	return (err ? NULL : fin);
}

checkprereq ()
{
#ifdef notdef
	Prod		*pr;
	Image		*im;
	Subsys		*ss;
	Prereq		*pq;
	Pqnode		*n1, *n2;

	for (pr = spec->prod; pr < spec->prodend; ++pr) {
		if ((pr->flags & Select) == 0) continue;
		for (im = pr->image; im < pr->imgend; ++im) {
			if ((im->flags & Select) == 0) continue;
			for (ss = im->subsys; ss < im->subend; ++ss) {
				if ((ss->flags & Select) == 0) continue;
				n1 = makepqnode (pr->name, im->name, ss->name,
					?set);
				for (pq = ss->prereq; pq < ss->preend; ++pq) {
					n2 = makepqnode (pq->pname, pq->iname,
						pq->sname, ?set);
					tsortadd (n2, n1, ?set);
				}
			}
		}
	}
	/* check ... */
#endif
}

install ()
{
	Prod		*pr;
	Image		*im;

	for (pr = spec->prod; pr < spec->prodend; ++pr) {
		if ((pr->flags & Select) == 0) continue;
		for (im = pr->image; im < pr->imgend; ++im) {
			if ((im->flags & Select) == 0) continue;
			if (installimage (pr, im) < 0)
				return (-1);
		}
	}
}

desireable (rec)
	Rec		*rec;
{
	int		i, j;
	Attr		*m;

	for (curss = curimage->subsys; curss < curimage->subend; ++curss) {
		if ((curss->flags & Select) &&
		    idb_getattr (cat (curprod->name, curimage->name,
		    curss->name), rec) != NULL) {
			if ((m = idb_getattr ("mach", rec)) == NULL ||
			    m->argc <= 0)
				return (1);
			for (i = 0; i < nmach; ++i) {
				for (j = 0; j < m->argc; ++j) {
					if (strcmp (mach [i], m->argv [j]) == 0)
						return (1);
				}
			}
			break;
		}
	}
	return (0);
}

installimage (pr, im)
	Prod		*pr;
	Image		*im;
{
	FILE		*idb;
	int		size, e;
	Rec		*rec;
	Subsys		*ss;
	char		buff [Strsize];

	if (openimage (pr, im) < 0) return (-1);
	if ((idb = getidb ()) == NULL) { closeimage (); return (-1); }
	printf ("\ninstalling %s.%s\n\n", pr->name, im->name);
	if (lockimage (pr, im) < 0) {
		fclose (idb);
		closeimage ();
		return (-1);
	}
	if (updating) {
#ifdef trouble
		fluffinit (hist);
		for (ss = im->subsys; ss < im->subend; ++ss) {
			if (ss->flags & Select) {
				fluffadd (pr, im, ss);
			}
		}
		fluff (1);
#endif
		if (checkspace (idb) < 0) return (-1);
	}
	for (ss = im->subsys; ss < im->subend; ++ss) {
		if (ss->flags & Select) {
			histaddss (hist, pr, im, ss, hset);
			histaddattr (hist, cat (pr->name, im->name, ss->name),
				hset);
		}
	}
	histaddattr (hist, "config", hset);
	e = 0;
	while ((rec = idb_read (idb, rset)) != NULL) {
		if (quit) {
			quit = 0;
			fprintf (stderr, "\nInterrupted.\n");
			if (!errorcont ()) break;
		}
		size = idb_intat (rec, "size");
		if (desireable (rec)) {
			e = installfile (rec, size);
		} else {
			e = skipfile (rec, size);
		}
		if (e < 0 && !errorcont ()) break;
	}
	if (rec != NULL) {
		fprintf (stderr, "installation incomplete.\n");
		e = -1;
	} else {
		unlockimage (im);
		if (verbose && e == 0)
			printf ("\n%s.%s installed.\n\n", pr->name, im->name);
	}
	if (histwrite (hist, idb_rpath (histfile)) < 0) {
		fprintf (stderr, "couldn't write versions file %s\n",
			idb_rpath (histfile));
	}
	fclose (idb);
	closeimage ();
	return (e);
}

extract (argc, argv)
	int		argc;
	char		*argv [];
{
	Image		*im;
	FILE		*idb;
	Rec		*rec;
	int		size, i;

	if (argc == 0) {
		return (0);
	}
	for (curprod = spec->prod; curprod < spec->prodend; ++curprod) {
		for (im = curprod->image; im < curprod->imgend; ++im) {
			if (openimage (curprod, im) < 0) {
				return (-1);
			}
			if ((idb = getidb ()) == NULL) {
				closeimage ();
				return (-1);
			}
			while ((rec = idb_read (idb, rset)) != NULL) {
				size = idb_intat (rec, "size");
				for (i = 0; i < argc; ++i) {
					if (filematch (rec->dstpath, argv [i]))
						break;
				}
				if (i >= argc) {
					if (skipfile (rec, size) < 0) {
						break;
					}
					continue;
				}
				if (installfile (rec, size) < 0) {
					break;
				}
			}
		}
		fclose (idb);
		closeimage ();
	}
	return (0);
}

/* listimages -- display the idb from each interesting image */

listimages (argc, argv)
	int		argc;
	char		*argv [];
{
	int		i, ch, size;
	FILE		*idb;
	Rec		*rec;
	Prod		*pr;
	Image		*im;
	char		prpat [Strsize], impat [Strsize], sspat [Strsize];

	if (argc == 0) {
		argv [0] = "*.*.*";
		argc = 1;
	} for (i = 0; i < argc; ++i) {
		uncat (argv [i], prpat, impat, sspat);
		if (*prpat == '\0') strcpy (prpat, "*");
		if (*impat == '\0') strcpy (impat, "*");
		if (*sspat == '\0') strcpy (sspat, "*");
		setscan (prpat, impat, NULL, spec);
		while (ch = nextscan (&pr, &im, NULL)) {
			if (openimage (pr, im) < 0) {
				fprintf (stderr, "%s.%s: can't open image\n",
					pr->name, im->name);
				continue;
			}
			if ((idb = getidb ()) == NULL) {
				fprintf (stderr, "%s.%s: bad image format\n",
					pr->name, im->name);
				closeimage ();
				continue;
			}
			printf ("\n%s.%s\t%s\n\n", pr->name, im->name, im->id);
			while ((rec = idb_read (idb, rset)) != NULL) {
				size = idb_intat (rec, "size");
				idb_write (stdout, rec);
				skipfile (rec, size);
			}
			fclose (idb);
			closeimage ();
		}
	}
}

checklock (pr, im)
	Prod		*pr;
	Image		*im;
{
	FILE		*f;
	char		line [Strsize], *argv [Maxargs], *p;
	Memset		*mset;
	int		i;

	if ((f = fopen (idb_rpath (lockfile), "r")) == NULL) return (0);
	if (fgets (line, sizeof (line), f) == NULL) {
		fclose (f);
		return (0);
	}
	fclose (f);
	if (words (line, argv, mset = idb_newset ()) < 3) {
		idb_dispose (mset); return (0);
	}
	if (strcmp (argv [1], pr->name) != 0 ||
	    strcmp (argv [2], im->name) != 0 ||
	    atol (argv [0]) != im->version) {
		for (p = line, i = 0; i < 3; ++i) {
			if (*p == '\0') { p = line; break; }
			while (*p != ' ') ++p;
			++p;
		}
		fprintf (stderr, "Warning:  A previous installation of %s.%s\n",
			argv [1], argv [2]);
		fprintf (stderr, "was not completed normally.  This software");
		fprintf (stderr, " image should be\nreinstalled correctly ");
		fprintf (stderr, "or removed with the \"versions\" command.");
		fprintf (stderr, "\n\n\t%s\n\n", p);
		idb_dispose (mset);
		return (errorcont () ? 0 : -1);
	} else {
		return (0);
	}
}

lockimage (pr, im)
	Prod		*pr;
	Image		*im;
{
	FILE		*f;

	makeway (idb_rpath (lockfile));
	if (checklock (pr, im) < 0) return (-1);
	if ((f = fopen (idb_rpath (lockfile), "w")) == NULL) {
		perror (idb_rpath (lockfile));
		fprintf (stderr, "Can't create lock file.\n");
		exit (1);
	}
	fprintf (f, "%ld %s %s '%s'\n", im->version, pr->name,
		im->name, im->id);
	fclose (f);
	return (0);
}

unlockimage (im)
	Image		*im;
{
	if (unlink (idb_rpath (lockfile)) < 0) {
		if (errno == ENOENT) {
			fprintf (stderr, "warning: no lock file for %s\n",
				im->name);
		} else {
			perror (idb_rpath (lockfile));
			fprintf (stderr, "warning: can't unlock %s\n",
				im->name);
		}
	}
}

checkspace (idb)
	FILE		*idb;
{
	extern long	toblocks ();
	long		size, rspace, uspace, ravail, uavail, rbsize, ubsize;
	long		required_usr, required_root, nodes_usr, nodes_root;
	long		bytes;
	int		trouble;
	Rec		*rec;
	struct statfs	fs;
	struct stat	st;

	rewind (idb);
	if (statfs (idb_rpath ("."), &fs, sizeof (fs), 0) < 0) {
		perror (idb_rpath ("."));
		fprintf (stderr, "Can't get root file system statistics; ");
		fprintf (stderr, "can't check disk space.\n");
		return (0);
	}
	ravail = fs.f_bfree * (rbsize = fs.f_bsize);
	if (statfs (idb_rpath ("usr"), &fs, sizeof (fs), 0) < 0) {
		perror (idb_rpath ("usr"));
		fprintf (stderr, "Can't get usr file system statistics; ");
		fprintf (stderr, "can't check disk space.\n");
		return (0);
	}
	uavail = fs.f_bfree * (ubsize = fs.f_bsize);
	required_usr = required_root = nodes_usr = nodes_root = 0;
	while ((rec = idb_read (idb, rset)) != NULL) {
		if (!desireable (rec)) continue;
		switch (rec->type) {
		case S_IFREG:
			bytes = idb_intat (rec, "size");
			if (bytes < 0) {
				size = 0;
			} else if (lstat (idb_rpath (rec->dstpath), &st) == 0) {
				size = toblocks (bytes - st.st_size);
			} else {
				size = toblocks (bytes);
			}
			break;
		case S_IFDIR:
			size = 2;			/* fudge */
			break;
		case S_IFLNK:
			size = 1;
			break;
		default:
			size = 0;
		}
		if (strncmp (rec->dstpath, "usr", 3) == 0) {
			required_usr += size;
			nodes_usr += 1;
		} else {
			required_root += size;
			nodes_root += 1;
		}
	}
	rewind (idb);
	rspace = (required_root + nodes_root / 4 + FudgeRoot) * rbsize;
	uspace = (required_usr + nodes_usr / 4 + FudgeUsr) * ubsize;
	trouble = 0;
	if (rspace > ravail) {
		fprintf (stderr, "\nroot file system: %ld kbytes free,",
			(ravail + 1023) / 1024);
		fprintf (stderr, " %ld kbytes needed for installation.",
			(rspace + 1023) / 1024);
		++trouble;
	}
	if (uspace > uavail) {
		fprintf (stderr, "\nusr file system: %ld kbytes free,",
			(uavail + 1023) / 1024);
		fprintf (stderr, " %ld kbytes needed for installation.",	
			(uspace + 1023) / 1024);
		++trouble;
	}
	if (trouble) {
		help ("_space");
		if (yesno ("Proceed with installation? [%s] ", "n", "_space")) {
			return (0);
		} else {
			return (-1);
		}
	} else {
		return (0);
	}
}

skipfile (rec, size)
	Rec		*rec;
	int		size;
{
	char		*syncname;

	if (rec->type != S_IFREG) return (0);
	if ((syncname = getstr (imagef, tset)) == NULL) {
		idb_freeset (tset);
		return (-1);
	}
	if (strcmp (syncname, rec->dstpath) != 0) {
		fprintf (stderr, "Out of sync in %s at data for %s\n",
		    curimage->name, rec->dstpath);
		idb_freeset (tset);
		return (-1);
	}
	idb_freeset (tset);
	if (vskip (imagef, size) < 0) {
		perror (curimage->name);
		fprintf (stderr, "Can't skip data for %s\n", rec->dstpath);
		return (-1);
	}
	return (0);
}

installfile (rec, size)
Rec		*rec;
int		size;
{
	int		node, maj, min, exists, differs, xtype;
	char		dest [Strsize], buff [Strsize];
	struct stat	st;
	Attr		*at, *cat, *cmd;
	H_node		*hn;
	long		cs_current, cs_inst, size_current;

	strcpy (dest, idb_rpath (rec->dstpath));
	if (verbose) printf ("%s\n", rec->dstpath);
	if ((cmd = idb_getattr ("preop", rec)) != NULL) {
		runop (cmd, rec, dest);
	}
	exists = lstat (dest, &st) >= 0;
	xtype = st.st_mode & S_IFMT;
	if (exists && xtype != rec->type) {
		if ((st.st_mode & S_IFMT) == S_IFLNK) {
			exists = stat (dest, &st) >= 0;
			xtype = st.st_mode & S_IFMT;
		}
	}
	if (updating && exists &&
	    (cat = idb_getattr ("config", rec)) != NULL) {
		if ((hn = histnode (hist, rec->dstpath)) == NULL) {
			hn = histadd (hist, rec, curprod, curimage, curss,
				hset);
			hn->chksum = 0;
		}
		differs = chksum (dest, &size_current, &cs_current) < 0 ||
			cs_current != hn->chksum;
		if (differs && size_current == size &&
		    cs_current == idb_intat (rec, "sum")) {
			differs = 0;
		}
		if (cat->argc <= 0 || strcmp (cat->argv [0], "update") == 0) {
			if (differs && rec->type == S_IFREG) {
				if (verbose && confignoise) 
					printf ("(saving old version as %s.O)\n", dest);
				copyconf (dest, ".O");
			}
		} else if (strcmp (cat->argv [0], "suggest") == 0) {
			if (differs && rec->type == S_IFREG) {
				if (verbose && confignoise)
					printf ("(installing new version as %s.N)\n", dest);
				strcat (dest, ".N");
				exists = lstat (dest, &st) >= 0;
			}
		} else if (strcmp (cat->argv [0], "noupdate") == 0) {
			if (verbose && confignoise)
				printf ("(old version retained)\n");
			return (skipfile (rec, size));
		}
	}
	node = 0;
	switch (rec->type) {
	case S_IFDIR:
		if (exists && xtype != rec->type) unlink (dest);
		if (!exists) {
			makeway (dest);
			sprintf (buff, "mkdir %s", dest);
			system (buff);
		}
		break;
	case S_IFBLK:
	case S_IFCHR:
		if ((maj = idb_intat (rec, "maj")) < 0 ||
		    (min = idb_intat (rec, "min")) < 0) {
			fprintf (stderr, "%s: bad major/minor spec\n",
			    rec->dstpath);
			return (-1);
		}
		node = makedev (maj, min);
	case S_IFIFO:
		if (exists) unlink (dest);
		if (mknod (dest, rec->type | rec->mode, node) < 0) {
			perror (dest);
			return (-1);
		}
		break;
	case S_IFLNK:
		if ((at = idb_getattr ("symval", rec)) == NULL ||
		    at->argc != 1) {
			fprintf (stderr, "%s: bad symval spec\n",
			    rec->dstpath);
			return (-1);
		}
		if (exists) unlink (dest);
		if (symlink (at->argv [0], dest) < 0) {
			perror (dest);
			return (-1);
		}
		break;
	case S_IFREG:
		if (exists) unlink (dest);
		copyfile (rec, dest, size);
		break;
	default:
		fprintf (stderr, "Unexpected file type for %s\n", rec->dstpath);
		return (-1);
	}
	if (lstat (dest, &st) < 0) {
		perror (dest); 
		return (-1);
	}
	if (!ext) histadd (hist, rec, curprod, curimage, curss, hset);
	if (makelinks (rec, dest) < 0) {
		fprintf (stderr, "couldn't create links to %s\n",
			rec->dstpath);
	}
	cmd = idb_getattr ("postop", rec);
	if ((st.st_mode & S_IFMT) == S_IFLNK) {
		if (cmd != NULL) runop (cmd, rec, dest);
		return (0);
	}
	if (chown (dest, idb_uid (rec->user), idb_gid (rec->group)) < 0 ||
	    chmod (dest, rec->type | rec->mode) < 0) {
		perror (dest);
		return (-1);
	}
	if (cmd != NULL) runop (cmd, rec, dest);
	return (0);
}

copyfile (rec, dest, size)
Rec		*rec;
char		*dest;
int		size;
{
	int		w, n, nr, nw;
	char		*p, copybuff [4096], *syncname;

	if ((syncname = getstr (imagef, tset)) == NULL) {
		idb_freeset (tset); return (-1);
	}
	if (strcmp (syncname, rec->dstpath) != 0) {
		fprintf (stderr, "Out of sync in %s at data for %s\n",
		    curimage->name, rec->dstpath);
		idb_freeset (tset); return (-1);
	}
	idb_freeset (tset);
	w = creat (dest, rec->mode);
	if (w < 0 && errno == ENOENT) {
		makeway (dest);
		w = creat (dest, rec->mode);
	}
	if (w < 0) {
		perror (dest); 
		exit (1);
	}
	while (size) {
		if ((n = sizeof (copybuff)) > size) n = size;
		if ((nr = vread (imagef, copybuff, n)) < 0) {
			fprintf (stderr, "Can't read data for %s\n", dest);
			close (w);
			exit (1);
		}
		if (nr == 0) break;
		size -= nr;
		p = copybuff;
		while (nr) {
			if ((nw = write (w, p, nr)) <= 0) {
				perror (dest); 
				close (w); 
				exit (1);
			}
			nr -= nw;
			p += nw;
		}
	}
	if (size) {
		fprintf (stderr, "unexpected eof on data for %s\n", dest);
		close (w);
		exit (1);
	}
	close (w);
	return (0);
}

copyconf (name, app)
	char		*name;
	char		*app;
{
	char		buff [Strsize];
	FILE		*fin, *fout;

	strcpy (buff, name);
	strcat (buff, app);
	if ((fin = fopen (name, "r")) == NULL) return;
	if ((fout = fopen (buff, "w")) == NULL) {
		fclose (fin);
		perror (buff); return;
	}
	while (fgets (buff, sizeof (buff), fin) != NULL) {
		fputs (buff, fout);
	}
	fclose (fin);
	fclose (fout);
}

ensuredir (name)
	char		*name;
{
	char		buff [1024];
	struct stat	st;
	
	if (lstat (name, &st) < 0) {
		sprintf (buff, "mkdir %s; chmod 755 %s; chown root %s ",
			name, name, name);
		system (buff);
	} else if ((st.st_mode & S_IFMT) != S_IFDIR) {
		fprintf (stderr, "%s: not a directory\n", name);
	}
}

makeway (name)
	char		*name;
{
	char		*p;
	int		c;

	for (p = name; *p; ++p) {
		if (p > name && *p == '/') {
			c = *p; 
			*p = '\0';
			ensuredir (name);
			*p = c;
		}
	}
}

makelinks (rec, dest)
Rec		*rec;
char		*dest;
{
	int		i, r;
	char		*to, *savedstpath;
	Attr		*at;

	if ((at = idb_getattr ("links", rec)) == NULL) return (0);
	savedstpath = rec->dstpath;
	for (i = 0; i < at->argc; ++i) {
		to = idb_rpath (at->argv [i]);
		unlink (to);
		if (verbose) printf ("... link %s\n", at->argv [i]);
		if ((r = link (dest, to)) < 0 && errno == ENOENT) {
			makeway (to);
			r = link (dest, to);
		}
		if (r < 0) perror (to);
		else if (!ext) {
			rec->dstpath = at->argv [i];
			histadd (hist, rec, curprod, curimage, curss, hset);
		}
	}
	rec->dstpath = savedstpath;
	return (0);
}

shell (line)
	char		*line;
{
	char		*argv [4];
	int		argc;

	while (*line != '\0' && isspace (*line)) ++line;
	while (*line != '\0' && !isspace (*line)) ++line;
	while (*line != '\0' && isspace (*line)) ++line;
	if ((argv [0] = getenv ("SHELL")) == NULL || *argv [0] == '\0')
		argv [0] = "/bin/csh";
	if (access (argv [0], 1) < 0) argv [0] = "/bin/sh";
	if (*line == '\0') {
		argv [1] = "-i";
		argc = 2;
	} else {
		argv [1] = "-c";
		argv [2] = line;
		argc = 3;
	}
	argv [argc] = NULL;
	run (argc, argv, Noisy);
}

caught (sig)
	int		sig;
{
	++quit;
	signal (sig, caught);
}

getdevices ()		/* find root and usr device names */
{
	char		*p;

	if (nofs) {
		strcpy (rootdev, "bogus");
		strcpy (usrdev, "bogus");
		return;
	}
	if ((p = devnm ("/")) == NULL) {
		fprintf (stderr, "Can't find `devnm /`\n");
		exit (1);
	}
	if (*rootdev == '\0' && derivedev (p, rootdev, Root) < 0 ||
	    *usrdev == '\0' && derivedev (p, usrdev, Usr) < 0) {
		fprintf (stderr, "Can't derive root and usr device names\n");
		exit (1);
	}
}

mountboth ()
{
	if (nofs) return (0);
	makeway (idb_rpath ("x"));
	if (mountfs (rootdev, idb_rpath ("")) < 0) return (-1);
	makeway (idb_rpath ("usr/x"));
	if (mountfs (usrdev, idb_rpath ("usr")) < 0) return (-1);
	return (0);
}

umountboth ()
{
	char		*p, buff [Strsize], realroot [Strsize];

	if (nofs) return (0);
	if ((p = devnm ("/")) == NULL) {
		fprintf (stderr, "Cannot find devnm of /\n");
		exit (1);
	}
	strcpy (realroot, p);
	if ((p = devnm (idb_rpath ("usr"))) != NULL &&
	    strcmp (p, realroot) != 0) {
		strcpy (buff, p);
		if ((p = devnm (idb_rpath ("."))) != NULL &&
		    strcmp (p, buff) != 0) {
			if (umountfs (usrdev) < 0) return (-1);
		}
	}
	if ((p = devnm (idb_rpath ("."))) != NULL &&
	    strcmp (p, realroot) != 0) {
		if (umountfs (rootdev) < 0) return (-1);
	}
	return (0);
}

mkfsboth ()
{
	if (nofs) return (0);
	umountboth ();
	return (mkfs (rootdev) < 0 || mkfs (usrdev) < 0 ? -1 : 0);
}

errorquit ()
{
	if (yesno ("Continue? [%s] ", "n", "_continue")) return;
	exit (1);
}

errorcont ()
{
	return (yesno ("Continue? [%s] ", "n", "_continue"));
}

showmem (s)
	char		*s;
{
	fprintf (stderr, "%s %ld\n", s, ((long) sbrk (0)) / 1024);
}

runop (cmdat, rec, dest)
	Attr		*cmdat;
	Rec		*rec;
	char		*dest;
{
	int		argc, i;
	char		*argv [64];
	Memset		*mset;

	mset = idb_newset ();
	for (i = 0; i < cmdat->argc; ++i) {
		argc = words (cmdat->argv [i], argv, mset);
		run (argc, argv, Noisy);
		idb_freeset (mset);
	}
	idb_dispose (mset);
}
