
/*
 * Test lookup up and down across mount points
 */

#include <sys/param.h>
#ifdef sgi
#include <sys/statfs.h>
#else
#include <sys/vfs.h>
#endif
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <stdio.h>
#include "tests.h"

int Tflag = 0;		/* print timing */
int Hflag = 0;		/* print help message */
int Fflag = 0;		/* test function only;  set count to 1, negate -t */

usage()
{
	fprintf(stdout, "usage: %s [-htf] [count]\n", Myname);
}

main(argc, argv)
	int argc;
	char *argv[];
{
	int count = 250;		/* times to do test */
	int ct;
	struct timeval time;
	struct statfs sfsb;
	int ret;
	struct stat statb;
	char *opts;
	char path[MAXPATHLEN];

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

	fprintf(stdout, "%s: lookups across mount point\n", Myname);

	if (Tflag) {
		starttime();
	}

	for (ct = 0; ct < count; ct++) {
		if (getwd(path) == NULL) {
			fprintf(stderr, "%s: getwd failed\n", Myname);
			exit(1);
		}
		if (stat(path, &statb) < 0) {
			error("can't stat %s after getwd", path);
			exit(1);
		}
	}

	if (Tflag) {
		endtime(&time);
	}
	fprintf(stdout, "\t%d getwd and stat calls", count * 2);
	if (Tflag) {
		fprintf(stdout, " in %d.%-2d seconds",
		    time.tv_sec, time.tv_usec / 10000);
	}
	fprintf(stdout, "\n");
	complete();
}
