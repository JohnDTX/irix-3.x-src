/*
 * test seek to negative offset
 */
#include <stdio.h>
#include <sys/file.h>

main(argc, argv)
	int argc;
	char *argv[];
{
	int fd, i;
	char buf[8192];
	extern int errno;

	if (argc != 2) {
		fprintf(stderr, "usage: negseek filename\n");
		exit(1);
	}
	fd = open(argv[1], O_CREAT|O_RDONLY, 0666);
	if (fd < 0) {
		perror(argv[1]);
		exit(1);
	}

	for ( i = 0; i>-10240 ;i -= 1024 ) {
		if (lseek(fd, i, 0) < 0) {
			/*
			perror("lseek");
			exit(1);
			*/
		}
		if (read(fd, buf, sizeof buf) < 0) {
			perror("read");
			exit(0);
		}
	}
	unlink(argv[1]);
	exit(0);
}
