/*
 *	printer - 
 *		Device independent routines for driving various
 *	printers like the Versatec, Mitsubishi, and Tektronix.
 *
 *				Paul Haeberli - 1986
 */
#include "prt.h"
#include "math.h"

#define NOISEPATS	64
static char *imagename, *cmapname;

PRINTER *prtnew(xsize,ysize,ppi,argc,argv)
int xsize, ysize;
float ppi;
int argc;
char **argv;
{
    register PRINTER *p;
    float scale;
    IMAGE *image, *cmap;
    int i, y, max;
    short buf[8];
    int maplen;
    int donescale;

    if( argc<2 ) {
	fprintf(stderr,"usage: %s infile [colormap] [-s scale] [-p ppi] [-w]\n",argv[0]);
	exit(0);
    } 

/* get a new prt struct */
    p = (PRINTER *)malloc(sizeof(PRINTER));

    p->imgppi = 0.0;
    p->ppi = ppi;

    scale = 1.0;
    imagename = cmapname = 0;
    for(i=1; i<argc; i++) {
	if(argv[i][0] == '-') {
	    switch(argv[i][1]) {
		case 's':
		    i++;
		    scale = atof(argv[i]);
		    break;
		case 'p':
		    i++;
		    p->imgppi = atof(argv[i]);
		    break;
		case 'w':
		    i++;
		    p->forcewhite = 1;
		    break;
	    }
	} else {
	    if(!imagename)
		imagename = argv[i];
	    else if(!cmapname)
		cmapname = argv[i];
	}
    }
    if(!imagename) {
	fprintf(stderr,"usage: %s infile [colormap] [-s scale]\n",argv[0]);
	exit(0);
    } 


    if( (image=iopen(imagename,"r")) == NULL ) {
	fprintf(stderr,"%s: can't open input file %s\n",argv[0],imagename);
	exit(0);
    }

/* read in the color map if it is given */
    if(image->zsize<3 && cmapname) {
	if( (cmap=iopen(cmapname,"r")) == NULL ) {
	    fprintf(stderr,"%s: can't open cmap file %s\n",argv[0],cmapname);
	    exit(0);
	}
	if(cmap->xsize > 4) {
	    fprintf(stderr,"%s: wierd color map file %s\n",argv[0],cmapname);
	    exit(0);
	}
	maplen = 4096;
	if(image->max>maplen) 
	   maplen = image->max;
	p->rmap = (short *)malloc(sizeof(short)*maplen);
	p->gmap = (short *)malloc(sizeof(short)*maplen);
	p->bmap = (short *)malloc(sizeof(short)*maplen);
	p->bwmap = (short *)malloc(sizeof(short)*maplen);
	for(y=0; y<cmap->ysize; y++) {
	    getrow(cmap,buf,y,0);
	    p->rmap[buf[0]] = buf[1];
	    p->gmap[buf[0]] = buf[2];
	    p->bmap[buf[0]] = buf[3];
	    p->bwmap[buf[0]] = rgbtobw(buf[1],buf[2],buf[3]);
	}
	max = checkmax(p->rmap,cmap->ysize,0);
	max = checkmax(p->rmap,cmap->ysize,max);
	max = checkmax(p->rmap,cmap->ysize,max);
	image->max = max;
	p->maplen = maplen;
    } else 
        p->maplen = 0;

/* set oneband for black and white images only */
    if(!cmapname && image->zsize<3)
	p->oneband = 1;
    else
	p->oneband = 0;

/* set the printer size */
    p->xsource = image->xsize;
    p->ysource = image->ysize;

/* scale the printed image */
    if(scale>1.0 || scale < 0.01) 	
	scale = 1.0;
    p->scale = scale;
    p->xmaxprint = xsize * scale;
    p->ymaxprint = ysize * scale;
    p->image = image;

/* scale the image file to the printer */
    donescale = 0;
    if(p->imgppi>0.0) {
	p->xprint = (p->ppi*p->xsource)/p->imgppi;
	p->yprint = (p->ppi*p->ysource)/p->imgppi;
	if(p->xprint <= p->xmaxprint && p->yprint <= p->ymaxprint) 
	    donescale = 1;
    }
    if(!donescale) {
	if(p->xsource*p->ymaxprint > p->ysource*p->xmaxprint) {
	    p->xprint = p->xmaxprint;
	    p->yprint = (p->ysource*p->xprint)/p->xsource;
	    if(p->yprint>p->ymaxprint)
		p->yprint = p->ymaxprint;
	} else {
	    p->yprint = p->ymaxprint;
	    p->xprint = (p->xsource*p->yprint)/p->ysource;
	    if(p->xprint>p->xmaxprint)
		p->xprint = p->xmaxprint;
	}
    }

/* on lazerwriter, sizes will be given as 0 */
    if(xsize>0) {
	prtreadpattern(p->pat,
		       "/usr/lib/print/patterns/printer.pat",p->forcewhite);
	if(p->xprint < p->xsource || p->yprint < p->ysource) { 
	    p->xprint = p->xsource;
	    p->yprint = p->ysource;
	} 
    }
    p->bytewidth = 1+(p->xprint-1)>>3;
    return p;
}

