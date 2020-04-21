/*
 *  check for lost reply on non-idempotent requests
 */
#include <stdio.h>

main(argc, argv)
	int argc;
	char *argv[];
{
	int count, i;
	int fd;
	int cfail, lfail, u1fail, u2fail, failcount;
	char name1[256];
	char name2[256];

	if (argc != 3) {
		fprintf(stderr, "usage: %s count name\n", argv[0]);
		exit(1);
	}
	setbuf(stdout, NULL);
	count = atoi(argv[1]);
	sprintf(name1, "%s1", argv[2]);
	sprintf(name2, "%s2", argv[2]);
	failcount = cfail = lfail = u1fail = u2fail = 0;
	for (i=count; i; i--) {
		if ((fd = creat(name1, 0666)) < 0) {
			cfail++;
			fprintf(stderr, "create ");
			perror(name1);
			continue;
		}
		close(fd);
		if (link(name1, name2) < 0) {
			lfail++;
			fprintf(stderr, "link %s %s", name1, name2);
			perror(" ");
		}
		if (unlink(name2) < 0) {
			u1fail++;
			fprintf(stderr, "unlink %s", name2);
			perror(" ");
		}
		if (unlink(name1) < 0) {
			u2fail++;
			fprintf(stderr, "unlink %s", name1);
			perror(" ");
		}
	}
	failcount = cfail + lfail + u1fail + u2fail;
	fprintf(stdout, "%d tries, %d lost replies\n", count, failcount);
	if (cfail) {
		fprintf(stdout, " %d bad create\n", cfail);
	}
	if (lfail) {
		fprintf(stdout, " %d bad link\n", lfail);
	}
	if (u1fail) {
		fprintf(stdout, " %d bad unlink 1\n", u1fail);
	}
	if (u2fail) {
		fprintf(stdout, " %d bad unlink 2\n", u2fail);
	}
	exit(failcount);
}
