/*
 *	textcolors - 
 *		Set the color indexes used for the textport.
 *
 *				Paul Haeberli - 1985
 *
 */
#include "stdio.h"

main(argc,argv)
int argc;
char **argv;
{
    if (argc < 5) {
	fprintf(stderr,"usage: textcolors <text> <page> <reverse> <cursor>\n");
	exit(1);
    }
    putchar(033);
    putchar('7');
    putchar('F');
    putchar('0'+atoi(argv[1]));
    putchar(033);
    putchar('7');
    putchar('B');
    putchar('0'+atoi(argv[2]));
    putchar(033);
    putchar('7');
    putchar('R');
    putchar('0'+atoi(argv[3]));
    putchar(033);
    putchar('7');
    putchar('C');
    putchar('0'+atoi(argv[4]));
    exit(0);
}
