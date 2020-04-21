# include "duart.h"
# include "ctype.h"
# include "ringbufs.h"
# include "common.h"

static
char KeybChars[100];
static
struct ring KeybRing;

static
int OutStopped;


#define CTRL_Q '\021'
#define CTRL_S '\023'

# define INPUTDEV	(ISMICROSW?SCREEN:LOCAL)

getcinit()
{
    RingInit(&KeybRing,KeybChars,sizeof KeybChars);
}

putchar(c)
    int c;
{
    while( lagetchar() == NOCHAR && OutStopped )
	;
    if( ISMICROSW ) ScreenChar(c); else putcraw(c,LOCAL);
}

flushinput()
{
    RingReset(&KeybRing);
    flush(INPUTDEV);
}

int
getchar()
{
    register int c;

    for( ;; )
    {
	if( (c = Ring_Get(&KeybRing)) >= 0 )
	    return c;

	while( lagetchar() == NOCHAR )
	    ;
    }
}

nwputchar(c)
    int c;
{
    return nwputcraw(c,INPUTDEV);
}

/*
 * lagetchar() --
 * look-ahead getchar.
 * returns the next character without using it up.
 * performs special-effects.
 */
static int
lagetchar()
{
    unsigned char ccell;

    register int c;

    c = nwgetchar();

    if( c == NOCHAR )
	return NOCHAR;

    if( c == CTRL_S )
    {
	OutStopped = 1;
	return NOCHAR;
    }
    OutStopped = 0;

    if( c == CTRL_Q )
	return NOCHAR;

    if( c == BREAKCHAR )
    {
	user_trapF();
	return NOCHAR;
    }

    ccell = c;
    Ring_Put(&KeybRing,ccell);
    return c;
}

nwgetchar()
{
    register int c;

    c = ISMICROSW ? nwgetkbd() : nwgetcraw(LOCAL);

    if( c == NOCHAR || c == BREAKCHAR )
	return c;

    c = toascii(c);
    return c;
}

ungetchar(c)
    unsigned char c;
{
    Ring_Put(&KeybRing,c);
}
