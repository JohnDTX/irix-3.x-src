/*
 *	tobw - 
 *		Convert a color image into a b/w image by combining the 
 *		three bands. Red contibutes 30%, green 59% and blue 11%.
 *
 */
#include "image.h"

short	rbuf[2048];
short	gbuf[2048];
short	bbuf[2048];
short	obuf[2048];

main(argc,argv)
int argc;
char **argv;
{
    IMAGE *iimage, *oimage;
    unsigned int xsize, ysize;
    unsigned int y;

    if( argc<3 ) {
	fprintf(stderr,"usage: tobw <image.ci> <outfile.bw>\n");
	exit(0);
    } 
    if( (iimage=iopen(argv[1],"r")) == NULL ) {
	fprintf(stderr,"%s: can't open input file %s\n",argv[0],argv[1]);
	exit(0);
    }
    if(iimage->dim<3) {
	fprintf(stderr,"%s: %s is not a color image\n",argv[0],argv[1]);
	exit(0);
    }
    xsize = iimage->xsize;
    ysize = iimage->ysize;
    oimage = iopen(argv[2],"w",RLE(1),2,xsize,ysize); 
    isetname(oimage,iimage->name);
    oimage->colormap = CM_NORMAL;
    for(y=0; y<ysize; y++) {
	getrow(iimage,rbuf,y,0);
	getrow(iimage,gbuf,y,1);
	getrow(iimage,bbuf,y,2);
	compress(rbuf,gbuf,bbuf,obuf,xsize);
	putrow(oimage,obuf,y,0);
    }
    iclose(oimage);
}

compress(rbuf,gbuf,bbuf,obuf,n)
register unsigned short *rbuf, *gbuf, *bbuf, *obuf;
register int n;
{
    register short i;

    for(i=n; i--; ) 
	    *obuf++ = (77*(*rbuf++) + 151*(*gbuf++) + 28*(*bbuf++))>>8;
}
