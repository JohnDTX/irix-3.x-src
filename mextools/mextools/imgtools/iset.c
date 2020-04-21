/*
 *	iset - 
 *		Set the colormap of an image.  This determines which part 
 *	        of the color map ipaste uses to display the image.
 *
 *				Paul Haeberli - 1984
 *
 */
#include "ctype.h"
#include "image.h"

IMAGE imheader;

main(argc,argv)
int argc;
char **argv;
{
    int imfile;
    int i;
    char type[60];
    int newtype;

    if( argc<3 ) {
	fprintf(stderr,"usage: iset newtype imgfiles\n");
	exit(1);
    } 
    strcpy(type,argv[1]);
    strtoupper(type);
    if(strcmp(type,"NORMAL") == 0)
	     newtype = CM_NORMAL;
    else if(strcmp(type,"DITHERED") == 0)
	     newtype = CM_DITHERED;
    else if(strcmp(type,"SCREEN") == 0)
	     newtype = CM_SCREEN;
    else if(strcmp(type,"COLORMAP") == 0)
	     newtype = CM_COLORMAP;
    else {
	fprintf(stderr,"usage: iset newtype imgfiles\n");
	exit(1);
    } 
    for(i=2; i<argc; i++) {
	if( (imfile=open(argv[i],2)) == -1) {
	    fprintf(stderr,"iset: can't open input file %s\n",argv[i]);
	    exit(1);
	}
	if(read(imfile,&imheader,sizeof(IMAGE)) != sizeof(IMAGE)) {
	    fprintf(stderr,"iset: error on read\n");
	    exit(1);
	}
	if(imheader.imagic == IMAGIC)
	    imheader.colormap = newtype;
	else
	    imheader.colormap = reverse(newtype);
	lseek(imfile,0,0);
	if(write(imfile,&imheader,sizeof(IMAGE)) != sizeof(IMAGE)) {
	    fprintf(stderr,"error on write\n");
	    exit(1);
	}
	close(imfile);
    }
}

strtoupper(str)
char *str;
{
    while(*str) {
	if(islower(*str))
		*str = toupper(*str);
	str++;
    }
}

reverse(lwrd) 
register unsigned long lwrd;
{
    return ((lwrd>>24) 		| 
	   (lwrd>>8 & 0xff00) 	| 
	   (lwrd<<8 & 0xff0000) | 
	   (lwrd<<24) 		);
}
