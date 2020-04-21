/*
 *	showpie - 
 *		Show the pieces that the hardware uses to display this window 
 *
 *		         Rocky Rhodes and Paul Haeberli - 1984
 *
 */
#include "gl.h"
#include "device.h"
#include "globals.h"

int num;

main(argc,argv)
int argc;
char **argv;
{
    short val, c;
    int win;

    if (argc>1) 
	c = atoi(argv[1]);
    else
	c = 7;
    winopen("showpie");
    qdevice(PIECECHANGE);
    qdevice(LEFTMOUSE);
    doit(c);
    while (1) {
        switch(qread(&val)) {
	    case PIECECHANGE:
		    doit(c);
		    break;
	    case LEFTMOUSE:
		    if (val) {
			win = winat(getvaluator(MOUSEX),getvaluator(MOUSEY));	
			printf("the window here is number %d\n",win); 
		    }
		    break;
	}
    }
}

doit(c)	
int c;
{
    short *sptr = gl_wstatep->rectlist;
    int xorg, yorg;
    int xsize, ysize;
    short x1, y1, x2, y2;
    short int i;

    reshapeviewport();
    getorigin(&xorg,&yorg);
    getsize(&xsize,&ysize);
    ortho2(-1.5,xsize+0.5,-1.5,ysize+0.5);
    color(0);
    clear();
    color(c);
    num = gl_wstatep->numrects;
    for (i=0; i<num; i++) {
	x1 = *sptr++; 
        y1 = *sptr++; 
        x2 = *sptr++; 
        y2 = *sptr++; 
	recti(x1-xorg,y1-yorg,x2-xorg,y2-yorg);
    }
}
