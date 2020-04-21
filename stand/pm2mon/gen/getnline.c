# include "common.h"
# include "ctype.h"

#define RUBC1	003
#define RUBC2	0177
#define CLRC	014
#define KILLC1	025
#define KILLC2	'@'
#define ERASEC2	'#'
#define ERASEC1	010
#define LNEXTC	026
#define EOLC	012

/*
**	getnline - get a line of text from the keyboard with a little
**		  editting
**
*/
getnline(buf,len)
    register char *buf;
    int len;
{
    register int c;
    register int n,weird;
    char litnext;

    len--;

    n = 0;
    weird = -1;
    litnext = 0;

    for( ;; )
    {
	c = getchar();
	if( c == 0 )
	    continue;

	if( litnext )
	{
	    litnext = 0;
	}
	else
	{
	    if( c == '\r' )
		c = EOLC;

	    if( c == EOLC )
		break;

	    if( c == LNEXTC )
	    {
		litnext++;
		putchar('^'); putchar('\b');
		continue;
	    }
	    if( ISMICROSW )
	    if( c == CLRC )
	    {
		ScreenClear();
		c = 0;
		break;
	    }
	    if( c == RUBC1 || c == RUBC2 )
	    {
		n = 0;
		break;
	    }

	    if( c == ERASEC1 || c == ERASEC2 )
	    {
		if( n > 0 )
		{
		    n--;
		    if (weird >= n)
			weird = reprint(buf,n);
		    else
			erase1();
		}
		continue;
	    }
	    if( c == KILLC1 || c == KILLC2 )
	    {
		if (weird >= 0)
		    n = 0 , weird = reprint(buf,n);
		else
		while (n) {
			erase1();
			--n;
		}
		continue;
	    }
	}

	/* add the character */
	if( n >= len )
	{
	    putchar('\7');
	    continue;
	}

	if( !isprint(c) )
	    weird = n;
	putchar(c);		/* Echo the character */

	buf[n++] = c;
    }

    if( c != 0 )
	newline();

    buf[n] = 000;
}

static
erase1()
{
    putchar('\b'); putchar(' '); putchar('\b');
}

static int
reprint(s,n)
    register char *s;
    int n;
{
    register int weird,i;

    putchar('\n');
    putchar('\r');

    weird = -1;

    for( i = 0; i < n; i++ )
    {
	if( !isprint(*s) )
	    weird = i;
	putchar(*s++);
    }

    return weird;
}
