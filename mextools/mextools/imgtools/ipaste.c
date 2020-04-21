/*
 *	ipaste - 
 *		Display an image on the iris.
 *
 *				Paul Haeberli - 1984	
 *
 */
#include "image.h"
#include "gl.h"
#include "device.h"
#include "port.h"

int colormap;

short rowbuf[2048]; 
short *rowdat[2048];
short factor = 1;
unsigned short xmap[256];
int xsize, ysize;
int xdim, ydim;

main(argc,argv)
int argc;
char **argv;
{
    register int i;
    short val;

    if( argc<2 ) {
	fprintf(stderr,"usage: ipaste infile [-m]\n");
	exit(1);
    } 
    for(i=1; i<argc; i++) {
	if(strcmp(argv[i],"-m") == 0)
	    factor = 2;
    }
    readit(argv[1]);
    while(1) {
	if(qread(&val) == REDRAW) {
     	    qreset();
	    drawit();
	}
    }
}

readit(filename)
{
    register IMAGE *image;
    register int r, g, b;
    register int i, y;

    if( (image=iopen(filename,"r")) == NULL ) {
	fprintf(stderr,"paste: can't open input file %s\n",filename);
	exit(1);
    }
    xdim = image->xsize;
    ydim = image->ysize;
    xsize = factor*xdim;
    ysize = factor*ydim;

    keepaspect(xsize,ysize);
    prefsize(xsize,ysize);
    winopen("paste");

    colormap = image->colormap;
    if(colormap == CM_DITHERED) {
	if(getplanes() <= 8) {
	    for(i=0; i<256; i++) {
		r = (i>>1) & 3; 
		g = (i>>4) & 3; 
		b = (i>>7) & 1; 
		xmap[i] = 32 + (b<<4) + (g<<2) + r;
	    }
	} else {
	    for(i=0; i<256; i++) 
		xmap[i] = 256+i;
	}
    } else if(colormap == CM_NORMAL) {
	if(getplanes() <= 8) {
	    for(i=0; i<256; i++) 
		xmap[i] = 16 + (i>>4);
	} else {
	    for(i=0; i<256; i++) 
		xmap[i] = 128+(i>>1);
	}		      
    }
    makeframe();
    for(y=0; y<ydim; y++) {
        getrow(image,rowbuf,y,0);
        rowdat[y] = (short *)malloc(sizeof(short)*xsize);
        compress(rowdat[y],rowbuf,xdim);
        if(factor>1)
	    expand(rowdat[y],xdim,factor);
        for(i=0; i<factor; i++) {
	    cmov2i(0,(factor*y)+i);
	    writepixels(xsize,rowdat[y]);
        }
    }
    iclose(image);
}

drawit()
{
    int i, y;

    makeframe();
    for(y=0; y<ydim; y++) {
        for(i=0; i<factor; i++) {
	    cmov2i(0,(factor*y)+i);
	    writepixels(xsize,rowdat[y]);
        }
    }
}

compress(obuff,ibuff,n)
register unsigned short *obuff;
unsigned char *ibuff;
int n;
{
    register short i;
    register unsigned short *sptr;

    sptr = (unsigned short *)ibuff;
    if(colormap == CM_SCREEN)
	for(i=n; i--; ) 
	    *obuff++ = *sptr++;
    else
	for(i=n; i--; ) 
	    *obuff++ = xmap[*sptr++];
}

expand(shortbuf,n,factor)
register unsigned short shortbuf[];
int n;
register int factor;
{
    register unsigned short *sptr, *dptr;
    register short j;

    sptr = &shortbuf[n];
    dptr = &shortbuf[n*factor];
    while(sptr != shortbuf) {
	sptr--;
	for(j=0; j<factor; j++) 
	    *--dptr =  *sptr;
    }
}

makeframe()
{
    reshapeviewport();
    viewport(0,xsize-1,0,ysize-1);
    ortho2(-0.5,(float)xsize-0.5,-0.5,(float)ysize-0.5);
}
