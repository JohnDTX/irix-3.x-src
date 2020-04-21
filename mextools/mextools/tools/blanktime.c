/*
 *	blanktime - 
 *		Set the number of frames before the screen blanks.  Nframes 
 *		of 0 frames means never blank the screen.
 *
 *				Paul Haeberli - 1985
 */
#include "grioctl.h"
#include "stdio.h"

main(argc,argv)
int argc;
char **argv;
{
    if (argc<2) {
	fprintf(stderr,"usage: blanktime <nframes>\n");
	exit(1);
    }
    blanktime(atoi(argv[1]));
}
