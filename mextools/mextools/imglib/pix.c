/*
 *	getpix and putpix -
 *
 *				Paul Haeberli - 1984
 *
 */
#include	<stdio.h>
#include	<errno.h>
#include	"image.h"

#undef getpix
#undef putpix

getpix(image)
IMAGE 	*image;
{
    unsigned short retval;

    if(--(image)->cnt>=0)
    	return *(image)->ptr++;
    else
	return ifilbuf(image);
}

putpix(image, pix)
register IMAGE *image;
unsigned long pix;
{
    if(--(image)->cnt>=0)
        return *(image)->ptr++ = pix;
    else
	return iflsbuf(image,pix);
}