prtreadpattern(pat,name,forcewhite)
char pat[8][64];
char *name;
int forcewhite;
{
    FILE *inf;
    int ipat[64];
    int *pos;
    char oneline[512];
    int n, lines;
    int i, x, y, val;

    inf = fopen(name,"r");
    if(!inf) {
	fprintf(stderr,"prtreadpattern: can't open pattern file %s\n",name);
	exit(0);
    }
    pos = ipat;
    while(fgets(oneline,512,inf)) {
       if(oneline[0] != '#') {
	    n = sscanf(oneline,"%d %d %d %d %d %d %d %d",pos,pos+1,pos+2,pos+3,
						      pos+4,pos+5,pos+6,pos+7);
	    if(n!=8) {
		fprintf(stderr,"prtreadpattern: wierdness reading pattern\n");
		exit(0);
	    }
	    pos += 8;
       }
    }
    for(i=0; i<64; i++) {
	for(y=0; y<8; y++) {
	    val = 0;
	    for(x=0; x<8; x++) {
		val <<= 1;
		if(ipat[8*y+x] < i)
		    val |= 1;
	    }
	    pat[y][i] = val;
	}
    }
    if(forcewhite) {
        for(y=0; y<8; y++) 
            pat[y][63] = 0xff;
    }
}

prtxscalerow(p,buf,expbuf)
PRINTER *p;
register short *buf;
register short *expbuf;
{
    register unsigned short ival; 
    register int x, xresamp;
    register int srcwidth, dstwidth;

    xresamp = 0; 
    srcwidth = p->xsource;
    dstwidth = p->xprint;
    if(dstwidth<srcwidth) {
	fprintf(stderr,"prtscalerow: bad craziness\n");
	exit(1);
    }
    x = srcwidth;
    while(1) {
	if(xresamp<=0) {
	    if(x-- == 0)
		return;
	    xresamp += dstwidth;
	    ival = (*buf++)>>2;
	}
	*expbuf++ = ival;
	xresamp -= srcwidth;
    }
}

static int noise[NOISEPATS+MAXIWIDTH];

static initnoise()
{
    register int n;
    register int *nptr;

    nptr = noise;
    for(n=(NOISEPATS+MAXIWIDTH); n--;) 
	*nptr++ = (rand()>>8)%4;
}

prtaddnoise(p,buf)
PRINTER *p;
register short *buf;
{
    register int len;
    register int *nptr;
    register int max;
    static int firsted = 0;

    if(p->forcewhite) {
	prtnormalize(p,buf);
	return;
    }
    if(!firsted) {
	initnoise();
	firsted++;
    }
    len = p->xsource;
    max = p->image->max;
    nptr = noise;
    nptr += ((rand()>>6)%NOISEPATS);
    while(len--) {
	if(len>8) {
	    *buf = (*nptr++) + (*buf*252)/max;
	    buf++;
	    *buf = (*nptr++) + (*buf*252)/max;
	    buf++;
	    *buf = (*nptr++) + (*buf*252)/max;
	    buf++;
	    *buf = (*nptr++) + (*buf*252)/max;
	    buf++;
	    *buf = (*nptr++) + (*buf*252)/max;
	    buf++;
	    *buf = (*nptr++) + (*buf*252)/max;
	    buf++;
	    *buf = (*nptr++) + (*buf*252)/max;
	    buf++;
	    *buf = (*nptr++) + (*buf*252)/max;
	    buf++;
	    len -= 7;
	} else {
	    *buf = (*nptr++) + (*buf*252)/max;
	    buf++;
	}
    }
}

