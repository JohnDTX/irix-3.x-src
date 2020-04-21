/*
 *	iopen -
 *
 *				Paul Haeberli - 1984
 *
 */
#include	<stdio.h>
#include	<errno.h>
#include	"image.h"

IMAGE *imgopen();

IMAGE *iopen(file, mode, type, dim, xsize, ysize, zsize)
char *file;
register char *mode;
unsigned int type, dim, xsize, ysize, zsize;
{
    return(imgopen(0, file, mode, type, dim, xsize, ysize, zsize));
}

IMAGE *fiopen(f, mode, type, dim, xsize, ysize, zsize)
int f;
register char *mode;
unsigned int type, dim, xsize, ysize, zsize;
{
    return(imgopen(f, 0, mode, type, dim, xsize, ysize, zsize));
}

IMAGE *imgopen(f, file, mode, type, dim, xsize, ysize, zsize)
char *file;
int f;
register char *mode;
unsigned int type, dim, xsize, ysize, zsize;
{
	register IMAGE 	*image;
	extern int errno;
	register rw;
	int tablesize;
	register int i, max;

	image = (IMAGE*)malloc(sizeof(IMAGE));
	bzero(image,sizeof(IMAGE));
	rw = mode[1] == '+';
	if(rw) {
	    fprintf(stderr,"iopen: read/write mode not supported\n");
	    exit(1);
	}
	if (*mode=='w') {
		if (file) {
		    f = creat(file, 0666);
		    if (rw && f>=0) {
			    close(f);
			    f = open(file, 2);
		    }
		}
		if (f < 0)
		    return(NULL);
		image->imagic = IMAGIC;
		image->type = type;
		image->dim = dim;
		image->xsize = xsize;
		image->ysize = 1;
		image->zsize = 1;
		if (dim>1)
		    image->ysize = ysize;
		if (dim>2)
		    image->zsize = zsize;
		image->min = 10000000;
		image->max = 0;
		isetname(image,"no name"); 
		image->wastebytes = 0;
		image->dorev = 0;
		if (write(f,image,sizeof(IMAGE)) != sizeof(IMAGE)) {
		    fprintf(stderr,"iopen: error on write of image header\n");
		    exit(1);
		}
	} else {
		if (file)
		    f = open(file, rw? 2: 0);
		if (f < 0)
		    return(NULL);
		if (read(f,image,sizeof(IMAGE)) != sizeof(IMAGE)) {
		    fprintf(stderr,"iopen: error on read of image header\n");
		    exit(1);
		}
		if( ((image->imagic>>8) | ((image->imagic&0xff)<<8)) 
							     == IMAGIC ) {
		    image->dorev = 1;
		    cvtimage(image);
		} else
		    image->dorev = 0;
		if (image->imagic != IMAGIC) {
		    fprintf(stderr, 
			"iopen: bad magic in image file %x\n",image->imagic);
		    exit(1);
		}
	}
	if (rw)
	    image->flags = _IORW;
	else if (*mode != 'r')
	    image->flags = _IOWRT;
	else
	    image->flags = _IOREAD;
	if(ISRLE(image->type)) {
	    tablesize = image->ysize*image->zsize*sizeof(long);
	    image->rowstart = (unsigned long *)malloc(tablesize);
	    image->rowsize = (long *)malloc(tablesize);
	    if( image->rowstart == 0 || image->rowsize == 0 ) {
		fprintf(stderr,"iopen: error on table alloc\n");
		exit(1);
	    }
	    image->rleend = 512L+2*tablesize;
	    if (*mode=='w') {
		max = image->ysize*image->zsize;
		for(i=0; i<max; i++) {
		    image->rowstart[i] = 0;
		    image->rowsize[i] = -1;
		}
	    } else {
		tablesize = image->ysize*image->zsize*sizeof(long);
		lseek(f, 512L, 0);
		if (read(f,image->rowstart,tablesize) != tablesize) {
		    fprintf(stderr,"iopen: error on read of rowstart\n");
		    exit(1);
		}
		if(image->dorev)
		    cvtlongs(image->rowstart,tablesize);
		if (read(f,image->rowsize,tablesize) != tablesize) {
		    fprintf(stderr,"iopen: error on read of rowsize\n");
		    exit(1);
		}
		if(image->dorev)
		    cvtlongs(image->rowsize,tablesize);
	    }
	}
	image->cnt = 0;
	image->ptr = 0;
	image->base = 0;
	if( (image->tmpbuf = ibufalloc(image)) == 0 ) {	
	    fprintf(stderr,"iopen: error on tmpbuf alloc %d\n",image->xsize);
	    exit(1);
	}
	image->x = image->y = image->z = 0;
	image->file = f;
	image->offset = 512L;			/* set up for img_optseek */
	lseek(image->file, 512L, 0);
	return(image);
}

unsigned short *ibufalloc(image)
register IMAGE *image;
{
    return (unsigned short *)malloc(IBUFSIZE(image->xsize));
}

reverse(lwrd) 
register unsigned long lwrd;
{
    return ((lwrd>>24) 		| 
	   (lwrd>>8 & 0xff00) 	| 
	   (lwrd<<8 & 0xff0000) | 
	   (lwrd<<24) 		);
}

cvtshorts( buffer, n)
register unsigned short buffer[];
register long n;
{
    register short i;
    register long nshorts = n>>1;
    register unsigned short swrd;

    for(i=0; i<nshorts; i++) {
	swrd = *buffer;
	*buffer++ = (swrd>>8) | (swrd<<8);
    }
}

cvtlongs( buffer, n)
register long buffer[];
register long n;
{
    register short i;
    register long nlongs = n>>2;
    register unsigned long lwrd;

    for(i=0; i<nlongs; i++) {
	lwrd = buffer[i];
	buffer[i] =     ((lwrd>>24) 		| 
	   		(lwrd>>8 & 0xff00) 	| 
	   		(lwrd<<8 & 0xff0000) 	| 
	   		(lwrd<<24) 		);
    }
}

cvtimage( buffer )
register long buffer[];
{
    register short i;
    register unsigned long lwrd;

    cvtshorts(buffer,12);
    cvtlongs(buffer+3,12);
    cvtlongs(buffer+26,4);
}
