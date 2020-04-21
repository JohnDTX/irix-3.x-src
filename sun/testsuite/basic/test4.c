/*
 * Test setattr, getattr and lookup
 */

#include <sys/param.h>
#ifndef major
#include <sys/types.h>
#endif
#include <sys/time.h>
#include <sys/stat.h>
#include <stdio.h>
#include "tests.h"

int Tflag = 0;		/* print timing */
int Hflag = 0;		/* print help message */
int Fflag = 0;		/* test function only;  set count to 1, negate -t */

usage()
{
	fprintf(stdout, "usage: %s [-htf] [files count]\n", Myname);
}

main(argc, argv)
	int argc;
	char *argv[];
{
	int files = 10;		/* number of files in each dir */
	int fi;
	int count = 50;	/* times to do each file */
	int ct;
	int totfiles = 0;
	int totdirs = 0;
	struct timeval time;
	char str[MAXPATHLEN];
	struct stat statb;
	char *opts;

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

	testdir(NULL);

	dirtree(1, files, 0, FNAME, DNAME, &totfiles, &totdirs);

	fprintf(stdout, "%s: setattr, getattr, and lookup\n", Myname);

	if (Tflag) {
		starttime();
	}

	for (ct = 0; ct < count; ct++) {
		for (fi = 0; fi < files; fi++) {
			sprintf(str, "%s.%d", FNAME, fi);
			if (chmod(str, 0) < 0) {
				error("can't chmod 0 %s", str);
				exit(0);
			}
			if (stat(str, &statb) < 0) {
				error("can't stat %s", str);
				exit(1);
			}
			if ((statb.st_mode & 0777) != 0) {
				error("%s has mode %o after chmod 0",
				    str, (statb.st_mode & 0777));
				exit(1);
			}
			if (chmod(str, 0666) < 0) {
				error("can't chmod 0666 %s", str);
				exit(0);
			}
			if (stat(str, &statb) < 0) {
				error("can't stat %s", str);
				exit(1);
			}
			if ((statb.st_mode & 0777) != 0666) {
				error("%s has mode %o after chmod 0666",
				    str, (statb.st_mode & 0777));
				exit(1);
			}
		}
	}

	if (Tflag) {
		endtime(&time);
	}
	fprintf(stdout, "\t%d chmods and stats on %d files",
		files * count * 2, files);
	if (Tflag) {
		fprintf(stdout, " in %d.%-2d seconds",
		    time.tv_sec, time.tv_usec / 10000);
	}
	fprintf(stdout, "\n");
	complete();
}
