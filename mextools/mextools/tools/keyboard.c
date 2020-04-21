/*
 * 	keyboard - 
 *		Draw a keyboard that displays which keys are held down.
 *
 *				Paul Haeberli - 1984
 *
 */
#include "gl.h"
#include "device.h"
#include "port.h"

#define NOTFOUND	(-1)

typedef struct
{
    float x;
    float y;
} XY;

typedef struct 
{
    XY center;
    XY size;
} Area;

struct keyrect {
    int keyno;
    int keytype;
    Area keyarea;
};

struct lamprect {
    int lampno;
    int lamptype;
    Area lamparea;
};

#define CONTROL 	1
#define ALPHANUM 	2

struct keyrect keys[] = {
	{ 0x01,  CONTROL, { { 6.0,22.0}, {6.0,4.0} } },
	{ 0x50,  CONTROL, { {49.0,22.0}, {4.0,4.0} } },
	{ 0x49,  CONTROL, { {53.0,22.0}, {4.0,4.0} } },
	{ 0x48,  CONTROL, { {57.0,22.0}, {4.0,4.0} } },
	{ 0x4f,  CONTROL, { {61.0,22.0}, {4.0,4.0} } },

	{ 0x47,  CONTROL, { {72.0,22.0}, {4.0,4.0} } },
	{ 0x46,  CONTROL, { {76.0,22.0}, {4.0,4.0} } },
	{ 0x4e,  CONTROL, { {80.0,22.0}, {4.0,4.0} } },
	{ 0x4d,  CONTROL, { {84.0,22.0}, {4.0,4.0} } },

	{ 0x06,  CONTROL, { { 5.0,18.0}, {4.0,4.0} } },
	{ 0x07, ALPHANUM, { { 9.0,18.0}, {4.0,4.0} } },
	{ 0x0d, ALPHANUM, { {13.0,18.0}, {4.0,4.0} } },
	{ 0x0e, ALPHANUM, { {17.0,18.0}, {4.0,4.0} } },
	{ 0x15, ALPHANUM, { {21.0,18.0}, {4.0,4.0} } },
	{ 0x16, ALPHANUM, { {25.0,18.0}, {4.0,4.0} } },
	{ 0x1d, ALPHANUM, { {29.0,18.0}, {4.0,4.0} } },
	{ 0x1e, ALPHANUM, { {33.0,18.0}, {4.0,4.0} } },
	{ 0x25, ALPHANUM, { {37.0,18.0}, {4.0,4.0} } },
	{ 0x26, ALPHANUM, { {41.0,18.0}, {4.0,4.0} } },
	{ 0x2d, ALPHANUM, { {45.0,18.0}, {4.0,4.0} } },
	{ 0x2e, ALPHANUM, { {49.0,18.0}, {4.0,4.0} } },
	{ 0x35, ALPHANUM, { {53.0,18.0}, {4.0,4.0} } },
	{ 0x36, ALPHANUM, { {57.0,18.0}, {4.0,4.0} } },
	{ 0x3c,  CONTROL, { {61.0,18.0}, {4.0,4.0} } },
	{ 0x00,  CONTROL, { {65.0,18.0}, {4.0,4.0} } },

	{ 0x42, ALPHANUM, { {72.0,18.0}, {4.0,4.0} } },
	{ 0x43, ALPHANUM, { {76.0,18.0}, {4.0,4.0} } },
	{ 0x4a, ALPHANUM, { {80.0,18.0}, {4.0,4.0} } },
	{ 0x4b,  CONTROL, { {84.0,18.0}, {4.0,4.0} } },

	{ 0x08,  CONTROL, { { 6.0,14.0}, {6.0,4.0} } },
	{ 0x09, ALPHANUM, { {11.0,14.0}, {4.0,4.0} } },
	{ 0x0f, ALPHANUM, { {15.0,14.0}, {4.0,4.0} } },
	{ 0x10, ALPHANUM, { {19.0,14.0}, {4.0,4.0} } },
	{ 0x17, ALPHANUM, { {23.0,14.0}, {4.0,4.0} } },
	{ 0x18, ALPHANUM, { {27.0,14.0}, {4.0,4.0} } },
	{ 0x1f, ALPHANUM, { {31.0,14.0}, {4.0,4.0} } },
	{ 0x20, ALPHANUM, { {35.0,14.0}, {4.0,4.0} } },
	{ 0x27, ALPHANUM, { {39.0,14.0}, {4.0,4.0} } },
	{ 0x28, ALPHANUM, { {43.0,14.0}, {4.0,4.0} } },
	{ 0x2f, ALPHANUM, { {47.0,14.0}, {4.0,4.0} } },
	{ 0x30, ALPHANUM, { {51.0,14.0}, {4.0,4.0} } },
	{ 0x37, ALPHANUM, { {55.0,14.0}, {4.0,4.0} } },
	{ 0x3d,  CONTROL, { {63.0,14.0}, {4.0,4.0} } },

