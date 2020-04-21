/*
 * Test readdir
 */

#include <sys/param.h>
#ifndef major
#include <sys/types.h>
#endif
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <stdio.h>
#include "tests.h"

int Tflag = 0;		/* print timing */
int Hflag = 0;		/* print help message */
int Fflag = 0;		/* test function only;  set count to 1, negate -t */

#define MAXFILES 512		/* maximum files allowed for this test */
#define BITMOD	 8		/* bits per u_char */
u_char bitmap[MAXFILES / BITMOD];
#define BIT(x)    (bitmap[(x) / BITMOD] &   (1 << ((x) % BITMOD)) )
#define SETBIT(x) (bitmap[(x) / BITMOD] |=  (1 << ((x) % BITMOD)) )
#define CLRBIT(x) (bitmap[(x) / BITMOD] &= ~(1 << ((x) % BITMOD)) )

usage()
{
	fprintf(stdout, "usage: %s [-htf] [files count]\n", Myname);
}

main(argc, argv)
	int argc;
	char *argv[];
{
	int files = 200;	/* number of files in each dir */
	int fi;
	int count = 200;	/* times to read dir */
	int ct;
	int entries = 0;
	int totfiles = 0;
	int totdirs = 0;
	DIR *dir;
	struct direct *dp;
	struct timeval time;
	char *p, str[MAXPATHLEN];
	struct stat statb;
	char *opts;
	int err, i, dot, dotdot;
	int nmoffset = strlen(FNAME) + 1;	/* "FNAME." */

	umask(0);
	setbuf(stdout, NULL);
	Myname = *argv++;
	argc--;
	while (argc && **argv == '-') {
		for (opts = &argv[0][1]; *opts; opts++) {
			switch (*opts) {
				case 'h':	/* help */
					usage();
					exit(1);

				case 't':	/* time */
					Tflag++;
					break;
				
				case 'f':	/* funtionality */
					Fflag++;
					break;
				
				default:
					error("unknown option '%c'", *opts);
					usage();
					exit(1);
			}
		}
		argc--;
		argv++;
	}

	if (argc) {
		files = getparm(*argv, 1, "files");
		argv++;
		argc--;
	}
	if (argc) {
		count = getparm(*argv, 1, "count");
		argv++;
		argc--;
	}
	if (argc) {
		usage();
		exit(1);
	}

	if (Fflag) {
		Tflag = 0;
		count = 1;
	}

	if (count > files) {
		error("count (%d) can't be greater than files (%d)",
			count, files);
		exit(1);
	}

	if (files > MAXFILES) {
		error("too many files requested (max is %d)", MAXFILES);
		exit(1);
	}

	testdir(NULL);

	dirtree(1, files, 0, FNAME, DNAME, &totfiles, &totdirs);

	fprintf(stdout, "%s: readdir\n", Myname);

	if (Tflag) {
		starttime();
	}

	if ((dir = opendir(".")) == NULL) {
		error("can't opendir %s", ".");
		exit(1);
	}

	for (ct = 0; ct < count; ct++) {
		rewinddir(dir);
		dot = 0;
		dotdot = 0;
		err = 0;
		for (i = 0; i < sizeof(bitmap); i++)
			bitmap[i] = 0;
		while ((dp = readdir(dir)) != NULL) {
			entries++;
			if (strcmp(".", dp->d_name) == 0) {
				if (dot) {
					/* already read dot */
					error("'.' dir entry read twice");
					exit(1);
				}
				dot++;
				continue;
			} else if (strcmp("..", dp->d_name) == 0) {
				if (dotdot) {
					/* already read dotdot */
					error("'..' dir entry read twice");
					exit(1);
				}
				dotdot++;
				continue;
			}

			/*
			 * at this point, should have entry of the form
			 *  FNAME.%d
			 */
			if (dp->d_namlen <= nmoffset) {
				error("unexpected dir entry '%s'",
					dp->d_name);
				exit(1);
			}
			/* get ptr to numeric part of name */
			p = dp->d_name + nmoffset;
			fi = atoi(p);
			if (fi < 0 || fi >= MAXFILES) {
				error("unexpected dir entry '%s'",
					dp->d_name);
				exit(1);
			}
			if (BIT(fi)) {
				error("duplicate '%s' dir entry read",
					dp->d_name);
				err++;
			} else
				SETBIT(fi);
		}	/* end readdir loop */
		if (!dot) {
			error("didn't read '.' dir entry, pass %d", ct);
			err++;
		}
		if (!dotdot) {
			error("didn't read '..' dir entry, pass %d", ct);
			err++;
		}
		for (fi = 0; fi < ct; fi++) {
			if (BIT(fi)) {
				sprintf(str, "%s.%d", FNAME, fi);
				error("unlinked '%s' dir entry read pass %d",
					str, ct);
				err++;
			}
		}
		for (fi = ct; fi < files; fi++) {
			if (!BIT(fi)) {
				sprintf(str, "%s.%d", FNAME, fi);
				error("\
didn't read expected '%s' dir entry, pass %d", str, ct);
				err++;
			}
		}
		if (err) {
			error("Test failed with %d errors", err);
			exit(1);
		}
		sprintf(str, "%s.%d", FNAME, ct);
		if (unlink(str) < 0) {
			error("can't unlink %s", str);
			exit(1);
		}
	}

	closedir(dir);

	if (Tflag) {
		endtime(&time);
	}
	fprintf(stdout, "\t%d entries read, %d files",
		entries, files);
	if (Tflag) {
		fprintf(stdout, " in %d.%-2d seconds",
		    time.tv_sec, time.tv_usec / 10000);
	}
	fprintf(stdout, "\n");
	complete();
}
