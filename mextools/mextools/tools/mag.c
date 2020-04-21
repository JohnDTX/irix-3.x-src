/*
 *	mag - 
 *		Magnify pixels on the screen by some factor.  You can use
 *		mag <factor> to magnify by higher powers.
 *	
 *				Paul Haeberli - 1984
 *
 */
#include "gl.h"
#include "device.h"
#include "port.h"

#define NOSRC	(-1)

Cursor glass = {
	0x0002, 0x0007, 0x000F, 0x041E, 
	0x3B3C, 0x40F8, 0x4030, 0x8420, 
	0x8410, 0x8410, 0x9F10, 0x8430, 
	0xC420, 0x4060, 0x71C0, 0x1F00
};

short *pixels;
short *lastpixels;
short *temp;
int factor = 6;
int dstxsize, dstysize;
int dstxorg, dstyorg;
int curx = NOSRC;
int cury = NOSRC;
int blacklines = 1;
int shaded = 0;
int menu;

#define GRIDCOLOR	0

main(argc,argv)
int argc;
char **argv;
{
    pixels = (short *) malloc(sizeof(short)*2048);
    lastpixels = (short *) malloc(sizeof(short)*2048);
    if (argc>1)
	factor = atoi(argv[1]);
    if (factor<1) 
	factor = 1;
    winopen("mag");
    wintitle("mag");
    defcursor(1,glass);
    curorigin(1,5,10);
    setcursor(1,RED,0xfff);
    menu = defpup("mag %t|grid off|grid on|interp|flat|mag");
    qdevice(MENUBUTTON);
    qdevice(MIDDLEMOUSE);
    qdevice(LEFTMOUSE);
    makeframe(1);
    while (1) {
	getloc(&curx,&cury);
	readit(curx,cury);
    }
}

readit(x,y)
int x, y;
{
    int srcxsize, srcysize;
    int i;
    int srcxorg, srcyorg;
    float xtrans, ytrans;

    srcxsize = dstxsize/factor+1; 
    srcxorg = x-srcxsize/2; 
    if (srcxorg<0) srcxorg = 0;

    srcysize = dstysize/factor+1; 
    srcyorg = y-srcysize/2;
    if (srcyorg<0) srcyorg = 0;

    screenspace();
    if (factor<4) {
	for (i=0; i<dstysize; i++) {
	    percentdone(100.0*i/dstysize);
	    if ((i%factor) == 0) {
		cmov2i(srcxorg,srcyorg+(i/factor));
		readpixels(srcxsize,pixels);
		expand(pixels,srcxsize,factor);
		if (blacklines && factor>2) {
		    color(GRIDCOLOR);
		    move2i(dstxorg,dstyorg+i);
		    draw2i(dstxorg+dstxsize,dstyorg+i);
		} else {
		    cmov2i(dstxorg,dstyorg+i);
		    writepixels(dstxsize,pixels);
		}
	    } else {
		cmov2i(dstxorg,dstyorg+i);
		writepixels(dstxsize,pixels);
	    }
	}
    } else {
	if (!shaded) {
	    xtrans = dstxorg;
	    ytrans = dstyorg;
	    for (i=0; i<dstysize; i+=factor) {
		percentdone(100.0*i/dstysize);
		cmov2i(srcxorg,srcyorg+(i/factor));
		readpixels(srcxsize,pixels);
		pushmatrix();
		    translate(xtrans,ytrans+i,0.0);
		    drawrects(pixels,srcxsize,factor);
		popmatrix();
	    }
	} else {
	    xtrans = dstxorg-(factor>>1);
	    ytrans = dstyorg-(factor>>1);
	    cmov2i(srcxorg-1,srcyorg-1);
	    readpixels(srcxsize+2,lastpixels);
	    for (i=0; i<dstysize+factor; i+=factor) {
		percentdone(100.0*i/dstysize);
		cmov2i(srcxorg-1,srcyorg+(i/factor));
		readpixels(srcxsize+2,pixels);
		pushmatrix();
		    translate(xtrans,ytrans+i,0.0);
		    drawsrects(lastpixels,pixels,srcxsize+2,factor);
		popmatrix();
		temp = lastpixels;
		lastpixels = pixels;
		pixels = temp;
	    }
	}
    }
    setcursor(1,RED,0xfff);
}

