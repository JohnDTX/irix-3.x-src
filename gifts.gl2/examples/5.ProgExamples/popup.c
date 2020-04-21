/* "popup.c" */
#include "gl.h"
#include "device.h"
#define LINE 1
#define POINTS 2
#define CIRCLE 3
#define RECT 4
#define RECTF 5
#define QUIT 6
typedef struct {
    short type;
    char *text;
} popupentry;

popupentry mainmenu[] = {
    {LINE, "Line"},
    {POINTS, "100 points"},
    {CIRCLE, "Filled circle"},
    {RECT, "Outlined rectangle"},
    {RECTF, "Filled rectangle"},
    {QUIT, "Quit"},
    {0, 0}	/* mark end of menu */
};
main()
{
    register i, j;
    short command;

    ginit();
    mapcolor(0, 0, 0, 0);	/* background only */
    mapcolor(1, 0, 255, 0);	/* drawing only */
    mapcolor(2, 0, 0, 0);	/* popup background */
    mapcolor(3, 0, 0, 0);	/* popup background over drawing */
    mapcolor(4, 255, 255, 255);	/* popup text only */
    mapcolor(5, 255, 255, 255);	/* popup text over drawing */
    mapcolor(6, 100, 100, 100);	/* popup highlight only */
    mapcolor(7, 100, 100, 100);	/* popup highlight over drawing */
    setcursor(0, 4, 7);
    qdevice(LEFTMOUSE);
    tie(LEFTMOUSE, MOUSEX, MOUSEY);

    cursoff();
    color(0);
    clear();
    curson();
    viewport(150, 850, 50, 750);
    ortho2(-1.0, 1.0, -1.0, 1.0);
    while (1) {
	command = popup(mainmenu);
	cursoff();
	color(0);
	clear();
	color(1);
	switch(command) {
	    case LINE:
		move2(-1.0, -1.0);
		draw2(1.0, 1.0);
		break;
	    case POINTS:
		for (i =  0; i < 10; i++)
		    for (j = 0; j < 10; j++)
			pnt2(i/20.0, j/20.0);
		break;
	    case CIRCLE:
		circf(0.0, 0.0, 0.5);
		break;
	    case RECT:
		rect(-0.5, -0.5, 0.5, 0.5);
		break;
	    case RECTF:
		rectf(-0.5, -0.5, 0.5, 0.5);
		break;
	    case QUIT:
		greset();
		gexit();
		exit(0);
	}
	curson();
    }
}

popup(names)
popupentry names[];
{
    register short i, menucount;
    short menutop, menubottom, menuleft, menuright;
    short lasthighlight = -1, highlight;
    Device dummy, x, y;
    short savecolor, savemask;
    short llx, lly, urx, ury;

    menucount = 0;
    qread(&dummy);
    qread(&x);
    qread(&y);
    savecolor = getcolor();		/* save the state of everything */
    savemask = getwritemask();
    getviewport(&llx, &urx, &lly, &ury);
    pushmatrix();
    viewport(0, 1023, 0, 767);		/* now setup to draw the menu */
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
    writemask(6);	/* restrict to menu planes */
    color(2);		/* menu background */
    cursoff();
    rectfi(menuleft, menubottom, menuright, menutop);
    color(4);		/* menu text */
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
		color(2);
		rectfi(menuleft+1, menutop - lasthighlight*16 - 15,
		       menuright-1, menutop - lasthighlight*16 - 1);
		color(4);				
		cmov2i(menuleft + 10, menutop - 14 - lasthighlight*16);
		charstr(names[lasthighlight].text);
	    }
	    if (lasthighlight != highlight) {
		color(6);
		rectfi(menuleft+1, menutop - highlight*16 - 15,
		       menuright-1, menutop - highlight*16 - 1);
		color(4);
		cmov2i(menuleft + 10, menutop - 14 - highlight*16);
		charstr(names[highlight].text);
	    }
	    lasthighlight = highlight;
	    curson();
	} else /* the cursor is outside the menu */ {
	    if (lasthighlight != -1) {
		cursoff();
		color(2);
		rectfi(menuleft+1, menutop - lasthighlight*16 - 15,
		       menuright-1, menutop - lasthighlight*16 - 1);
		color(4);				
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
    popmatrix();	/* now restore the state to what the user had */
    color(savecolor);
    writemask(savemask);
    viewport(llx, urx, lly, ury);
    return names[x].type;
}
