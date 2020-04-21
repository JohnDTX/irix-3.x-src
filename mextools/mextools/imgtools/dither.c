/*
 *	dither - 
 *		Convert a color image into a 1 byte per pixel image
 *		to be displayed with a colormap that has 3 bits for red,
 *		3 bit for green, and 2 bits for blue.
 *
 *				Paul Haeberli - 1985
 *
 */
#include "image.h"

short	rbuf[2048];
short	gbuf[2048];
short	bbuf[2048];
short	obuf[2048];
int 	rerr[2048];
int 	gerr[2048];
int 	berr[2048];

short dithmat[4][4] = {
	0, 8, 2, 10,
	12, 4, 14, 6,
	3, 11, 1, 9,
	15, 7, 13, 5,
};

short rmat[4][4];
short gmat[4][4];
short bmat[4][4];

int quant = 0;
int ordered = 0;

main(argc,argv)
int argc;
char **argv;
{
    IMAGE *iimage, *oimage;
    unsigned int xsize, ysize;
    unsigned int y;

    if( argc<3 ) {
	fprintf(stderr,"usage: dither [-o] [-q] <inimage> <outimage>\n");
	exit(1);
    } 
    argv++;
    while (argv[0][0] == '-') {
	if(argv[0][1] == 'o') 
	    ordered++;
	if(argv[0][1] == 'q') 
	    quant++;
	argv++;
    }
    if( (iimage=iopen(argv[0],"r")) == NULL ) {
	fprintf(stderr,"dither: can't open input file %s\n",argv[0]);
	exit(1);
    }
    oimage = iopen(argv[1],"w",RLE(1),2,iimage->xsize,iimage->ysize); 
    isetname(oimage,iimage->name);
    xsize = iimage->xsize;
    ysize = iimage->ysize;
    if(iimage->dim<3)  {
	fprintf(stderr,"dither: input must be a color image\n");
	exit(1);
    }
    oimage->colormap = CM_DITHERED;
    makemats();
    for(y=0; y<ysize; y++) {
	getrow(iimage,rbuf,y,0);
	getrow(iimage,gbuf,y,1);
	getrow(iimage,bbuf,y,2);
	if(quant) 
	    doquant(rbuf,gbuf,bbuf,obuf,xsize,iimage->max);
	else if(ordered) 
	    doordither(rbuf,gbuf,bbuf,obuf,xsize,iimage->max,
		       &rmat[y&0x3][0], &gmat[y&0x3][0], &bmat[y&0x3][0]);
	else 
	    dodither(rbuf,gbuf,bbuf,obuf,xsize,iimage->max);
	putrow(oimage,obuf,y,0);
    }
    iclose(oimage);
}

#define WRAP(x)	((x)&0x3)

makemats()
{
    int i, j;

    for(i=0; i<4; i++) 
	for(j=0; j<4; j++) {
	    rmat[i][j] = ((dithmat[i][j]-8) * 256)/(7*15);
	    gmat[i][j] = ((dithmat[WRAP(i+2)][j]-8) * 256)/(7*15);
	    bmat[i][j] = ((dithmat[WRAP(i+1)][WRAP(j+2)]-8) * 256)/(3*15);
	}
}

randomize(buf,len)
short *buf;
int len;
{
    register int i, j;
    short temp;

    srand(getpid());
    for(i=0; i<len; i++) {
	j = (rand()>>3)%len;
	temp = buf[i];
	buf[i] = buf[j];
	buf[j] = temp;
    }
}

doquant(rbuf,gbuf,bbuf,obuf,n,max)
register unsigned short *rbuf, *gbuf, *bbuf, *obuf;
int n;
register int max;
{
    register short i;
    register int val;
    register int temp;

    for(i=0; i<n; i++ ) {
	temp = ((*rbuf++)*255)/max + (128/7);
	if (temp>255)
	    val = 7<<0;
	else if(temp>0) 
	    val = temp/(256/7);
	else
	    val = 0<<0;

	temp = ((*gbuf++)*255)/max + (128/7);
	if (temp>255)
	    val += 7<<3;
	else if(temp>0) 
	    val += (temp/(256/7))<<3;
	else
	    val += 0<<3;

	temp = ((*bbuf++)*255)/max;
	if (temp>(255-42))
	    val += 3<<6;
	else if(temp>(255-85-42)) 
	    val += 2<<6;
	else if(temp>(255-85-85-42)) 
	    val += 1<<6;
	else 
	    val += 0<<6;
	*obuf++ = val;
    } 
}

