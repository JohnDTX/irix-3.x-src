#include "stdio.h"
#include <gl/image.h>

typedef struct PRINTER {
    IMAGE *image;
    int xprint, yprint;
    int xmaxprint, ymaxprint;
    int xsource, ysource;
    int bytewidth, oneband;
    int maplen;
    int forcewhite;
    float scale, ppi, imgppi;
    short *rmap, *gmap, *bmap, *bwmap;
    unsigned char pat[8][64];
} PRINTER;

#define MAXIWIDTH	4096	/* max image size for printing */

PRINTER *prtnew();

