/* "popuputil.c" */
#include "gl.h"
#include "device.h"
#include "popup.h"

initpopup()
{
    register i;

    mapcolor(BLACKDRAW, 0, 0, 0);	/* background only */
    mapcolor(GREENDRAW, 0, 255, 0);	/* green drawing */
    mapcolor(REDDRAW, 255, 0, 0);	/* red drawing */
    mapcolor(YELLOWDRAW, 255, 255, 0);	/* yellow drawing */
    for (i = 4; i < 8; i++)
	mapcolor(i, 0, 0, 0);	/* popup background */
    for (i = 8; i < 12; i++)
	mapcolor(i, 255, 255, 255);	/* popup text */
    for (i = 12; i < 16; i++)
	mapcolor(i, 150, 150, 150);	/* popup highlight */
    setcursor(0, 8, 15);
    qdevice(LEFTMOUSE);
    tie(LEFTMOUSE, MOUSEX, MOUSEY);
}

popup(names)
popupentry names[];
{
    register short i, menucount;
    short menutop, menubottom, menuleft, menuright;
    short lasthighlight = -1, highlight;
    Device dummy, x, y;
    short savecolor, savemask;

    menucount = 0;
    qread(&dummy);
    qread(&x);
    qread(&y);
    pushattributes();
    pushviewport();
    pushmatrix();
    viewport(0, 1023, 0, 767);
    ortho2(-0.5, 1023.5, -0.5, 767.5);
    while (names[menucount].type)
	menucount++;
    menutop = y + menucount*8;
    menubottom = y - menucount*8;
    if (menutop > 767) {
	menutop = 767;
	menubottom = menutop - menucount*16;
    }
    if (menubottom < 0) {
	menubottom = 0;
	menutop = menubottom + menucount*16;
    }
    menuleft = x - 100;
    menuright = x + 100;
    if (menuleft < 0) {
	menuleft = 0;
	menuright = menuleft + 200;
    }
    if (menuright > 1023) {
	menuright = 1023;
	menuleft = 823;
    }
    writemask(12);	/* restrict to menu planes */
    color(4);		/* menu background */
    cursoff();
    rectfi(menuleft, menubottom, menuright, menutop);
    color(8);		/* menu text */
    move2i(menuleft, menubottom);
    draw2i(menuleft, menutop);
    draw2i(menuright, menutop);
    draw2i(menuright, menubottom);
    for (i = 0; i < menucount; i++) {
	move2i(menuleft, menutop - (i+1)*16);
	draw2i(menuright, menutop - (i+1)*16);
	cmov2i(menuleft + 10, menutop - 14 - i*16);
	charstr(names[i].text);
    }
    curson();
    while (1) {
	x = getvaluator(MOUSEX);
	y = getvaluator(MOUSEY);
	if (menuleft < x && x < menuright && menubottom < y && y < menutop) {
	    highlight = (menutop - y)/16;
	    cursoff();
	    if (lasthighlight != -1 && lasthighlight != highlight) {
		color(4);
		rectfi(menuleft+1, menutop - lasthighlight*16 - 15,
		       menuright-1, menutop - lasthighlight*16 - 1);
		color(8);				
		cmov2i(menuleft + 10, menutop - 14 - lasthighlight*16);
		charstr(names[lasthighlight].text);
	    }
	    if (lasthighlight != highlight) {
		color(12);
		rectfi(menuleft+1, menutop - highlight*16 - 15,
		       menuright-1, menutop - highlight*16 - 1);
		color(8);
		cmov2i(menuleft + 10, menutop - 14 - highlight*16);
		charstr(names[highlight].text);
	    }
	    lasthighlight = highlight;
	    curson();
	} else /* the cursor is outside the menu */ {
	    if (lasthighlight != -1) {
		cursoff();
		color(4);
		rectfi(menuleft+1, menutop - lasthighlight*16 - 15,
		       menuright-1, menutop - lasthighlight*16 - 1);
		color(8);				
		cmov2i(menuleft + 10, menutop - 14 - lasthighlight*16);
		charstr(names[lasthighlight].text);
		curson();
		lasthighlight = -1;
	    }
	}
	if (qtest()) {
	    qread(&dummy);
	    qread(&x);
	    qread(&y);
	    color(0);
	    cursoff();
	    rectfi(menuleft, menubottom, menuright, menutop);
	    curson();
	    if (menuleft<x && x<menuright && menubottom<y && y<menutop)
		x = (menutop - y)/16;
	    else
		x = 0;
	    break;
	}
    }
    popmatrix();
    popviewport();
    popattributes();
    return names[x].type;
}
