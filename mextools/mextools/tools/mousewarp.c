/*
 *  	mousewarp -
 *		Set the mouse warping parameters.  Min is the smallest
 *		raw mouse movement that will be maginified by Mult.
 *
 *				Paul Haeberli - 1986
 */
#include "gl.h"
#include "grioctl.h"
#include "stdio.h"

float atof();

main(argc,argv)
int argc;
char **argv;
{
    short *data;

    if(argc<3) {
	fprintf(stderr,"usage: mousewarp <min> <mult>\n");
	exit(1);
    }
    noport();
    winopen("mousewarp");

    data = (short *)gl_smallbufaddr();
    data[0] = atof(argv[1]);
    data[1] = 16*atof(argv[2]);
    grioctl(GR_MOUSEWARP,0);
}
