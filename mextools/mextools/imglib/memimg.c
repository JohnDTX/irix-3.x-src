/*
 *	memimg -
 *
 *			Rocky Rhodes and Paul Haeberli - 1985
 */
#ifdef NOTDEF
#include "gl2/globals.h"
#include "image.h"
#include "port.h"
#include "gl2/immed.h"
#endif
#include "image.h"
#include "port.h"

static unsigned short xmap[256];

MEMIMAGE *newimage(xsize,ysize,colormap)
int xsize, ysize, colormap;
{
    MEMIMAGE *mimage;

    mimage = (MEMIMAGE*)malloc(sizeof(MEMIMAGE));
    mimage->type = VERBATIM(2);
    mimage->colormap = colormap;
    mimage->xsize = xsize;
    mimage->ysize = ysize;
    mimage->pixels = 0;
    mimage->rowstart = 0;
    mimage->rowsize = 0;
    return mimage;
}

freeimage(mimage)
MEMIMAGE *mimage;
{
    if(mimage) {
	if(mimage->pixels)
	    free(mimage->pixels);
	if(mimage->rowstart)
	    free(mimage->rowstart);
	if(mimage->rowsize)
	    free(mimage->rowsize);
	free(mimage);
    }
}

#ifdef NOTDEF
drawrleimage(xorg,yorg,mimage)
register int xorg, yorg;
register MEMIMAGE *mimage;
{
    im_setup;
    register int y, x, count;
    register short *pixptr;
    register a, b, c;

    if(mimage->colormap != CM_SCREEN) {
printf("eeeek\n");
	img_makexmap(mimage->colormap);
	pixptr = mimage->pixels;
	for(y=0; y<mimage->ysize; y++)
	    while(count = *pixptr & 0x7f)
		if(*pixptr++ & 0x80) {
		    img_transtoscreen(pixptr,count);
		    pixptr += count;
		} else
		    img_transtoscreen(pixptr++,1);
        mimage->colormap = CM_SCREEN;
    }
    pushmatrix();
	img_setpixelortho();
	a = 512+2*mimage->ysize*sizeof(long);
	for(y=0,x=xorg; y<mimage->ysize; y++,x=xorg) {
	    b = mimage->rowstart[y]-a;
	    pixptr = mimage->pixels+b/2;
	    while(count = (*pixptr & 0x7f))
		if(*pixptr++ & 0x80) {
		    im_cmov2s(x, yorg+y);
		    writepixels(count, pixptr);
		    x += count;
		    pixptr += count;
		} else {
/*
		    move2s(x, yorg+y);
		    x += count - 1;
*/
		    im_do_color(*pixptr++);
		    im_screenclear(x,yorg+y,x+count-1,yorg+y);
		    x += count;
/*
		    draw2s(x++, yorg+y);
*/
		}
	}
    popmatrix();
}
#endif

drawimage(xorg,yorg,mimage)
register int xorg, yorg;
register MEMIMAGE *mimage;
{
    register int y;
    register short *pixptr;

    if(mimage->colormap != CM_SCREEN) {
	img_makexmap(mimage->colormap);
	pixptr = mimage->pixels;
	for(y=0; y<mimage->ysize; y++) {
	    img_transtoscreen(pixptr,mimage->xsize);
	    pixptr+=mimage->xsize;
	}
        mimage->colormap = CM_SCREEN;
    }

    pushmatrix();
	img_setpixelortho();
	pixptr = mimage->pixels;
	for(y=0; y<mimage->ysize; y++) {
	    cmov2i(xorg,yorg+y);
	    writepixels(mimage->xsize,pixptr);
	    pixptr+=mimage->xsize;
	}
    popmatrix();
}

MEMIMAGE *readimage(name)
char *name;
{
    char homepath[100];
    register int y;
    register short *pixptr;
    register IMAGE *image;
    MEMIMAGE *mimage;
    char *cptr;
    int xsize, ysize;

    cptr = (char *)getenv("HOME");
    if(!cptr) { 
	printf("drawimg: error in getenv\n");
	return;
    }
    image = iopen(name,"r");
    if(!image) {
	strcpy(homepath,cptr);
	strcat(homepath,"/.images/");
	strcat(homepath,name);
	image = iopen(homepath,"r");
	if(!image) {
	    printf("readimage: can't find image [%s] anywhere\n",name);
	    exit(0);
	}
    }
    xsize = image->xsize;
    ysize = image->ysize;
    mimage = newimage(xsize,ysize,image->colormap); 
    pixptr = mimage->pixels = (short *)malloc(xsize*ysize*sizeof(short));
    for(y=0; y<ysize; y++) {
	getrow(image,pixptr,y,0);
	pixptr+=mimage->xsize;
    }
    iclose(image);
    return mimage;
}

MEMIMAGE *readrleimage(name)
char *name;
{
    char homepath[100];
    register int y;
    register short *pixptr;
    register IMAGE *image;
    MEMIMAGE *mimage;
    char *cptr;
    int xsize, ysize;
    int pixeldata;

    cptr = (char *)getenv("HOME");
    if(!cptr) { 
	printf("drawimg: error in getenv\n");
	return;
    }
    image = iopen(name,"r");
    if(!image) {
	strcpy(homepath,cptr);
	strcat(homepath,"/.images/");
	strcat(homepath,name);
	image = iopen(homepath,"r");
	if(!image) {
	    printf("readimage: can't find image [%s] anywhere\n",name);
	    exit(0);
	}
    }
    if(!ISRLE(image->type)) {
	printf("hey!! don't try reading a non'rle image with this buster\n");
	exit(1);
    }
    xsize = image->xsize;
    ysize = image->ysize;
    mimage = newimage(xsize,ysize,image->colormap); 
    mimage->type = RLE(2);
    mimage->rowstart = (unsigned long *)malloc(ysize*sizeof(long));
    mimage->rowsize = (long *)malloc(ysize*sizeof(long));
    bcopy(image->rowstart,mimage->rowstart,ysize*sizeof(long));
    bcopy(image->rowsize,mimage->rowsize,ysize*sizeof(long));
    pixeldata = image->rowstart[image->ysize-1]+image->rowsize[image->ysize-1];
    pixeldata -= 512+2*image->ysize*sizeof(long);
    mimage->pixels = (short *)malloc(pixeldata);

    img_optseek(image, 512L + 2 * ysize * sizeof(long));
    img_read(image, mimage->pixels, pixeldata);
    iclose(image);
    return mimage;
}

img_transtoscreen(buf,n)
register unsigned short *buf;
int n;
{
    register short i;

    for(i=n; i--; )  {
	*buf = xmap[*buf];
	buf++;
    }
}

img_setpixelortho()
{
    short vx1, vx2, vy1, vy2;
    int dx, dy;

    getviewport(&vx1,&vx2,&vy1,&vy2);
    dx = ABS(vx1-vx2);
    dy = ABS(vy1-vy2);
    ortho2(-0.5,dx+0.5,-0.5,dy+0.5);
}

img_makexmap(colormap)
int colormap;
{
    register int r, g, b;
    register int i;

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
}
