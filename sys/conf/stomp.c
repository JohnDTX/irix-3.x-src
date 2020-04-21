#include <a.out.h>

/*
 * stomp vmunix:
 *	- takes an sgi vmunix and stomps on the load address so that the
 *	  proms can load the program
 */

main(argc, argv)
	int argc;
	char *argv[];
{
	int fd;
	struct exec hdr;

	if (argc != 2) {
		printf("usage: stomp vmunix\n");
		exit(-1);
	}

	if ((fd = open(argv[1], 2)) == -1) {
		perror("stomp");
		exit(-1);
	}

	lseek(fd, 0L, 0);
	read(fd, &hdr, sizeof(hdr));
	hdr.a_entry = 0x400;
	lseek(fd, 0L, 0);
	write(fd, &hdr, sizeof(hdr));
	close(fd);
}