	{ 0x3e, ALPHANUM, { {72.0,14.0}, {4.0,4.0} } },
	{ 0x44, ALPHANUM, { {76.0,14.0}, {4.0,4.0} } },
	{ 0x45, ALPHANUM, { {80.0,14.0}, {4.0,4.0} } },
	{ 0x4c,  CONTROL, { {84.0,14.0}, {4.0,4.0} } },

	{ 0x02,  CONTROL, { { 2.0,10.0}, {4.0,4.0} } },
	{ 0x03,  CONTROL, { { 7.0,10.0}, {6.0,4.0} } },
	{ 0x0a, ALPHANUM, { {12.0,10.0}, {4.0,4.0} } },
	{ 0x0b, ALPHANUM, { {16.0,10.0}, {4.0,4.0} } },
	{ 0x11, ALPHANUM, { {20.0,10.0}, {4.0,4.0} } },
	{ 0x12, ALPHANUM, { {24.0,10.0}, {4.0,4.0} } },
	{ 0x19, ALPHANUM, { {28.0,10.0}, {4.0,4.0} } },
	{ 0x1a, ALPHANUM, { {32.0,10.0}, {4.0,4.0} } },
	{ 0x21, ALPHANUM, { {36.0,10.0}, {4.0,4.0} } },
	{ 0x22, ALPHANUM, { {40.0,10.0}, {4.0,4.0} } },
	{ 0x29, ALPHANUM, { {44.0,10.0}, {4.0,4.0} } },
	{ 0x2a, ALPHANUM, { {48.0,10.0}, {4.0,4.0} } },
	{ 0x31, ALPHANUM, { {52.0,10.0}, {4.0,4.0} } },
	{ 0x32,  CONTROL, { {57.5,10.0}, {7.0,4.0} } },
	{ 0x38,  CONTROL, { {63.0,10.0}, {4.0,4.0} } },

	{ 0x39, ALPHANUM, { {72.0,10.0}, {4.0,4.0} } },
	{ 0x3f, ALPHANUM, { {76.0,10.0}, {4.0,4.0} } },
	{ 0x40, ALPHANUM, { {80.0,10.0}, {4.0,4.0} } },
	{ 0x51,  CONTROL, { {84.0, 8.0}, {4.0,8.0} } },

	{ 0x0c,  CONTROL, { { 2.0, 6.0}, {4.0,4.0} } },
	{ 0x05,  CONTROL, { { 8.0, 6.0}, {8.0,4.0} } },
	{ 0x13, ALPHANUM, { {14.0, 6.0}, {4.0,4.0} } },
	{ 0x14, ALPHANUM, { {18.0, 6.0}, {4.0,4.0} } },
	{ 0x1b, ALPHANUM, { {22.0, 6.0}, {4.0,4.0} } },
	{ 0x1c, ALPHANUM, { {26.0, 6.0}, {4.0,4.0} } },
	{ 0x23, ALPHANUM, { {30.0, 6.0}, {4.0,4.0} } },
	{ 0x24, ALPHANUM, { {34.0, 6.0}, {4.0,4.0} } },
	{ 0x2b, ALPHANUM, { {38.0, 6.0}, {4.0,4.0} } },
	{ 0x2c, ALPHANUM, { {42.0, 6.0}, {4.0,4.0} } },
	{ 0x33, ALPHANUM, { {46.0, 6.0}, {4.0,4.0} } },
	{ 0x34, ALPHANUM, { {50.0, 6.0}, {4.0,4.0} } },
	{ 0x04,  CONTROL, { {56.0, 6.0}, {8.0,4.0} } },
	{ 0x3b,  CONTROL, { {62.0, 6.0}, {4.0,4.0} } },

	{ 0x3a, ALPHANUM, { {74.0, 6.0}, {8.0,4.0} } },
	{ 0x41, ALPHANUM, { {80.0, 6.0}, {4.0,4.0} } },

	{ 0x52, ALPHANUM, { {31.0, 2.0}, {36.0,4.0} } }
};

