/*
 * Stat a file n times
 */
#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/time.h>

int stats = 0;
int readdirs = 0;

main(argc, argv)
	int argc;
	char *argv[];
{
	struct timeval stim, etim;
	float elapsed;
	register int count;
	register int i;
	struct stat statb;

	if (argc != 2) {
		fprintf(stderr, "usage: %s count\n", argv[0]);
		exit(1);
	}

	count = atoi(argv[1]);
	gettimeofday(&stim, 0);
	for (i=0; i<count; i++) {
		stat(argv[0], &statb);
		stats++;
	}
	gettimeofday(&etim, 0);
	elapsed = (float) (etim.tv_sec - stim.tv_sec) +
	    (float)(etim.tv_usec - stim.tv_usec) / 1000000.0;
	if (elapsed == 0.0) {
		fprintf(stdout, "%d calls 0.0 seconds\n", count);
	} else {
		fprintf(stdout,
		    "%d calls %.2f seconds %.2f calls/sec %.2f msec/call\n",
		    count, elapsed, (float)count / elapsed,
		    1000.0 * elapsed / (float)count);
	}
	exit(0);
}
