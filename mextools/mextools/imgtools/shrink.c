/*
 *	shrink - 
 *		Shrink an image by an integer factor. 
 *
 *				Paul Haeberli - 1984
 *
 */
#include "image.h"

unsigned long sumrow[4096];
unsigned short row[4096];

main(argc,argv)
int argc;
char **argv;
{
    register unsigned long *lptr;
    register unsigned short *sptr;
    register int x, xcount;
    register IMAGE *iimage, *oimage;
    register int ix, iy, iz;
    register int ox, oy, oz;
    register int ycount;
    register long sum;
    register int factor, f2;
    int xsize, ysize;

    if( argc<4 ) {
	fprintf(stderr,
		"usage: %s <shrink factor> <infile> <outfile>\n",argv[0]);
	exit(1);
    } 
    factor = atoi(argv[1]);
    f2 = factor*factor;
    if( (iimage=iopen(argv[2],"r")) == NULL ) {
	fprintf(stderr,"%s: can't open input file %s\n",argv[0],argv[2]);
	exit(1);
    }
    xsize = iimage->xsize/factor;
    ysize = iimage->ysize/factor;
    oimage = iopen(argv[3],"w",iimage->type,iimage->dim,
					xsize,ysize,iimage->zsize); 
    isetname(oimage,iimage->name);
    oimage->colormap = iimage->colormap;
    for(iz=0; iz<iimage->zsize; iz++) {
	lptr = sumrow;
	for(x=xsize; x--;) 
	    *lptr++ = 0;
	ycount = factor;
	for(iy=0; iy<iimage->ysize; iy++) {
	    getrow(iimage,row,iy,iz);
	    xcount = factor;
	    sptr = row;
	    lptr = sumrow;
	    for(x=iimage->xsize; x--;) {
		*lptr += *sptr++;
		if(--xcount==0) {
		    lptr++;
		    xcount = factor;
		}
	    }
	    if(--ycount == 0) {
		compress(row,sumrow,xsize,f2);
		putrow(oimage,row,iy/factor,iz);
		lptr = sumrow;
		for(x=xsize; x--;) 
		    *lptr++ = 0;
		ycount = factor;
	    }
	}
    }
    iclose(oimage);
}

compress(sptr,lptr,n,div)
register unsigned short *sptr;
register unsigned long *lptr;
register int n;
register int div;
{
    while(n--)
	*sptr++ = *lptr++/div;
}