doordither(rbuf,gbuf,bbuf,obuf,n,max,rmat,gmat,bmat)
register unsigned short *rbuf, *gbuf, *bbuf, *obuf;
int n;
register int max;
short *rmat, *gmat, *bmat;
{
    register short i;
    register short val;
    register short temp;

    for(i=0; i<n; i++ ) {
	temp = ((*rbuf++)*255)/max + (128/7) + rmat[i&0x3];
	if (temp>255)
	    val = 7<<0;
	else if(temp>0) 
	    val = temp/(256/7);
	else
	    val = 0<<0;

	temp = ((*gbuf++)*255)/max + (128/7) + gmat[i&0x3];
	if (temp>255)
	    val += 7<<3;
	else if(temp>0) 
	    val += (temp/(256/7))<<3;
	else
	    val += 0<<3;

	temp = ((*bbuf++)*255)/max + bmat[i&0x3];
	if (temp>(255-42))
	    val += 3<<6;
	else if(temp>(255-85-42)) 
	    val += 2<<6;
	else if(temp>(255-85-85-42)) 
	    val += 1<<6;
	else 
	    val += 0<<6;
	*obuf++ = val;
    }
}

dodither(rbuf,gbuf,bbuf,obuf,n,max)
register unsigned short *rbuf, *gbuf, *bbuf, *obuf;
int n;
register int max;
{
    register short i;
    register int want, error;
    int rval, gval, bval;
    int rerror, rnext;
    int gerror, gnext;
    int berror, bnext;
    int temp;

    rerror = gerror = berror = 0;
    rnext = rerr[0] = 0;
    gnext = gerr[0] = 0;
    bnext = berr[0] = 0;
    for(i=0; i<n; i++ ) {
	    *rbuf = ((*rbuf)*(7*255))/max;
	    want = (*rbuf++)+rerror+rnext;
	    temp = (want+128)>>8;
	    if (temp>=7) {
		    error = want - (255*7);
		    rval = 7;
	    } else if(temp>0) {
		    error = want - (255*temp);
		    rval = temp;
	    } else {
		    error = want;
		    rval = 0;
	    }
	    rerror = (3*error)>>3;
	    rerr[i] += rerror;
	    rnext = rerr[i+1];
	    rerr[i+1] = error>>2;

	    *gbuf = ((*gbuf)*(7*255))/max;
	    want = (*gbuf++)+gerror+gnext;
	    temp = (want+128)>>8;
	    if (temp>=7) {
		    error = want - (255*7);
		    gval = 7<<3;
	    } else if(temp>0) {
		    error = want - (255*temp);
		    gval = temp<<3;
	    } else {
		    error = want;
		    gval = 0<<3;
	    }
	    gerror = (3*error)>>3;
	    gerr[i] += gerror;
	    gnext = gerr[i+1];
	    gerr[i+1] = error>>2;

	    *bbuf = ((*bbuf)*(3*255))/max;
	    want = (*bbuf++)+berror+bnext;
	    temp = (want+128)>>8;
	    if (temp>=3) {
		    error = want - (255*3);
		    bval = 3<<6;
	    } else if(temp == 2) {
		    error = want - (255*2);
		    bval = 2<<6;
	    } else if(temp == 1) {
		    error = want - (255*1);
		    bval = 1<<6;
	    } else {
		    error = want;
		    bval = 0<<6;
	    }
	    berror = (3*error)>>3;
	    berr[i] += berror;
	    bnext = berr[i+1];
	    berr[i+1] = error>>2;
	    *obuf++ = rval + gval + bval;
	}
}
