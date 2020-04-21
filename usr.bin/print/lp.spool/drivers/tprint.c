/*
 *	tprint - 
 *		Print an image on the tektronix 4692.
 *
 *				Paul Haeberli - 1986
 */
#include "prt.h"

unsigned short buf[MAXIWIDTH];
unsigned short roneline[MAXIWIDTH];
unsigned short goneline[MAXIWIDTH];
unsigned short boneline[MAXIWIDTH];

main(argc,argv)
int argc;
char **argv;
{
    PRINTER *printer;

    printer = prtnew(1536,1152,154.0,argc,argv);
    tek_reset();
    tek_head("c1",printer->xprint,printer->yprint);
    prtpass(printer);
    tek_close();
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
