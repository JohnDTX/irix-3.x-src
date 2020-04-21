/*
 *	fed - 
 *		A simple font editor.  I hope this is better than nothing.
 *
 *				Paul Haeberli 1984
 */
#include "gl.h"
#include "device.h"
#include "port.h"
#include "ctype.h"
#include "stdio.h"
#include "rect.h"

#define NCHARS 		128
#define CHARWIDTH 	8

#define XDIM		(NCHARS*CHARWIDTH)
#define YDIM		20

#define BXMIN		8
#define BXMAX		407
#define BYMIN		50
#define BYMAX		209

#define PXMIN		8
#define PXMAX		407
#define PYMIN		16
#define PYMAX		36
	
#define DXMIN		0
#define DXMAX		415
#define DYMIN		0
#define DYMAX		237
	
#define BITON		GREY(2)
#define BITOFF		GREY(15)

#define MARK		23

int mmid, mleft;
int mx, my;
int curcenter, curchar;
int dwidth, dheight;
int pen;
int menu;
short ht, nc, nr; 	/* for the current font */
Fontchar chars[256];
short raster[256*256];
int charwidth;
Rect WIDTHrect, HEIGHTrect, CURCHARrect;
char bits[YDIM][XDIM];
short shorts[XDIM];
int xorg, yorg;
int outwidth = 0;
char filename[80];
char title[128];

main(argc,argv)
int argc;
char **argv;
{
    short dev, val;

    if (argc<2) {
	fprintf(stderr,"usage: fed <iris.fnt>\n");
	exit(0);
    }
    if (argc>2) 
	outwidth = atoi(argv[2]);
    WIDTHrect = rectarea(8.0,8.0+100.0,214.0,229.0);
    HEIGHTrect = rectarea(407.0-100.0,407.0,214.0,229.0);
    CURCHARrect = rectarea(157.0,258.0,214.0,229.0);

    strcpy(filename,argv[1]);
    readeditfont();

    prefsize(DXMAX,DYMAX);
    winopen("fed");
    sprintf(title,"fed %s",filename);
    wintitle(title);
    menu = defpup("fed %t|height|width|write");
    dwidth = BXMAX-BXMIN+1;
    dheight = BYMAX-BYMIN+1;
    curcenter = 4+(65*CHARWIDTH);
    makeframe();
    qdevice(MIDDLEMOUSE);
    qdevice(LEFTMOUSE);
    qdevice(MENUBUTTON);
    qdevice(MOUSEX);
    qdevice(MOUSEY);
    qdevice(LEFTARROWKEY);
    qdevice(RIGHTARROWKEY);
    mx = my = -1;
    color(7);
    while (1) {
	do { 
	    dev = qread(&val);
	    switch (dev) {
		case MENUBUTTON: 
		    if (val) {
			if (dopup(menu) == 3)
			    writeeditfont();
		     }
		     break;
		case MIDDLEMOUSE: 
		    mmid = val;
		    if (val) 
			newpos();
		    break;
		case LEFTMOUSE: 
		    mleft = val;
		    if (val) {
			if(pixelvalue())
			    pen = 0;
			else
			    pen = 1;
		    }
		    break;
		case MOUSEX: 
		    mx = val-xorg;
		    break;
		case MOUSEY: 
		    my = val-yorg;
		    break;
		case LEFTARROWKEY:
		    if(val) {
			ht--;
			showsize();
		    }
		    break;
		case RIGHTARROWKEY:
		    if(val) {
			ht++;
			showsize();
		    }
		    break;
		case REDRAW: 
		    reshapeviewport();
		    makeframe();
		    break;
	    }
	} while(qtest());
	if (mleft)
	    changepixel();
    }
}

drawborder()
{
    color(GREY(10));
    rectfi(DXMIN,DYMIN,DXMAX,DYMAX);
    color(GREY(12));
    recti(DXMIN  ,DYMIN  ,DXMAX  ,DYMAX  );
    recti(DXMIN+1,DYMIN+1,DXMAX-1,DYMAX-1);
    recti(BXMIN-2,BYMIN-2,BXMAX+2,BYMAX+2);
    recti(BXMIN-3,BYMIN-3,BXMAX+3,BYMAX+3);
    recti(PXMIN-2,PYMIN-2,PXMAX+2,PYMAX+2);
    recti(PXMIN-3,PYMIN-3,PXMAX+3,PYMAX+3);
    gflush();
}

