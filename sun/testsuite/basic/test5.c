/*
 * Test read and write
 */

#include <sys/param.h>
#ifndef major
#include <sys/types.h>
#endif
#include <sys/time.h>
#include <sys/stat.h>
#include <stdio.h>
#include "tests.h"

#define	BUFSZ	8192
#define	DSIZE	1048576

int Tflag = 0;		/* print timing */
int Hflag = 0;		/* print help message */
int Fflag = 0;		/* test function only;  set count to 1, negate -t */

usage()
{
	fprintf(stdout, "usage: %s [-htf] [size count]\n", Myname);
}

main(argc, argv)
	int argc;
	char *argv[];
{
	int count = DCOUNT;	/* times to do each file */
	int ct;
	int size = DSIZE;
	int si;
	int i;
	int fd;
	int bytes;
	struct timeval time;
	char str[MAXPATHLEN];
	struct stat statb;
	char *opts;
	char buf[BUFSZ];

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
		size = getparm(*argv, 1, "size");
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

	fprintf(stdout, "%s: read and write\n", Myname);

	for (i=0; i < BUFSZ / sizeof(int); i++) {
		((int *)buf)[i] = i;
	}

	if (Tflag) {
		starttime();
	}

	for (ct = 0; ct < count; ct++) {
		if ((fd = creat("bigfile", 0666)) < 0) {
			error("can't create 'bigfile'");
			exit(1);
		}
		if (fstat(fd, &statb) < 0) {
			error("can't stat 'bigfile'");
			exit(1);
		}
		if (statb.st_size != 0) {
			error("'bigfile' has size %d, should be 0",
			    statb.st_size);
			exit(1);
		}
		for (si = size; si; si -= BUFSZ) {
			bytes = MIN(BUFSZ, si);
			if (write(fd, buf, bytes) != bytes) {
				error("'bigfile' write failed");
				exit(1);
			}
		}
		close(fd);
		if (stat("bigfile", &statb) < 0) {
			error("can't stat 'bigfile'");
			exit(1);
		}
		if (statb.st_size != size) {
			error("'bigfile' has size %d, should be %d",
			    statb.st_size, size);
			exit(1);
		}
	}

	if (Tflag) {
		endtime(&time);
	}

	if ((fd = open("bigfile", 0)) < 0) {
		error("can't open 'bigfile'");
		exit(1);
	}
	for (si = size; si; si -= BUFSZ) {
		bytes = MIN(BUFSZ, si);
		if (read(fd, buf, bytes) != bytes) {
			error("'bigfile' read failed");
			exit(1);
		}
		for (i = 0; i < bytes / sizeof(int); i++) {
			if (((int *)buf)[i] != i) {
				error("bad data in 'bigfile'");
				exit(1);
			}
		}
	}
	close(fd);

	fprintf(stdout, "\twrote %d byte file %d times", size, count);
	if (Tflag) {
		fprintf(stdout, " in %d.%-2d seconds (%d bytes/sec)",
		    time.tv_sec, time.tv_usec / 10000, size*count/time.tv_sec);
	}
	fprintf(stdout, "\n");
	if (Tflag) {
		starttime();
	}

	for (ct = 0; ct < count; ct++) {
		if ((fd = open("bigfile", 0)) < 0) {
			error("can't open 'bigfile'");
			exit(1);
		}
		for (si = size; si; si -= BUFSZ) {
			bytes = MIN(BUFSZ, si);
			if (read(fd, buf, bytes) != bytes) {
				error("'bigfile' read failed");
				exit(1);
			}
		}
		close(fd);
	}

	if (Tflag) {
		endtime(&time);
	}
	fprintf(stdout, "\tread %d byte file %d times", size, count);
	if (Tflag) {
		fprintf(stdout, " in %d.%-2d seconds (%d bytes/sec)",
		    time.tv_sec, time.tv_usec / 10000, size*count/time.tv_sec);
	}
	fprintf(stdout, "\n");

	if (unlink("bigfile") < 0) {
		error("can't unlink 'bigfile'");
		exit(1);
	}
	complete();
}
