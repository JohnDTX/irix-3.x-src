/*
 *	vis - 
 *		Copy the bytes of a file to the screen - this will only work
 *	        right on a system with at least 12 bit planes. 	 Try looking 
 *		at excecutables, libraries, textfiles.
 *
 *				Paul Haeberli - 1985
 *
 */
#include "stdio.h"
#include "port.h"
#include "device.h"

char cbuf[1024];
short sbuf[1024];
int inf;
int forcemode;
int widthstep = 16;

main(argc, argv)
int argc;
char **argv;
{
    int i;

    if (argc<2) {
	fprintf(stderr,"usage: vis <file> [-f] [-w widthstep] \n");
	exit(1);
    }
    for (i=2; i<argc; i++) {
	if (strcmp(argv[i],"-f") == 0)
	    forcemode++;
	else if (strcmp(argv[i],"-w") == 0)
	    widthstep = atoi(argv[++i]);
    }
    inf = open(argv[1],0);
    if (inf<0) {
	fprintf(stderr,"couldn't open file %s\n",argv[1]);
	exit(1);
    }
    stepunit(widthstep,1);
    winopen("vis");
    wintitle("vis");
    if(forcemode) {
	noise(TIMER0,60);
	qdevice(TIMER0);
    }
    while(1)
	drawit();
}

drawit()
{
    int xsize, ysize;
    int i, y, nbytes;
    int xpos;
    short val;

    reshapeviewport();
    getsize(&xsize,&ysize);
    ortho2(-0.5,xsize+0.5,-0.5,ysize+0.5); 
    color(GREY(8));
    clear();
    lseek(inf,0,0);
    for (y=0; y<ysize; y++) {
	nbytes = read(inf,cbuf,xsize);
	expand(cbuf,sbuf,nbytes);
	cmov2i(0,y);
	writepixels(nbytes,sbuf);
	xpos = nbytes;
	if (nbytes != xsize) 
	    break;
    }
    while (1) {
	switch(qread(&val)) {
	    case REDRAW:
		return;
	    case TIMER0:
		while (y<ysize) {
		    nbytes = read(inf,cbuf,xsize-xpos);
		    if (nbytes) {
			expand(cbuf,sbuf,nbytes);
			cmov2i(xpos,y);
			writepixels(nbytes,sbuf);
		    }
		    if (nbytes != xsize-xpos) {
			xpos += nbytes;
		        break;
		    }
		    y++;
		    xpos = 0;
		}
		break;
	}
    }
}

expand(cptr,sptr,n)
register unsigned char *cptr;
register unsigned short *sptr;
register int n;
{
    while (n--)
	*sptr++ = 256 + *cptr++;
}