newpos()
{
    register int x, y;
    register int changex;
    register int ocurcenter;

    x = getxblock();
    y = getyblock();
    ocurcenter = curcenter;
    if (blockisonscreen(x,y)) {
	changex = x-(50-CHARWIDTH)/2;
	changex -= (changex+(1000*CHARWIDTH))%CHARWIDTH;
	curcenter += changex;
    }
    if (mouseisinpixelport()) {
	changex = mx-(PXMAX+PXMIN-CHARWIDTH)/2;
	changex -= (changex+(1000*CHARWIDTH))%CHARWIDTH;
	curcenter += changex;
    }
    if (curcenter<4)
	curcenter = 4;
    if (curcenter>CHARWIDTH*NCHARS)
	curcenter=(CHARWIDTH*NCHARS);
    if (curcenter != ocurcenter)
	drawbits();
}

pixelvalue()
{
    int xindex;
    int x, y;

    x = getxblock();
    y = getyblock();
    if (blockisonscreen(x,y)) {
	xindex = x + (curcenter-50/2); 
	if (xindex>=0 && xindex<NCHARS*CHARWIDTH) 
  	     return bits[y][xindex];
	else
  	     return 0;
    } else
	return 0;
}

changepixel()
{
    short val;
    int xindex;
    int x, y;
    int sx, sy;

    x = getxblock();
    y = getyblock();
    sx = xblocktoscreen(x);
    sy = yblocktoscreen(y);
    if (blockisonscreen(x,y)) {
	xindex = x + (curcenter-50/2); 
	if (xindex>=0 && xindex<NCHARS*CHARWIDTH) {
	    if (pen)  {
		bits[y][xindex] = 1;
		color(BITON);
		val = 0;
	    } else {
		bits[y][xindex] = 0;
		color(BITOFF);
		val = 7;
	    }
	    rectfi(sx+1,sy+1,sx+6,sy+6);
	    cmov2i((PXMIN+PXMAX)/2+x-50/2,PYMIN+y);
	    writepixels(1,&val);
	}
    }
}

drawbits()
{
    int x, y;
    int sx, sy;
    int xindex;

    curchar = (curcenter-4)/CHARWIDTH;
    showsize();
    drawwherestr(curchar);

    color(BITOFF);
    rectfi(BXMIN-1,BYMIN-1,BXMAX+1,BYMAX+1);
    color(GREY(0));
    move2i(BXMIN,BYMIN+55);
    draw2i(BXMAX,BYMIN+55);
    move2i(1+(BXMIN+BXMAX+8*CHARWIDTH)/2,BYMIN);
    draw2i(1+(BXMIN+BXMAX+8*CHARWIDTH)/2,BYMAX);
    move2i(1+(BXMIN+BXMAX-8*CHARWIDTH)/2,BYMIN);
    draw2i(1+(BXMIN+BXMAX-8*CHARWIDTH)/2,BYMAX);
    rectfi(PXMIN-1,PYMIN-1,PXMAX+1,PYMAX+1);
    color(BITON);
    for (y=0; y<YDIM; y++) {
	sy = ((dheight*y)/20)+BYMIN; 
	for (x=0; x<50; x++) {
	    sx = ((dwidth*x)/50)+BXMIN; 
	    xindex = x+curcenter-50/2;
	    if (xindex>=0 && xindex<NCHARS*CHARWIDTH)
		if (bits[y][xindex])
		    rectfi(sx+1,sy+1,sx+6,sy+6);
   	}
    }
    for (x=5; x<50; x+=8) {
	sx = ((dwidth*x)/50)+BXMIN; 
	xindex = x+curcenter-50/2;
	if (xindex>=0 && xindex<NCHARS*CHARWIDTH) {
	    pushmatrix();
		translate((float)sx,0.0,0.0);
		drawmark();
	    popmatrix();
	}
    }
    for (y=0; y<YDIM; y++) {
	cmov2i(PXMIN,PYMIN+y);
	xformrow(PXMAX-PXMIN,curcenter-(PXMAX-PXMIN)/2,y,shorts);
	writepixels(PXMAX-PXMIN,shorts);
    }
}

drawwherestr(curchar)
int curchar;
{
    char tempstr[10];

    if (isprint(curchar))
	sprintf(tempstr,"%d | '%c'",curchar,curchar);
    else
	sprintf(tempstr,"%d",curchar);
    strinrect(tempstr,CURCHARrect);
}

xformrow(n,x,y,sptr)
int n, x, y;
short *sptr;
{
    register char *cptr;
    register int i;

    cptr = &bits[y][x];
    for (i=0; i<n; i++)  {
	if (x>=0 && x<NCHARS*CHARWIDTH) {
	    if (*cptr++)
		*sptr++ = 0;
	    else
		*sptr++ = 7;
        } else {
	    cptr++;
	    *sptr++ = 7;
	}
	x++;
    }
}

blockisonscreen(x,y)
int x, y;
{
    return (x>=0 && x<50 && y>=0 && y<20);
}

mouseisinpixelport()
{
    return (mx>=PXMIN && mx<=PXMAX && my>=PYMIN && my<=PYMAX);
}

