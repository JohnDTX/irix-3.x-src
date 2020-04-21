/*
 * Test rename, link
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

#define NNAME "newfile"	/* new filename for rename and link */

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
	int count = 10;	/* times to do each file */
	int ct;
	int totfiles = 0;
	int totdirs = 0;
	struct timeval time;
	char str[MAXPATHLEN];
	char new[MAXPATHLEN];
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

	fprintf(stdout, "%s: link and rename\n", Myname);

	if (Tflag) {
		starttime();
	}

	for (ct = 0; ct < count; ct++) {
		for (fi = 0; fi < files; fi++) {
			sprintf(str, "%s.%d", FNAME, fi);
			sprintf(new, "%s.%d", NNAME, fi);
			if (rename(str, new) < 0) {
				error("can't rename %s to %s", str, new);
				exit(1);
			}
			if (stat(str, &statb) == 0) {
				error("%s exists after rename", str);
				exit(1);
			}
			if (stat(new, &statb) < 0) {
				error("can't stat %s after rename", new);
				exit(1);
			}
			if (statb.st_nlink != 1) {
				error("%s has %d links after rename (expect 1)",
					new, statb.st_nlink);
				exit(1);
			}
			if (link(new, str) < 0) {
				error("can't link %s to %s", new, str);
				exit(1);
			}
			if (stat(new, &statb) < 0) {
				error("can't stat %s after link", new);
				exit(1);
			}
			if (statb.st_nlink != 2) {
				error("%s has %d links after link (expect 2)",
					new, statb.st_nlink);
				exit(1);
			}
			if (stat(str, &statb) < 0) {
				error("can't stat %s after link", str);
				exit(1);
			}
			if (statb.st_nlink != 2) {
				error("%s has %d links after link (expect 2)",
					str, statb.st_nlink);
				exit(1);
			}
			if (unlink(new) < 0) {
				error("can't unlink %s", new);
				exit(1);
			}
			if (stat(str, &statb) < 0) {
				error("can't stat %s after unlink %s",
					str, new);
				exit(1);
			}
			if (statb.st_nlink != 1) {
				error("%s has %d links after unlink (expect 1)",
					str, statb.st_nlink);
				exit(1);
			}
		}
	}

	if (Tflag) {
		endtime(&time);
	}
	fprintf(stdout, "\t%d renames and links on %d files",
		files * count * 2, files);
	if (Tflag) {
		fprintf(stdout, " in %d.%-2d seconds",
		    time.tv_sec, time.tv_usec / 10000);
	}
	fprintf(stdout, "\n");
	complete();
}
