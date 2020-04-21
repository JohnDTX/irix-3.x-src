/*
 *	mapimg - 
 *		Use a color map to transform a screen image into an rgb
 *	 	image.
 *
 */
#include "image.h"
unsigned short	rbuf[2048];
unsigned short	gbuf[2048];
unsigned short	bbuf[2048];
unsigned short	ibuf[2048];

unsigned short rmap[4096];
unsigned short gmap[4096];
unsigned short bmap[4096];

main(argc,argv)
int argc;
char **argv;
{
    IMAGE *iimage, *oimage, *mapfile;
    unsigned int xsize, ysize;
    unsigned int y;

    if( argc<4 ) {
	fprintf(stderr,"usage: mapimg <inimg> <outimg> <mapfile>\n");
	exit(0);
    } 
    if( (iimage=iopen(argv[1],"r")) == NULL ) {
	fprintf(stderr,"%s: can't open input file %s\n",argv[0],argv[1]);
	exit(0);
    }
    if( (mapfile=iopen(argv[3],"r")) == NULL ) {
	fprintf(stderr,"%s: can't open input file %s\n",argv[0],argv[3]);
	exit(0);
    }
    readmap(mapfile,rmap,gmap,bmap);
    xsize = iimage->xsize;
    ysize = iimage->ysize;
    oimage = iopen(argv[2],"w",RLE(1),3,xsize,ysize,3); 
    isetname(oimage,iimage->name);
    oimage->colormap = iimage->colormap;
    for(y=0; y<ysize; y++) {
	getrow(iimage,ibuf,y,0);
	domap(ibuf,rbuf,gbuf,bbuf,xsize);
	putrow(oimage,rbuf,y,0);
	putrow(oimage,gbuf,y,1);
	putrow(oimage,bbuf,y,2);
    }
    iclose(oimage);
}

domap(ibuf,rbuf,gbuf,bbuf,n)
register unsigned short *ibuf, *rbuf, *gbuf, *bbuf;
register int n;
{
    while(n--) {
	    *rbuf++ = rmap[*ibuf];
	    *gbuf++ = gmap[*ibuf];
	    *bbuf++ = bmap[*ibuf++];
    }
}

short rowbuf[10];

readmap(mapfile,rmap,gmap,bmap)
register IMAGE *mapfile;
register unsigned short *rmap, *gmap, *bmap;
{
    register int i, index;

    if(mapfile->xsize != 4) {
	fprintf(stderr,"readmap: wierd map file!! \n");
	exit(1);
    }
    for(i=0; i<4096; i++) {
	rmap[i] = 0;
	gmap[i] = 0;
	bmap[i] = 0;
    }
    for(i=0; i<mapfile->ysize; i++) {
	getrow(mapfile,rowbuf,i,0);
	index = rowbuf[0];
	if(index > 4096) {
	    fprintf(stderr,"readmap: index way out there!!\n");
	    exit(1);
	}
	rmap[index] = rowbuf[1];
	gmap[index] = rowbuf[2];
	bmap[index] = rowbuf[3];
    }
}