getxblock()
{
    return (50 * (mx-BXMIN))/dwidth;
}

getyblock()
{
    return (20 * (my-BYMIN))/dheight;
}

xblocktoscreen(x)
int x;
{
    return ((dwidth*x)/50)+BXMIN; 
}

yblocktoscreen(y)
int y;
{
    return ((dheight*y)/20)+BYMIN; 
}

tobyte(bits)
char *bits;
{
    register int i;
    register int byte;

    byte = 0;
    for (i=0; i<8; i++) 
	if (bits[i])
	    byte = (byte<<1) + 1; 
	else
	    byte = byte<<1; 
    return byte;
}

tobits(val,bits)
int val;
char *bits;
{
    register int i;
    register int mask;

    mask = 0x80;
    for (i=0; i<8; i++)  {
	if (val&mask)
	    bits[i] = 1;
	else
	    bits[i] = 0;
	mask = mask >> 1;
    }
}

readeditfont()
{
    register int i, j;
    int offset,h,yoff;
    int bitindex;
    int f;

    f = open(filename,0);  
    if (f < 0) {
	fprintf(stderr,"fed: couldn't open input file %s\n",filename);
	exit(0);
    }
    readfont(f,&ht,&nc,chars,&nr,raster);
    offset = 0;
    for (i=0; i<nc; i++) {
	h = chars[i].h;
	yoff = chars[i].yoff+7;
	bitindex = i*CHARWIDTH;
	for (j=0; j<yoff; j++)
	    tobits(0,&bits[j][bitindex]);
	for (; j<YDIM; j++){
	    tobits((raster[offset++]>>8) & 0xff,&bits[j][bitindex]);
	    if (--h == 0) {
		j++;
		break;
	    }
	}
	for (; j<YDIM; j++)
	    tobits(0,&bits[j][bitindex]);
    }
    charwidth = chars['n'].width;
    if (outwidth)
	charwidth = outwidth;
    close(f);
}

writeeditfont()
{
    int i, j, k;
    int offset, ymin, ymax;
    int f;

    f = open(filename,1);  
    offset = 0;
    for (i=0; i<NCHARS; i++) {
	for (j=0; j<YDIM; j++){
	    if (tobyte(&bits[j][i*CHARWIDTH]) != 0)
		break;
	}
	ymin = j;
	for (j=YDIM-1; j>0; j--) {
	    if (tobyte(&bits[j][i*CHARWIDTH]) != 0)
		break;
	}
	ymax = j;
	chars[i].offset = offset;
	chars[i].w = charwidth;
	if (ymin<=ymax) {
	    chars[i].h = ymax-ymin+1;
	    chars[i].xoff = 0;
	    chars[i].yoff = ymin-7;
	    chars[i].width = charwidth;
	} else {
	    chars[i].h = 1;
	    chars[i].xoff = 0;
	    chars[i].yoff = 0;
	    chars[i].width = charwidth;
	    ymax = ymin = 1;
	}

	for (j=ymin; j<=ymax; j++){
	    raster[offset] = tobyte(&bits[j][i*CHARWIDTH]);
	    raster[offset] <<= 8;
	    offset++;
	}
    }
    nr = offset;
    writefont(f,ht,nc,chars,nr,raster);
    close(f);
}

makeframe()
{
    getorigin(&xorg,&yorg);
    ortho2(-0.5,DXMAX-0.5,-0.5,DYMAX-0.5);
    drawborder();
    drawbits();
}

clearbits()
{
    register int x, y;

    for (y=0; y<YDIM; y++)
	for (x=0; x<XDIM; x++)
	    bits[y][x] = 0;
}

showsize()
{
    char tempstr[20];

    sprintf(tempstr,"Width: %d",charwidth);
    strinrect(tempstr,WIDTHrect);
    sprintf(tempstr,"Height: %d",ht);
    strinrect(tempstr,HEIGHTrect);
}

strinrect(str,r)
char str[];
Rect r;
{
    int width;

    width = strwidth(str);
    color(GREY(14));
    rectf(r.origin.x,r.origin.y,r.corner.x,r.corner.y);
    color(GREY(1));
    rect(r.origin.x,r.origin.y,r.corner.x,r.corner.y);
    cmov2((r.origin.x+r.corner.x-width)/2.0,r.origin.y+4.0);
    charstr(str);
}

drawmark()
{
	color(GREY(2));
	move2i(-3,2+BYMIN+0);
	draw2i( 3,2+BYMIN+0);
	draw2i( 0,2+BYMIN+1);
	draw2i( 0,2+BYMIN+8);
	draw2i( 0,2+BYMIN+1);
	draw2i(-3,2+BYMIN+0);
}
