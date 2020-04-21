/*
 *	interp -
 *		Interpolate between colors.	
 *
 *				Henry Moreton - 1985
 */
#include "gl.h"
#include "math.h"
#include "stdio.h"
#include "device.h"

#define MINX 0.0
#define MINY 0.0
#define MAXX 20.0
#define MAXY 30.0

#define MIN(x,y) ((x) > (y) ? (y) : (x))

int startcolor = 128;
int endcolor = 255;
char pickingstart = TRUE;
float startr,startg,startb;
float endr,endg,endb;

char *colormodels[] = 	{
    "rgb", "cmy", "hsv", "hls"
};

main(argc, argv)
int arc;
char **argv;
{
    short	dev, val;
    int		scrx, scry;
    short	mousex, mousey;
    int 	titles;
    int		sel, menu;

    titles = 0;
    if (argc > 1) {
	setcolorsys(atoi(argv[1]));
        titles = 1;
    }
    keepaspect(3,2);
    winopen("interp");
    ortho2(MINX, MAXX, MINY, MAXY);
    qdevice(MIDDLEMOUSE);
    qdevice(MENUBUTTON);
    qdevice(LEFTMOUSE);
    tie(LEFTMOUSE, MOUSEX, MOUSEY);
    menu = defpup("colorsys %t|rgb|cmy|hsv|hls");
    if (titles)
	wintitle(colormodels[atoi(argv[1])-1]);
    reshapeviewport();
    drawit();
    while (1)  {
	switch (dev = qread(&val)) {
	    case REDRAW :
		reshapeviewport();
		drawit();
		break;
	    case LEFTMOUSE :
		dev = qread(&mousex);
		dev = qread(&mousey);
		if (val) {
		    if (pickingstart)
			startcolor = getapixel(mousex,mousey);
		    else
			endcolor = getapixel(mousex,mousey);
		    pickingstart = !pickingstart ;
		    drawit();
		}
		break;
	    case MIDDLEMOUSE :
		if (val) 
		    interpolate();
	        break;
	    case MENUBUTTON :
		sel = dopup(menu);
		if ((sel > 0) && (sel <= 4)) {
		    setcolorsys(sel);
		    if (titles)
			wintitle(colormodels[sel-1]);
		    drawit();
		    interpolate();
		}
		break;
	}
    }
}

drawit()
{
    color(WHITE);
    clear();
    setupstartcolor();
    setupendcolor();
    drawramp();
}

setupstartcolor()
{
    unsigned short sr,sg,sb;
    float fr,fg,fb;

    color(startcolor);
    rectfi((int)(1+MINX), 
	   (int)(MINY+1+(MAXY-MINY)*0.25), 
	   (int)(MINX-1+(MAXX-MINX)/2.0), 
	   (int)(MAXY-1));

    if (pickingstart) {
	linewidth(3);
	color(RED);
	recti((int)(1+MINX), (int)(MINY+1+(MAXY-MINY)*0.25), 
	   (int)(MINX-1+(MAXX-MINX)/2.0), (int)(MAXY-1));
        linewidth(1);
    }
    color(BLACK);
    recti((int)(1+MINX), (int)(MINY+1+(MAXY-MINY)*0.25), 
       (int)(MINX-1+(MAXX-MINX)/2.0), (int)(MAXY-1));

    modgetmcolor(startcolor,&startr,&startg,&startb);
}

setupendcolor()
{
    color(endcolor);
    rectfi((int)(1+MINX+(MAXX-MINX)/2.0), 
	   (int)(MINY+1+(MAXY-MINY)*0.25), 
	   (int)(MAXX-1.0), 
	   (int)(MAXY-1));
    if (!pickingstart) {
	linewidth(3);
	color(RED);
	recti((int)(1+MINX+(MAXX-MINX)/2.0), (int)(MINY+1+(MAXY-MINY)*0.25), 
	   (int)(MAXX-1.0), (int)(MAXY-1));
        linewidth(1);
    }
    color(BLACK);
    recti((int)(1+MINX+(MAXX-MINX)/2.0), (int)(MINY+1+(MAXY-MINY)*0.25), 
       (int)(MAXX-1.0), (int)(MAXY-1));

    modgetmcolor(endcolor,&endr,&endg,&endb);
}

interpolate()
{
    float a, b, c;
    float da, db, dc;
    float red, green, blue;
    unsigned int ired, igreen, iblue;
    int color, delcolor;

    modgetmcolor(startcolor,&startr,&startg,&startb);
    modgetmcolor(endcolor,&endr,&endg,&endb);
    delcolor = endcolor-startcolor;
    a = startr; 
    b = startg; 
    c = startb;
    da = endr - a;
    db = endg - b;
    dc = endb - c;

    if (delcolor) {
	da /= abs(delcolor);
	db /= abs(delcolor);
	dc /= abs(delcolor);
    }
    if (delcolor < 0)
	for (color = startcolor; color >= endcolor; color--) {
	    if(color!=startcolor && color !=endcolor) 
		modmapcolor(color,a,b,c);
	    a += da;
	    b += db;
	    c += dc;

	}
    else
	for (color = startcolor; color <= endcolor; color++) {
	    if(color!=startcolor && color !=endcolor) 
		modmapcolor(color,a,b,c);
	    a += da;
	    b += db;
	    c += dc;
	}
}

drawramp()
{
    short intens[4];
    float parray[4][2];

    intens[0] = startcolor;
    intens[1] = startcolor;
    intens[2] = endcolor;
    intens[3] = endcolor;

    parray[0][0] = MINX+1.0; 
    parray[0][1] = MINY+1.0; 
    parray[1][0] = MINX+1.0; 
    parray[1][1] = MINY-1.0+(MAXY-MINY)*0.25;
    parray[2][0] = MAXX-1.0;
    parray[2][1] = MINY-1.0+(MAXY-MINY)*0.25;
    parray[3][0] = MAXX-1.0;
    parray[3][1] = MINY+1.0; 

    splf2(4, parray, intens);
    color(BLACK);
    rect(MINX+1.0, MINY+1.0, MAXX-1.0, MINY-1.0+(MAXY-MINY)*0.25);
}

float clamp(f)
float f;
{
    if(f<0.0)
	f = 0.0;
    if(f>1.0)
	f = 1.0;
    return f;
}

modmapcolor(c,r,g,b)
int c;
float r, g, b;
{
    float fr, fg, fb;
    int ir, ig, ib;

    torgb(r,g,b,&fr,&fg,&fb);
    rgb_to_irgb(fr,fg,fb,&ir,&ig,&ib);
    gammapcolor(c,ir,ig,ib);
}

modgetmcolor(c,r,g,b)
int c;
float *r, *g, *b;
{
    unsigned short cr, cg, cb;
    float fr, fg, fb;

    gamgetmcolor(c,&cr,&cg,&cb);
    irgb_to_rgb(cr,cg,cb,&fr,&fg,&fb);
    fromrgb(fr,fg,fb,r,g,b);
    *r = clamp(*r);
    *g = clamp(*g);
    *b = clamp(*b);
}
