/*
 *	img_seek, img_write, img_read, img_optseek -
 *
 *				Paul Haeberli - 1984
 *
 */
#include	<stdio.h>
#include	<errno.h>
#include	"image.h"

img_seek(image, y, z)
IMAGE 		*image;
unsigned int 	y, z;
{
    image->x = 0;
    image->y = y;
    image->z = z;
    if(ISVERBATIM(image->type)) {
	switch(image->dim) {
	    case 1:
		return img_optseek(image, 512L);
	    case 2: 
		return img_optseek(image,512L+(y*image->xsize)*BPP(image->type));
	    case 3: 
		return img_optseek(image,
		    512L+(y*image->xsize+z*image->xsize*image->ysize)*
							BPP(image->type));
	    default:
		fprintf(stderr,"img_seek: wierd dim\n");
		break;
	}
    } else if(ISRLE(image->type)) {
	switch(image->dim) {
	    case 1:
		return img_optseek(image, image->rowstart[0]);
	    case 2: 
		return img_optseek(image, image->rowstart[y]);
	    case 3: 
		return img_optseek(image, image->rowstart[y+z*image->ysize]);
	    default:
		fprintf(stderr,"img_seek: wierd dim\n");
		break;
	}
    } else 
	fprintf(stderr,"putrow: wierd image type\n");
}

img_write(image,buffer,count)
IMAGE *image;
char *buffer;
long count;
{
    long retval;

    retval =  write(image->file,buffer,count);
    if(retval == count) 
	image->offset += count;
    else
	image->offset = -1;
    return retval;
}

img_read(image,buffer,count)
IMAGE *image;
char *buffer;
long count;
{
    long retval;

    retval =  read(image->file,buffer,count);
    if(retval == count) 
	image->offset += count;
    else
	image->offset = -1;
    return retval;
}

img_optseek(image,offset)
IMAGE *image;
unsigned long 	offset;
{
    if(image->offset != offset) {
       image->offset = offset;
       return lseek(image->file,offset,0);
   }
   return offset;
}
