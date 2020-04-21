/*
 *	putrow, getrow -
 *
 *				Paul Haeberli - 1984
 *
 */
#include	<stdio.h>
#include	<errno.h>
#include	"image.h"

putrow(image,buffer,y,z) 
register IMAGE	*image;
unsigned short	*buffer;
unsigned  	y, z;
{
    register unsigned short 	*sptr;
    register unsigned char      *cptr;
    register unsigned int x;
    register unsigned long min, max;
    register long cnt;

    if( !(image->flags & (_IORW|_IOWRT)) )
	return -1;
    if(ISVERBATIM(image->type)) {
	switch(BPP(image->type)) {
	    case 1: 
		min = image->min;
		max = image->max;
		cptr = (unsigned char *)image->tmpbuf;
		sptr = buffer;
		for(x=image->xsize; x--;) { 
		    *cptr = *sptr++;
		    if (*cptr > max) max = *cptr;
		    if (*cptr < min) min = *cptr;
		    cptr++;
		}
		image->min = min;
		image->max = max;
		img_seek(image,y,z);
		cnt = image->xsize;
		if (img_write(image,image->tmpbuf,cnt) != cnt) 
		    return -1;
		else
		    return cnt;

	    case 2: 
		min = image->min;
		max = image->max;
		sptr = buffer;
		for(x=image->xsize; x--;) { 
		    if (*sptr > max) max = *sptr;
		    if (*sptr < min) min = *sptr;
		    sptr++;
		}
		image->min = min;
		image->max = max;
		img_seek(image,y,z);
		cnt = image->xsize<<1;
		if(image->dorev)	
		    cvtshorts(buffer,cnt);
		if (img_write(image,buffer,cnt) != cnt) {
		    if(image->dorev)	
			cvtshorts(buffer,cnt);
		    return -1;
		} else {
		    if(image->dorev)	
			cvtshorts(buffer,cnt);
		    return image->xsize;
		}

	    default:
		fprintf(stderr,"putrow: wierd bpp\n");
	}
    } else if(ISRLE(image->type)) {
	switch(BPP(image->type)) {
	    case 1: 
		min = image->min;
		max = image->max;
		sptr = buffer;
		for(x=image->xsize; x--;) { 
		    if (*sptr > max) max = *sptr;
		    if (*sptr < min) min = *sptr;
		    sptr++;
		}
		image->min = min;
		image->max = max;
		cnt = img_rle_compact(buffer,2,image->tmpbuf,1,image->xsize);
		img_setrowsize(image,cnt,y,z);
		img_seek(image,y,z);
		if (img_write(image,image->tmpbuf,cnt) != cnt) 
		    return -1;
		else
		    return image->xsize;
		break;

	    case 2: 
		min = image->min;
		max = image->max;
		sptr = buffer;
		for(x=image->xsize; x--;) { 
		    if (*sptr > max) max = *sptr;
		    if (*sptr < min) min = *sptr;
		    sptr++;
		}
		image->min = min;
		image->max = max;
		cnt = img_rle_compact(buffer,2,image->tmpbuf,2,image->xsize);
		cnt <<= 1;
		img_setrowsize(image,cnt,y,z);
		img_seek(image,y,z);
		if(image->dorev)
		    cvtshorts(image->tmpbuf,cnt);
		if (img_write(image,image->tmpbuf,cnt) != cnt) {
		    if(image->dorev)
			cvtshorts(image->tmpbuf,cnt);
		    return -1;
		} else {
		    if(image->dorev)
			cvtshorts(image->tmpbuf,cnt);
		    return image->xsize;
		}
		break;

	    default:
		fprintf(stderr,"putrow: wierd bpp\n");
	}
    } else 
	fprintf(stderr,"putrow: wierd image type\n");
}

getrow(image,buffer,y,z) 
register IMAGE *image;
unsigned short *buffer;
register unsigned int y, z;
{
    register short i;
    register unsigned char *cptr;
    register unsigned short *sptr;
    register short cnt; 

    if( !(image->flags & (_IORW|_IOREAD)) )
	return -1;
    img_seek(image, y, z);
    if(ISVERBATIM(image->type)) {
	switch(BPP(image->type)) {
	    case 1: 
		if (img_read(image,image->tmpbuf,image->xsize) 
							    != image->xsize) 
		    return -1;
		else {
		    cptr = (unsigned char *)image->tmpbuf;
		    sptr = buffer;
		    for(i=image->xsize; i--;)
			*sptr++ = *cptr++;
		}
		return image->xsize;
	    case 2: 
		cnt = image->xsize<<1; 
		if (img_read(image,buffer,cnt) != cnt)
		    return -1;
		else {
		    if(image->dorev)
			cvtshorts(buffer,cnt);
		    return image->xsize;
		}
	    default:
		fprintf(stderr,"getrow: wierd bpp\n");
		break;
	}
    } else if(ISRLE(image->type)) {
	switch(BPP(image->type)) {
	    case 1: 
		if( (cnt = img_getrowsize(image)) == -1 )
		    return -1;
		if( img_read(image,image->tmpbuf,cnt) != cnt )
		    return -1;
		else {
		    img_rle_expand(image->tmpbuf,1,buffer,2);
		    return image->xsize;
		}
	    case 2: 
		if( (cnt = img_getrowsize(image)) == -1 )
		    return -1;
		if( cnt != img_read(image,image->tmpbuf,cnt) )
		    return -1;
		else {
		    if(image->dorev)
			cvtshorts(image->tmpbuf,cnt);
		    img_rle_expand(image->tmpbuf,2,buffer,2);
		    return image->xsize;
		}
	    default:
		fprintf(stderr,"getrow: wierd bpp\n");
		break;
	}
    } else 
	fprintf(stderr,"getrow: wierd image type\n");
}
