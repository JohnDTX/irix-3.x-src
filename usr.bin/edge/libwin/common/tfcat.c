#include "tf.h"
#include "tferr.h"
#include "stdio.h"

main(argc, argv)
	int argc;
	char *argv[];
{
	textframe *tf;
	char buf[4096];
	int nb;

fprintf(stderr, "Start=%x\n", sbrk(0));
	tf = tfnew();
	for (;;) {
		if (!fgets(buf, sizeof(buf), stdin))
			break;
		if (tfputascii(tf, buf, strlen(buf)))
			abort();
		tfsplit(tf);
	}

	/* spit text back out... a slow cat */
	if (argc != 1)
		tfdumptext(tf, stdout);
	else
		tfdumpascii(tf, stdout);
fprintf(stderr, "End=%x\n", sbrk(0));
	exit(0);
}
