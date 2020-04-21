#ifdef	SHRINK
/*
 *	ipaste - 
 *		Display an image on the iris.
 *
 *				Paul Haeberli - 1984	
 *
 */
#include "/usr/people/gifts/mextools/include/image.h"
#include "gl.h"
#include "device.h"
#include "gsh.h"
#include "/usr/people/gifts/mextools/include/port.h"

int colormap;

short rowbuf[2048]; 
short *rowdat[2048];
short factor = 1;
unsigned short xmap[256];
int img_xsize, img_ysize;
int xdim, ydim;
char noicon;

img_readit(filename)
{
    register IMAGE *image;
    register int r, g, b;
    register int i, y;

    if( (image=iopen(filename,"r")) == NULL ) {
	char newname[500];

	/* try filename.icon */
	sprintf(newname, "%s.icon", filename);
	if ( (image = iopen(newname, "r")) == NULL ) {

	    /* try ICONLIB/filename */
	    sprintf(newname, "%s/%s", ICONLIB, filename);
	    if ( (image = iopen(newname, "r")) == NULL ) {

		/* try ICONLIB/filename.icon */
		sprintf(newname, "%s/%s.icon", ICONLIB, filename);
		if ( (image = iopen(newname, "r")) == NULL ) {
		    fprintf(stderr, "gsh: can't open icon file %s\n",filename);
		    noicon = 1;
		    return;
		}
	    }
	}
    }
    xdim = image->xsize;
    ydim = image->ysize;
    img_xsize = factor*xdim;
    img_ysize = factor*ydim;


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
        getrow(image,rowbuf,y,0);
        rowdat[y] = (short *)malloc(sizeof(short)*img_xsize);
        img_compress(rowdat[y],rowbuf,xdim);
        if(factor>1)
	    img_expand(rowdat[y],xdim,factor);
    }
    iclose(image);
}

img_drawit()
{
    int i, y;

    img_makeframe();
    for(y=0; y<ydim; y++) {
        for(i=0; i<factor; i++) {
	    cmov2i(0,(factor*y)+i);
	    writepixels(img_xsize,rowdat[y]);
        }
    }
}

img_compress(obuff,ibuff,n)
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

img_expand(shortbuf,n,factor)
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

img_makeframe()
{
    reshapeviewport();
    viewport(0,img_xsize-1,0,img_ysize-1);
    ortho2(-0.5,(float)img_xsize-0.5,-0.5,(float)img_ysize-0.5);
}
#endif
