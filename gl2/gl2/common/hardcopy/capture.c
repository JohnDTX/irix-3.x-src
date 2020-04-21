#include <stdio.h>
#include <gl.h>
/* documentation says that these don't return values */

void
capture(name,cmap)
    char *name;
    RGBvalue cmap[][3];
{
    void rcapture();

    rcapture(name,cmap,0,XMAXSCREEN,0,YMAXSCREEN,(long)0,(short *)0,(long)4);
}

/* FORTRAN version */
void
captur(name,len,cmap)
    char *name;
    long len;
    RGBvalue cmap[][3];
{
    void rcaptu();

    rcaptu(name,len,cmap,0,XMAXSCREEN,0,YMAXSCREEN,(long)0,(short *)0,(long)4);
}

void
rcapture(name,cmap,left,right,bottom,top,dithsize,dithmat,res)
    char *name;
    RGBvalue cmap[][3];
    Screencoord left,right,bottom,top;
    long dithsize;
    short *dithmat;
    long res;
{
    fprintf(stderr,"Capture and rcapture have been replaced with scrsave()\n");
    fprintf(stderr,"a routine available in the image library.\n");
    exit(1);
}


/* FORTRAN version */
void
rcaptu(name,len,cmap,left,right,bottom,top,dithsize,dithmat,res)
    char *name;
    long len;
    RGBvalue cmap[][3];
    Screencoord left,right,bottom,top;
    long dithsize;
    short *dithmat;
    long res;
{
    fprintf(stderr,"Captur and rcaptu have been replaced with scrsav()\n");
    fprintf(stderr,"a routine available in the image library.\n");
    exit(1);
}
