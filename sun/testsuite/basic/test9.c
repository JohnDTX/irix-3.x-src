/*
 * Test statfs
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
	int count = 1500;	/* times to do statfs call */
	int ct;
	struct timeval time;
	struct statfs sfsb;
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

	fprintf(stdout, "%s: statfs\n", Myname);

	if (Tflag) {
		starttime();
	}

	for (ct = 0; ct < count; ct++) {
#ifdef sgi
		if (statfs(".", &sfsb, sizeof sfsb, 0) < 0) {
#else
		if (statfs(".", &sfsb) < 0) {
#endif
			error("can't do statfs on \".\"");
			exit(1);
		}
	}

	if (Tflag) {
		endtime(&time);
	}
#ifdef sgi
	fprintf(stdout, "\ttype=%d, bsize=%d, blocks=%d, bfree=%d,\n\
\tbavail=%d, files=%d, ffree=%d, fname=%.*s, fpack=%.*s\n",
		sfsb.f_fstyp, sfsb.f_bsize, sfsb.f_blocks, sfsb.f_bfree,
		sfsb.f_blocks - sfsb.f_bfree, sfsb.f_files, sfsb.f_ffree,
		sizeof sfsb.f_fname, sfsb.f_fname,
		sizeof sfsb.f_fpack, sfsb.f_fpack);
#else
	fprintf(stdout, "\ttype=%d, bsize=%d, blocks=%d, bfree=%d\n\
\t  bavail=%d, files=%d, ffree=%d, fsid=0x%x 0x%x\n",
		sfsb.f_type, sfsb.f_bsize, sfsb.f_blocks, sfsb.f_bfree,
		sfsb.f_bavail, sfsb.f_files, sfsb.f_ffree,
		sfsb.f_fsid[0], sfsb.f_fsid[1]);
#endif
	fprintf(stdout, "\t%d statfs calls", count);
	if (Tflag) {
		fprintf(stdout, " in %d.%-2d seconds",
		    time.tv_sec, time.tv_usec / 10000);
	}
	fprintf(stdout, "\n");
	complete();
}
