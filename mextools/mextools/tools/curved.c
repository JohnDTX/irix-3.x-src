/*
 *	curved - 
 *		A minimal curve editor.
 *
 *				Paul Haeerli - 1985
 *
 */
#include "gl.h"
#include "device.h"
#include "port.h"

float endgeom[4][3];
float geom[100][3];
int pt[100];

#define ADDPOINT	1
#define MOVEPOINT	2
#define INSERTPOINT	3
#define DELETEPOINT	4
#define CHANGEPOINT	5

#define BACKGROUND    	GREY(10)
#define LINE 	   	GREY(0)

#define ROUND		1
#define SQUARE		2

#define MOUSEXMAP(x)	( (100.0*((x)-xorg))/xsize )
#define MOUSEYMAP(y)	( (100.0*((y)-yorg))/ysize )

short raster[] = {
    0xf800, 0x8800, 0x8800, 0x8800, 0xf800,
    0x7000, 0x8800, 0x8800, 0x8800, 0x7000,
};

Fontchar chars[] = {
    {0,0,0,0,0,0},
    {0,5,5,-2,-2,5},
    {5,5,5,-2,-2,5},
};

Matrix b_spline = {
	{-1.0/6.0,	1.0/2.0,	-1.0/2.0,	1.0/6.0},
	{1.0/2.0,	-1.0,		1.0/2.0,	0.0},
	{-1.0/2.0, 	0.0, 		1.0/2.0, 	0.0},
	{1.0/6.0, 	2.0/3.0, 	1.0/6.0, 	0.0}
};

Matrix lob_spline = {
	{0.0,		0.0,		0.0,		0.0},
	{1.0/3.0,	-2.0/3.0,	1.0/3.0,	0.0},
	{-7.0/6.0,	4.0/3.0,	-1.0/6.0,	0.0},
	{1.0,		0.0,		0.0,		0.0},
};

Matrix hib_spline = {
	{0.0,		0.0,		0.0,		0.0},
	{0.0,		1.0/3.0,	-2.0/3.0,	1.0/3.0},
	{0.0,		-1.0/2.0,	0.0,		1.0/2.0},
	{0.0,		1.0/6.0,	2.0/3.0,	1.0/6.0},
};

Matrix c_spline =  {
	{-0.5, 		1.5, 		-1.5, 		0.5},
 	{1.0, 		-2.5, 		2.0, 		-0.5},
	{-0.5, 		0.0, 		0.5, 		0.0},
	{0.0, 		1.0,		0.0, 		0.0}
};

Matrix loc_spline = {
	{0.0, 		0.0, 		0.0, 		0.0},
	{0.5,		-1.0, 		0.5, 		0.0},
	{-1.5,		2.0,		-0.5, 		0.0},
	{1.0, 		0.0, 		0.0, 		0.0}
};

Matrix hic_spline = {
	{0.0, 		0.0, 		0.0, 		0.0},
	{0.0, 		0.5,		-1.0, 		0.5},
	{0.0,		-0.5, 		0.0, 		0.5},
	{0.0, 		0.0, 		1.0, 		0.0}
};

#define BSPLINE		100
#define LOBSPLINE	101
#define HIBSPLINE	102

int xsize, ysize;
int xorg, yorg;
float mx, my;
int curmode = ADDPOINT;
int points;
int menu;

main(argc,argv)
int argc;
char **argv;
{
    winopen("curved");
    wintitle("curved");
    menu = defpup("curved %t|add|move|insert|delete|change");
    defrasterfont(1,7,3,chars,10,raster);
    font(1);
    deflinestyle(1,0xf0f0);
    if (argc == 1) {
	defbasis(BSPLINE,b_spline);
	defbasis(LOBSPLINE,lob_spline);
	defbasis(HIBSPLINE,hib_spline);
    } else {
	defbasis(BSPLINE,c_spline);
	defbasis(LOBSPLINE,loc_spline);
	defbasis(HIBSPLINE,hic_spline);
    }
    curveprecision(6);
    makeframe();
    initdevices();
    points = 0;
    while (1) 
	getinput();
}

initdevices()
{
    qdevice(MOUSEX);
    qdevice(MOUSEY);
    qdevice(LEFTMOUSE);
    qdevice(MENUBUTTON);
    qdevice(KEYBD);
}

getinput()
{
    short dev, val;
    int sel;

    do {
	if (!qtest())
	    mouseevent(2,mx,my);
	dev = qread(&val);
	switch (dev) {
	    case MENUBUTTON:
			if (val) {
			    sel = dopup(menu);
			    if (sel>0) 
				    curmode = sel;
			}
			font(1);
			break;
	    case LEFTMOUSE: 
			mouseevent(val,mx,my);
			break;
	    case MOUSEX: 
			mx = MOUSEXMAP(val);
			break;
	    case MOUSEY: 
			my = MOUSEYMAP(val);
			break;
	    case KEYBD: 
			switch (val) {
			    case 'a': 
				    break;
			    case 'i': 
				    curmode = INSERTPOINT;
				    break;
			    case 'm': 
				    curmode = MOVEPOINT;
				    break;
			    case 'd': 
				    curmode = DELETEPOINT;
				    break;
			    case 'c': 
				    curmode = CHANGEPOINT;
				    break;
			}
			break;
	    case REDRAW: 
			reshapeviewport();
			makeframe();
			break;
	 }
    } while (qtest());
}

int curpoint;
int moving;

