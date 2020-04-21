/*
 *	readimage - 
 *		Read an image file but don't do anything with it!
 *
 *				Paul Haeberli - 1984
 *
 */
#include "image.h"

short rowbuf[4096]; 

main(argc,argv)
int argc;
char **argv;
{
    register IMAGE *image;
    register int i, y, ysize;
    register int z, zsize;

    if( argc<2 ) {
	fprintf(stderr,"usage: readimage infile\n");
	exit(1);
    } 
    if( (image=iopen(argv[1],"r")) == NULL ) {
	fprintf(stderr,"readimage: can't open input file %s\n",argv[1]);
	exit(1);
    }
    ysize = image->ysize;
    zsize = image->zsize;
    for(z=0; z<zsize; z++)
	for(y=0; y<ysize; y++)
	    getrow(image,rowbuf,y,z);
}
