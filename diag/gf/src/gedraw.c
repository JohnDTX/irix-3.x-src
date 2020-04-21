/* gedraw.c
 * 
 *	interesting pattern drawing
 *	GL2 - merge scaler
 */

#include "fbcld.h"
#include "gfdev.h"
#include "../geofdef.h"
#ifdef GF2
#include "gl2cmds.h"
#else
#include "betacodes.h"
#endif

extern char line[];	/* command line buffer */
extern short ix;	/* command line index */
extern short interrupts;
extern unsigned short realdrawsetup[];
extern unsigned short realdraw[];
extern unsigned short realdots[];
extern unsigned short realpoly[];
extern short printwd;
extern short devstatus;
extern short GEstatus;

unsigned short *pgearray;

unsigned short gtab[] = {
	0,	/* dummy */
	0x09,	0x0a,	0x0b,	0x0c,	0x10,	0x11,	0x12,	0x13,	0x14,
	0x15,	0x20,	0x21,	8	/* merge scaler */
};
unsigned short fbuf[100];
unsigned short goodbuf[100];

gedraw()
{
    register num;
    char cc;

    cc = line[ix++];
    num = getnum();
    switch (cc) {
	case 'b':	boxes(num);
		break;
	case 'd':	dots(num);
		break;
	case 'c':	copypix(num);
		break;
	case 'p':	passthrus(num);
		break;
	case 'r':	retrace(num);
		break;
	case 'v':	depthvecs(num);
		break;
	case 'D':	depthdots(num);
		break;
	case 'm':	matrix(num);
		break;
	case 'f':	feedtest(num);
		break;
	case 'R':	runlen(num);
		break;
	default:
			printf("   boxes\n   dots\n");
			printf("   copypixels (x=<n>)\n   passthru dots\n");
			printf("   vecs depthcued\n   Depthcued dots\n");
			printf("   retrace test <nframes>\n");
			printf("   Runlength draw\n");
    }
}

boxes(num)
    int num;
{
    short i;
    short color=1;
    register unsigned short *pdat;

#ifdef GF2
    smartconfigure(gtab);
#endif
if (num <=0) num = 1;
while (num-- > 0)
    {
	for (pdat = realdrawsetup; *pdat!=GEOF; ) gesend(*pdat++);
	for (i=0; i<72; i++)
	    {
#ifdef GF2
		gesendlong(0x1080014);
		gesend(1+color);
#else
		gesendlong(0x2080014);
		gesend(1+color);
		gesend(0xffff);
#endif GF2
		for (pdat = realdraw; *pdat!=GEOF; ) gesend(*pdat++);
		color = 1-color;
		while (GEflags & HIWATER_BIT) ;
	    }
    }
}

dots(num)
    register int num;
{
    register	short	*GEaddr = (short *)&GEdata;
    register unsigned short *pdat;
    register short nwds;
    static short color = 0;
    static short dat = 0;

if (num<=0) num = 1000;
#ifdef GF2
    smartconfigure(gtab);
    gesend(0xff08);
#endif
if (interrupts) {
	for (pdat = realdrawsetup; *pdat!=GEOF; ) gesend(*pdat++);
	gesend(0xff08);
	while (--num >=0) {
#ifdef GF2
		gesendlong(0x1080014);
		gesend((++color%7)+1);
#else
		gesendlong(0x2080014);
		gesend((++color%7)+1);
		gesend(0xffff);
#endif GF2
		for (pdat = realpoly; *pdat != GEOF; ) *GEaddr = *pdat++;
		dat += 16;
		realdots[2] = realdots[4] = dat;
		pdat = realdots;
#ifdef ASMM
		nwds = 36;	/* no. of longs in realdots[] */
		asm(".LL11:	");
		asm("	movl	a4@+,a5@");	/* GEaddr = *pdat++ (long) */
		asm("	dbgt	d6,.LL11");
#endif
		for (nwds = 72; --nwds>=0;) *GEaddr = *pdat++;
	}
}
else printf("do 'ii' instead of 'ir'\n");

}

