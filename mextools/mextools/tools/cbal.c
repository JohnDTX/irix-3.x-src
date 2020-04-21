/*
 * 	cbal - 
 *		Set the rgb color balance of the display.  This will 
 *		affect the colors mapped by gammapcolor.
 *
 *				Paul Haeberli - 1985
 *
 */
#include "stdio.h"

main(argc, argv)
int argc;
char **argv;
{
    int r, g, b;

    if (argc>5) 
	 fprintf(stderr,"usage: %s: <rbal gbal bbal> \n",argv[0]);
    else if (argc>1)
	setcolorbal(atoi(argv[1]),atoi(argv[2]),atoi(argv[3]));
    else {
	getcolorbal(&r,&g,&b);
	printf("%d %d %d\n",r,g,b);
    }
}