struct lamprect lamps[] = 
{
	{ 0x00,  CONTROL, { {19.8,22.666}, {2.0,2.0} } },
	{ 0x01,  CONTROL, { {23.0,22.666}, {2.0,2.0} } },
	{ 0x02,  CONTROL, { {26.2,22.666}, {2.0,2.0} } },
	{ 0x03,  CONTROL, { {29.4,22.666}, {2.0,2.0} } },
	{ 0x04,  CONTROL, { {32.6,22.666}, {2.0,2.0} } },
	{ 0x05,  CONTROL, { {35.8,22.666}, {2.0,2.0} } },
	{ 0x06,  CONTROL, { {39.0,22.666}, {2.0,2.0} } }
};

#define NKEYS (sizeof(keys) / sizeof(struct keyrect))
#define NLAMPS (sizeof(lamps) / sizeof(struct lamprect))

/*
 *	xy - return an xy pair
 *
 */
XY xy(x,y)
float x, y;
{
    XY p;

    p.x = x;
    p.y = y;
    return p;
}

/*
 *	area - return an area with the given center and size
 *
 */
Area area( center, size )
XY center, size;
{
    Area a;

    a.center = center;
    a.size = size;
    return a;
}

/*
 *	trans - translate an area
 *
 */
Area trans( a, pnt )
Area a;
XY pnt;
{
    a.center.x += pnt.x;
    a.center.y += pnt.y;
    return a;
}

/*
 *	grow - make an area bigger
 *
 */
Area grow( a, amount )
Area a; 
float amount;
{
    a.size.x += amount;
    a.size.y += amount;
    return a;
}

/*
 *	shrink - make an area smaller
 *
 */
Area shrink( a, amount )
Area a; 
float amount;
{
    return grow(a,-amount);
}

/*
 *	fill - fill an area
 *
 */
fill( a )
Area a;
{
    rectf(a.center.x-a.size.x/2,a.center.y-a.size.y/2,
    	     		a.center.x+a.size.x/2,a.center.y+a.size.y/2);
}

/*
 *	outline - draw the outline of an area 
 *
 */
outline( a )
Area a;
{
    rect(a.center.x-a.size.x/2,a.center.y-a.size.y/2,
    	      		a.center.x+a.size.x/2,a.center.y+a.size.y/2);
}

main()
{
    int i;
    short val, dev;
    int button, keyno;
  
    keepaspect(106,31);
    winopen("keyboard");
    makeframe();

    for (i=BUT0; i<=MAXKBDBUT; i++) 
	qdevice(i);
    while (1) {
	switch (dev=qread(&val)) {
	    case REDRAW: 
		reshapeviewport();
		makeframe();
		break;
	}
	button = dev;
	if ((keyno=findbutton(button-1)) != NOTFOUND) {
	    if (val == 1) {
		color(GREY(3));
		fill(shrink(keys[keyno].keyarea,0.8));
	    } else {
		if (keys[keyno].keytype == ALPHANUM)
		    color(GREY(15));
		else
		    color(GREY(12));
		fill(shrink(keys[keyno].keyarea,0.8));
	    }
	    color(GREY(0));
	    outline(shrink(keys[keyno].keyarea,0.8));
	}
    }
}

findbutton( buttonno )
int buttonno;
{
    register int i;

    for (i=0; i<NKEYS; i++) {
        if (keys[i].keyno == buttonno)
	    return i;
    }
    return NOTFOUND;
}

makeframe()
{
    int i;
    XY shadelta;
    static int firsted = 0;

    if (!firsted) {
	makeobj(10);
	color(GREY(12));
	clear();
	ortho2(-10.0,96.0,-4.0,27.0);

	linewidth(2);
	color(GREY(0));
	shadelta = xy(-0.4,-0.4);

	for (i=0; i<NLAMPS; i++) {
	    color(GREY(0));
	    fill(trans(shrink(lamps[i].lamparea,0.8),shadelta));
	    color(RED);
	    fill(shrink(lamps[i].lamparea,0.8));
	    color(GREY(0));
	    outline(shrink(lamps[i].lamparea,0.8));
	}

	for (i=0; i<NKEYS; i++) {
	    fill(trans(shrink(keys[i].keyarea,0.8),shadelta));
	    if (keys[i].keytype == ALPHANUM)
		color(GREY(15));
	    else
		color(GREY(12));
	    fill(shrink(keys[i].keyarea,0.8));
	    color(GREY(0));
	    outline(shrink(keys[i].keyarea,0.8));
	}
	closeobj();
	firsted = 1;
    }
    callobj(10);
}
