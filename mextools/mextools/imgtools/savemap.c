/*
 *	savemap - 
 *		Save part or all of the color map in a file.
 *
 *				Paul Haeberli - 1984
 *
 */
#include "image.h"
#include "gl.h"

short rowbuf[4096]; 

main(argc,argv)
int argc;
char **argv;
{
    register IMAGE *image;
    register int i, min, max;
    unsigned short r, g, b;
    char mapname[20];		/* unix length limit + .map */

    if( argc<2 ) {
	fprintf(stderr,"usage: savemap <outfile> [-r min max]\n");
	exit(1);
    } 
    if((argc == 5) && (strcmp(argv[2],"-r") == 0) ) {
	min = atoi(argv[3]);
	max = atoi(argv[4]);
    } else {
	min = 0;
	max = 511;
    }
    foreground();
    noport();
    winopen("savemap");
    if( (image=iopen(argv[1],"w",VERBATIM(2),2,4,max-min+1)) == NULL ) {
	fprintf(stderr,"savemap: can't open input file %s\n",argv[1]);
	exit(1);
    }

	/* append .map to name for printer useage */
    strncpy(mapname,argv[1],15);
    mapname[15] = '\0';
    strcat(mapname,".map");
    isetname(image,mapname);
    image->colormap = CM_COLORMAP;

    if(getplanes() < 8)  {
	if(max > 63)
	     max = 63;
    }
    for(i=min; i<=max; i++) {
	gamgetmcolor(i,&r,&g,&b);
	putpix(image,i);
	putpix(image,r);
	putpix(image,g);
	putpix(image,b);
    }
    iclose(image);
    gexit();
}
