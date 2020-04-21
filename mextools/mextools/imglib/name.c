/*
 *	isetname and isetcolormap -
 *
 *				Paul Haeberli - 1984
 *
 */
#include	<stdio.h>
#include	<errno.h>
#include	"image.h"

isetname(image,name)
IMAGE *image;
char *name;
{
    strncpy(image->name,name,80);
}

isetcolormap(image,colormap)
IMAGE *image;
int colormap;
{
    image->colormap = colormap;
}