prtnormalize(p,buf)
PRINTER *p;
register short *buf;
{
    register int len;
    register int max;

    len = p->xsource;
    max = p->image->max;
    while(len--) {
	if(len>8) {
	    *buf = (*buf*255)/max;
	    buf++;
	    *buf = (*buf*255)/max;
	    buf++;
	    *buf = (*buf*255)/max;
	    buf++;
	    *buf = (*buf*255)/max;
	    buf++;
	    *buf = (*buf*255)/max;
	    buf++;
	    *buf = (*buf*255)/max;
	    buf++;
	    *buf = (*buf*255)/max;
	    buf++;
	    *buf = (*buf*255)/max;
	    buf++;
	    len -= 7;
	} else {
	    *buf = (*buf*255)/max;
	    buf++;
	}
    }
}

prtgreyrow(rbuf,gbuf,bbuf,greybuf,n)
register short *rbuf, *gbuf, *bbuf, *greybuf;
register int n;
{
    register int max;

    while(n--) {
	max = *rbuf;
	if(*gbuf > max)
	    max = *gbuf;
	if(*bbuf > max)
	    max = *bbuf;
	rbuf++;
	gbuf++;
	bbuf++;
	*greybuf++ = max;
    }
}

prtgetrow(p,buf,y,z)
PRINTER *p;
short *buf;
int y, z;
{
    register int n;
    register short *sptr;
    register short *map;

    if(p->maplen == 0) {
	return getrow(p->image,buf,y,z);
    } else {
	getrow(p->image,buf,y,0);
	sptr = buf;
	switch(z) {
	    case 0:
		map = p->rmap;
		break;
	    case 1:
		map = p->gmap;
		break;
	    case 2:
		map = p->bmap;
		break;
	}
	for(n=p->xsource; n--;) {
	    if(n>8) {
		*sptr = map[*sptr]; 
		sptr++;
		*sptr = map[*sptr]; 
		sptr++;
		*sptr = map[*sptr]; 
		sptr++;
		*sptr = map[*sptr]; 
		sptr++;
		*sptr = map[*sptr]; 
		sptr++;
		*sptr = map[*sptr]; 
		sptr++;
		*sptr = map[*sptr]; 
		sptr++;
		*sptr = map[*sptr]; 
		sptr++;
		n -= 7;
	    } else {
		*sptr = map[*sptr]; 
		sptr++;
	    }
	}
    }
}

static short rbuf[MAXIWIDTH];
static short gbuf[MAXIWIDTH];
static short bbuf[MAXIWIDTH];

prtgetbwrow(p,buf,y)
PRINTER *p;
short *buf;
int y;
{
    register int n;
    register short *sptr;
    register short *map;

    if(p->maplen == 0) {		/* 1 or 3 band image */
    	if(p->oneband) {
	    getrow(p->image,buf,y,0);
	} else {
	    getrow(p->image,rbuf,y,0);	/* if 3 band, gotta make bw */
	    getrow(p->image,gbuf,y,1);
	    getrow(p->image,bbuf,y,2);
	    rgbrowtobw(rbuf,gbuf,bbuf,buf);
	}
    } else {				/* image with colormap */
	getrow(p->image,buf,y,0);
	map = p->bwmap;
	sptr = buf;
	for(n=p->xsource; n--;) {
	    if(n>8) {
		*sptr = map[*sptr]; 
		sptr++;
		*sptr = map[*sptr]; 
		sptr++;
		*sptr = map[*sptr]; 
		sptr++;
		*sptr = map[*sptr]; 
		sptr++;
		*sptr = map[*sptr]; 
		sptr++;
		*sptr = map[*sptr]; 
		sptr++;
		*sptr = map[*sptr]; 
		sptr++;
		*sptr = map[*sptr]; 
		sptr++;
		n -= 7;
	    } else {
		*sptr = map[*sptr]; 
		sptr++;
	    }
	}
    }
}

static checkmax(buf,len,max)
register short *buf;
register int len;
register int max;
{
    while(len--) {
	if(*buf>max)
	    max = *buf;
	buf++;
    }
    return max;
}

static rgbtobw(r,g,b)
{
    return (77*r + 150*g+ 28*b)>>8;
}

static rgbrowtobw(rbuf,gbuf,bbuf,obuf,n)
register unsigned short *rbuf, *gbuf, *bbuf, *obuf;
int n;
{
    register short i;
    int rval, gval, bval;
    int temp;

    for(i=n; i--; ) 
	    *obuf++ = (77*(*rbuf++) + 150*(*gbuf++) + 28*(*bbuf++))>>8;
}
