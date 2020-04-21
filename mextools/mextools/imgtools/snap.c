/*
 *	snap - 
 *		Grab a section of the screen into a file.
 *
 *				Peter Broadwell - 1986
 *
 */
#include "image.h"
#include "gl.h"
#include "device.h"
#include "port.h"

int xsize, ysize;
short **img;

main(argc,argv)
int argc;
char **argv;
{
    register int y;

    if( argc<2 ) {
	fprintf(stderr,"usage: snap <outfile> [xsize [ysize]]\n");
	exit(1);
    } 
    if(argc >= 3) {
	ysize = xsize = atoi(argv[2]);
	if(argc == 4)
	    ysize = atoi(argv[3]);
	prefsize(xsize,ysize);
    }
    getport("snap");
    getsize(&xsize,&ysize);
    img = (short **)malloc(ysize*sizeof(short *));
    for(y=0; y<ysize; y++) {
    	percentdone((100.0*y)/ysize);
	img[y] = (short *)malloc(xsize*sizeof(short));
	cmov2i(0,y);
	readpixels(xsize,img[y]);
    }
    percentdone(100.0);
    writeit(argv[1]);
}

writeit(filename)
char *filename;
{
    register IMAGE *oimage;
    register int y;

    oimage = iopen(filename,"w",RLE(2),2,xsize,ysize,0);
    if( oimage == NULL ) {
	fprintf(stderr,"imged: can't create output file %s\n",filename);
	exit(0);
    }
    oimage->colormap = CM_SCREEN;
    isetname(oimage,filename);
    for(y=0; y<ysize; y++) 
	putrow(oimage,img[y],y,0);
    iclose(oimage);
    percentdone(100.0);
}

