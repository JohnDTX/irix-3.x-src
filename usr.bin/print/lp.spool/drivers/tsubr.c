#
/*
 * teksubr.c --
 * routines for sending an image to a tek4692.
 * stdout is assumed to be /dev/centronics.
 *
	tek_reset();
	tek_head("", hsize, vsize);
	for each raster line
		for each pixel in the line
			tek_pixel(r, g, b);
		tek_eol();
	tek_close();
 *
 */
# include "stdio.h"
# include "ctype.h"
# include "ik_ioctl.h"
# include "prt.h"
#	define IK_SR	0x01


# define msk(x)		(~(~0<<(x)))
# define BITSPERBYTE	8

# define STATIC

/* software representation of tek state */
struct tekstate
{
    unsigned char sizing;		/* sizing flag */
    unsigned char invert;		/* invert flag */
    unsigned char orientation;		/* landscape, bottom, top, center */
    unsigned char colorbits;		/* bits per color */
    unsigned char retrace;		/* repaint retrace */
    unsigned int hsize, vsize;		/* host x and y */

    int (*pixfunc)();
};
struct tekstate tek_state;


/* various constants */
# define TEK_HMAX		1536
# define TEK_VMAX		1152

/* command bit */
# define T_DATABIT	(1<<7)
# define T_SYNCBIT	(1<<6)

/* command bytes */
# define T_NULL		0x00
# define T_EOT		0x01
# define T_EOL		0x02
# define T_ABORT	0x03
# define T_COPY		0x04
# define T_RESERVE	0x05
# define T_BITPROMPT	0x07
# define T_SENDSTATUS	0x08

struct copyreq
{
    unsigned char copymode;
    unsigned char retrace;
    unsigned char hres0;
    unsigned char hres1;
    unsigned char vres0;
    unsigned char vres1;
    unsigned char cksum;
};

/* bits in copymode */
# define CM_STREAMING	(01<<6)
#   define STREAMING_OFF	(00<<6)
#   define STREAMING_ON		(01<<6)
# define CM_COLORBITS	(03<<4)
#   define COLORBITS_1		(00<<4)
#   define COLORBITS_2		(01<<4)
#   define COLORBITS_4		(02<<4)
# define CM_OR		(03<<2)
#   define OR_LANDSCAPE		(00<<2)
#   define OR_BOTTOM		(01<<2)
#   define OR_CENTER		(02<<2)
#   define OR_TOP		(03<<2)
# define CM_SIZING	(01<<1)
#   define SIZING_ON		(00<<1)
#   define SIZING_OFF		(01<<1)
# define CM_INVERT	(01<<0)
#   define INVERT_OFF		(00<<0)
#   define INVERT_ON		(01<<0)

struct statusret
{
    char length;
    char devchar;
    char devid;
    char version;
    char options;
    char long_res0;
    char long_res1;
    char short_res0;
    char short_res1;
    char colorbits;
    char err1;
    char err2;
    char prevcmd;
    char cksum;
};

static int phase, lagchar;

/*
 * tek_head() --
 * setup tek header.
 *	image format (*landscape, bottom, center, top)
 *	sizing (*off, on)
 *	invert (*off, on)
 *	bits per color (*1, 2, 4)
 *	retrace: (0x81-0x87)
 *	horizontal resolution
 *	vertical resolution
 */
