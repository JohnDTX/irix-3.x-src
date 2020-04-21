/*
 *	paint -
 *		A minimal object space paint program.
 *
 *				Paul Haeberli - 1985
 *
 */
#include "port.h"
#include "gl.h"
#include "device.h"
#include "math.h"

#define MOUSE		12
#define TABLET		13

#define DRAWLINE	2
#define NEWCOLOR	3
#define CLEAR		4
#define NEWSIZE		5

#define MOUSEXMAP(x)	( (100.0*((x)-xorg))/(xsize) )
#define MOUSEYMAP(y)	( (100.0*((y)-yorg))/ysize )
#define BPSCALE 16.0

struct event {
     struct event *next;
     int func;
     float arg1;
     float arg2;
     float arg3;
     float arg4;
}; 

int xsize, ysize;
int xorg, yorg;
int mx, my;
int bpx, bpy;
int mmiddle, mleft;
int curcolor = 7;
int lastcurcolor = 7;
float curx, cury, cursize;
curdev = MOUSE;
struct event *histstart = 0;
struct event *histend = 0;
float xpos, ypos;
int pendown;
int brushsides;
float brushcoords[30][2];
int menu;

main()
{
    cursize = 1.0;
    winopen("paint");
    wintitle("paint");
    menu = defpup("paint %t|mouse|tablet");
    makebrush();
    makeframe();
    getinput();
}

getinput()
{
    short dev, val;
    float x, y;

    while(1) {
	do {
	    dev = qread(&val);
	    switch (dev) {
		case MOUSEX: 
		    mx = val;
		    if (curdev == MOUSE)
		        xpos = MOUSEXMAP(val);
		    break;
		case MOUSEY: 
		    my = val;
		    if (curdev == MOUSE)
			ypos = MOUSEYMAP(val);
		    break;
		case BPADX:  
		    bpx = val;
		    if (curdev == TABLET)
		 	xpos = val/BPSCALE;
		    break;
		case BPADY:  
		    bpy = val;
		    if (curdev == TABLET)
			ypos = val/BPSCALE;
		    break;
		case BPAD0: 
		    if (curdev == TABLET)
		    	pendown = val;
		    if (val) {
			curx = xpos = bpx/BPSCALE;
			cury = ypos = bpy/BPSCALE;
		    }
		    break;
		case MENUBUTTON: 
		    if(val) {
			switch (dopup(menu)) {
			    case 1:
				curdev = MOUSE;
				break;
			    case 2:
				curdev = TABLET;
				break;
			}
		    }
		    break;
		case MIDDLEMOUSE: 
		    mmiddle = val;
		    if (mmiddle) {
			clearscreen();
			history(CLEAR);
		    }
		    break;
		case LEFTMOUSE: 
		    mleft = val;
		    if (mleft) {
			if (!inside(mx-xorg,my-yorg,0,xsize,0,ysize,0)) {
			    newcolor(getapixel(mx,my));
			    history(NEWCOLOR,(float)curcolor);
			}
		    }
		    if (curdev == MOUSE) {
			pendown = val;
			curx = xpos = MOUSEXMAP(mx);
			cury = ypos = MOUSEYMAP(my);
		    }
		    break;
		case REDRAW: 
		    makeframe();
		    replay();
		    break;
	    }
	} while (qtest());
	if (pendown) {
	    x = xpos;
	    y = ypos;
	    drawbrush(x,y,curx,cury);
	    history(DRAWLINE,x,y,curx,cury);
	    curx = x;
	    cury = y;
	}
    }
}

clearscreen()
{
    color(curcolor);
    clear();
}

newcolor(c)
int c;
{
    lastcurcolor = curcolor;
    curcolor = c;
    paintport();
}