drawrects(pixels,len,size)
register short *pixels;
register int len, size;
{
    register int x, nx;

    if (blacklines) {
	x = 0;
	color(GRIDCOLOR);
	rectfi(x,0,size*len,size);
	size -= 2;
	while (len--) {
	    color(*pixels++);
	    nx = x+size;
	    rectfi(x,0,nx,size);
	    x = nx+2;
	}
    } else {
	x = 0;
	size -= 1;
	while (len--) {
	    color(*pixels++);
	    nx = x+size;
	    rectfi(x,0,nx,size);
	    x = nx+1;
	}
    }
}

drawsrects(lastpixels, pixels,len,size)
register short *pixels;
register short *lastpixels;
register int len, size;
{
    register int x, nx;

    len--;
    if (blacklines) {
	x = 0;
	color(GRIDCOLOR);
	rectfi(x,0,size*len,size);
	size -= 2;
	while (len--) {
	    nx = x+size;
	    setshade(*lastpixels++);
	    pmv2i(x,0);
	    setshade(*pixels++);
	    pdr2i(x,size);
	    setshade(*pixels);
	    pdr2i(nx,size);
	    setshade(*lastpixels);
	    pdr2i(nx,0);
	    spclos();
	    x = nx+2;
	}
    } else {
	x = 0;
	size -= 1;
	while (len--) {
	    nx = x+size;
	    setshade(*lastpixels++);
	    pmv2i(x,0);
	    setshade(*pixels++);
	    pdr2i(x,size);
	    setshade(*pixels);
	    pdr2i(nx,size);
	    setshade(*lastpixels);
	    pdr2i(nx,0);
	    spclos();
	    x = nx+1;
	}
    }
}

expand(shortbuf,n,factor)
register unsigned short shortbuf[];
int n;
register int factor;
{
    register unsigned short *sptr, *dptr;
    register short j;

    sptr = &shortbuf[n];
    dptr = &shortbuf[n*factor];
    while (sptr != shortbuf) {
	sptr--;
	for (j=0; j<factor; j++) {
	    *--dptr =  *sptr;
	}
    }
    if (blacklines && factor>2) {
	sptr = shortbuf;
	for (j=0; j<n; j++) {
	    *sptr = GRIDCOLOR;
	    sptr += factor;
	}
    }
}

getloc( x, y )
int *x, *y;
{
    short dev, val;
    short gotit = 0;
    int mx, my;

    curson();
    qreset();
    while (1) {
	dev = qread(&val);
	switch (dev) {
	    case MIDDLEMOUSE:
		if (val) {
		    mx = getvaluator(MOUSEX);
		    my = getvaluator(MOUSEY);
		    if (mx > dstxorg && mx < dstxorg+dstxsize &&
			       my > dstyorg && my < dstyorg+dstysize ) {
			factor = (60*(mx-dstxorg))/dstxsize;
			if (factor<2) 
			    factor = 2;
			makeframe(0);
		    }
		}
		break;
	    case LEFTMOUSE:
		if ((val == 1)) {
		    *x = getvaluator(MOUSEX);
		    *y = getvaluator(MOUSEY);
		    gotit++;
		} else if (gotit) {
		    return;
		}
		break;
	    case REDRAW:
		reshapeviewport();
		makeframe(0);
		break;
	    case MENUBUTTON:
		if (val) {
		    switch(dopup(menu)) {
			case 1:
			    blacklines = 0;
			    makeframe(0);
			    break;
			case 2:
			    blacklines = 1;
			    makeframe(0);
			    break;
			case 3:
			    shaded = 1;
			    makeframe(0);
			    break;
			case 4:
			    shaded = 0;
			    makeframe(0);
			    break;
			case 5:
			    blacklines = 1;
			    dosystem("mag");
			    break;
		    }
		    setcursor(1,RED,0xfff);
		}
		break;
	}
    }
}

makeframe(doclear)
int doclear;
{
    getsize(&dstxsize,&dstysize);
    dstxsize++;
    dstysize++;
    getorigin(&dstxorg,&dstyorg);
    if ((curx == NOSRC) || doclear) {
	color(GREY(8));
	clear();
    }
    if(curx != NOSRC) 
 	readit(curx,cury); 
}
