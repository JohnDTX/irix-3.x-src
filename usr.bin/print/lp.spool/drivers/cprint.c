/*
 *	cprint -
 *		Print an image on the Chroma Jet (Polaroid) printer.
 *
 *				Paul Haeberli - 1986
 */
#include "prt.h"

static int rowwidth;
unsigned short buf[MAXIWIDTH];
unsigned short roneline[MAXIWIDTH];
unsigned short goneline[MAXIWIDTH];
unsigned short boneline[MAXIWIDTH];

main(argc,argv)
int argc;
char **argv;
{
    PRINTER *printer;

    printer = prtnew(2400,2400,240.0,argc,argv);
    chromabegin(printer->xprint,printer->yprint);
    prtpass(printer);
    chromaend();
}

prtpass(p)
PRINTER *p;
{
    register int rowno, y;
    register int yresamp;
    register int srcheight, dstheight;

    yresamp = 0; 
    rowno = 0;
    srcheight = p->ysource;
    dstheight = p->yprint;
    if(p->oneband) {
	for(y=srcheight; y--;) {
	    prtgetrow(p,buf,y,0);
	    prtaddnoise(p,buf);
	    prtxscalerow(p,buf,roneline);
	    yresamp += dstheight;
	    while(yresamp>0) {
		toprinter(p,roneline,rowno++);
		yresamp -= srcheight;
	    }
	}
    } else {
	for(y=srcheight; y--;) {
	    prtgetrow(p,buf,y,0);
	    prtaddnoise(p,buf);
	    prtxscalerow(p,buf,roneline);
	    prtgetrow(p,buf,y,1);
	    prtaddnoise(p,buf);
	    prtxscalerow(p,buf,goneline);
	    prtgetrow(p,buf,y,2);
	    prtaddnoise(p,buf);
	    prtxscalerow(p,buf,boneline);
	    yresamp += dstheight;
	    while(yresamp>=0) {
		toprinter3(p,roneline,goneline,boneline,rowno++);
		yresamp -= srcheight;
	    }
	}
    }
}

toprinter(p,buf,rowno)
PRINTER *p;
register short *buf;
int rowno;
{
    register unsigned char *cptr;
    register int ival, mbit, x;

    cptr = &p->pat[rowno&7][0];
    mbit = 0x80;
    startrow();
    for(x=rowwidth/8; x--;) {
	ival = 0;
	if (cptr[*buf++] & 0x80)
	    ival |= 0x80;
	if (cptr[*buf++] & 0x40)
	    ival |= 0x40;
	if (cptr[*buf++] & 0x20)
	    ival |= 0x20;
	if (cptr[*buf++] & 0x10)
	    ival |= 0x10;
	if (cptr[*buf++] & 0x08)
	    ival |= 0x08;
	if (cptr[*buf++] & 0x04)
	    ival |= 0x04;
	if (cptr[*buf++] & 0x02)
	    ival |= 0x02;
	if (cptr[*buf++] & 0x01)
	    ival |= 0x01;
	putchar(ival);
	putchar(ival);
	putchar(ival);
    }
}

toprinter3(p,rbuf,gbuf,bbuf,rowno)
PRINTER *p;
register short *rbuf, *gbuf, *bbuf;
int rowno;
{
    register unsigned char *cptr;
    register int ival, mbit, x;

    cptr = &p->pat[rowno&7][0];
    mbit = 0x80;
    for(x=rowwidth/8; x--;) {
/* red */
	ival = 0;
	if (cptr[*rbuf++] & 0x80)
	    ival |= 0x80;
	if (cptr[*rbuf++] & 0x40)
	    ival |= 0x40;
	if (cptr[*rbuf++] & 0x20)
	    ival |= 0x20;
	if (cptr[*rbuf++] & 0x10)
	    ival |= 0x10;
	if (cptr[*rbuf++] & 0x08)
	    ival |= 0x08;
	if (cptr[*rbuf++] & 0x04)
	    ival |= 0x04;
	if (cptr[*rbuf++] & 0x02)
	    ival |= 0x02;
	if (cptr[*rbuf++] & 0x01)
	    ival |= 0x01;
	putchar(ival);

/* green */
	ival = 0;
	if (cptr[*bbuf++] & 0x80)
	    ival |= 0x80;
	if (cptr[*bbuf++] & 0x40)
	    ival |= 0x40;
	if (cptr[*bbuf++] & 0x20)
	    ival |= 0x20;
	if (cptr[*bbuf++] & 0x10)
	    ival |= 0x10;
	if (cptr[*bbuf++] & 0x08)
	    ival |= 0x08;
	if (cptr[*bbuf++] & 0x04)
	    ival |= 0x04;
	if (cptr[*bbuf++] & 0x02)
	    ival |= 0x02;
	if (cptr[*bbuf++] & 0x01)
	    ival |= 0x01;
	putchar(ival);
/* blue */
	ival = 0;
	if (cptr[*gbuf++] & 0x80)
	    ival |= 0x80;
	if (cptr[*gbuf++] & 0x40)
	    ival |= 0x40;
	if (cptr[*gbuf++] & 0x20)
	    ival |= 0x20;
	if (cptr[*gbuf++] & 0x10)
	    ival |= 0x10;
	if (cptr[*gbuf++] & 0x08)
	    ival |= 0x08;
	if (cptr[*gbuf++] & 0x04)
	    ival |= 0x04;
	if (cptr[*gbuf++] & 0x02)
	    ival |= 0x02;
	if (cptr[*gbuf++] & 0x01)
	    ival |= 0x01;
	putchar(ival);
    }
}

chromabegin(xsize,ysize)
int xsize,ysize;
{
    graphicsmode();
    size(xsize,ysize);
    setres(300);
    nozoom();
    compactmode();
}

chromaend()
{
    endpage();
}

/* low level commands */

graphicsmode()
{
    putchar(0x03);
}

alphamode()
{
    putchar(0x04);
}

compactmode()
{
    putchar(0x14);
}

endpage()
{
    putchar(0x0c);
}

nozoom()
{
    putchar(0x15);
}

escape()
{
    putchar(0x1b);
}

startrow()
{
    putchar(0x07);
}

size(x,y)
int x, y;
{
    x = 8*(1+(x-1)/8);	/* must be divisible by 8!! */
    y = 8*(1+(y-1)/8);
    rowwidth = x;
    putchar(0x02);
    putchar(x&0xff);
    putchar((x>>8)&0xff);
    putchar(y&0xff);
    putchar((y>>8)&0xff);
}

setres(perinch)
int perinch;
{
    if(perinch > 250)
	perinch = 250;
    if(perinch < 60)
	perinch = 60;
    putchar(0x09);
    putval(perinch);
    putchar(0x0a);
    putval(perinch);
}

putval(value)
int value;
{
    int val;

    val = value/100;
    putchar('0'+val);
    value -= 100*val;
    val = value/10;
    putchar('0'+val);
    value -= 10*val;
    val = value;
    putchar('0'+val);
    putchar('$');
}