extern pix1(), pix2(), pix4();
int (*pixfuncs[])() = { 0, pix1, pix2, 0, pix4 };
int
tek_head(flags, hsize, vsize)
    char *flags;
    int hsize, vsize;
{
    extern char *gettok();

    register struct tekstate *rp;

    register char *sp;
    int minus;

    phase = 0;
    rp = &tek_state;
    rp->sizing = 0;
    rp->invert = 0;
    rp->orientation = OR_LANDSCAPE;

    rp->pixfunc = pixfuncs[rp->colorbits = 1];
    rp->retrace = 1;
    rp->hsize = hsize > 0 ? hsize : TEK_HMAX;
    rp->vsize = vsize > 0 ? vsize : TEK_VMAX;

    while( *(sp = gettok(&flags)) != '\0' )
    {
	if( sp[0] == 'c' && sp[2] == '\0'
	 && (sp[1] == '1' || sp[1] == '2' || sp[1] == '4') )
	{
	    rp->pixfunc = pixfuncs[rp->colorbits = sp[1]-'0'];
	    continue;
	}
	if( sp[0] == 'r' && sp[2] == '\0'
	 && ('1' <= sp[1] && sp[1] <= '7') )
	{
	    rp->retrace = sp[1]-'0';
	    continue;
	}
	if( strcmp(sp, "landscape") == 0 )
	{
	    rp->orientation = OR_LANDSCAPE;
	    continue;
	}
	if( strcmp(sp, "bottom") == 0 )
	{
	    rp->orientation = OR_LANDSCAPE;
	    continue;
	}
	if( strcmp(sp, "center") == 0 )
	{
	    rp->orientation = OR_LANDSCAPE;
	    continue;
	}
	if( strcmp(sp, "top") == 0 )
	{
	    rp->orientation = OR_LANDSCAPE;
	    continue;
	}

	if( minus = *sp == '-' )
	    sp++;
	if( strcmp(sp, "invert") == 0 )
	{
	    rp->invert = !minus;
	    continue;
	}
	if( strcmp(sp, "sizing") == 0 )
	{
	    rp->sizing = !minus;
	    continue;
	}

	return -1;
    }

    tek_putc(T_RESERVE);
    puthead(rp); 
    return 0;
}

static char cbmap[] = { 0, COLORBITS_1, COLORBITS_2, 0, COLORBITS_4 };
static
puthead(rp)
    register struct tekstate *rp;
{
    register int mode;
    register struct copyreq c;

    mode = 0;
    if( rp->sizing )
	mode |= SIZING_ON;
    else
	mode |= SIZING_OFF;
    if( rp->invert )
	mode |= INVERT_ON;
    else
	mode |= INVERT_OFF;
    mode |= rp->orientation;
    mode |= cbmap[rp->colorbits];

    c.copymode = T_DATABIT | mode;
    c.retrace = T_DATABIT | rp->retrace;
    c.hres0 = T_DATABIT | (rp->hsize >> 7);
    c.hres1 = T_DATABIT | (rp->hsize >> 0);
    c.vres0 = T_DATABIT | (rp->vsize >> 7);
    c.vres1 = T_DATABIT | (rp->vsize >> 0);
    c.cksum = T_DATABIT | tek_cksum(&c.copymode, &c.cksum - &c.copymode);

    tek_putc(T_COPY);
    tek_puts(&c.copymode, &c.cksum - &c.copymode + 1);
    tek_putc(T_EOL);
}

static char *
gettok(_str)
    char **(_str);
{
    extern char *malloc();

    register char *sp, *ap, *tp;
    int n;

    ap = *_str;
    while( isspace(*ap) )
	ap++;

    sp = ap;
    while( *sp != '\0' && !isspace(*sp) )
	sp++;
    n = sp-ap;
    if( (tp = malloc(n+1)) == 0 )
	return 0;
    strncpy(tp, ap, n);

    *_str = sp;
    return ap;
}


tek_pixel(r, g, b)
    int r, g, b;
{
    (*tek_state.pixfunc)(r, g, b);
}


# define rgb(r, g, b, n) \
	((((b)&msk(n))<<(n) | (g)&msk(n))<<(n) | (r)&msk(n))

static
pix1(r, g, b)
    int r, g, b;
{
    register int a;

    a = rgb(r, g, b, 1);

    if( !phase )
    {
	phase = 1;
	lagchar = T_DATABIT|T_SYNCBIT | a;
    }
    else
    {
	phase = 0;
	putchar(lagchar | a<<3);
    }
}

static
pix2(r, g, b)
    int r, g, b;
{
    putchar(T_DATABIT|T_SYNCBIT | rgb(r, g, b, 2));
}

