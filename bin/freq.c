char _Origin_[] = "System V";

/*
 * freq - print the frequency of occurance of letters
 *
 *	Usage:
 *		freq [file1 [file2 [...] ] ]
 */

#include <stdio.h>

long freq[256];

char *ctrl[] = {
	"nul", "soh", "stx", "etx", "eot", "enq", "ack", "bel",
	"bs ", "ht ", "lf ", "vt ", "ff ", "cr ", "so ", "si ",
	"dle", "dc1", "dc2", "dc3", "dc4", "nak", "syn", "etb",
	"can", "em ", "sub", "esc", "fs ", "gs ", "rs ", "us "
};

char *del = "del";

main(argc, argv)
char **argv;
{
	register FILE *inp;
	char obuf[BUFSIZ];

	setbuf(stdout, obuf);
	argv++;
	if (argc == 1)
		accum(stdin);
	else {
		while (--argc > 0) {
			if ((inp = fopen(*argv, "r")) == NULL)
				fprintf(stderr, "Can't open %s\n", *argv);
			else {
				accum(inp);
				fclose(inp);
			}
			argv++;
		}
	}
	output();
	fflush(stdout);
}

accum(inp)
register FILE *inp;
{
	register c;

	while ((c = getc(inp)) != EOF)
		freq[c & 0177]++;
}

#define INC	4

/*
 * output results
 */
output()
{
	register i, j;

	for (i = 0; i < 128; i += INC) {
		for (j = i; j < i+INC; j++) {
			printf("|");
			if (j < 040)
				printf("%s", ctrl[j]);
			else if (j == 127)
				printf("%s", del);
			else
				printf("%c  ", j);
			printf("%8ld", freq[j]);
		}
		printf("|\n");
	}
}
