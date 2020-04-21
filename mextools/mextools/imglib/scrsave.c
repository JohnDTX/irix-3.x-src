#include <stdio.h>
#include "gl.h"
#include "image.h"
#include "port.h"

/* scrsave -- save a rectangular region of the screen into a file.
 *	saves screen of any mode, RGB, single and double buffer,
 *	although it will save the back buffer in double buffer mode.
 *	name is filename to save into.
 *	cmap is flag about saving the color map as well (into name.map,
 *		ignored in RGB).
 *	left,right,bottom,top specify the rectangle to save.
 *
 *	Returns 0 on success, non-zero on failure.
 */
scrsave(name,cmap,left,right,bottom,top)
    char *name;
    long cmap;
    Screencoord left,right,bottom,top;
{
    IMAGE *image;
    char *fname;
    int res = 0;

    if(left > right || bottom > top ||
	    left < 0 || right > XMAXSCREEN ||
	    bottom < 0 || top > YMAXSCREEN) {
	fprintf(stderr,"scrsave: immposible screen coordinates\n");
	return -1;
    }
    if(getdisplaymode() == 0) {			/* RGB mode */
	if(strlen(name) == 0)
	    fname = "image.rgb";
	else
	    fname = name;
	res = savergb(fname,left,right,bottom,top);
    } else {
	if(strlen(name) == 0)
	    fname = "image.sc";
	else
	    fname = name;
	res = savescreen(fname,left,right,bottom,top);
	if(cmap) res |= savemap(fname);
    }
    return res;
}


/* FORTRAN version */
fortran 
scrsav(top,bottom,right,left,cmap,len,name)
    char *name;
    long len;
    long *cmap;
    long *left,*right,*bottom,*top;
{
    register char *newstr;
    register char *to;

    if (len > 0) {
	newstr = (char *)malloc(1+len);
	if (!newstr) {
	    gl_outmem("scrsav");
	    return;
	}
	to = newstr;
	while (len > 0) {
	    *to++ = *name++;
	    len--;
	}
	*to = 0;
    }
    else
	newstr = 0;
    scrsave(newstr,*cmap,*left,*right,*bottom,*top);
}

static
savescreen(name,left,right,bottom,top)
    char *name;
    Screencoord left,right,bottom,top;
{
    IMAGE *image;
    long i, y;
    Colorindex buff[1024];
    long xsize, ysize;


    xsize = right - left + 1;
    ysize = top - bottom + 1;

    if((image = iopen(name,"w",RLE(2),2,xsize,ysize,0)) == NULL){
	fprintf(stderr,"scrsave: couldn't open file '%s'\n",name);
	return -1;
    }
    isetname(image,name);
    isetcolormap(image,CM_SCREEN);

    pushviewport();
    pushmatrix();
    viewport(0,1023,0,767);
    ortho2(-0.5,1023.5,-0.5,767.5);
    for(y=bottom; y<=top; y++) {
	cmov2i(left,y);
	i = readpixels(xsize,buff);
	putrow(image,buff,y-bottom,0);
    }
    popmatrix();
    popviewport();

    iclose(image);
    return 0;
}

static
savergb(name,left,right,bottom,top)
    char *name;
    Screencoord left,right,bottom,top;
{
    IMAGE *image;
    long i, y;
    Colorindex buff[1024];
    RGBvalue rbuff[1024],gbuff[1024],bbuff[1024];
    long xsize, ysize;


    xsize = right - left + 1;
    ysize = top - bottom + 1;

    if((image = iopen(name,"w",RLE(1),3,xsize,ysize,3)) == NULL){
	fprintf(stderr,"scrsave: couldn't open file '%s'\n",name);
	return -1;
    }
    isetname(image,name);
    isetcolormap(image,CM_NORMAL);

    pushviewport();
    pushmatrix();
    viewport(0,1023,0,767);
    ortho2(-0.5,1023.5,-0.5,767.5);
    for(y=bottom; y<=top; y++) {
	cmov2i(left,y);
	i = readRGB(xsize,rbuff,gbuff,bbuff);
	cvtbuf(buff,rbuff,i); putrow(image,buff,y-bottom,0);
	cvtbuf(buff,gbuff,i); putrow(image,buff,y-bottom,1);
	cvtbuf(buff,bbuff,i); putrow(image,buff,y-bottom,2);
    }
    popmatrix();
    popviewport();

    iclose(image);
    return 0;
}

static
cvtbuf(dest,src,len)
    register Colorindex * dest;
    register RGBvalue * src;
    register int len;
{
    while(len-- > 0) *dest++ = *src++;
}

static
savemap(name)
    char *name;
{
    register IMAGE *image;
    register int i, max;
    unsigned short r, g, b;
    char mapname[15];			/* unix stikes again */

    strncpy(mapname,name,10);
    mapname[10] = '\0';
    strcat(mapname,".map");

    max = (1<<getplanes());
    if( (image=iopen(mapname,"w",VERBATIM(2),2,4,max)) == NULL ) {
	fprintf(stderr,"savemap: can't open map file '%s'\n",mapname);
	return -1;
    }
    isetname(image,mapname);
    isetcolormap(image,CM_COLORMAP);

    for(i=0; i<max; i++) {
	gamgetmcolor(i,&r,&g,&b);
	putpix(image,i);
	putpix(image,r);
	putpix(image,g);
	putpix(image,b);
    }
    iclose(image);
    return 0;
}