makeframe()
{
    qdevice(MOUSEX);
    qdevice(MOUSEY);
    qdevice(MENUBUTTON);
    qdevice(MIDDLEMOUSE);
    qdevice(LEFTMOUSE);
    qdevice(BPADX);
    qdevice(BPADY);
    qdevice(BPAD0);
    getsize(&xsize,&ysize);
    getorigin(&xorg,&yorg);
    paintport();
    newcolor(0);
    clearscreen();
    newcolor(255);
    newcolor(128+32);
}

paintport()
{
    viewport(0,xsize-1,0,ysize);
    ortho2(-0.5,99.5,-0.5,99.5);
}

inside(x,y,xmin,xmax,ymin,ymax,fudge)
int x, y, xmin, xmax, ymin, ymax, fudge;
{
    if (x>xmin-fudge && x<xmax+fudge && y>ymin-fudge && y<ymax+fudge)
	return 1;
    else
	return 0;
}

makebrush()
{
    int i;

    brushsides = 4;
    brushcoords[0][0] = -0.6;
    brushcoords[0][1] = -0.2;
    brushcoords[1][0] = -0.6;
    brushcoords[1][1] = -0.4;
    brushcoords[2][0] =  0.6;
    brushcoords[2][1] =  0.2;
    brushcoords[3][0] =  0.6;
    brushcoords[3][1] =  0.4;
    for (i=0; i<brushsides; i++) {
	brushcoords[i][0] = 0.5*brushcoords[i][0];
	brushcoords[i][1] = 0.5*brushcoords[i][1];
    }
}

drawbrush(x,y,ox,oy)
float x, y, ox, oy;
{
    register int i, n;
    register float dx, dy;
    float quad[4][2];
    float delta;
    int c;

    dx = ox-x;
    dy = oy-y;
    if (lastcurcolor != curcolor) {
	delta = sqrt(dx*dx+dy*dy);
	if (delta<0.001) 
	    return;
	c = (int) (curcolor + (lastcurcolor-curcolor)*(ABS(dx)/delta) );
	color(c);
    } else
	color(curcolor);
    pushmatrix();
	translate(x,y,0.0);
	for (i=0; i<brushsides; i++) {
	    n = (i+1) % brushsides;
	    quad[0][0] = brushcoords[i][0];
	    quad[0][1] = brushcoords[i][1];
	    quad[1][0] = brushcoords[n][0];
	    quad[1][1] = brushcoords[n][1];
	    quad[2][0] = quad[1][0]+dx;
	    quad[2][1] = quad[1][1]+dy;
	    quad[3][0] = quad[0][0]+dx;
	    quad[3][1] = quad[0][1]+dy;
	    polf2(4,quad);
	}
	polf2(brushsides,brushcoords);
    popmatrix();
}

history(func,arg1,arg2,arg3,arg4)
int func;
float arg1, arg2, arg3, arg4;
{
    register struct event *e, *n;

    e = (struct event *)malloc(sizeof(struct event));
    switch (func) {
	case CLEAR: 
		zaphistory();
		history(NEWCOLOR,(float)curcolor);
	case NEWCOLOR:  
	case DRAWLINE: 
		e->func = func;
		e->arg1 = arg1;
		e->arg2 = arg2;
		e->arg3 = arg3;
		e->arg4 = arg4;
		e->next = 0;
		if (!histstart) {
		    histstart = histend = e;
		} else {
		    histend->next = e;
		    histend = e;
		}
		break;
    }
}

zaphistory()
{
    register struct event *e, *n;

    e = histstart; 
    while (e) {
	n = e->next; 
        free(e);
        e = n;
    }
    histstart = histend = 0;
}

replay()
{
    register struct event *e;
    register int i;

    i = 0;
    e = histstart; 
    while (e) {
       switch (e->func) {
	   case NEWCOLOR: 
		newcolor((int)e->arg1);
		break;
	   case DRAWLINE: 
		drawbrush(e->arg1,e->arg2,e->arg3,e->arg4);
		break;
	   case CLEAR: 
		clearscreen();
		break;
       }
       e = e->next;
       i++;
    }
}
