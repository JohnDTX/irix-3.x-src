/*
 *	istat - 
 *		Print the header of an image file.
 *
 *				Paul Haeberli - 1984
 *
 */
#include "image.h"

main(argc,argv)
int argc;
char **argv;
{
    IMAGE *image;
    int i;

    if( argc<2 ) {
	fprintf(stderr,"usage: istat imgfiles\n");
	exit(1);
    } 
    for(i=1; i<argc; i++) {
	if(argc>2)
	    printf("\n%s:\n",argv[i]);
	if( (image=iopen(argv[i],"r")) == NULL ) {
	    fprintf(stderr,"istat: can't open input file %s\n",argv[1]);
	    exit(1);
	}
	if(ISRLE(image->type)) 
	    printf("rle image - bytes wasted %d:\n",image->wastebytes);
	else
	    printf("verbatum image:\n");
	printf("Bytes per pixel: %d    Dimensions: %d ",
					BPP(image->type),image->dim);
	printf("    x: %d",image->xsize);
	printf("    y: %d",image->ysize);
	printf("    z: %d",image->zsize);
	printf("\n");
	printf("Min: %d    Max: %d\n",image->min,image->max);
	printf("Name: %s     ",image->name);
	if(image->colormap == CM_NORMAL)
	    printf("Image is of type NORMAL\n");
	else if(image->colormap == CM_DITHERED)
	    printf("Image is of type DITHERED\n");
	else if(image->colormap == CM_SCREEN)
	    printf("Image is of type SCREEN\n");
	else if(image->colormap == CM_COLORMAP)
	    printf("Image is of type COLORMAP\n");
	else
	    printf("Image Type is wierd!!\n");
	iclose(image);
    }
}
