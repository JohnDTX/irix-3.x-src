char _Origin_[] = "System V";

#include <stdio.h>

/*
 * Create a LARGE zero filled file for the purpose of clearing
 * all blocks on a disk. Then remove the file.
 */

char fname[] = ".zero";
char buf[BUFSIZ];

main()
{
	register int i, fp;

	if ((fp = creat(fname, 0600)) < 0) {
		printf("Can't creat %s\n", fname);
		exit(-1);
	}
	for (i = 0; ; i++)
		if (write(fp, buf, BUFSIZ) != BUFSIZ)
			break;
	printf("%d blocks written\n", i);
	printf("Unlinking %s\n", fname);
	unlink(fname);
}
