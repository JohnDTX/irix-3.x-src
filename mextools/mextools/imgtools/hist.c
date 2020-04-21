/*
 *	hist - 
 *		Determine the histogram of an image file.
 *
 *				Paul Haeberli - 1984
 *
 */
#include "port.h"
#include "image.h"
#include "gl.h"
#include "device.h"

#define NBUCKETS 100

short rowbuf[4096];
int bucket[NBUCKETS];
unsigned int min, max, bmax;
unsigned int delta;

main(argc,argv)
int argc;
char **argv;
{
    register IMAGE *image;
    register unsigned int i, val, index;
    short tempval;
    int y, z;

    if( argc<2 ) {
	fprintf(stderr,"usage: hist infile\n");
	exit(0);
    } 
    if( (image=iopen(argv[1],"r")) == NULL ) {
	fprintf(stderr,"hist: can't open input file %s\n",argv[1]);
	exit(0);
    }
    min = image->min;
    max = image->max;
    delta = max - min;
    if(delta<=0) {
	fprintf(stderr,"hist: wierd min and max vals\n");
	exit(0);
    }
    keepaspect(1,1);
    winopen("hist");
    settitle(argv[1],min,max);
    for(i=0; i<NBUCKETS; i++)
	bucket[i] = 0;
    for(z=0; y<image->zsize; z++) 
	for(y=0; y<image->ysize; y++) {
	    getrow(image,rowbuf,y,z);
	    addtohist(rowbuf,image->xsize,min,delta);
	}
    bmax = 0;
    for(i=0; i<NBUCKETS; i++) 
	if(bucket[i]>bmax)
	    bmax = bucket[i];

    showhist();
    while(1) {
	if(qread(&tempval) == REDRAW) {
	    showhist();
	}
    }
}

showhist()
{
    int i;

    reshapeviewport();
    color(GREY(2));
    clear();
    if(max<=255) {
	ortho2(0.0,256.0,0.0,(float)bmax);
	translate((float)min,0.0,0.0);
	scale(delta/NBUCKETS.0,1.0,0.0);
	for(i=0; i<NBUCKETS; i++)  {
	    color(GREY(7));
	    rectf(i+0.1,0.0,i+0.9,(float)bucket[i]);
	}
    } else {
	ortho2(-NBUCKETS/2.0,NBUCKETS/2.0,-max/2.0,max/2.0);
	translate(-NBUCKETS/2.0,-max/2.0,0.0);
	for(i=0; i<NBUCKETS; i++)  {
	    color(GREY(7));
	    rectf(i+0.1,0.0,i+0.9,(float)bucket[i]);
	}
    }
}


addtohist(buff,n,min,delta)
unsigned short *buff;
int n;
register int min;
register unsigned int delta;
{
    register short i;
    register unsigned short *sptr;
    register unsigned short index;

    sptr = (unsigned short *)buff;
    for(i=n; i--; )  {
	index = ((*sptr++-min)*NBUCKETS)/delta;
	if(index>=NBUCKETS) index = NBUCKETS-1;
	bucket[index]++;
    }
}

settitle(name,min,max)
char *name;
int min, max;
{
    char oneline[256];

    sprintf(oneline,"hist %s min: %d max: %d\n",name,min,max);
    wintitle(oneline);
}
