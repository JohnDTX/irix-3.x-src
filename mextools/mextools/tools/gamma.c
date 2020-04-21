/*
 * 	gamma - 
 *		Get or set the gamma value stored in ~/.gamma . This 
 *		value is used by the program makemap and the library 
 *		function gammapcolor to do gamma correction.
 *
 *				Paul Haeberli - 1984
 *
 */
#include "stdio.h"
#include "math.h"

main(argc, argv)
int argc;
char **argv;
{
    if (argc>3) 
	 fprintf(stderr,"usage: %s <gammavalue>\n",argv[0]);
    else if (argc>1)
	setgamma(atof(argv[1]));
    else
	printf("%f\n",getgamma());
}
