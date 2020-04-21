    /*
     * typer -- a test program for use of textframes in an editor
     *
     *				Rob Myers, June 30, 1986
     */

#include <gl.h>
#include <device.h>
#include <fcntl.h>
#include <stdio.h>
#include "tf.h"
/*
 #include "bitmap.h"
 #include "debug.h"
*/
#define	Dbg(x, y)

#define WINWD 800
#define WINHT 400
#define WINMINHT 30

	    /* methods for redrawing text; changed lines only or all lines */
#define CHANGED 0
#define ALL 1
	    /* methods for drawing and undrawing text input point */
#define OFF 0
#define ON  1
	    /* methods for hilighting the selection */
#define HILIGHT   0
#define DEHILIGHT 1

typedef struct {
    int x, y;
    int inbounds;
} locate;

typedef struct {
    int col, row;
} colrow;

textframe *tf;
textview *tv;
/*Font *fnt;*/
int xorg, yorg, xsize, ysize;

char *getenv();
char *fullfontname();

main(argc,argv)
    int argc;
    char **argv;
{
    char *font = NULL;
    char *file = NULL;
    short val;
    int menu;
    locate loc;
    int inbounds;

    if (argc<2) {
	fprintf(stderr,"usage: %s [-f font] file\n", argv[0]);
	exit(1);
    }

    while (--argc) {
	if ((*++argv)[0] == '-') {
	    switch ((*argv)[1]) {
		case 'f':
		    font = *++argv;
		    argc--;
		    break;
		default:
		    break;
	    }
	}
	else {
	    file = *argv;
	    break;
	}
    }

    foreground();
    prefsize(WINWD, WINHT);
    winopen("tv");
    minsize(WINWD, WINMINHT);
    winconstraints();

    qdevice(LEFTMOUSE);
    qdevice(MIDDLEMOUSE);
    qdevice(MENUBUTTON);

    tf = tfnew();
/*
    fntdefine(1,fntread(fullfontname(font)));
    tffont(tf,1);
*/
    tftextcolor(tf,0);
    tfbackcolor(tf,7);

    readtext(tf, file);
    tv = tvnew(tf);
    tvhighlight(tv, 9, 0);

    tvtoprow(tv,0);
    tvleftpix(tv,2);	/* leave room for text input cursor before column 0 */
    makeframe();

    menu = defpup("text edit %t|cut|copy|paste|quit");

    while(1) {
	switch(qread(&val)) {
	    case REDRAW:
		makeframe();
		break;
	    case MENUBUTTON:
		if (val) {
		    switch(dopup(menu)) {
			case 1:
			    printf("typer: hit cut button\n");
			    break;
			case 2:
			    printf("typer: hit copy button\n");
			    break;
			case 3:
			    printf("typer: hit paste button\n");
			    break;
			case 4:
			    exit();
			    break;
			default:
			    break;
		    }
		}
		break;
	    case LEFTMOUSE:
		if (val) {				/* button comes down */
		    if (inbounds = getloc(&loc)) {	/*   in text window  */
			qdevice(MOUSEX);
			qdevice(MOUSEY);
			begsel(&loc);
		    }
		}
		else if (inbounds) {			/* button comes up   */
		    unqdevice(MOUSEX);
		    unqdevice(MOUSEY);
		    getloc(&loc);
		    endsel(&loc);
		}
		break;
	    case MOUSEX:
	    case MOUSEY:
		if (!qtest() && getloc(&loc))		/* button stays down, */
		    contsel(&loc);			/*   dragged          */
		break;
	}
    }
}

colrow beg = {0, 0};
colrow sel = {0, 0};	/* default selection before 1st character */
colrow cur;
int begselpending;

begsel(loc)
    locate *loc;
{
    begselpending = TRUE;
    if (tvpixtopos(tv, loc->x, loc->y, &cur.row, &cur.col)) {
	/* tracesel("begin select", loc, &cur); */
	begselpending = FALSE;

	plotptsel(OFF);
	if (dir(&beg, &sel) == 0) {
	    sel.col++;
	    selhilight(HILIGHT, &beg, &sel);	/* repair text under ptsel */
	}
	selhilight(DEHILIGHT, &beg, &sel);

	beg = cur;
	sel = cur;
	plotptsel(ON);
    }
}

contsel(loc)
    locate *loc;
{
    if (begselpending)
	begsel(loc);

    if (tvpixtopos(tv, loc->x, loc->y, &cur.row, &cur.col)) {
	/* tracesel("continue select", loc, &cur); */
	extendsel(loc, &cur);
    }
}