passthrus(num)		/* do a screenful of dots */
    int num;
{
    register unsigned short *GE = &GEdata;
    register int x;
    register int y;
    int i;

    if (num==0) num = 1;
    if (interrupts) {
	for (i=0; i<num; i++)
	    for (y=0; y<=800; y+=10)
		for (x=0; x<=1000; x+=10) {
			*GE = 0x208;
			*GE = 0x12;	/* passthru point */
			*GE = x;
			*GE = y;
		}
    }
    else {
	for (i=0; i<num; i++)
	    for (y=0; y<=800; y+=10)
		for (x=0; x<=1000; x+=10) {
			gesend(0x208);
			gesend(0x12);
			gesend(x);
			gesend(y);
		}
    }
}


gesend(num)
    short num;
{
	if (printwd) printf(" %x ",num);
	GEdata = num;
	if (!interrupts) while (GEflags & HIWATER_BIT) ;
	return(1);
}

gesendlong(num)
    int num;
{
	if (printwd) printf(" %x %x ",(num&0xffff0000)>>16,num&0xffff);
	*(int *)&GEdata = num;
	if (!interrupts) while (GEflags & HIWATER_BIT) ;
	return(1);
}


depthvecs(num)
    register num;
{
    register i,j;

    if (num <=0) num = 1023;
    for (i=0; i<0x100; i++) {
	gesendlong(0x4080010);
	gesend(0);
	gesend(i);
	gesend(0);
	gesend(i);
	gesendlong(0x4080011);
	gesend(num);
	gesend(i);
	gesendlong(0);
    }
    for (i=256,j=1023; i<768; i++) {
	gesendlong(0x4080010);
	gesend(0);
	gesend(i);
	gesend(0);
	gesend(0xff);
	gesendlong(0x4080011);
	gesend(j);
	gesend(i);
	gesendlong(0);
	j -= 2;
    }
}


depthdots(num)	/* someone else must put in depth mode */
    int num;
{
    register line,pix;

    if (num <=0) num = 1;
    while (num-- > 0) {
	for (line = 767; line > 730; line--)
		for (pix= 0; pix < 1024; pix++) {
			gesendlong(0x4080012);
			gesend(pix);
			gesend(line);
			gesend(0);
			gesend(pix);
		}
    }
}

copypix(pix)
   register int pix;
{
    register line;

	for (line = 0; line <0x80; line++) {
		gesendlong(0x508003d);
		gesend(pix+line);	/* xfrom */
		gesend(line);
		gesend(0x100+line);	/* xto */
		gesend(line);
		gesend(pix+(0x100-line));
	}
	gesend(0xff08);
}

runlen(num)
    int num;
{
	register short x;
	register short y;

	gesendlong(0x208002f);	/* pixel setup */
	gesendlong(0x107ff);
	if (num==0) num = 1;
	while (num-- > 0) {	 /* diagonal rainbow 6 wide */
		for (x = 1016, y=0; x > 1; --x) {
			gesendlong(0x308001a);
			gesendlong(0x120000);
			gesend(y++);
			gesendlong(0xb080042);	/* draw runlength */
			gesend(x);
			gesendlong(0xfffa);
			gesendlong(0x20005);
			gesendlong(0x30006);
			gesendlong(0x40007);
			gesend(1018-x);
			gesend(1);
		}
	}
}

retrace(num)
	int num;
{
	register coord = 0x100;
	short save,i;

	save = spl7();
	while (num-- >=0) {
		while (!(FBCflags & VERTINT_BIT)) ;

		FBCdata = coord;
		FBCflags = devstatus | HOSTFLAG;
		gesendlong(0x80008);
		while (FBCflags & INTERRUPT_BIT_) ;
		FBCdata = 0x100 - coord;
		FBCclrint;
	for (i=0; i<100; i++) ;
		FBCflags = devstatus;
		if (coord-- <= 0) coord = 0x100;

		while (FBCflags & VERTINT_BIT) ;
		while (!(FBCflags & VERTINT_BIT)) ;
		while (FBCflags & VERTINT_BIT) ;
	}
	splx(save);
}

