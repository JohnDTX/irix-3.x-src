# include "ctype.h"
# include "common.h"

	   
# define BYTESPERLINE 16
# define LINESPERPAGE 16

/*
 * wdumpmem() --
 * print arbitrary data in readable format.
 XX XX XX XX XX XX XX XX  XX XX XX XX XX XX XX XX  cccccccc cccccccc
 */
wdumpmem(addr,width,count)
    char *addr;
    int width,count;
{
    char asciibuf[BYTESPERLINE+2];
    char dm_fmt[6];
    register char *ap;
    register int col,linestogo;
    register int len;
    short cwidth;
    union
    {
	long l;
	char c[1];
    }   x;

    cwidth = width<<1;
    len = (short)width*count;
    if( width > 1 )
	addr = (char *)((int)addr&~01);

    ap = dm_fmt;
    *ap++ = '%'; *ap++ = '0';
    *ap++ = cwidth+'0';
    *ap++ = 'x'; *ap = 000;

    linestogo = LINESPERPAGE;

    while( len > 0 )
    {
	if( --linestogo < 0 )
	{
	    if( (linestogo = dm_query()) < 0 )
		return;
	    linestogo--;
	}
	printf("%6x*",addr);

	ap = asciibuf;
	for( col = 0; col < BYTESPERLINE; col += width )
	{
	    if( col == BYTESPERLINE/2 )
	    {
		putchar(' ');
		*ap++ = ' ';
	    }
	    putchar(' ');
	    if( col >= len )
	    {
		putnchar(cwidth,' ');
		*ap++ = 000;
	    }
	    else
	    {
		register int ccc;
		register int www;

		x.l = 0;
		if( memread(addr,&x.l,width) == 0 )
		    putnchar(cwidth,'?');
		else
		    printf(dm_fmt,x.l);
		for( www = 4-width; www < 4; www++ )
		{
		    ccc = toascii(x.c[www]);
		    *ap++ = isprint(ccc)?ccc:'.';
		}
	    }
	    addr += width;
	}

	*ap = 000;
	printf("  %s\n",asciibuf);

	len -= BYTESPERLINE;
    }
}

int
dm_query()
{
    register int c;

    printf("more?");
    c = getchar();
    if( isupper(c) )
	c = tolower(c);
    FlushKeyIn();
    putchar('\r');
    switch(c)
    {
    case 0177:
    case 003:
    case 'n':
    case 'q':
	return -1;
    case 'y':
    case ' ':
	return LINESPERPAGE;
    case 014:
	if( ISMICROSW )
	    ScreenClear();
	/* fall through */
    default:
	if(isdigit(c))
	    return c-'0';
	return 1;
    }
}

static
putnchar(n,c)
    int n;
{
    while( --n >= 0 )
	putchar(c);
}
