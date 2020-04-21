/*
 * (C) 1983 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */
#include <a.out.h>

struct bhdr bhdr;

main(argc, argv)
char **argv;
{
	int fp;

	if (argc != 2) {
		printf("usage:fixbss file\n");
		exit(1);
	}
	if ((fp = open(argv[1], 2)) < 0) {
		printf("fixbss: can't open %s\n", argv[1]);
		exit(1);
	}
	if (read(fp, &bhdr, sizeof(bhdr)) != sizeof(bhdr)) {
		printf("fixbss: can't read header\n");
		exit(1);
	}
	if (bhdr.fmagic != 0407) {
		printf("fixbss: file %s not a 0407 executable image\n", argv[1]);
		exit(1);
	}
	bhdr.bsize = 10;
	lseek(fp, 0, 0);
	if (write(fp, &bhdr, sizeof(bhdr)) != sizeof(bhdr)) {
		printf("fixbss: can't rewrite header\n");
		exit(1);
	}
	exit(0);
}
