/*
 *	movie - 
 *		Show a series of images as a movie.
 *
 *				Paul Haeberli - 1985	
 *
 */
#include "image.h"
#include "gl.h"
#include "device.h"
#include "port.h"

#define XBORDER		8
#define TOPBORDER	8
#define BOTBORDER	40

#define STOPPED		1
#define LOOPING		2
#define MOUSEPOS	3
#define FORBACK		4
#define SWINGING	5

int xsize, ysize;
int xframe, yframe;
int sxmode = 0;
MEMIMAGE *movie[1000];
int nframes;
int curframe;
int randmode = 0;
int menu;
int roword[1000];
int mode = FORBACK;

main(argc,argv)
int argc;
char **argv;
{
    register short *sptr;
    register int i, j = 0;
    short val;
    int inf;

    sxmode = 0;
    for(i=1; i<argc; i++) {
	if(strcmp(argv[i],"-sx") == 0) 
	    sxmode = 1;
    }
    curframe = 0;
    for(i=1; i<argc; i++) {
	if(strcmp(argv[i],"-sx") != 0) 
	    addframe(argv[i]);
   	if(nframes>0) 
	    percentdone(100.0*i/argc);
    }
    if(nframes == 0) {
	fprintf(stderr,"no images in this movie\n");
	exit(1);
    }
    percentdone(100.0);
    qdevice(LEFTMOUSE);
    qdevice(MIDDLEMOUSE);
    qdevice(MENUBUTTON);
    qdevice(INPUTCHANGE);
    makeframe(0);
    menu = defpup("movie %t|instant|smooth|stop|loop|swing|mousepos|for/back");
    while(1) {
	if(mode == LOOPING) {
	    while(!qtest()) 
		forward();
	}
	if(mode == SWINGING) {
	    while(!qtest()) 
		swing();
	}
	switch(qread(&val)) {
   	    case INPUTCHANGE:
		if(val) {
		    doublebuffer();
		    gconfig();
		    mode = FORBACK;
		} else {
		    makeframe(curframe);
		    singlebuffer();
		    gconfig();
		    mode = FORBACK;
		}
   	    case MIDDLEMOUSE:
		if(mode == FORBACK)
		    while(getbutton(MIDDLEMOUSE))
			back();
		break;
   	    case LEFTMOUSE:
		if(mode == FORBACK)
		    while(getbutton(LEFTMOUSE))
			forward();
		else if(mode == MOUSEPOS)
		    while(getbutton(LEFTMOUSE))
			gotoframe( (nframes*getvaluator(MOUSEX))/1024 );
		break;
   	    case MENUBUTTON:
		switch(dopup(menu)) {
			case 1:
			    randmode = 0; 
			    break;
			case 2:
			    randmode = 1; 
			    break;
			case 3:
			    mode = STOPPED; 
			    break;
			case 4:
			    mode = LOOPING; 
			    break;
			case 5:
			    mode = SWINGING; 
			    break;
			case 6:
			    mode = MOUSEPOS; 
			    break;
			case 7:
			    mode = FORBACK; 
			    break;
		}
		while(getbutton(MIDDLEMOUSE))
		    back();
		break;
   	    case REDRAW:
		frontbuffer(1);
		makeframe(curframe);
		frontbuffer(0);
		break;
	}
    }
}

forward()
{
    curframe--;
    if(curframe<0)
	curframe = nframes-1;
    makeframe(curframe);
    swapbuffers();
}

back()
{
    curframe++;
    if(curframe >= nframes)
	curframe = 0;
    makeframe(curframe);
    swapbuffers();
}

swing()
{
    static int dir = 1;

    curframe += dir;
    if(curframe <= 0 || curframe >= (nframes-1))
	dir = -dir;
    makeframe(curframe);
    swapbuffers();
}

gotoframe(n)
int n;
{
    if(n!=curframe) {
	curframe = n;
	if(curframe >= nframes)
	    curframe = nframes-1;
	if(curframe<0)
	    curframe = 0;
	makeframe(curframe);
	swapbuffers();
    }
}

addframe(filename)
char *filename;
{
    static int firsted = 0;

    movie[nframes] = readimage(filename);
    xsize = movie[nframes]->xsize;
    ysize = movie[nframes]->ysize;
    if(!firsted) {
	if(sxmode) {
	    xframe = xsize+XBORDER+XBORDER;
	    yframe = ysize+BOTBORDER+TOPBORDER;
	} else {
	    xframe = xsize;
	    yframe = ysize;
	}
	firsted++;
	prefsize(xframe,yframe);
	winopen("movie");
	doublebuffer();
	gconfig();
	makeroword(ysize);
    }
    makeframe(nframes++);
    swapbuffers();
}

makeframe(frameno)
int frameno;
{
    int xoff, yoff;

    curframe = frameno;
    viewport(0,xframe-1,0,yframe-1);
    ortho2(-0.5,xframe-0.5,-0.5,yframe-0.5);
    if(sxmode) {
	color(7);
	rectfi(0,0,xframe-1,BOTBORDER-1);
	rectfi(0,0,XBORDER-1,yframe-1);
	rectfi(xframe-XBORDER,0,xframe-1,yframe-1);
	rectfi(0,yframe-TOPBORDER,xframe-1,yframe-1);
	color(0);
	recti(XBORDER-1,BOTBORDER-1,xframe-XBORDER,yframe-TOPBORDER);
    }
    if(sxmode) {
	xoff = XBORDER;
	yoff = BOTBORDER;
    } else {
	xoff = 0;
	yoff = 0;
    }
    if(randmode)
	randdrawimage(xoff,yoff,movie[frameno]);
    else
	drawimage(xoff,yoff,movie[frameno]);
}


randdrawimage(xorg,yorg,mimage)
register int xorg, yorg;
register MEMIMAGE *mimage;
{
    register int y;
    register short *pixptr;

    frontbuffer(1);
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
	for(y=0; y<mimage->ysize; y++) {
	    cmov2i(xorg,yorg+roword[y]);
	    writepixels(mimage->xsize,mimage->pixels+mimage->xsize*roword[y]);
	}
    popmatrix();
    frontbuffer(0);
}

makeroword(ysize)
register int ysize;
{
   register int i, j;
   register temp;

   for(i=0; i<ysize; i++) 
       roword[i] = i;
   for(i=0; i<(ysize-1); i++) {
       temp = roword[i];
       j = i+rand()%(ysize-i);
       roword[i] = roword[j];
       roword[j] = temp;
   }
}
