/*
 *  	loadfont -
 *		Load a font to replace the system font.  If the window 
 *		manager is running, this will not work.
 *
 */
#include "gl.h"
#include "grioctl.h"
#include "stdio.h"


main(argc, argv)
int argc;
char **argv;
{
    short ht, nc, nr;
    Fontchar *chars;
    short *raster;
    short *data = (short *)gl_smallbufaddr();

    int file, i;

    if(argc < 2) {
	fprintf(stderr, "usage: loadfont <iris.fnt>\n");
	exit(1);
    }
    if( (file = pathopen(argv[1], 0)) < 0 ) {
	fprintf(stderr, "can't open input file\n");
	exit(1);
    }
    if(ismex()) {
	fprintf(stderr, "newfont: can't be run if mex is running\n");
	exit(1);
    }
    chars = (Fontchar *)malloc(256*sizeof(Fontchar));
    raster = (short *)malloc(256*256);
    if(chars == 0 || raster == 0) {
	printf(stderr, "loadfont: malloc failed\n");	
	exit(1);
    }
    readfont(file, &ht, &nc, chars, &nr, raster);
    ginit();
    
    data[0] = gl_findwidth(chars,nc);		/* width */
    data[1] = ht;				/* height */
    data[2] = gl_finddescender(chars,nc);	/* descender */
    grioctl(GR_SETCHARINFO, 0);

    data[0] = nc;
    grioctl(GR_SETCHAROFFSETS, chars);

    *(long *)data = (long)raster;
    data[2] = nr; 
    grioctl(GR_SETCHARMASKS, 0);

    color(0);
    cursoff();
    clear();
    textport(0, XMAXSCREEN, 0, YMAXSCREEN);
    gexit();
}