matrix(num)
	int num;
{
#define XMAX	1023
#define YMAX	767
#define GEZERO	0x00000000

	static long mat[] = {
		0x3b000000,	0,		0,		0xbf7fc000,
		0,		0x3b2aaaaa,	0,		0xbf7faaaa,
		0,		0,		0xbf800000,	0,
		0,		0,		0,		0x3f800000,
	};
	static short ctab[] = {
		0,	9,	0xa,	0xb,	0xc,
			0x15,	0x10,	0x11,	0x12,
			0x13,	0x14,	0x20,	0x21,
		8
	};
	register i;
	register long xmax,ymax;

	smarterconfigure(ctab);
	xmax = (XMAX+1) <<7;
	ymax = (YMAX+1) <<7;
	gesend(GEloadviewport);
	gesendlong(xmax);
	gesendlong(ymax);
	gesendlong(xmax);
	gesendlong(ymax);
	gesendlong(GEZERO);
	gesendlong(xmax);
	gesendlong(GEZERO);
	gesendlong(xmax);

	gesend(GEloadmm);
	for (i=0; i<16; i++)
		gesendlong(mat[i]);

	gesend(0x408);
	gesend(FBCcolor);
	gesend(2);
	gesend(FBClinestipple);
	gesendlong(0xffff);

	gmov(0,0);
	gdrw(XMAX,0);
	gdrw(XMAX,YMAX);
	gdrw(0,YMAX);
	gdrw(0,0);
}

gmov(x,y)
	int x,y;
{
	gesend(GEmove | 0x900);		/* 2d short */
	gesend(x);
	gesend(y);
}

gdrw(x,y)
	int x,y;
{
	gesend(GEdraw | 0x900);		/* 2d short */
	gesend(x);
	gesend(y);
}

smarterconfigure(table)
	short *table;
{
	extern short GEmask;
	extern short GEfound;

	/* configure GEs */
	reset_GE();
	reset_GE();

	poutshort( 0x3a02 );	/* FIFO chip with high water mark at 63 */
				/* and low water mark at 2?		*/
	if (GEmask & 0x2000) poutshort( (short) ((GEfound-1)<<8) +table[0] );
	justconfigure(table);
	poutshort( 0x3a02 );	/* FIFO chip at end of pipe    */
	if (GEmask & 1) poutshort(0xff00 + table[13]);
}


feedtest(num)
	int num;
{
#ifdef GF2
	register short i;
	register unsigned short *fbcdata = &FBCdata;
	register unsigned short *fp;
	register unsigned short *clr = &FBCpixel;
	int hpmerror;
	short dum;
	short k;

	GEflags = GEstatus | ENABFBCINT_BIT_;
	gesend(1);
	k = 0001;
	fp = goodbuf;
	*fp++ = 3;	/* storemat command */
	for (i=0; i<16; i++) {
		gesend(k);
		*fp++ = k;
		k <<= 1;
	}
	k = 0xfffe;
	for (i=0; i<16; i++) {
		gesend(k);
		*fp++ = k;
		k <<= 1;
	}
	*fp++ = 0x108;
	*fp++ = 0x84;
	*fp   = 0x42;

	while (--num > -1) {
		gesendlong(0x80025);
		gesend(3);
		gesendlong(0x1080084);
		gesend(0x42);
		while (FBCflags & INTERRUPT_BIT_) ;
		GEflags = GEstatus | AUTOCLEAR_BIT | ENABFBCINT_BIT_;
		dum = FBCdata;	/* int code */
		/**clr = 1;*/
		if ((i=FBCdata) != 36)
			printf("got %d wds\n",i);
		/**clr = 1;*/
		fp = fbuf;
		for (dum=0; dum<36; dum++) {
			*fp++ = *fbcdata;
			/**clr = 1;*/
		}
		if (!(FBCflags & INTERRUPT_BIT_)) {
			printf("xtra.");
			while (!(FBCflags & INTERRUPT_BIT_)) {
				i = *fbcdata;
				/**clr = 1;*/
			}
		}
		GEflags = GEstatus | ENABFBCINT_BIT_;
		fp = fbuf;
		hpmerror = 0;
		for (i=0; i<36; i++)
			if (*fp++ != goodbuf[i])
			    hpmerror = 1;
			    /*printf("%x/%x ",goodbuf[i],fbuf[i]);*/
	}
	if (hpmerror)
	    printf("error in getmatrix %x is first number\n",fbuf[0]);
	else
	    printf("\n");
	GEflags = GEstatus;
#endif GF2
}
