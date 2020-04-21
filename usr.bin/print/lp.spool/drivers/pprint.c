/*
 *	pprint -
 *		Print an image file on an adobe post script printer.
 *		This thing knows how to print it with either 1, 2, 4,
 *		or 8 bits per pixel. Postscript data is written to 
 *		standard out.
 *
 *				Paul Haeberli - 1986
 *
 */
#include "prt.h"

int hi[256], low[256];
short buf[MAXIWIDTH];
int maxval;

#define NEW 

#define HSIZE	528.0
#define VSIZE	700.0
#define MAX	255

main(argc,argv)
int argc;
char **argv;
{
    PRINTER *printer;

    maxval = 250;
    printer = prtnew(0,0,300.0,argc,argv);
    if(printer->forcewhite)
	maxval = 255;
    prtpass(printer);
}

prtpass(p)
PRINTER *p;
{
    pprint(p,8);
    putchar(4);
    fflush(stdout);
}

pprint(p,bitsper)
register PRINTER *p;
int bitsper;
{
    register IMAGE *image;
    int n, i;
    register int x, y;
    register int val;
    register int togo, pixperchar;
    int picstrlen;
    int leftover;
    float doscale, ppiscale;

    image = p->image;
    maketables();
    putchar('%');
    putchar('!');
    putchar('\n');
    printf("initgraphics\n");
    picstrlen = image->xsize*bitsper;
    picstrlen = (picstrlen+7)/8;
    printf("/picstr %d string def\n",picstrlen);

#ifdef NEW
    printf("45 750 translate\n");
    if(image->ysize/image->xsize < VSIZE/HSIZE) 
	doscale = HSIZE/image->xsize;
    else 
	doscale = VSIZE/image->ysize;
    if(p->imgppi > 0.0) {
	ppiscale = 72.0/p->imgppi;
	if(ppiscale<doscale)
	    doscale = ppiscale;
    }
    printf("%f %f scale\n",doscale*image->xsize,-doscale*image->ysize);
    printf("%f %f scale\n",p->scale,p->scale);
    printf("%d %d %d\n",image->xsize,image->ysize,bitsper);
    printf("[%d 0 0 -%d 0 %d]\n",image->xsize,image->ysize,image->ysize); 
#endif

#ifdef PORTRAIT
    printf("582 30 translate\n");
    printf("90 rotate\n");
    printf("%f %f scale\n",VSIZE,VSIZE*image->ysize/image->xsize);
    printf("%f %f scale\n",p->scale,p->scale);
    printf("%d %d %d\n",image->xsize,image->ysize,bitsper);
    printf("[%d 0 0 -%d 0 %d]\n",image->xsize,image->ysize,image->ysize); 
#endif

    printf("{ currentfile\n");
    printf("picstr readhexstring pop}\n");
    printf("image\n");
    for( y=0; y<image->ysize; y++ ) {
	prtgetbwrow(p,buf,y);
	prtnormalize(p,buf);
	switch(bitsper) {
	    case 1:
		x=0;
		for(n=2*picstrlen; n--; ) {
		    val = 0;
		    for(i=0; i<4; i++) {
			val <<= 1;
			buf[x] = (maxval*buf[x])/MAX;
			val |= (buf[x]&0x80) >> 7;
			x++;
		    }
		    psputchar("0123456789abcdef"[val]);
		}
		break;	
	    case 2:
		x=0;
		for(n=2*picstrlen; n--; ) {
		    val = 0;
		    for(i=0; i<2; i++) {
			val <<= 2;
			buf[x] = (maxval*buf[x])/MAX;
			val |= (buf[x]&0xc0) >> 6;
			x++;
		    }
		    psputchar("0123456789abcdef"[val]);
		}
		break;	
	    case 4:
		x=0;
		for(n=2*picstrlen; n--; ) {
		    val = (maxval*buf[x])/MAX;
		    val = (val&0xf0) >> 4;
		    x++;
		    psputchar("0123456789abcdef"[val]);
		}
		break;	
	    case 8:
		x=0;
		for(n=2*picstrlen; n--; ) {
		    val = (maxval*buf[x])/MAX;
		    val = val&0xff;
		    x++;
		    n--;
		    psputchar(hi[val]);
		    psputchar(low[val]);
		}
		break;
	    default:
		fprintf(stderr,"bits per pixel must be a power of 2!!\n");
		exit(1);
	}
    }
    printf("\nshowpage\n");
}

maketables()
{
    register int i;

    for(i=0; i<256; i++) {
	hi[i] = "0123456789abcdef"[i>>4];
	low[i] = "0123456789abcdef"[i&0xf];
    }
}

static int pos = 0;

psputchar(c)
int c;
{
    putchar(c);
    if(++pos == 50) {
	putchar('\n');
	pos = 0;
    }
}