mouseevent(state,x,y)
int state; 
float x, y;
{
    int nextpoint;

    switch (curmode) {
	case ADDPOINT:
		if (state == 1) {
		    curpoint = duppoint(points);
		    geom[curpoint][0] = x;
		    geom[curpoint][1] = y;
		    pt[curpoint] = SQUARE;
		    drawline(LINE);
		}
		break;
	case MOVEPOINT:
		if (state == 1) {
		    curpoint = findpoint(x,y); 
		    moving = 1;
		} else if (state == 2) {
		    if (moving) {
			drawline(BACKGROUND);
			geom[curpoint][0] = x;
			geom[curpoint][1] = y;
			drawline(LINE);
		    }
		} else if (state == 0) 
		    moving = 0;
		break;
	case INSERTPOINT:
		if (state == 1) {
		    curpoint = findpoint(x,y); 
		    if (curpoint < 0) 
			curpoint = duppoint(points);
		    else
			curpoint = duppoint(curpoint);
		    drawline(BACKGROUND);
		    geom[curpoint][0] = x;
		    geom[curpoint][1] = y;
		    pt[curpoint] = SQUARE;
		    drawline(LINE);
		}
		break;
	case DELETEPOINT:
		if (state == 1) {
		    curpoint = findpoint(x,y); 
		    if (curpoint >= 0) {
			drawline(BACKGROUND);
			delpoint(curpoint);
			drawline(LINE);
		    }
		}
		break;
	case CHANGEPOINT:
		if (state == 1) {
		    curpoint = findpoint(x,y); 
		    if (curpoint >= 0) {
			drawline(BACKGROUND);
			if (pt[curpoint] == ROUND)
			    pt[curpoint] = SQUARE;
			else
			    pt[curpoint] = ROUND;
			drawline(LINE);
		    }
		}
		break;
    }
}

makeframe()
{
    getorigin(&xorg,&yorg);
    getsize(&xsize,&ysize);
    ortho2(0.0,100.0,0.0,100.0);
    color(BACKGROUND);
    clear();
    drawline(LINE);
}  

float ppdist(x1,y1,x2,y2)
float x1,y1,x2,y2;
{
    register float dx, dy;

    dx = x2-x1;
    if (dx<0) dx = -dx;
    dy = y2-y1;
    if (dy<0) dy = -dy;
    return dx+dy;
}

float pldist(x,y,x1,y1,x2,y2)
float x, y, x1, y1, x2, y2;
{
    register float dx, dy, c;
    
    dx = x2-x1;
    dy = y2-y1;
    c = dy*x1-dx*y1;
}

findpoint(x,y)
float x, y;
{
    register float mindist;
    register int minpnt;
    register int i;
    float dist;

    mindist = 100000.0;
    minpnt = -1;
    for (i=0; i<points; i++) {
	dist = ppdist(geom[i][0],geom[i][1],x,y);
	if (dist<mindist) {
	    mindist = dist;
	    minpnt = i;
	}
    }
    return minpnt;
}

findedge(x,y)
float x, y;
{
    register float mindist;
    register int minpnt;
    register int i;
    float dist;

    mindist = 100000.0;
    minpnt = -1;
    for (i=0; i<points; i++) {
	dist = ppdist(geom[i][0],geom[i][1],x,y);
	if (dist<mindist) {
	    mindist = dist;
	    minpnt = i;
	}
    }
    return minpnt;
}

duppoint(pnt)
int pnt;
{
    register int i;

    for (i=points; i>pnt; i--) {
	geom[i][0] = geom[i-1][0];
	geom[i][1] = geom[i-1][1];
	pt[i] = pt[i-1];
    }
    points++;
    return pnt;
}

delpoint(pnt)
int pnt;
{
    register int i;

    for (i=pnt; i<points; i++) {
	geom[i][0] = geom[i+1][0];
	geom[i][1] = geom[i+1][1];
	pt[i] = pt[i+1];
    }
    points--;
}

drawline(c)
int c;
{
    register int i, j;

    pt[0] = SQUARE;
    pt[points-1] = SQUARE;
    color(c);
    if (c == BACKGROUND) 
	clear();
    else {
	setlinestyle(1);
	move2(geom[0][0],geom[0][1]);
	for (i=0; i<points; i++) {
	    draw2(geom[i][0],geom[i][1]);
	    putsym(i);
	}
	setlinestyle(0);
	move2(geom[0][0],geom[0][1]);
	for (i=1; i<points; i++) {
	    if(pt[i] == SQUARE) {
		if(pt[i-1] == ROUND) {
		    for (j=0; j<4; j++) {
			endgeom[j][0] = geom[i-2+j][0];
			endgeom[j][1] = geom[i-2+j][1];
		    }
		    endgeom[3][0] = endgeom[2][0];
		    endgeom[3][1] = endgeom[2][1];
		    curvebasis(BSPLINE);
		    crv(endgeom); 
		    draw2(geom[i][0],geom[i][1]);
		} else {
		    draw2(geom[i][0],geom[i][1]);
		}
	    } else {
		if (pt[i-1] == SQUARE) {
		    for (j=0; j<4; j++) {
			endgeom[j][0] = geom[i-2+j][0];
			endgeom[j][1] = geom[i-2+j][1];
		    }
		    endgeom[0][0] = endgeom[1][0];
		    endgeom[0][1] = endgeom[1][1];
		    curvebasis(BSPLINE);
		    crv(endgeom); 
		} else {
		    curvebasis(BSPLINE);
		    crv(&geom[i-2][0]); 
		}
	    }
	}
    }
}

putsym(i)
register int i;
{
    char buf[2];

    cmov2(geom[i][0],geom[i][1]);
    if (pt[i] == SQUARE)
	buf[0] = 1;
    else
	buf[0] = 2;
    buf[1] = 0;
    charstr(buf);
}
