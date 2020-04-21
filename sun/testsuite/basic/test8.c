/*
 * Test symlink, readlink
 */

#include <sys/param.h>
#ifndef major
#include <sys/types.h>
#endif
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <stdio.h>
#include "tests.h"

int Tflag = 0;		/* print timing */
int Hflag = 0;		/* print help message */
int Fflag = 0;		/* test function only;  set count to 1, negate -t */

#define SNAME "/this/is/a/symlink"	/* symlink prefix */

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
	int count = 20;	/* times to do each file */
	int ct;
	int totfiles = 0;
	int totdirs = 0;
	struct timeval time;
	char str[MAXPATHLEN];
	char new[MAXPATHLEN];
	char buf[MAXPATHLEN];
	int ret;
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

#ifndef S_IFLNK
	fprintf(stdout, "\
%s: symlink and readlink not supported on this client\n", Myname);
#else S_IFLNK
	if (Fflag) {
		Tflag = 0;
		count = 1;
	}

	testdir(NULL);

	fprintf(stdout, "%s: symlink and readlink\n", Myname);

	if (Tflag) {
		starttime();
	}

	for (ct = 0; ct < count; ct++) {
		for (fi = 0; fi < files; fi++) {
			sprintf(str, "%s.%d", FNAME, fi);
			sprintf(new, "%s.%d", SNAME, fi);
			if (symlink(new, str) < 0) {
				error("can't make symlink %s", str);
				if (errno == EOPNOTSUPP)
					complete();
				else
					exit(1);
			}
                        if (lstat(str, &statb) < 0) {
                                error("can't stat %s after symlink", str);
                                exit(1);
                        }
			if ((statb.st_mode & S_IFMT) != S_IFLNK) {
				error("mode of %s not symlink");
				exit(1);
			}
			if ((ret = readlink(str, buf, MAXPATHLEN))
			     != strlen(new)) {
				error("readlink %s ret %d, expect %d",
					str, ret, strlen(new));
				exit(1);
			}
			if (strncmp(new, buf, ret) != NULL) {
				error("readlink %s returned bad linkname",
					str);
				exit(1);
			}
			if (unlink(str) < 0) {
				error("can't unlink %s", str);
				exit(1);
			}
		}
	}

	if (Tflag) {
		endtime(&time);
	}
	fprintf(stdout, "\t%d symlinks and readlinks on %d files",
		files * count * 2, files);
	if (Tflag) {
		fprintf(stdout, " in %d.%-2d seconds",
		    time.tv_sec, time.tv_usec / 10000);
	}
	fprintf(stdout, "\n");
#endif S_IFLNK
	complete();
}