endsel(loc)
    locate *loc;
{
    if (begselpending)
	begsel(loc);

    if (tvpixtopos(tv, loc->x, loc->y, &cur.row, &cur.col)) {
	/* tracesel("end select", loc, &cur); */
	extendsel(loc, &cur);
    }
}

extendsel(loc)
    locate *loc;
{
    static int prevseldirection = 0;
    int seldirection, extdirection;

    /* tracesel("extend select cur", loc, &cur); */

    if (extdirection = dir(&sel, &cur)) {
	seldirection = dir(&beg, &cur);
	plotptsel(OFF);
	if (seldirection == prevseldirection) {
	    if (extdirection != seldirection)
		 selhilight(DEHILIGHT, &sel, &cur);
	    else selhilight(HILIGHT, &sel, &cur);
	}
	else {
	    selhilight(DEHILIGHT, &sel, &beg);
	    selhilight(HILIGHT, &beg, &cur);
	    prevseldirection = seldirection;
	}
	sel = cur;
	plotptsel(ON);
    }
}

selhilight(action, end1, end2)
    int action;
    colrow *end1, *end2;
{
    long looks;

    switch (action) {
        case HILIGHT:
	    looks = LOOKS_SELECT;
	    break;
        case DEHILIGHT:
	    looks = 0;
	    break;
	default:
	    return;
    }

    tfsetmark(tf, end1->row, end1->col);
    tfsetpoint(tf, end2->row, end2->col);

    tfsetlooks(tf, looks);
    tfsetwritemask(tf, LOOKS_SELECT);
    tfchangelooks(tf);
    drawtext(CHANGED);
}

plotptsel(status)
    int status;
{
    long x, y, rowht, clr;

    if (status == ON)
	 clr = BLUE;
    else clr = WHITE;

    tvpostopix(tv, sel.row, sel.col, &x, &y);
    rowht = tvrowheight(tv, sel.row);
    color(clr);
    rectfi(x-1, y, x, y+rowht-1);
}

tracesel(string, loc, cur)
    char *string;
    locate *loc;
    colrow *cur;
{
    printf("typer: %s at (%d,%d)%d", string, loc->x,loc->y,loc->inbounds);
    printf(" row=%d, col=%d\n", cur->row, cur->col);
}

readtext(tf,name)
textframe *tf;
char *name;
{
    FILE *f;
    char oneline[1024];
    int n;

    f = fopen(name,"r");
    if(!f) {
	fprintf(stderr,"can't open file %s\n",name);
	exit(1);
    }
    while(1) {
	if(!fgets(oneline,1024,f)) {
	    fclose(f);
  	    return;
	}
	tfputascii(tf,oneline,strlen(oneline));
	tfsplit(tf);
    }
}

makeframe()
{
    reshapeviewport();
    getsize(&xsize, &ysize);
    getorigin(&xorg, &yorg);
    ortho2(-0.5, xsize-0.5, -0.5, ysize-0.5);
    tvviewsize(tv, xsize, ysize);
    drawtext(ALL);
    plotptsel(ON);
}

drawtext(how)
    long how;
{
    tvdraw(tv, how);
}

getloc(loc)
    register locate *loc;
{
    int xclamped, yclamped;

    loc->x = clamp(getvaluator(CURSORX)-xorg, 0, xsize-1, &xclamped);
    loc->y = clamp(getvaluator(CURSORY)-yorg, 0, ysize-1, &yclamped);

    return (loc->inbounds = !(xclamped || yclamped));
}

clamp(value, minvalue, maxvalue, clamped)
    register int value, minvalue, maxvalue;
    register int *clamped;
{
    *clamped = TRUE;
    if (value < minvalue)
	return minvalue;
    if (value > maxvalue)
	return maxvalue;

    *clamped = FALSE;
    return value;
}

#define sgn(a) (((a)==0) ? 0 : (((a)<0) ? -1 : 1))

dir(old, new)
    colrow *old, *new;
{
    int dir;

    dir = sgn(new->row - old->row);
    if (dir == 0)
	dir = sgn(new->col - old->col);
    return dir;
}

    char *
fullfontname(preffont)
    char *preffont;
{
    static char fontname[BUFSIZ];
    char *s;

    fontname[0] = '\0';
    if (s=getenv("FONTLIB")) {		/* use env's pathname by default */
	strcat(fontname, s);
	strcat(fontname, "/");
    }
    if (preffont) {
	if (preffont[0] == '.' || preffont[0] == '/')
	     strcpy(fontname, preffont);	/* clobber pathname */
	else strcat(fontname, preffont);
    }
    else {
	if (s=getenv("FONT"))
	     strcat(fontname, s);
	else strcat(fontname, "default.fnt");
    }
    return fontname;
}
