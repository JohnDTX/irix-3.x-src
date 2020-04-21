/*
 *	showci - 
 *		Display a color image on the iris. This can not be used if
 *	the window manager is running.
 *
 *				Paul Haeberli - 1985
 */
#include <stdio.h>
#include "gl.h"
#include "device.h"
#include "image.h"
#include "math.h"

float gammacorrect();
float getgamma();

static unsigned char gamtable[256];

unsigned short rs[2048];
unsigned short gs[2048];
unsigned short bs[2048];

unsigned short rb[2048];
unsigned short gb[2048];
unsigned short bb[2048];

main(argc,argv)
int argc;
char **argv;
{
    IMAGE *image;
    int y, xsize, ysize;
    short val;

    if( argc<2 ) {
	printf("usage: showci infile\n");
	exit(1);
    } 
    if( (image=iopen(argv[1],"r")) == NULL ) {
	printf("showci: can't open input file %s\n",argv[1]);
	exit(1);
    }
    if( image->zsize < 3 ) {
	printf("showci: input image is not a color image");
	exit(1);
    }
    if( ismex() ) {
	printf("showci: can't be used while running the windowe manager\n");
	exit(1);
    }
    xsize = image->xsize;
    ysize = image->ysize;

    ginit();
    maketable();
    viewport((XMAXSCREEN-xsize)/2,((XMAXSCREEN-xsize)/2)+xsize-1,
		(YMAXSCREEN-ysize)/2,((YMAXSCREEN-ysize)/2)+ysize-1);
    ortho2(-0.5,(float)xsize-0.5,-0.5,(float)ysize-0.5);
    RGBmode();
    gconfig();
    RGBcolor(0,0,0);
    RGBwritemask(0xffff,0xffff,0xffff);
    clear();
    for(y=0; y<ysize; y++) {
	getrow(image,rs,y,0);
	compress(rs,rb,xsize);
	getrow(image,gs,y,1);
	compress(gs,gb,xsize);
	getrow(image,bs,y,2);
	compress(bs,bb,xsize);
	cmov2i(0,y);
	writeRGB(xsize,rb,gb,bb);
    }
    qdevice(LEFTMOUSE);
    while(1) {
	if(qread(&val) == LEFTMOUSE) {
	    singlebuffer();
	    gconfig();
	    gexit();
	    exit(0);
	}
    }
}

compress(sptr,bptr,n)
register unsigned short *sptr;
register unsigned char *bptr;
register short n;
{
    while(n--) 
	*bptr++ = gamtable[*sptr++ & 0xff];

}


maketable()
{
    short i;
    float gammaval;

    gammaval = getgamma();
    for(i=0; i<256; i++)
	gamtable[i] = (unsigned char)(255*gammacorrect(i/255.0,gammaval));
}
