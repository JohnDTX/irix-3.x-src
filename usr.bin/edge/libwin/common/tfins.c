#include "stdio.h"
#include "tf.h"

main(argc, argv)
	int argc;
	char *argv[];
{
	textframe *tf;
	char buf[4096];
	int i;

	tf = tfnew();
	tfsetpoint(tf, 0, 0);
fprintf(stderr, "Start=%x\n", sbrk(0));
	tf = tfnew();
	for (;;) {
		if (!fgets(buf, sizeof(buf), stdin))
			break;
		tfsetpoint(tf, 0, 0);
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
