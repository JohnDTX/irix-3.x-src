/*
 *	mprint -
 *		Print an image on the mitsubishi G500 printer.
 *
 *				Paul Haeberli - 1986
 */
#include "prt.h"

unsigned short buf[MAXIWIDTH];
unsigned short oneline[MAXIWIDTH];

main(argc,argv)
int argc;
char **argv;
{
    PRINTER *printer;

    printer = prtnew(2032,2160,240.0,argc,argv);
    prtpass3(printer);
}

prtpass3(p)
PRINTER *p;
{
    if(p->oneband) {
	startprint();
	prtpass(p,0);
	prtpass(p,0);
	prtpass(p,0);
    } else {
	startprint();
	prtpass(p,2);
	prtpass(p,1);
	prtpass(p,0);
    }
}

prtpass(p,band)
PRINTER *p;
int band;
{
    register int rowno, y;
    register int yresamp;
    register int srcheight, dstheight;

    yresamp = 0; 
    rowno = 0;
    srcheight = p->ysource;
    dstheight = p->yprint;
    for(y=srcheight; y--;) {
	prtgetrow(p,buf,y,band);
	prtaddnoise(p,buf);
	prtxscalerow(p,buf,oneline);
	yresamp += dstheight;
	while(yresamp>0) {
	    toprinter(p,oneline,rowno++);
	    yresamp -= srcheight;
	}
    }
    colorend();
}

toprinter(p,oneline,rowno)
PRINTER *p;
register short *oneline;
int rowno;
{
    register unsigned char *cptr;
    register int x;
    register int ival;

    cptr = &p->pat[rowno&7][0];
    startrow(p->bytewidth);
    for(x=p->bytewidth; x--;) {
	ival = 0;		
	if(cptr[*oneline++] & 0x80)
	    ival |= 0x80;
	if(cptr[*oneline++] & 0x40)
	    ival |= 0x40;
	if(cptr[*oneline++] & 0x20)
	    ival |= 0x20;
	if(cptr[*oneline++] & 0x10)
	    ival |= 0x10;
	if(cptr[*oneline++] & 0x08)
	    ival |= 0x08;
	if(cptr[*oneline++] & 0x04)
	    ival |= 0x04;
	if(cptr[*oneline++] & 0x02)
	    ival |= 0x02;
	if(cptr[*oneline++] & 0x01)
	    ival |= 0x01;
	putchar(~ival);
    }
    endrow();
}

static startprint()
{
    fprintf(stdout, "\21");
}

static startrow(n)
int n;
{
    escape("K");
    fputc(n, stdout);
    fputc(0, stdout);
}

static endrow()
{
    fprintf(stdout, "\n");
}

static colorend()
{
    escape("%E");
}

static escape(s)
char *s;
{
    fprintf(stdout, "\33%s", s);
}

