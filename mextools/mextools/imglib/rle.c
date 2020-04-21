/*
 *	img_getrowsize, img_setrowsize, img_rle_compact, img_rle_expand -
 *
 *				Paul Haeberli - 1984
 *
 */
#include	<stdio.h>
#include	<errno.h>
#include	"image.h"

img_getrowsize(image)
register IMAGE *image;
{
    switch(image->dim) {
	case 1:
	    return image->rowsize[0];
	case 2:
	    return image->rowsize[image->y];
	case 3:
	    return image->rowsize[image->y+image->z*image->ysize];
    }
}

img_setrowsize(image,cnt,y,z)
register IMAGE *image;
long y, z, cnt;
{
    long *sizeptr;

    switch(image->dim) {
	case 1:
	    sizeptr = &image->rowsize[0];
	    image->rowstart[0] = image->rleend;
	    break;
	case 2:
	    sizeptr = &image->rowsize[y];
	    image->rowstart[y] = image->rleend;
	    break;
	case 3:
	    sizeptr = &image->rowsize[y+z*image->ysize];
	    image->rowstart[y+z*image->ysize] = image->rleend;
    }	
    if(*sizeptr != -1) 
	image->wastebytes += *sizeptr;
    *sizeptr = cnt;
    image->rleend += cnt;
}

#define docompact 							\
	while(iptr<ibufend) {						\
	    sptr = iptr;						\
	    iptr += 2;							\
	    while((iptr<ibufend)&&((iptr[-2]!=iptr[-1])||(iptr[-1]!=iptr[0])))\
		iptr++;							\
	    iptr -= 2;							\
	    count = iptr-sptr;						\
	    while(count) {						\
		todo = count>126 ? 126:count; 				\
		count -= todo;						\
		*optr++ = 0x80|todo;					\
		while(todo--)						\
		    *optr++ = *sptr++;					\
	    }								\
	    sptr = iptr;						\
	    cc = *iptr++;						\
	    while( (iptr<ibufend) && (*iptr == cc) )			\
		iptr++;							\
	    count = iptr-sptr;						\
	    while(count) {						\
		todo = count>126 ? 126:count; 				\
		count -= todo;						\
		*optr++ = todo;						\
		*optr++ = cc;						\
	    }								\
	}								\
	*optr++ = 0;

img_rle_compact(expbuf,ibpp,rlebuf,obpp,cnt)
unsigned short *expbuf;
int ibpp;
unsigned short *rlebuf;
int obpp, cnt;
{
    if(ibpp == 1 && obpp == 1) {
	register unsigned char *iptr = (unsigned char *)expbuf;
	register unsigned char *ibufend = iptr+cnt;
	register unsigned char *sptr;
	register unsigned char *optr = (unsigned char *)rlebuf;
	register short todo, cc;
	register long count;

	docompact;
	return optr - (unsigned char *)rlebuf;
    } else if(ibpp == 1 && obpp == 2) {
	register unsigned char *iptr = (unsigned char *)expbuf;
	register unsigned char *ibufend = iptr+cnt;
	register unsigned char *sptr;
	register unsigned short *optr = rlebuf;
	register short todo, cc;
	register long count;

	docompact;
	return optr - rlebuf;
    } else if(ibpp == 2 && obpp == 1) {
	register unsigned short *iptr = expbuf;
	register unsigned short *ibufend = iptr+cnt;
	register unsigned short *sptr;
	register unsigned char *optr = (unsigned char *)rlebuf;
	register short todo, cc;
	register long count;

	docompact;
	return optr - (unsigned char *)rlebuf;
    } else if(ibpp == 2 && obpp == 2) {
	register unsigned short *iptr = expbuf;
	register unsigned short *ibufend = iptr+cnt;
	register unsigned short *sptr;
	register unsigned short *optr = rlebuf;
	register short todo, cc;
	register long count;

	docompact;
	return optr - rlebuf;
    } else 
	printf(stderr,"rle_compact: bad bpp: %d %d\n",ibpp,obpp);
}

#define doexpand				\
	while(1) {				\
	    pixel = *iptr++;			\
	    if ( !(count = (pixel & 0x7f)) )	\
		return;				\
	    if(pixel & 0x80) {			\
	       while(count--)			\
		    *optr++ = *iptr++;		\
	    } else {				\
	       pixel = *iptr++;			\
	       while(count--)			\
		    *optr++ = pixel;		\
	    }					\
	}

img_rle_expand(rlebuf,ibpp,expbuf,obpp)
unsigned short *rlebuf;
int ibpp;
unsigned short *expbuf;
int obpp;
{
    if(ibpp == 1 && obpp == 1) {
	register unsigned char *iptr = (unsigned char *)rlebuf;
	register unsigned char *optr = (unsigned char *)expbuf;
	register unsigned short pixel,count;

	doexpand;
    } else if(ibpp == 1 && obpp == 2) {
	register unsigned char *iptr = (unsigned char *)rlebuf;
	register unsigned short *optr = expbuf;
	register unsigned short pixel,count;

	doexpand;
    } else if(ibpp == 2 && obpp == 1) {
	register unsigned short *iptr = rlebuf;
	register unsigned char  *optr = (unsigned char *)expbuf;
	register unsigned short pixel,count;

	doexpand;
    } else if(ibpp == 2 && obpp == 2) {
	register unsigned short *iptr = rlebuf;
	register unsigned short *optr = expbuf;
	register unsigned short pixel,count;

	doexpand;
    } else 
	printf(stderr,"rle_expand: bad bpp: %d %d\n",ibpp,obpp);
}
