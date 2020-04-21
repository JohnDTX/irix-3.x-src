/*
 *	iclose and iflush -
 *
 *				Paul Haeberli - 1984
 *
 */
#include	<stdio.h>
#include	<errno.h>
#include	"image.h"

iclose(image)
register IMAGE 	*image;
{
    long tablesize;

    iflush(image);
    img_optseek(image, 0);
    if (image->flags&_IOWRT) {
	if(image->dorev)
	    cvtimage(image);
	if (img_write(image,image,sizeof(IMAGE)) != sizeof(IMAGE)) {
	    fprintf(stderr,"iflush: error on write of image header\n");
	    exit(1);
	}
	if(image->dorev)
	    cvtimage(image);
	if(ISRLE(image->type)) {
	    img_optseek(image, 512L);
	    tablesize = image->ysize*image->zsize*sizeof(long);
	    if(image->dorev)
		cvtlongs(image->rowstart,tablesize);
	    if (img_write(image,image->rowstart,tablesize) != tablesize) {
		fprintf(stderr,"iflush: error on write of rowstart\n");
		exit(1);
	    }
	    if(image->dorev)
		cvtlongs(image->rowsize,tablesize);
	    if (img_write(image,image->rowsize,tablesize) != tablesize) {
		fprintf(stderr,"iflush: error on write of rowsize\n");
		exit(1);
	    }
	}
    }
    if(image->base) {
	free(image->base);
	image->base = 0;
    }
    if(image->tmpbuf) {
	free(image->tmpbuf);
	image->tmpbuf = 0;
    }
    if(ISRLE(image->type)) {
	free(image->rowstart);
	image->rowstart = 0;
	free(image->rowsize);
	image->rowsize = 0;
    }
    return(close(image->file));
}

iflush(image)
register IMAGE 	*image;
{
    unsigned short *base;

    if ( (image->flags&_IOWRT)
     && (base=image->base)!=NULL && (image->ptr-base)>0) {
	    if (putrow(image, base, image->y,image->z)!=image->xsize) {
		    image->flags |= _IOERR;
		    return(EOF);
	    }
    }
}
