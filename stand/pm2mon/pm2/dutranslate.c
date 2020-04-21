# include "ctype.h"
# undef isascii
# define isascii(x)	(!((x)&0200))
# include "duart.h"

# define PROMSTATIC

# define SHIFTED	01	/* state bits for indexing into codes */
# define CONTROLLED	02

extern struct { unsigned char codes[4]; } KeyMap[];

#define LSHIFT_KEY	0x5	/* buttons of interest */
#define RSHIFT_KEY	0x4
#define LOCK_KEY	0x3
#define CTRL_KEY	0x2
#define BREAK_KEY	0x0

PROMSTATIC	char ctrl,shift,capslocked;

int
dutranslate(c)
    int c;
{
    register unsigned char downstroke,t;
    register short i;

    t = c;
    downstroke = !(t&0x80);
    t &= ~0x80;

    if( t <= LSHIFT_KEY )
    {
	switch(t)
	{
	case BREAK_KEY:
	    if( downstroke )
		return BREAKCHAR;
	    break;

	case LSHIFT_KEY:
	case RSHIFT_KEY:
	    if( downstroke )
		shift++;
	    else
		shift--;
	    break;

	case LOCK_KEY:
	    if( downstroke )
		capslocked = !capslocked;
	    break;

	case CTRL_KEY:
	    ctrl = downstroke;
	    break;
	}

	downstroke = 0;
    }

    if( !downstroke )
	return NOCHAR;

    /*
     * get the mapped-to value of this key.
     * NOTE only alpha keys are affected by
     * caps-lock!
     */
    i = (ctrl?CONTROLLED:0) | (shift?SHIFTED:0);
    t = KeyMap[t].codes[i];

    if( capslocked )
	if( i == 0 && isascii(t) && islower(t) )
	    t = toupper(t);

    return (int)t;
}