static
pix4(r, g, b)
    int r, g, b;
{
    putchar(T_DATABIT|T_SYNCBIT | rgb(r>>2, g>>2, b>>2, 2));
    putchar(T_DATABIT | rgb(r, g, b, 2));
}

tek_close()
{
    tek_putc(T_EOT);
    tek_putc(T_ABORT);
    phase = 0;
}

tek_eol()
{
    if( phase )
	putchar(lagchar);
    tek_putc(T_EOL);
}

tek_ff()
{
    tek_putc(T_EOT);
}

tek_reset()
{
    ioctl(fileno(stdout), IKIORESET, 0);
    tek_putc(T_ABORT);
    phase = 0;
}

tek_getstatus(sp)
    struct statusret *sp;
{
    register int a, c;
    register int i, j;
    register char *cp;

    cp = (char *)sp;
    c = T_SENDSTATUS;
    for( i = sizeof *sp; --i >= 0; )
    {
	a = 0;
	for( j = BITSPERBYTE; --j >= 0; )
	{
	    tek_putc(c);
	    a = (a << 1) | (tek_inreg(IK_SR)&01);
	    c = T_BITPROMPT;
	}
	*cp++ = a;
    }
}

static int
tek_inreg(regno)
    int regno;
{
    ioctl(fileno(stdout), IKIOPEEK, &regno);
    return regno;
}

static int
tek_cksum(b, n)
    register unsigned char *b;
    int n;
{
    register int s;

    s = T_COPY;
    while( --n >= 0 )
	s += *b++;
    return s & msk(7);
}

STATIC
tek_puts(b, n)
    register unsigned char *b;
    int n;
{
    fflush(stdout);
    /* { int i; i = 1; ioctl(fileno(stdout), IKIOPIOMODE, &i); } */
    fwrite(b, 1, n, stdout);
    fflush(stdout);
    /* { int i; i = 0; ioctl(fileno(stdout), IKIOPIOMODE, &i); } */
}

STATIC
tek_putc(i)
    int i;
{
    static char c;

    c = i;
    tek_puts(&c, 1);
}

outrow1(raw,n)
register char *raw;
register int n;
{
    int temp;

    while(n--) {
	if(!phase) {
	    phase = 1;
	    lagchar = T_DATABIT|T_SYNCBIT | *raw++;
	} else {
	    phase = 0;
	    putchar(lagchar | ((*raw++)<<3) );
	}
    }
    tek_eol();
}

toprinter(p,buf,rowno)
PRINTER *p;
register short *buf;
int rowno;
{
    register int ival, mbit, x;
    register unsigned char *cptr;

    cptr = &p->pat[rowno&7][0];
    mbit = 0x80;
    for(x=p->xprint; x--;) {
	if (cptr[*buf++] & mbit)
	    ival = 7;
	else
	    ival = 0;
	if(!phase) {
	    phase = 1;
	    lagchar = T_DATABIT|T_SYNCBIT | ival;
	} else {
	    phase = 0;
	    putchar(lagchar | (ival<<3));
	}
	mbit >>= 1;
	if(!mbit)
	    mbit = 0x80;
    }
    tek_eol();
}

toprinter3(p,rbuf,gbuf,bbuf,rowno)
PRINTER *p;
register short *rbuf, *gbuf, *bbuf;
int rowno;
{
    register int ival, mbit, x;
    register unsigned char *cptr;

    cptr = &p->pat[rowno&7][0];
    mbit = 0x80;
    for(x=p->xprint; x--;) {
	ival = 0;
	if (cptr[*rbuf++] & mbit)
	    ival |= 1;
	if (cptr[*gbuf++] & mbit)
	    ival |= 2;
	if (cptr[*bbuf++] & mbit)
	    ival |= 4;
	if(!phase) {
	    phase = 1;
	    lagchar = T_DATABIT|T_SYNCBIT | ival;
	} else {
	    phase = 0;
	    putchar(lagchar | (ival<<3));
	}
	mbit >>= 1;
	if(!mbit)
	    mbit = 0x80;
    }
    tek_eol();
}

