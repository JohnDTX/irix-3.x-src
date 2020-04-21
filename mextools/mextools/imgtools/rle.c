/*
 *	rle - 
 *		Convert a verbatim image into an rle image.
 *
 * 				Paul Haeberli - 1984 
 */
#include "image.h"

short rowbuf[4096];

main(argc,argv)
int argc;
char **argv;
{
    register IMAGE *iimage, *oimage;
    register int y, z;
    int xsize, ysize, zsize;

    if( argc<3 ) {
	fprintf(stderr,"usage: rle infile outfile\n");
	exit(0);
    } 
    if( (iimage=iopen(argv[1],"r")) == NULL ) {
	fprintf(stderr,"rle: can't open input file %s\n",argv[1]);
	exit(0);
    }
    xsize = iimage->xsize;
    ysize = iimage->ysize;
    zsize = iimage->zsize;
    oimage = iopen(argv[2],"w",RLE(BPP(iimage->type)),
					iimage->dim,xsize,ysize,zsize); 
    isetname(oimage,iimage->name);
    oimage->colormap = iimage->colormap;
    for(z=0; z<zsize; z++)
	for(y=0; y<ysize; y++) {
	    getrow(iimage,rowbuf,y,z);
	    putrow(oimage,rowbuf,y,z);
	}
    iclose(oimage);
}
