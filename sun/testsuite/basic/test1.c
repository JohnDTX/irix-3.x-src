/*
 * Test file and directory creation and removal
 * Builds a tree on 
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
int Sflag = 0;		/* don't print non-error messages */
int Fflag = 0;		/* test function only;  set count to 1, negate -t */

usage()
{
	fprintf(stdout, "usage: %s [-htf] [levels files dirs fname dname]\n",
	    Myname);
}

main(argc, argv)
	int argc;
	char *argv[];
{
	int files = DFILS;	/* number of files in each dir */
	int totfiles = 0;
	int dirs = DDIRS;	/* directories in each dir */
	int totdirs = 0;
	int levels = DLEVS;	/* levels deep */
	char *fname = FNAME;
	char *dname = DNAME;
	struct timeval time;
	char command[MAXPATHLEN];
	struct stat statb;
	char *opts;

	setbuf(stdout, NULL);
	Myname = *argv++;
	argc--;
	while (argc && **argv == '-') {
		for (opts = &argv[0][1]; *opts; opts++) {
			switch (*opts) {
				case 'h':	/* help */
					usage();
					exit(1);

				case 's':	/* silent */
					Sflag++;
					break;
				
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
		levels = getparm(*argv, 1, "levels");
		argv++;
		argc--;
	}
	if (argc) {
		files = getparm(*argv, 0, "files");
		argv++;
		argc--;
	}
	if (argc) {
		dirs = getparm(*argv, 0, "dirs");
		if (dirs == 0 && levels != 1) {
			error("Illegal dirs parameter, must be at least 1");
			exit(1);
		}
		argv++;
		argc--;
	}
	if (argc) {
		fname = *argv;
		argc--;
		argv++;
	}
	if (argc) {
		dname = *argv;
		argc--;
		argv++;
	}
	if (argc != 0) {
		error("too many parameters");
		usage();
		exit(1);
	}

	if (Fflag) {
		Tflag = 0;
		levels = 2;
		files = 2;
		dirs = 2;
	}
	
	testdir(NULL);

	if (!Sflag) {
		fprintf(stdout, "%s: File and directory creation test\n",
		    Myname);
	}

	if (Tflag && !Sflag) {
		starttime();
	}
	dirtree(levels, files, dirs, fname, dname, &totfiles, &totdirs);
	if (Tflag && !Sflag) {
		endtime(&time);
	}
	if (!Sflag) {
		fprintf(stdout,
		    "\tcreated %d files %d directories %d levels deep",
		    totfiles, totdirs, levels);
	}
	if (Tflag && !Sflag) {
		fprintf(stdout, " in %d.%-2d seconds",
		    time.tv_sec, time.tv_usec / 10000);
	}
	if (!Sflag) {
		fprintf(stdout, "\n");
	}
	complete();
}
