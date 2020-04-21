/*
 *	melt - melt the screen of the iris
 *
 *			Paul Haeberli - 1984	
 *
 */
#include "image.h"
#include "gl.h"
#include "device.h"
#include "port.h"

int xsize, ysize;
int block = 1;

main(argc,argv)
int argc;
char **argv;
{
    register int i;
    short val;

    if(argc>1)
	block = atoi(argv[1]);
    winopen("melt");
    qdevice(LEFTMOUSE);
    makeframe();
    doscramble();
    while(1) {
	switch(qread(&val)) {
	    case REDRAW:
		makeframe();
		break;
	    case LEFTMOUSE:
		if(val) 
		    doscramble();
		break;
	}
    }
}

makeframe()
{
    getsize(&xsize,&ysize);
    viewport(0,xsize-1,0,ysize-1);
    ortho2(-0.5,xsize-0.5,-0.5,ysize-0.5);
}

randpos(max)
int max;
{
	return (((rand()>>3)%max)/block)*block;
}

doscramble()
{
    register int x, y;
    register int nx, ny;
    register int i;

    x = randpos(xsize);
    y = randpos(ysize);
    for(i=0; i<1000; i++) {
	nx = randpos(xsize);
	if(nx>x) 
	    rectcopy(x+block,y,nx+block-1,y+block-1,x,y);
	else if(x>nx)
	    rectcopy(nx,y,x-1,y+block-1,nx+block,y);
	x = nx;
	ny = randpos(ysize);
	if(ny>y) 
	    rectcopy(x,y+block,x+block-1,ny+block-1,x,y);
	else if(y>ny)
	    rectcopy(x,ny,x+block-1,y-1,x,ny+block);
	y = ny;
    }
}
