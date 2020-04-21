/*
 *	vprint -
 *		Print an image on the versatec.
 *
 *				Paul Haeberli - 1986
 */
#include "prt.h"
#include "vcmd.h"

#define BWPRINTERWIDTH	(8*1024)
#define CPRINTERWIDTH	(8*1000)
#define PRINTERHEIGHT	40000

#define COLORBUFLEN	1000
#define BWBUFLEN	1024

unsigned short buf[MAXIWIDTH];
unsigned short rbuf[MAXIWIDTH];
unsigned short gbuf[MAXIWIDTH];
unsigned short bbuf[MAXIWIDTH];
unsigned short oneline[BWPRINTERWIDTH];
unsigned char pixline[BWPRINTERWIDTH];

int pfile = 1;

main(argc,argv)
int argc;
char **argv;
{
    PRINTER *printer;

    printer = prtnew(CPRINTERWIDTH,PRINTERHEIGHT,200.0,argc,argv);
    prtpass3(printer);
}

prtpass3(p)
PRINTER *p;
{
    if(p->oneband) {
	prtpass(p,0,0);
	vers_formfeed(pfile);
    } else {
	prtpass(p,0,1);
	vers_rewind(pfile); 
	prtpass(p,0,2);
	vers_rewind(pfile); 
	prtpass(p,1,3);
	vers_rewind(pfile); 
	prtpass(p,2,4);
	vers_formfeed(pfile);
    }
}

prtpass(p,band,colormode)
PRINTER *p;
int band;
int colormode;
{
    register int rowno, y;
    register int yresamp;
    register int srcheight, dstheight;

    yresamp = 0;
    rowno = 0;
    srcheight = p->ysource;
    dstheight = p->yprint;
    if(colormode>0) {
	switch(colormode) {
	    case 1:
		vers_preamble(pfile,1+(p->yprint-1)/200,4+0);
		break;
	    case 2:
		vers_preamble(pfile,0,8+1);
		break;
	    case 3:
		vers_preamble(pfile,0,8+2);
		break;
	    case 4:
		vers_preamble(pfile,0,8+3);
		break;
	}
    }
    vers_speed(pfile,2);
    vers_plotmode(pfile);

    for(y=srcheight; y--;) {
	if(colormode == 1) {
	    prtgetrow(p,rbuf,y,0);
	    prtgetrow(p,gbuf,y,1);
	    prtgetrow(p,bbuf,y,2);
	    prtgreyrow(rbuf,gbuf,bbuf,buf,p->xsource);
	} else
	    prtgetrow(p,buf,y,band);
	prtaddnoise(p,buf);
	prtxscalerow(p,buf,oneline);
	yresamp += dstheight;
	while (yresamp>0) {
	    toprinter(p,oneline,rowno++);
	    yresamp -= srcheight;
	}
    }
    vers_printmode(pfile);
}

toprinter(p,oneline,rowno)
PRINTER *p;
register short *oneline;
int rowno;
{
    register unsigned char *cptr, *bptr;
    register int x;
    register ival;

    rowno &= 0x7;
    cptr = &p->pat[rowno&7][0];
    bptr = pixline;
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
	*bptr++ = ~ival;
    }
    if(p->oneband)
	write(pfile, pixline, BWBUFLEN);
    else
	write(pfile, pixline, COLORBUFLEN);
}
