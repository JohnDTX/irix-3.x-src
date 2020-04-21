/*
 *	fade - 
 *		Make a background that fades windows out using a special
 *		pixel hack.
 *
 *				Paul Haeberli - 1985
 *
 */
#include "gl.h"
#include "device.h"
#include "port.h"

short tex[16];
int slow;

int shifts[16] = {
    0, 2, 2, 0,
    1, 3, 3, 1,
    0, 2, 2, 0,
    1, 3, 3, 1,
};

int wheres[16] = {
    0, 2, 0, 2,
    1, 3, 1, 3,
    1, 3, 1, 3,
    0, 2, 0, 2,
};

main(argc,argv)
int argc;
char **argv;
{
    short val;
    int i, texno;

    slow = 0;
    if (argc>1)
	slow = atoi(argv[1]);
    imakebackground();
    winopen("fade");
    background();
    while (1) {
	if (qread(&val) == REDRAW) {
	    qreset();
	    background();
	}
    }
}

background()
{
    int i, k, texno;
    register int shift, where, pattern;

    for (texno = 0; texno<16; texno++) {
	for (i=0; i<16; i++)
	    tex[i] = 0;
	shift = shifts[texno]; 	
	where = wheres[texno]; 	
	pattern = 0x1111<<shift;
	tex[where+0] = pattern;
	tex[where+4] = pattern;
	tex[where+8] = pattern;
	tex[where+12] = pattern;
	defpattern(2,16,tex);	/* define a pattern */
	color(BACKGROUND2);
	setpattern(2);
	if(slow)
	    sleep(slow);
	clear();
    }
}
