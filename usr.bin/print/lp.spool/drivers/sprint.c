/*
 *	sprint -
 *		Print an image on the Seiko CH-5312 printer
 *
 *				Paul Haeberli - 1986
 */
#include "prt.h"

#define PRINTERWIDTH	2048
#define PRINTERHEIGHT	3072

unsigned short buf[MAXIWIDTH];
unsigned short rbuf[MAXIWIDTH];
unsigned short gbuf[MAXIWIDTH];
unsigned short bbuf[MAXIWIDTH];
unsigned short oneline[PRINTERWIDTH];
unsigned char pixline[PRINTERWIDTH];

int pfile = 1;

main(argc,argv)
int argc;
char **argv;
{
    PRINTER *printer;

    printer = prtnew(PRINTERWIDTH,PRINTERHEIGHT,202.5,argc,argv);
    prtpass3(printer);
}

prtpass3(p)
PRINTER *p;
{
    if(p->oneband) {
	prtpass(p,0,1);
	vers_rewind(pfile); 
	prtpass(p,0,2);
	vers_rewind(pfile); 
	prtpass(p,0,3);
    } else {
	prtpass(p,2,1);
	vers_rewind(pfile); 
	prtpass(p,1,2);
	vers_rewind(pfile); 
	prtpass(p,0,3);
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
    switch(colormode) {
	case 1:
	    vers_preamble(pfile,p->yprint,4+3);
	    break;
	case 2:
	    vers_preamble(pfile,0,8+2);
	    break;
	case 3:
	    vers_preamble(pfile,0,8+1);
	    break;
    }
    for(y=srcheight; y--;) {
	prtgetrow(p,buf,y,band);
	prtaddnoise(p,buf);
	prtxscalerow(p,buf,oneline);
	yresamp += dstheight;
	while (yresamp>0) {
	    toprinter(p,oneline,rowno++);
	    yresamp -= srcheight;
	}
    }
}

toprinter(p,oneline,rowno)
PRINTER *p;
register short *oneline;
int rowno;
{
    register unsigned char *cptr, *bptr;
    register int x;
    register ival;
    register int i;

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
    write(pfile, pixline, PRINTERWIDTH/8);
}

#define VESCAPE 0x9b

vers_preamble (f,length,control)
int f;
unsigned short length;
unsigned char control;
{
    static unsigned char buf[] = { VESCAPE, 'P', 0, 4, 0, 0, 0, 0 };

    buf[4] = length>>8;
    buf[5] = length;
    buf[6] = control;
    write (f,buf,8);
}

vers_rewind (f)
int f;
{
    vers_command (f,'W');
}

vers_command (f,cmd)
int f;
unsigned char cmd;
{
    static unsigned char buf[] = { VESCAPE, 0, 0, 0 };

    buf[1] = cmd;
    write (f,buf,4);
}

