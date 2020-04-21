/*
 *	imged - 
 *		A minimal image editor.
 *
 *				Paul Haeberli - 1985
 *
 */
#include "image.h"
#include "gl.h"
#include "device.h"
#include "port.h"

#define FAT	1
#define THIN	2

short *rowdat[2048];
int xdim, ydim;
short xmap[256];
int drawmode = THIN;
int menu;
int curcolor;
int xorg, yorg;
int xsize, ysize;
int inside = 0;
int blksize = 4;

main(argc,argv)
int argc;
char **argv;
{
    register int i;
    short val;
    int px, py;
    int mx, my;

    if( argc<2 ) {
	fprintf(stderr,"usage: imged <infile> [xsize ysize]\n");
	exit(1);
    } 
    if(argc>=4) {
	xdim = atoi(argv[2]);
	ydim = atoi(argv[3]);
	initsize(xdim,ydim);
    } else 
	readit(argv[1],0,0);
    wintitle("imged");
    menu = defpup("imged %t|fat|thin|clear|rect|write");
    qdevice(LEFTMOUSE);
    qdevice(MIDDLEMOUSE);
    qdevice(MENUBUTTON);
    makeframe();
    while(1) {
	switch(qread(&val)) {
	    case REDRAW:
		makeframe();
		break;
	    case LEFTMOUSE:
		if(val) {
		    while(getbutton(LEFTMOUSE)) {
			mx = getvaluator(MOUSEX);
			my = getvaluator(MOUSEY);
			if(insideport(mx,my)) {
			    px = (mx - xorg)/blksize;
			    px = MIN(px,xdim-1);
			    px = MAX(px,0);
			    py = (my - yorg)/blksize;
			    py = MIN(py,ydim-1);
			    py = MAX(py,0);
			    changepix(px,py,curcolor);
			}
		    }
		}
		break;
	    case MIDDLEMOUSE:
  		if(val) {
		    mx = getvaluator(MOUSEX);
		    my = getvaluator(MOUSEY);
		    curcolor = getapixel(mx,my);
		}
		break;
	    case MENUBUTTON:
		if(val) {
		    switch(dopup(menu)) {
			case 1:
			    if(drawmode != FAT) {
				drawmode = FAT;
				makeframe();
			    }
			    break;
			case 2:
			    if(drawmode != THIN) {
				drawmode = THIN;
				makeframe();
			    }
			    break;
			case 3:
			    clearit();
			    drawit();
			    break;
			case 4:
			    break;
			case 5:
			    writeit(argv[1]);
			    break;
		    }
		}
		break;
	}
    }
}

insideport(x,y)
int x, y;
{
    if(x<xorg)
	return 0;
    if(x>(xorg+xsize))
	return 0;
    if(y<yorg)
	return 0;
    if(y>(yorg+ysize))
	return 0;
    return 1;
}

initsize(xs,ys)
int xs, ys;
{
    int y;

    xdim = MIN(xs,XMAXSCREEN/blksize);
    ydim = MIN(ys,YMAXSCREEN/blksize);
    for(y=0; y<ydim; y++) 
	rowdat[y] = (short *)malloc(sizeof(short)*xs);
    fudge(1,1);
    stepunit(xs,ys);
    minsize(2*xs,2*ys);
    keepaspect(xs,ys);
    winopen("imged");
    color(GREY(8));
    clear();
}

readit(filename)
char *filename;
{
    register IMAGE *image;
    register int r, g, b;
    register int i, y;
    int colormap;

    if( (image=iopen(filename,"r")) == NULL ) {
	fprintf(stderr,"imged: can't open input file %s\n",filename);
	exit(1);
    }
    xdim = image->xsize;
    ydim = image->ysize;
    initsize(xdim,ydim);
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
    for(y=0; y<ydim; y++) {
	getrow(image,rowdat[y],y,0);
	if(colormap != CM_SCREEN)
	    compress(rowdat[y],xdim);
    }
    iclose(image);
}

writeit(filename)
char *filename;
{
    register IMAGE *oimage;
    register int y;

    oimage = iopen(filename,"w",RLE(2),2,xdim,ydim,0);
    if( oimage == NULL ) {
	fprintf(stderr,"imged: can't create output file %s\n",filename);
	exit(0);
    }
    oimage->colormap = CM_SCREEN;
    isetname(oimage,filename);
    for(y=0; y<ydim; y++) 
	putrow(oimage,rowdat[y],y,0);
    iclose(oimage);
    percentdone(100.0);
}

makeframe()
{
    reshapeviewport();
    getorigin(&xorg,&yorg);
    getsize(&xsize,&ysize);
    ortho2(-0.5,xsize-0.5,-0.5,ysize-0.5);
    color(0);
    clear();
    drawit();
}

drawit()
{
    register int x, y;
    register int xo, yo;
    register short *sptr;
    register int bs;

    blksize = xsize/xdim;
    if(drawmode == FAT) 
	bs = blksize-1;
    else
	bs = blksize-2;
    yo = 1;
    for(y=0; y<ydim; y++) {
	xo = 1;
	sptr = rowdat[y];
	for(x=0; x<xdim; x++) {
	    color(*sptr++);
	    rectfi(xo,yo,xo+bs,yo+bs);
	    xo+=blksize; 
	}
	yo+=blksize;
    }
}

changepix(x,y,c)
int x, y, c;
{
    register int xo, yo;
    register short bs;

    rowdat[y][x] = c;
    xo = 1+blksize*x;
    yo = 1+blksize*y;
    color(c);
    if(drawmode == FAT) 
	bs = blksize-1;
    else
	bs = blksize-2;
    rectfi(xo,yo,xo+bs,yo+bs);
}

compress(buf,n)
register unsigned short *buf;
int n;
{
    register short i;

    for(i=n; i--; ) {
	*buf = xmap[*buf];
	*buf++;
    }
}

clearit()
{
    register int x, y;
    register short *sptr;

    for(y=0; y<ydim; y++) {
	sptr = rowdat[y];
	for(x=0; x<xdim; x++) 
	    *sptr++ = curcolor;
    }
}
