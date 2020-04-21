#include <gl.h>
#include <device.h>
#include <fcntl.h>
#include <stdio.h>
#include "tf.h"

textframe *tf;
textview *tv;

main(argc,argv)
int argc;
char **argv;
{
    short val;
    int toprow;
    int xsize, ysize;
    float pct;
    short leftmouse, mousex, mousey;
    char needupdate;

    if(argc<2) {
	fprintf(stderr,"usage: grtest file\n");
	exit(1);
    }
    tf = tfnew();
    tfsetlooks(tf, (0 << LOOKS_FONTSHIFT) |
		   (0 << LOOKS_FGSHIFT) |
		   (7 << LOOKS_BGSHIFT));
    readtext(tf,argv[1]);

    if (getenv("DEBUG"))
	foreground();
    /* prefsize(400, 500); */
    winopen("tv");
    tv = tvnew(tf);
    getsize(&xsize, &ysize);
    tvviewsize(tv, xsize, ysize);
    tvmapfont(tv, 0, "mel.fnt");
    tvdraw(tv,1);

    toprow = 0;
    qdevice(MENUBUTTON);
    qdevice(LEFTMOUSE);
    needupdate = 0;
    for (;;) {
	if (qtest(&val) == 0) {
		if (needupdate) {
			tvtoprow(tv, toprow);
			tvdraw(tv, 0);
			needupdate = 0;
		}
	}
	switch(qread(&val)) {
	    case REDRAW:
		getsize(&xsize, &ysize);
		viewport(0, xsize - 1, 0, ysize - 1);
		ortho2(-0.5, xsize - 1 + 0.5, -0.5, ysize + 0.5);
		tvviewsize(tv, xsize, ysize);
		tvdraw(tv,1);
		break;
	    case LEFTMOUSE:
		if (val)
			qdevice(MOUSEY);
		else
			unqdevice(MOUSEY);
		break;
	    case MOUSEY:
		pct = ((float) YMAXSCREEN - (float) val) /
				(float) YMAXSCREEN;
		toprow = tfnumrows(tf) * pct;
		needupdate = 1;
		break;
	    case MENUBUTTON:
	   	exit();
		break;
	}
    }
}

readtext(tf,name)
textframe *tf;
char *name;
{
    FILE *f;
    char oneline[1024];
    int n;
    long row, col;

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
